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

std::set<std::string> g_sCommandNames;
std::map<std::string, std::string> g_sCommandModules;

extern Application app;

void DumpCommands(std::string outputPath)
{
    auto cvar = (ICvar*)app.GetCVar();

    nlohmann::json commands;

    ConCommandData* data = cvar->GetConCommandData(ConCommandRef());
    for (ConCommandRef ref = ConCommandRef((uint16)0); ref.GetRawData() != data; ref = ConCommandRef(ref.GetAccessIndex() + 1))
    {
        commands.push_back({
            {"name", ref.GetName()},
            {"description", ref.GetHelpText()},
            {"flags", ParseFlags(ref.GetFlags())},
            {"attributes", {
                {"has_callback", ref.HasCallback()},
                {"has_completion_callback", ref.HasCompletionCallback()}
            }},
            {"module", g_sCommandModules[ref.GetName()]},
        });
    }

    WriteJSON(outputPath + "/commands.json", commands);
}