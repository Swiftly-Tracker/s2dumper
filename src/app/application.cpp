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

#include "public/tier1/utldict.h"
#include "public/entity2/entityclass.h"

#define CS2_INIT_CODEGEN "48 89 5C 24 ? 57 48 83 EC ? 48 8B 1D ? ? ? ? 8B F9"
#define CS2_GET_CODEGEN "48 8D 05 ? ? ? ? C3 ? ? ? ? ? ? ? ? 45 8B C8"

extern Application app;

std::map<std::string, IAppSystem *> g_mAppSystems;
std::set<std::string> g_sQueriedInterfaces;

extern std::set<std::string> g_sConvarNames;
extern std::map<std::string, std::string> g_sConvarModules;
extern std::set<std::string> g_sCommandNames;
extern std::map<std::string, std::string> g_sCommandModules;

void *ApplicationCreateInterface(const char *pName, int *pReturnCode)
{
    std::string iface_name = pName;
    g_sQueriedInterfaces.insert(iface_name);
 
    if (iface_name == CVAR_INTERFACE_VERSION)
    {
        return app.GetCVar();
    }

    if (iface_name == SCHEMASYSTEM_INTERFACE_VERSION)
    {
        return app.GetSchemaSystem();
    }

    if (iface_name == APPLICATION_INTERFACE_VERSION)
    {
        return &app;
    }

    auto it = g_mAppSystems.find(iface_name);
    if (it != g_mAppSystems.end())
    {
        if (pReturnCode)
            *pReturnCode = 0;

        return it->second;
    }

    if (pReturnCode)
        *pReturnCode = 1;

    return nullptr;
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

void PopulateConStuff(std::string module_name)
{
    ICvar *icvar = (ICvar *)app.GetCVar();

    for (ConVarRefAbstract ref(ConVarRef((uint16)0)); ref.IsValidRef(); ref = ConVarRefAbstract(ConVarRef(ref.GetAccessIndex() + 1)))
    {
        std::string name = ref.GetName();
        if(g_sConvarNames.contains(name))
            continue;

        g_sConvarNames.insert(name);
        g_sConvarModules[name] = module_name;
    }

    ConCommandData* data = icvar->GetConCommandData(ConCommandRef());
    for (ConCommandRef ref = ConCommandRef((uint16)0); ref.GetRawData() != data; ref = ConCommandRef(ref.GetAccessIndex() + 1))
    {
        std::string name = ref.GetName();
        if(g_sCommandNames.contains(name))
            continue;

        g_sCommandNames.insert(name);
        g_sCommandModules[name] = module_name;
    }
}

VFunctionHook SetConvarValueHook;

void Application::Initialize(std::string outputPath, std::string game)
{
    tier0 = new Binary("tier0", game);
    schema = new Binary("schemasystem", game);

    m_szName = game;

    void *cvarInterface = tier0->GetInterface(CVAR_INTERFACE_VERSION);
    m_pCVar = static_cast<ICvar *>(cvarInterface);
    g_pCVar = m_pCVar;

    m_pCVar->Connect(tier0->GetFactory());
    m_pCVar->Init();

    PopulateConStuff("tier0");

    m_pSchemaSystem = static_cast<CSchemaSystem *>(schema->GetInterface(SCHEMASYSTEM_INTERFACE_VERSION));
    m_pSchemaSystem->Connect(&ApplicationCreateInterface);
    m_pSchemaSystem->Init();

    PopulateConStuff("schemasystem");

    s2binlib_initialize("../..", game.c_str());

    SetConvarValueHook.SetHookFunction(*(void**)m_pCVar, 14, (void*)&SetConVarValue, true);
    SetConvarValueHook.Enable();

    LoadModules();
}

void Application::Shutdown()
{
    SetConvarValueHook.Disable();
}

void *Application::GetCVar()
{
    return m_pCVar;
}

void *Application::GetSchemaSystem()
{
    return m_pSchemaSystem;
}

void Application::LoadModules()
{
    for (int i = 0; i < sizeof(s_GameModules) / sizeof(GameModule); i++)
    {
        GameModule &module = s_GameModules[i];
        printf("[Application] Loading module: %s\n", module.m_szModuleName);

        Binary *binary = new Binary(module.m_szModuleName, m_szName);
        if (!binary->IsValid())
        {
            printf("[Application] Failed to load module: %s\n", module.m_szModuleName);
            delete binary;
            continue;
        }

        IAppSystem *binaryInterface = static_cast<IAppSystem *>(binary->GetInterface(module.m_szInterfaceName));
        if (!binaryInterface)
        {
            printf("[Application] Failed to get interface: %s from module: %s\n", module.m_szInterfaceName, module.m_szModuleName);
            delete binary;
            continue;
        }
        g_mAppSystems[module.m_szInterfaceName] = binaryInterface;

        binaryInterface->Connect(&ApplicationCreateInterface);
        if (module.m_bInit)
        {
            binaryInterface->Init();

            PopulateConStuff(module.m_szModuleName);
        }
        else
        {
            typedef void *(*InstallSchemaBindings)(const char *interfaceName, void *pSchemaSystem);
            InstallSchemaBindings installSchemaBindings = reinterpret_cast<InstallSchemaBindings>(binary->GetExport("InstallSchemaBindings"));
            if (installSchemaBindings)
            {
                installSchemaBindings(SCHEMASYSTEM_INTERFACE_VERSION, m_pSchemaSystem);
            }
        }
    }
}

void Application::UnloadModules()
{
    for(auto it = g_mAppSystems.rbegin(); it != g_mAppSystems.rend(); ++it)
    {
        IAppSystem *binaryInterface = it->second;
        if (binaryInterface)
        {
            binaryInterface->Shutdown();
            binaryInterface->Disconnect();
        }
    }

    g_mAppSystems.clear();
}

std::set<std::string> Application::GetQueriedInterfaces()
{
    return g_sQueriedInterfaces;
}
