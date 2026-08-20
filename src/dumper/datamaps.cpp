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
#include "../shared/jsonc.h"
#include "../app/application.h"
#include "../hooks/vfunction.h"

#include <s2binlib/s2binlib.h>

extern Application app;

std::set<std::string> g_ThinkFunctionNames;
std::set<std::string> g_ProcessedClassNames;
std::map<std::string, std::vector<std::string>> g_MappedThinkFunctions;
std::map<std::string, std::string> g_EntityClassParent;

std::string GetDatamapFieldTypeName(fieldtype_t type)
{
    switch (type)
    {
    case FIELD_VOID:
        return "void";
    case FIELD_FLOAT32:
        return "float";
    case FIELD_STRING:
        return "string";
    case FIELD_VECTOR:
        return "vector";
    case FIELD_QUATERNION:
        return "quaternion";
    case FIELD_INT32:
        return "int";
    case FIELD_BOOLEAN:
        return "boolean";
    case FIELD_INT16:
        return "int";
    case FIELD_CHARACTER:
        return "char";
    case FIELD_COLOR32:
        return "color32";
    case FIELD_EMBEDDED:
        return "embedded";
    case FIELD_CUSTOM:
        return "custom";
    case FIELD_CLASSPTR:
        return "classptr";
    case FIELD_EHANDLE:
        return "ehandle";
    case FIELD_POSITION_VECTOR:
        return "position_vector";
    case FIELD_TIME:
        return "time";
    case FIELD_TICK:
        return "tick";
    case FIELD_SOUNDNAME:
        return "soundname";
    case FIELD_INPUT:
        return "input";
    case FIELD_FUNCTION:
        return "function";
    case FIELD_VMATRIX:
        return "vmatrix";
    case FIELD_VMATRIX_WORLDSPACE:
        return "vmatrix_worldspace";
    case FIELD_MATRIX3X4_WORLDSPACE:
        return "matrix3x4_worldspace";
    case FIELD_INTERVAL:
        return "interval";
    case FIELD_UNUSED:
        return "unused";
    case FIELD_VECTOR2D:
        return "vector2d";
    case FIELD_INT64:
        return "int";
    case FIELD_VECTOR4D:
        return "vector4d";
    case FIELD_RESOURCE:
        return "resource";
    case FIELD_TYPEUNKNOWN:
        return "typeunknown";
    case FIELD_CSTRING:
        return "cstring";
    case FIELD_HSCRIPT:
        return "hscript";
    case FIELD_VARIANT:
        return "variant";
    case FIELD_UINT64:
        return "uint64";
    case FIELD_FLOAT64:
        return "float64";
    case FIELD_POSITIVEINTEGER_OR_NULL:
        return "positiveinteger_or_null";
    case FIELD_HSCRIPT_NEW_INSTANCE:
        return "hscript_new_instance";
    case FIELD_UINT32:
        return "uint32";
    case FIELD_UTLSTRINGTOKEN:
        return "utlstringtoken";
    case FIELD_QANGLE:
        return "qangle";
    case FIELD_NETWORK_ORIGIN_CELL_QUANTIZED_VECTOR:
        return "network_origin_cell_quantized_vector";
    case FIELD_HMATERIAL:
        return "hmaterial";
    case FIELD_HMODEL:
        return "hmodel";
    case FIELD_NETWORK_QUANTIZED_VECTOR:
        return "network_quantized_vector";
    case FIELD_NETWORK_QUANTIZED_FLOAT:
        return "network_quantized_float";
    case FIELD_DIRECTION_VECTOR_WORLDSPACE:
        return "direction_vector_worldspace";
    case FIELD_QANGLE_WORLDSPACE:
        return "qangle_worldspace";
    case FIELD_QUATERNION_WORLDSPACE:
        return "quaternion_worldspace";
    case FIELD_HSCRIPT_LIGHTBINDING:
        return "hscript_lightbinding";
    case FIELD_V8_VALUE:
        return "v8_value";
    case FIELD_V8_OBJECT:
        return "v8_object";
    case FIELD_V8_ARRAY:
        return "v8_array";
    case FIELD_V8_CALLBACK_INFO:
        return "v8_callback_info";
    case FIELD_UTLSTRING:
        return "utlstring";
    case FIELD_NETWORK_ORIGIN_CELL_QUANTIZED_POSITION_VECTOR:
        return "network_origin_cell_quantized_position_vector";
    case FIELD_HRENDERTEXTURE:
        return "hrendertexture";
    case FIELD_HPARTICLESYSTEMDEFINITION:
        return "hparticlesystemdefinition";
    case FIELD_UINT8:
        return "uint8";
    case FIELD_UINT16:
        return "uint16";
    case FIELD_CTRANSFORM:
        return "ctransform";
    case FIELD_CTRANSFORM_WORLDSPACE:
        return "ctransform_worldspace";
    case FIELD_HPOSTPROCESSING:
        return "hpostprocessing";
    case FIELD_MATRIX3X4:
        return "matrix3x4";
    case FIELD_SHIM:
        return "shim";
    case FIELD_CMOTIONTRANSFORM:
        return "cmotiontransform";
    case FIELD_CMOTIONTRANSFORM_WORLDSPACE:
        return "cmotiontransform_worldspace";
    case FIELD_ATTACHMENT_HANDLE:
        return "attachment_handle";
    case FIELD_AMMO_INDEX:
        return "ammo_index";
    case FIELD_CONDITION_ID:
        return "condition_id";
    case FIELD_AI_SCHEDULE_BITS:
        return "ai_schedule_bits";
    case FIELD_MODIFIER_HANDLE:
        return "modifier_handle";
    case FIELD_ROTATION_VECTOR:
        return "rotation_vector";
    case FIELD_ROTATION_VECTOR_WORLDSPACE:
        return "rotation_vector_worldspace";
    case FIELD_HVDATA:
        return "hvdata";
    case FIELD_SCALE32:
        return "scale32";
    case FIELD_STRING_AND_TOKEN:
        return "string_and_token";
    case FIELD_ENGINE_TIME:
        return "engine_time";
    case FIELD_ENGINE_TICK:
        return "engine_tick";
    case FIELD_WORLD_GROUP_ID:
        return "world_group_id";
    case FIELD_GLOBALSYMBOL:
        return "globalsymbol";
    case FIELD_HNMGRAPHDEFINITION:
        return "hnmgraphdefinition";
    case FIELD_TYPECOUNT:
        return "typecount";
    }

    return "unknown";
}

void CollectDatamapFields(datamap_t *map, nlohmann::json &fields, int &fieldsCount)
{
    if (!map)
        return;

    if (!map->dataDesc || map->dataNumFields <= 0)
        return;

    fieldsCount += map->dataNumFields;

    for (int i = 0; i < map->dataNumFields; i++)
    {
        auto &desc = map->dataDesc[i];

        std::string fieldName = desc.fieldName ? desc.fieldName : "";
        std::string externalName = (desc.externalName && desc.externalName[0]) ? desc.externalName : "";

        bool isInput = (desc.flags & FTYPEDESC_WAS_INPUT) != 0 || (desc.flags & FTYPEDESC_INPUT) != 0;
        bool isOutput = (desc.flags & FTYPEDESC_WAS_OUTPUT) != 0 || (desc.flags & FTYPEDESC_OUTPUT) != 0;

        if (!isInput && !isOutput && (fieldName == "" || externalName == ""))
            continue;

        nlohmann::json fieldJson = {
            {"fieldType", GetDatamapFieldTypeName(desc.fieldType)},
            {"fieldName", fieldName},
            {"externalName", externalName},
        };

        if (isInput)
        {
            auto &inputs = fields["inputs"];
            inputs.push_back(fieldJson);
        }
        else if (isOutput)
        {
            auto &outputs = fields["outputs"];
            outputs.push_back(fieldJson);
        }
        else
        {
            auto &members = fields["members"];
            members.push_back(fieldJson);
        }
    }
}

void ReadClassDatamap(CSchemaType_DeclaredClass *declClass, nlohmann::json &outJson, int &fieldsCount)
{
    auto classInfo = declClass->m_pClassInfo;
    if (!classInfo)
        return;

    if (g_ProcessedClassNames.contains(classInfo->m_pszName))
        return;
    g_ProcessedClassNames.insert(classInfo->m_pszName);

    auto map = classInfo->m_pDataDescMap;
    if (!map)
        return;

    auto &datamaps = outJson["datamaps"];

    nlohmann::json fields = nlohmann::json::object();
    CollectDatamapFields(map, fields, fieldsCount);

    std::vector<std::string> thinkfunctions;
    auto mappedThinkFuncsIt = g_MappedThinkFunctions.find(classInfo->m_pszName);
    if (mappedThinkFuncsIt != g_MappedThinkFunctions.end())
        thinkfunctions = mappedThinkFuncsIt->second;

    datamaps.push_back({
        {"class_name", classInfo->m_pszName},
        {"data_class_name", map->dataClassName ? map->dataClassName : classInfo->m_pszName},
        {"fields", fields},
        {"think_functions", thinkfunctions},
    });
}

VFunctionHook strcmpHook;
std::string current_dumping_class;

int64_t thinkStrcmp(char *a1, char *a2)
{
    if (!strcmp(a1, "s2dumper"))
    {
        g_ThinkFunctionNames.insert(a2);
        g_MappedThinkFunctions[current_dumping_class].push_back(a2);
    }
    else if (!strcmp(a2, "s2dumper"))
    {
        g_ThinkFunctionNames.insert(a1);
        g_MappedThinkFunctions[current_dumping_class].push_back(a1);
    }

    return reinterpret_cast<decltype(&thinkStrcmp)>(strcmpHook.GetOriginal())(a1, a2);
}

void CollectThinkFunctions(std::string outputPath)
{
    CEntitySystem *entitySystem = (CEntitySystem *)app.GetEntitySystem();
    auto module = app.GetGameModule("server");

    void *schemaBindingsExport = module->m_pBinary->GetExport("InstallSchemaBindings");

    void *thinkFuncStrcmp = nullptr;
    s2binlib_find_call_with_string_arg("server", schemaBindingsExport, SCHEMASYSTEM_INTERFACE_VERSION, &thinkFuncStrcmp);

    strcmpHook.SetHookFunction(thinkFuncStrcmp, (void *)thinkStrcmp);
    strcmpHook.Enable();

    FOR_EACH_MAP_FAST(entitySystem->m_entClassesByCPPClassname, i)
    {
        auto entityClass = entitySystem->m_entClassesByCPPClassname[i];
        if (entityClass->m_NameToThinkFunc == 0)
            continue;

        current_dumping_class = entitySystem->m_entClassesByCPPClassname.Key(i);
        if (!g_MappedThinkFunctions.contains(current_dumping_class))
            g_MappedThinkFunctions.insert({current_dumping_class, {}});

        if (entityClass->m_pBaseClassInfo && entityClass->m_pBaseClassInfo->m_pszCPPClassname)
            g_EntityClassParent[current_dumping_class] = entityClass->m_pBaseClassInfo->m_pszCPPClassname;

        (void)entityClass->m_NameToThinkFunc("s2dumper");
    }

    strcmpHook.Disable();

    for (auto &[className, thinkFuncs] : g_MappedThinkFunctions)
    {
        auto parentIt = g_EntityClassParent.find(className);
        if (parentIt == g_EntityClassParent.end())
            continue;

        auto parentThinkFuncsIt = g_MappedThinkFunctions.find(parentIt->second);
        if (parentThinkFuncsIt == g_MappedThinkFunctions.end())
            continue;

        std::set<std::string> parentThinkFuncs(parentThinkFuncsIt->second.begin(), parentThinkFuncsIt->second.end());

        std::vector<std::string> ownThinkFuncs;
        for (auto &thinkFuncName : thinkFuncs)
            if (!parentThinkFuncs.contains(thinkFuncName))
                ownThinkFuncs.push_back(thinkFuncName);

        thinkFuncs = std::move(ownThinkFuncs);
    }

    printf("Dumped %d think functions.\n", g_ThinkFunctionNames.size());

    std::string output = "";
    for (const auto &functionName : g_ThinkFunctionNames)
    {
        if (output != "")
            output += "\n";
        output += functionName;
    }

    std::string path = outputPath + "/think_functions.txt";
    auto f = std::fopen(path.c_str(), "w");
    if (f)
    {
        fprintf(f, "%s", output.c_str());
        fclose(f);
    }
}

void DumpDatamaps(std::string outputPath)
{
    CollectThinkFunctions(outputPath);

    CSchemaSystem *schemaSystem = (CSchemaSystem *)app.GetSchemaSystem();

    nlohmann::json datamapsJson;
    int fieldsCount = 0;

    auto globalTypeScope = schemaSystem->GlobalTypeScope();

    int classes_count = globalTypeScope->m_DeclaredClasses.m_Map.Count();
    FOR_EACH_MAP(globalTypeScope->m_DeclaredClasses.m_Map, iter)
    {
        ReadClassDatamap(globalTypeScope->m_DeclaredClasses.m_Map.Element(iter), datamapsJson, fieldsCount);
    }

    for (int i = 0; i < schemaSystem->m_TypeScopes.GetNumStrings(); i++)
    {
        auto ts = schemaSystem->m_TypeScopes[i];

        FOR_EACH_MAP(ts->m_DeclaredClasses.m_Map, iter)
        {
            ReadClassDatamap(ts->m_DeclaredClasses.m_Map.Element(iter), datamapsJson, fieldsCount);
        }
    }

    printf("Dumped %d datamaps with a total of %d fields\n", classes_count, fieldsCount);

    WriteJSON(outputPath + "/datamaps.json", datamapsJson);
}