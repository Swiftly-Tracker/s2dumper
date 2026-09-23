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

 #include "shared.h"

 #include <map>
 #include <public/icvar.h>
 #include <set>

extern std::set<std::string> g_sConvarNames;
extern std::map<std::string, std::string> g_sConvarModules;
extern std::set<std::string> g_sCommandNames;
extern std::map<std::string, std::string> g_sCommandModules;
extern std::map<uint64_t, std::string> g_mFlags;

std::vector<std::string> ParseFlags(uint64_t flags)
{
    std::vector<std::string> result;

    for (const auto& [flag, name] : g_mFlags)
        if ((flags & flag) != 0)
            result.push_back(name);

    return result;
}

void PopulateConStuff(std::string module_name)
{
    for (ConVarRefAbstract ref(ConVarRef((uint16)0)); ref.IsValidRef(); ref = ConVarRefAbstract(ConVarRef(ref.GetAccessIndex() + 1)))
    {
        std::string name = ref.GetName();
        if(g_sConvarNames.contains(name))
            continue;

        g_sConvarNames.insert(name);
        g_sConvarModules[name] = module_name;
    }

    ConCommandData* data = g_pCVar->GetConCommandData(ConCommandRef());
    for (ConCommandRef ref = ConCommandRef((uint16)0); ref.GetRawData() != data; ref = ConCommandRef(ref.GetAccessIndex() + 1))
    {
        std::string name = ref.GetName();
        if(g_sCommandNames.contains(name))
            continue;

        g_sCommandNames.insert(name);
        g_sCommandModules[name] = module_name;
    }
}
