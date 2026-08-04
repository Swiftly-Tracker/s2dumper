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

#ifndef _src_app_application_h
#define _src_app_application_h

#include <string>
#include <set>
#include <vector>

#include "../binary/binary.h"
#include "../dynlib/dynlib.h"

#include "public/schemasystem/schemasystem.h"
#include "public/eiface.h"
#include "public/tier0/interface.h"

struct GameModule
{
    const char *m_szModuleName;
    const char *m_szInterfaceName;
    bool m_bInit = true;
};

class Application : public CTier0AppSystem<IAppSystem>
{
public:
    virtual void Destructor() {};
#ifndef _WIN32
    virtual void Destructor2() {};
#endif
    virtual void PreShutdown() {};
    virtual BuildType_t GetBuildType() { return kBuildTypeRelease; };
    virtual void Reconnect(CreateInterfaceFn factory, const char *interfaceName) {};

    virtual int AddSystem(IAppSystem *pAppSystem, const char *interfaceName, bool errorOut) { return 0; };
    virtual int AddSystem(const char *unk, const char *interfaceName, bool errorOut) { return 0; };
    virtual int AddSystem(IAppSystem *pAppSystem, const char *interfaceName) { return 0; };
    virtual void RemoveSystem(IAppSystem *pAppSystem) {};
    virtual int AddSystems(int count, void **pAppSystems) { return 0; };
    virtual void *FindSystem(const char *interfaceName) { return nullptr; };
    virtual void *GetGameInfo()
    {
        return nullptr;
    };
    virtual unsigned int unk1() { return -1; };
    virtual int GetUILanguage(int languageType) { return 0; };
    virtual int GetAudioLanguage(int languageType) { return 0; };
    virtual bool IsInToolsMode() { return false; };
    virtual bool unk2() { return false; };
    virtual bool unk3() { return false; };
    virtual bool unk4() { return false; };
    virtual void *unk5() { return nullptr; };
    virtual void *unk6() { return nullptr; };
    virtual void *unk7() { return nullptr; };
    virtual void *unk8() { return nullptr; };
    virtual void *unk9() { return nullptr; };
    virtual void *unk10(void *a) { return a; };
    virtual void *unk11() { return nullptr; };
    virtual void *AddSystemDontLoadStartupManifests(const char *a, const char *b) { return nullptr; };
    virtual void *unk12() { return nullptr; };
    virtual void *unk13() { return nullptr; };

    void Initialize(std::string outputPath, std::string game);
    void Shutdown();

    void LoadModules();
    void UnloadModules();

    void *GetCVar();
    void *GetSchemaSystem();
    void* GetGameEntitySystem();

    std::set<std::string> GetQueriedInterfaces();

private:
    std::string m_szName;

    Binary *tier0;
    Binary *schema;

    CSchemaSystem *m_pSchemaSystem;
    ICvar *m_pCVar;

    std::vector<Binary *> m_vpBinaries;

    GameModule s_GameModules[32] = {
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
        {"tools/hammer", "ToolSystem2_001"},
        {"tools/met", "ToolSystem2_001", false },
	    {"tools/pet", "ToolSystem2_001", false },
        {"tools/cs2_item_editor", "ToolSystem2_001", false },
        {"tools/cs2_workshop_manager", "ToolSystem2_001", false },
        {"tools/modeldoc_editor", "ToolSystem2_ModelDoc", false},
    };
};

#endif