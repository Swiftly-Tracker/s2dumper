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

#include <map>

extern Application app;

std::map<std::string, IAppSystem *> g_mAppSystems;

void *ApplicationCreateInterface(const char *pName, int *pReturnCode)
{
    std::string iface_name = pName;
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

void Application::Initialize(std::string outputPath, std::string game)
{
    tier0 = new Binary("tier0");
    schema = new Binary("schemasystem");

    void *cvarInterface = tier0->GetInterface(CVAR_INTERFACE_VERSION);
    m_pCVar = static_cast<ICvar *>(cvarInterface);
    g_pCVar = m_pCVar;

    m_pCVar->Connect(tier0->GetFactory());
    m_pCVar->Init();

    m_pSchemaSystem = static_cast<CSchemaSystem *>(schema->GetInterface(SCHEMASYSTEM_INTERFACE_VERSION));
    m_pSchemaSystem->Connect(&ApplicationCreateInterface);
    m_pSchemaSystem->Init();

    LoadModules();
}

void Application::Shutdown()
{
    delete tier0;
    delete schema;
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
        Binary *binary = new Binary(module.m_szModuleName);
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
            printf("[Application] Initializing module: %s\n", module.m_szModuleName);
            binaryInterface->Init();
        }
        else
        {
            printf("[Application] Skipping initialization for module: %s\n", module.m_szModuleName);
            typedef void *(*InstallSchemaBindings)(const char *interfaceName, void *pSchemaSystem);
            InstallSchemaBindings installSchemaBindings = reinterpret_cast<InstallSchemaBindings>(binary->GetExport("InstallSchemaBindings"));
            if (installSchemaBindings)
            {
                installSchemaBindings(SCHEMASYSTEM_INTERFACE_VERSION, m_pSchemaSystem);
            }
        }

        delete binary;
    }
}

void Application::UnloadModules()
{
    for (auto &pair : g_mAppSystems)
    {
        IAppSystem *binaryInterface = pair.second;
        if (binaryInterface)
        {
            binaryInterface->Shutdown();
            binaryInterface->Disconnect();
        }
    }

    g_mAppSystems.clear();
}