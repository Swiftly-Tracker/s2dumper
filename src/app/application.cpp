/************************************************************************************************
 *  S2Dumper is a dumper for various properties of Source2-based games.
 *  Copyright (C) 2026 Swiftly Solution SRL via Sava Andrei-Sebastian and it's contributors
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

extern Application app;

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

    if (pReturnCode)
        *pReturnCode = 1;

    return nullptr;
}

void Application::Initialize(std::string outputPath, std::string game)
{
    tier0 = new Binary(true, "tier0", game);
    schema = new Binary(true, "schemasystem", game);

    void *cvarInterface = tier0->GetInterface(CVAR_INTERFACE_VERSION);
    m_pCVar = static_cast<ICvar *>(cvarInterface);
    g_pCVar = m_pCVar;

    m_pCVar->Connect(tier0->GetFactory());
    m_pCVar->Init();

    m_pSchemaSystem = static_cast<CSchemaSystem *>(schema->GetInterface(SCHEMASYSTEM_INTERFACE_VERSION));
    m_pSchemaSystem->Connect(&ApplicationCreateInterface);
    m_pSchemaSystem->Init();

    auto gts = m_pSchemaSystem->GlobalTypeScope();

    FOR_EACH_MAP(gts->m_DeclaredClasses.m_Map, i)
    {
        auto classType = gts->m_DeclaredClasses.m_Map.Element(i);
        auto classInfo = classType->m_pClassInfo;
        printf("Class: %s\n", classInfo->m_pszName);
    }
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