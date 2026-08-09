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
#include <map>

#include "../binary/binary.h"
#include "../dynlib/dynlib.h"
#include "module.h"

#include "public/schemasystem/schemasystem.h"
#include "public/entity2/entitysystem.h"
#include "public/entity2/entityclass.h"
#include "public/eiface.h"
#include "public/tier0/interface.h"

class Application : public CTier0AppSystem<IAppSystem>
{
public:
    virtual void Destructor() {};
#ifndef _WIN32
    virtual void Destructor2() {};
#endif
    virtual void PreShutdown() {};
    virtual BuildType_t GetBuildType() { return kBuildTypeRelease; };
    virtual void Reconnect(CreateIFace factory, const char *interfaceName) {};

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
    CSchemaSystem* GetSchemaSystem();
    CNetworkSerializerCodeGenDatabase* GetCodeGenDatabase();
    CEntitySystem* GetEntitySystem();
    GameModule* GetGameModule(std::string name);

    void* FindModuleIFace(const char* name);
    void InitModule(std::string name, IAppSystem* system, const char* interfaceName, CreateIFace factory, bool shouldInit = true);

private:
    std::string m_szName;
    std::map<std::string, GameModule*> m_mModules;
    std::map<std::string, IAppSystem*> m_mInitializedModules;
};

extern Application app;

void PopulateConStuff(std::string module_name);

#endif