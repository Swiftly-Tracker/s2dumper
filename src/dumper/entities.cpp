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

#include "../shared/jsonc.h"
#include "../app/application.h"
#include "shared.h"

extern Application app;

std::vector<std::string> ProcessEntityClassFlags(uint32_t flags)
{
    std::vector<std::string> flagNames;
    if((flags & FENTCLASS_NON_NETWORKABLE) != 0) flagNames.push_back("non_networkable");
    if((flags & FENTCLASS_ALIAS) != 0) flagNames.push_back("alias");
    if((flags & FENTCLASS_NO_SPAWNGROUP) != 0) flagNames.push_back("no_spawngroup");
    if((flags & FENTCLASS_FORCE_EHANDLE) != 0) flagNames.push_back("force_ehandle");
    if((flags & FENTCLASS_UNK004) != 0) flagNames.push_back("unk004");
    if((flags & FENTCLASS_SUSPEND_OUTSIDE_PVS) != 0) flagNames.push_back("suspend_outside_pvs");
    if((flags & FENTCLASS_ANONYMOUS) != 0) flagNames.push_back("anonymous");
    if((flags & FENTCLASS_UNK007) != 0) flagNames.push_back("unk007");
    if((flags & FENTCLASS_UNK008) != 0) flagNames.push_back("unk008");
    if((flags & FENTCLASS_UNK009) != 0) flagNames.push_back("unk009");
    if((flags & FENTCLASS_FORCE_WORLDGROUPID) != 0) flagNames.push_back("force_worldgroupid");
    return flagNames;
}

void DumpEntitySystem(std::string outputPath)
{
    auto entitySystem = app.GetEntitySystem();

    std::map<std::string, CEntityClass*> entityClassesMap;

    FOR_EACH_MAP_FAST(entitySystem->m_entClassesByClassname, i)
    {
        entityClassesMap[entitySystem->m_entClassesByClassname.Key(i)] = entitySystem->m_entClassesByClassname[i];
    }

    nlohmann::json entitySystemJson;

    auto& entityClasses = entitySystemJson["entity_classes"];
    for(auto& [designerName, entityClass] : entityClassesMap)
    {
        nlohmann::json entry;
        entry["class_name"] = entityClass->m_pClassInfo->m_pszCPPClassname;
        entry["designer_name"] = designerName;
        entry["flags"] = ProcessEntityClassFlags(entityClass->m_flags);
        entityClasses.push_back(entry);
    }

    std::sort(entityClasses.begin(), entityClasses.end(),
    [](const nlohmann::json& a, const nlohmann::json& b) {
        return a["designer_name"].get<std::string>() < b["designer_name"].get<std::string>();
    });

    printf("Dumped %zu entities\n", entityClasses.size());

    WriteJSON(outputPath + "/entities.json", entitySystemJson);
}