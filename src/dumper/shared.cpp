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

#define FCVAR_MISSING1	(1ull<<30)
#define FCVAR_MISSING2	(1ull<<31)

extern std::set<std::string> g_sConvarNames;
extern std::map<std::string, std::string> g_sConvarModules;
extern std::set<std::string> g_sCommandNames;
extern std::map<std::string, std::string> g_sCommandModules;

std::map<uint64_t, std::string> g_mFlags = {
    {FCVAR_LINKED_CONCOMMAND, "linked_concommand"},
	{FCVAR_DEVELOPMENTONLY, "developmentonly"},
	{FCVAR_GAMEDLL, "gamedll"},
	{FCVAR_CLIENTDLL, "clientdll"},
	{FCVAR_HIDDEN, "hidden"},
	{FCVAR_PROTECTED, "protected"},
	{FCVAR_SPONLY, "sponly"},
	{FCVAR_ARCHIVE, "archive"},
	{FCVAR_NOTIFY, "notify"},
	{FCVAR_USERINFO, "userinfo"},
	{FCVAR_REFERENCE, "reference"},
	{FCVAR_UNLOGGED, "unlogged"},
	{FCVAR_INITIAL_SETVALUE, "initial_setvalue"},
	{FCVAR_REPLICATED, "replicated"},
	{FCVAR_CHEAT, "cheat"},
	{FCVAR_PER_USER, "per_user"},
	{FCVAR_DEMO, "demo"},
	{FCVAR_DONTRECORD, "dontrecord"},
	{FCVAR_PERFORMING_CALLBACKS, "performing_callbacks"},
	{FCVAR_RELEASE, "release"},
	{FCVAR_MENUBAR_ITEM, "menubar_item"},
	{FCVAR_COMMANDLINE_ENFORCED, "commandline_enforced"},
	{FCVAR_NOT_CONNECTED, "notconnected"},
	{FCVAR_VCONSOLE_FUZZY_MATCHING, "vconsole_fuzzy_matching"},
	{FCVAR_SERVER_CAN_EXECUTE, "server_can_execute"},
	{FCVAR_CLIENT_CAN_EXECUTE, "client_can_execute"},
	{FCVAR_SERVER_CANNOT_QUERY, "server_cannot_query"},
	{FCVAR_VCONSOLE_SET_FOCUS, "vconsole_set_focus"},
	{FCVAR_CLIENTCMD_CAN_EXECUTE, "clientcmd_can_execute"},
	{FCVAR_EXECUTE_PER_TICK, "execute_per_tick"},
	{FCVAR_MISSING1, "missing1"},
	{FCVAR_MISSING2, "missing2"},
	{FCVAR_DEFENSIVE, "defensive"}
};

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
