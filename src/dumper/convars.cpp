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

std::set<std::string> g_sConvarNames;
std::map<std::string, std::string> g_sConvarModules;

std::string ConVarTypeToString(EConVarType cvarType)
{
    if(cvarType == EConVarType_Invalid) return "invalid";
    else if(cvarType == EConVarType_Bool) return "bool";
	else if(cvarType == EConVarType_Int16) return "int16";
	else if(cvarType == EConVarType_UInt16) return "uint16";
	else if(cvarType == EConVarType_Int32) return "int32";
	else if(cvarType == EConVarType_UInt32) return "uint32";
	else if(cvarType == EConVarType_Int64) return "int64";
	else if(cvarType == EConVarType_UInt64) return "uint64";
	else if(cvarType == EConVarType_Float32) return "float32";
	else if(cvarType == EConVarType_Float64) return "float64";
    else if(cvarType == EConVarType_String) return "string";
    else if(cvarType == EConVarType_Color) return "color";
    else if(cvarType == EConVarType_Vector2) return "vector2";
    else if(cvarType == EConVarType_Vector3) return "vector3";
    else if(cvarType == EConVarType_Vector4) return "vector4";
    else if(cvarType == EConVarType_Qangle) return "qangle";
    else if(cvarType == EConVarType_VectorWS) return "vectorws";
    else if(cvarType == EConVarType_MAX) return "max";
    else return "unknown";
}

void DumpConVars(std::string outputPath)
{
    nlohmann::json convars;

    for (ConVarRefAbstract ref(ConVarRef((uint16)0)); ref.IsValidRef(); ref = ConVarRefAbstract(ConVarRef(ref.GetAccessIndex() + 1)))
    {
        convars.push_back({
            {"name", ref.GetName()},
            {"description", ref.GetHelpText()},
            {"flags", ParseFlags(ref.GetFlags())},
            {"attributes", {
                {"has_callback", ref.GetConVarData()->GetCallbackIndex() != -1},
                {"has_min", ref.HasMin()},
                {"has_max", ref.HasMax()},
                {"has_default", ref.HasDefault()},
            }},
            {"module", g_sConvarModules[ref.GetName()]},
            {"datatype", ConVarTypeToString(ref.GetType())},
            {"datatype_raw", (int)ref.GetType()},
        });

        auto& cvar = convars.back();
        
        CBufferString minValue, maxValue, defaultValue;
        if (ref.HasMin()) {
            ref.GetMinAsString(minValue);
            cvar["min"] = minValue.Get();
        }
        if (ref.HasMax()) {
            ref.GetMaxAsString(maxValue);
            cvar["max"] = maxValue.Get();
        }
        if (ref.HasDefault()) {
            ref.GetDefaultAsString(defaultValue);
            if(std::string(ref.GetName()) == "cl_color") {
                cvar["default"] = minValue.Get();
            } else {
                cvar["default"] = defaultValue.Get();
            }
        }
    }

    std::sort(convars.begin(), convars.end(),
    [](const nlohmann::json& a, const nlohmann::json& b) {
        return a["name"].get<std::string>() < b["name"].get<std::string>();
    });

    printf("Dumped %zu convars\n", convars.size());

    WriteJSON(outputPath + "/convars.json", convars);
}