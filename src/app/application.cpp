/************************************************************************************************
 *  S2Dumper is a dumper for various properties of Source2-based games.
 *  Copyright (C) 2026 Sava Andrei-Sebastian and it's contributors
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <https://www.gnu.org/licenses/>.
 ************************************************************************************************/

#include "application.h"
#include "../hooks/vfunction.h"

#include <s2binlib/s2binlib.h>

#include <map>
#include <set>
#include <filesystem>

#include "public/tier1/utldict.h"
#include "public/entity2/entityclass.h"
#include "../dumper/shared.h"

typedef uint8_t(*InstallSchemaBindingsFn)(const char*, void*);

std::map<std::string, IAppSystem*> g_mInitializedInterfaces;

struct InitGameModule
{
    const char *m_szModuleName;
    const char *m_szInterfaceName;
    bool m_bInit = true;
};

static const InitGameModule gs_GameModules[28] = {
    {"filesystem_stdio", FILESYSTEM_INTERFACE_VERSION},
    {"resourcesystem", RESOURCESYSTEM_INTERFACE_VERSION},
    {"client", "Source2ClientConfig001"},
    {"engine2", SOURCE2ENGINETOSERVER_INTERFACE_VERSION},
    {"host", "GameSystem2HostHook"},
    {"modtools", "Source2ModTools001"},
    {"matchmaking", MATCHFRAMEWORK_INTERFACE_VERSION},
    {"server", SOURCE2SERVERCONFIG_INTERFACE_VERSION},
    {"animationsystem", ANIMATIONSYSTEM_INTERFACE_VERSION},
    {"materialsystem2", TEXTLAYOUT_INTERFACE_VERSION},
    {"meshsystem", MESHSYSTEM_INTERFACE_VERSION, false},
    {"networksystem", NETWORKSYSTEM_INTERFACE_VERSION, false},
    {"panorama", PANORAMAUIENGINE_INTERFACE_VERSION},
    {"particles", PARTICLESYSTEMMGR_INTERFACE_VERSION, false},
    {"pulse_system", PULSESYSTEM_INTERFACE_VERSION},
    {WIN_LIN("rendersystemdx11", "rendersystemvulkan"), RENDER_UTILS_INTERFACE_VERSION},
    {"scenefilecache", "SceneFileCache002"},
    {"scenesystem", SCENEUTILS_INTERFACE_VERSION},
    {"soundsystem", SOUNDOPSYSTEMEDIT_INTERFACE_VERSION},
    {"steamaudio", STEAMAUDIO_INTERFACE_VERSION, false},
    {"vphysics2", VPHYSICS2_INTERFACE_VERSION},
    {"worldrenderer", WORLD_RENDERER_MGR_INTERFACE_VERSION},
    {"assetsystem", ASSETSYSTEM_INTERFACE_VERSION, false},
    {"assetpreview", ASSETPREVIEWSYSTEM_INTERFACE_VERSION, false},
    {"assetbrowser", ASSETBROWSERSYSTEM_INTERFACE_VERSION, false},
    {"resourcecompiler", RESOURCECOMPILERSYSTEM_INTERFACE_VERSION, false},
    {"hammer", "ToolSystem2_001"},
    {"modeldoc_editor", "ToolSystem2_ModelDoc", false},
};

void* RawFactory(const char* name, int* returnCode)
{
    InterfaceQueried(name);
    auto module = app.FindModuleIFace(name);
    if(!module) module = &app;
    if(returnCode) *returnCode = 0;

    return module;
}

void* FullFactory(const char* name, int* returnCode)
{
    InterfaceQueried(name);
  
    std::string ifaceName = name;
    void* returnIface = nullptr;
    if(ifaceName == CVAR_INTERFACE_VERSION) {
        returnIface = g_pCVar;
    } else if(ifaceName == SCHEMASYSTEM_INTERFACE_VERSION) {
        returnIface = g_pSchemaSystem;
    } else if(ifaceName == APPLICATION_INTERFACE_VERSION) {
        returnIface = &app;
    } else {
        if(g_mInitializedInterfaces.contains(ifaceName)) {
            returnIface = g_mInitializedInterfaces[ifaceName];
        }
    }

    if(returnCode) {
        *returnCode = returnIface != nullptr ? 0 : 1;
    }

    return returnIface;
}

void SetConVarValue(ICvar* icvar, ConVarRef ref)
{
	ConVarRefAbstract cvar(ref);
	if (!cvar.IsConVarDataValid())
		return;

	if (!strcmp("r_dopixelvisibility", cvar.GetName()))
	{
		cvar.SetBool(false);
	}
}

VFunctionHook SetConvarValueHook;

void Application::Initialize(std::string outputPath, std::string game)
{
    m_szName = game;
    s2binlib_initialize("../..", game.c_str());

    LoadModules();

    auto tier0It = m_mModules.find("tier0");
    if(tier0It == m_mModules.end())
        return;

    auto tier0Factory = tier0It->second->m_pCreateInterface;

    g_pCVar = (ICvar*)(tier0Factory(CVAR_INTERFACE_VERSION, nullptr));
    g_pCVar->Connect(tier0Factory);
    g_pCVar->Init();

    SetConvarValueHook.SetHookFunction(*(void**)g_pCVar, 14, (void*)&SetConVarValue, true);
    SetConvarValueHook.Enable();

    PopulateConStuff("tier0");

    auto schemaSystem = GetSchemaSystem();
    g_pSchemaSystem = schemaSystem;
    schemaSystem->PrintSchemaStats("");

    PopulateConStuff("schemasystem");

    (void)GetCodeGenDatabase();
}

void Application::Shutdown()
{
}

std::vector<std::string> FetchFileNamesFromDirectory(const std::string& directoryPath)
{
    std::vector<std::string> fileNames;
    for (const auto& entry : std::filesystem::directory_iterator(directoryPath))
    {
        if (entry.is_directory())
        {
            std::vector<std::string> fls = FetchFileNamesFromDirectory(entry.path().string());
            for (auto fl : fls) fileNames.push_back(fl);
        } else {
            fileNames.push_back(std::filesystem::absolute(entry.path()).string());
        }
    }
    return fileNames;
}

std::string NormalizeModuleName(std::string filename) {
	if (auto dot = filename.find('.'); dot != std::string::npos)
		filename = filename.substr(0, dot);

	if (filename.find("lib") == 0)
		filename = filename.substr(3);
	
    return filename;
}

void Application::LoadModules()
{
    std::vector<std::string> files = FetchFileNamesFromDirectory("../../..");
    for (auto file : files)
    {
        auto filePath = std::filesystem::path(file);
        if (filePath.extension() == WIN_LIN("dll", "so"))
        {
            auto moduleName = NormalizeModuleName(filePath.filename().string());
            if(m_mModules.contains(moduleName))
                continue;

            if(moduleName == "met" || moduleName == "pet" || moduleName == "cs2_item_editor" || moduleName == "cs2_workshop_manager") continue;

            Binary* binary = new Binary(file);
            auto factory = binary->GetFactory();

            GameModule* module = new GameModule();
            module->m_pBinary = binary;
            module->m_pCreateInterface = factory;
            module->m_szPath = file;

            m_mModules[moduleName] = module;
        }
    }
}

bool g_bSchemaBindingsInstalled = false;

CSchemaSystem* Application::GetSchemaSystem()
{
    auto module = m_mModules.find("schemasystem");
    if (module == m_mModules.end())
        return nullptr;

    static auto schemaSystem = (CSchemaSystem*)(module->second->m_pCreateInterface(SCHEMASYSTEM_INTERFACE_VERSION, nullptr));
    if(!schemaSystem) return nullptr;

    if(!g_bSchemaBindingsInstalled)
    {
        g_bSchemaBindingsInstalled = true;
        for(auto& [name, mod] : m_mModules)
        {
            auto installBindings = (InstallSchemaBindingsFn)(mod->m_pBinary->GetExport("InstallSchemaBindings"));
            if(installBindings)
            {
                printf("Installing schema bindings for module: %s\n", name.c_str());
                installBindings(SCHEMASYSTEM_INTERFACE_VERSION, schemaSystem);
            }
        }
    }

    return schemaSystem;
}

CNetworkSerializerCodeGenDatabase* Application::GetCodeGenDatabase()
{
    static CNetworkSerializerCodeGenDatabase* codeGenDatabase = nullptr;
    if(codeGenDatabase) return codeGenDatabase;

    auto iServer = m_mModules.find("server");
    auto iTier0 = m_mModules.find("tier0");
    if (iServer == m_mModules.end() || iTier0 == m_mModules.end())
        return nullptr;

    auto serverFactory = iServer->second->m_pCreateInterface;
    auto tier0Factory = iTier0->second->m_pCreateInterface;

    IAppSystem* serverIface = (IAppSystem*)serverFactory(SOURCE2SERVER_INTERFACE_VERSION, nullptr);
    if(serverIface)
    {
        InitModule("server", serverIface, SOURCE2SERVER_INTERFACE_VERSION, RawFactory, false);

        void* CBaseEntityVTable = nullptr;
        s2binlib_find_vtable("server", "CBaseEntity", &CBaseEntityVTable);

        CNetworkSerializerClassInfo* classInfo = reinterpret_cast<CNetworkSerializerClassInfo*(*)()>(((void**)CBaseEntityVTable)[0])();
        codeGenDatabase = classInfo->m_pDatabase;
    }

    if(g_pSchemaSystem)
    {
        InitModule("schemasystem", g_pSchemaSystem, SCHEMASYSTEM_INTERFACE_VERSION, FullFactory, true);
    }

    for(int i = 0; i < sizeof(gs_GameModules) / sizeof(gs_GameModules[0]); i++)
    {
        auto& module = gs_GameModules[i];
        IAppSystem* appSystem = (IAppSystem*)FindModuleIFace(module.m_szInterfaceName);
        if(!appSystem) continue;

        g_mInitializedInterfaces[module.m_szInterfaceName] = appSystem;
        InitModule(module.m_szModuleName, appSystem, module.m_szInterfaceName, FullFactory, module.m_bInit);
        
        PopulateConStuff(module.m_szModuleName);
    }

    return codeGenDatabase;
}

void* Application::FindModuleIFace(const char* name)
{
    int rc = 0;
    for(auto& [modName, mod] : m_mModules)
    {
        if(mod->m_pCreateInterface)
        {
            void* iface = mod->m_pCreateInterface(name, &rc);
            if(iface) return iface;
        }
    }
    return nullptr;
}

void Application::InitModule(std::string name, IAppSystem* system, const char* interfaceName, CreateIFace factory, bool shouldInit)
{
    system->Connect(factory);
    if(shouldInit) {
        system->Init();
    }

    m_mInitializedModules[name] = system;
}