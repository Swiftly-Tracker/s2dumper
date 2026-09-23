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
extern std::map<fieldtype_t, std::string> g_mFieldTypes;

std::set<std::string> g_ThinkFunctionNames;
std::set<std::string> g_ProcessedClassNames;

struct MemberData
{
    std::string type;
    std::string name;
    std::string schemaName;
};

struct InputData
{
    std::string name;
    std::string raw_name;
    std::string description;
    std::string return_type;
    int variant_count;
    int parameter_count;
    std::vector<std::string> parameter_types;
    std::string parameter_names;
};

struct OutputData
{
    std::string name;
    std::string schema_name;
};

struct EntityClassMetadata
{
    CEntityClass* entityClass;
    ScriptClassDesc_t* scriptDesc;
    std::string entityParent;

    std::set<std::string> thinkFunctions;
    std::vector<MemberData> members;
    std::vector<InputData> inputs;
    std::vector<OutputData> outputs;
};

std::map<std::string, EntityClassMetadata> g_EntityClassMetadata;

void EnsureEntityClassMetadata(std::string className, CEntityClass* entityClass, ScriptClassDesc_t* scriptDesc, std::string entityParent)
{
    if(!g_EntityClassMetadata.contains(className))
    {
        g_EntityClassMetadata[className] = EntityClassMetadata{entityClass, scriptDesc, entityParent};
    }
}

void PopulateEntityMetadataScriptDesc(ScriptClassDesc_t* scriptDesc)
{
    if(scriptDesc == nullptr) return;

    EnsureEntityClassMetadata(scriptDesc->m_pszClassname, nullptr, scriptDesc, "");
    if(scriptDesc->m_pBaseDesc)
        PopulateEntityMetadataScriptDesc(scriptDesc->m_pBaseDesc);
}

void CollectClassData()
{
    CEntitySystem *entitySystem = (CEntitySystem *)app.GetEntitySystem();
    
    FOR_EACH_MAP_FAST(entitySystem->m_entClassesByCPPClassname, i)
    {
        auto entityClass = entitySystem->m_entClassesByCPPClassname[i];
        auto dumpingClass = entitySystem->m_entClassesByCPPClassname.Key(i);

        std::string parentClassName = "";
        if(entityClass->m_pBaseClassInfo && entityClass->m_pBaseClassInfo->m_pszCPPClassname)   
            parentClassName = entityClass->m_pBaseClassInfo->m_pszCPPClassname;

        EnsureEntityClassMetadata(dumpingClass, entityClass, entityClass->m_pScriptDesc, parentClassName);
    }

    for(auto& [className, metadata] : g_EntityClassMetadata)
    {
        auto scriptDesc = metadata.scriptDesc;
        if(!scriptDesc) continue;

        PopulateEntityMetadataScriptDesc(scriptDesc);
    }
}

VFunctionHook strcmpHook;
EntityClassMetadata* current_dumping_class = nullptr;

int64_t thinkStrcmp(char *a1, char *a2)
{
    if (!strcmp(a1, "s2dumper"))
    {
        g_ThinkFunctionNames.insert(a2);
        current_dumping_class->thinkFunctions.insert(a2);
    }
    else if (!strcmp(a2, "s2dumper"))
    {
        g_ThinkFunctionNames.insert(a1);
        current_dumping_class->thinkFunctions.insert(a1);
    }

    return reinterpret_cast<decltype(&thinkStrcmp)>(strcmpHook.GetOriginal())(a1, a2);
}

void* RawFactory(const char* name, int* returnCode);

void CollectClassThinkFunctions()
{
    auto module = app.GetGameModule("server");

    void *schemaBindingsExport = module->m_pBinary->GetExport("InstallSchemaBindings");

    void *thinkFuncStrcmp = nullptr;
    s2binlib_find_call_with_string_arg("server", schemaBindingsExport, SCHEMASYSTEM_INTERFACE_VERSION, &thinkFuncStrcmp);

    strcmpHook.SetHookFunction(thinkFuncStrcmp, (void *)thinkStrcmp);
    strcmpHook.Enable();

    for (auto& [className, metadata] : g_EntityClassMetadata)
    {
        current_dumping_class = &metadata;

        if (metadata.entityClass->m_NameToThinkFunc != 0)
            (void)metadata.entityClass->m_NameToThinkFunc("s2dumper");
    }

    strcmpHook.Disable();

    auto rawThinkFunctions = g_EntityClassMetadata;

    for (auto &[className, metadata] : g_EntityClassMetadata)
    {
        std::set<std::string> ancestorThinkFuncs;
        std::set<std::string> visitedClassNames;

        std::string parentClassName = className;
        while (true)
        {
            auto parentIt = g_EntityClassMetadata.find(parentClassName);
            if (parentIt == g_EntityClassMetadata.end())
                break;

            parentClassName = parentIt->second.entityParent;
            if (!visitedClassNames.insert(parentClassName).second)
                break;

            auto parentThinkFuncsIt = rawThinkFunctions.find(parentClassName);
            if (parentThinkFuncsIt != rawThinkFunctions.end())
                ancestorThinkFuncs.insert(parentThinkFuncsIt->second.thinkFunctions.begin(), parentThinkFuncsIt->second.thinkFunctions.end());
        }

        std::set<std::string> ownThinkFuncs;
        for (auto &thinkFuncName : metadata.thinkFunctions)
            if (!ancestorThinkFuncs.contains(thinkFuncName))
                ownThinkFuncs.insert(thinkFuncName);

        metadata.thinkFunctions = std::move(ownThinkFuncs);
    }

    printf("Dumped %zu think functions.\n", g_ThinkFunctionNames.size());
}

void CollectClassInputFunctions()
{
    for(auto& [className, metadata] : g_EntityClassMetadata)
    {
        auto scriptDesc = metadata.scriptDesc;
        if(!scriptDesc) return;

        auto& bindings = scriptDesc->m_FunctionBindings;
        FOR_EACH_VEC(bindings, i)
        {
            auto& binding = bindings[i];

            InputData data;
            data.name = binding.m_desc.m_pszScriptName;
            data.raw_name = binding.m_desc.m_pszFunction;
            data.description = binding.m_desc.m_pszDescription;
            data.return_type = g_mFieldTypes[binding.m_desc.m_ReturnType];
            data.variant_count = binding.m_desc.m_iVariantCount;
            data.parameter_count = binding.m_desc.m_iParamCount;
            data.parameter_names = binding.m_desc.m_pszParameterNames ? binding.m_desc.m_pszParameterNames : "";
            for(int i = 0; i < binding.m_desc.m_iParamCount; i++)
                data.parameter_types.push_back(g_mFieldTypes[binding.m_desc.m_Parameters[i]]);

            metadata.inputs.push_back(data);
        }
    }
}

std::string ReadFieldType(CSchemaType* field);

void CollectClassMembers(CSchemaType_DeclaredClass* declaredClass)
{
    auto classInfo = declaredClass->m_pClassInfo;
    if(classInfo == nullptr) return;

    std::string className = classInfo->m_pszCPPName;
    if(g_ProcessedClassNames.contains(className)) return;
    g_ProcessedClassNames.insert(className);

    EnsureEntityClassMetadata(
        className, nullptr, nullptr, 
        classInfo->m_pBaseClasses ? classInfo->m_pBaseClasses->m_pClass->m_pszCPPName : ""
    );

    auto& metadata = g_EntityClassMetadata[className];

    auto dataMap = classInfo->m_pDataDescMap;
    if (!dataMap)
        return;

    {
        if (!dataMap)
           return;

        if (!dataMap->dataDesc || dataMap->dataNumFields <= 0)
            return;

        for (int i = 0; i < dataMap->dataNumFields; i++)
        {
            auto &desc = dataMap->dataDesc[i];

            std::string fieldName = desc.fieldName ? desc.fieldName : "";
            std::string externalName = (desc.externalName && desc.externalName[0]) ? desc.externalName : "";

            MemberData memberData;
            memberData.type = g_mFieldTypes[desc.fieldType];
            memberData.name = externalName;
            memberData.schemaName = fieldName;

            metadata.members.push_back(memberData);
        }
    }

    {
        auto field_size = classInfo->m_nFieldCount;
        auto fields = classInfo->m_pFields;

        for (int i = 0; i < field_size; i++)
        {
            auto fieldType = ReadFieldType(fields[i].m_pType);
            if(fieldType == "CEntityIOOutput")
            {
                OutputData outputData;
                outputData.schema_name = fields[i].m_pszName;
                outputData.name = outputData.schema_name.substr(2);
                metadata.outputs.push_back(outputData);
            }
        }
    }
}

void CollectClassesMembers()
{
    CSchemaSystem *schemaSystem = (CSchemaSystem *)app.GetSchemaSystem();

    auto globalTypeScope = schemaSystem->GlobalTypeScope();

    FOR_EACH_MAP(globalTypeScope->m_DeclaredClasses.m_Map, iter)
    {
        CollectClassMembers(globalTypeScope->m_DeclaredClasses.m_Map.Element(iter));
    }

    for (int i = 0; i < schemaSystem->m_TypeScopes.GetNumStrings(); i++)
    {
        auto ts = schemaSystem->m_TypeScopes[i];

        FOR_EACH_MAP(ts->m_DeclaredClasses.m_Map, iter)
        {
            CollectClassMembers(ts->m_DeclaredClasses.m_Map.Element(iter));
        }
    }
}

void DumpDatamaps(std::string outputPath)
{
    CollectClassData();
    CollectClassThinkFunctions();
    CollectClassInputFunctions();
    CollectClassesMembers();

    int membersCount = 0;
    int outputsCount = 0;
    int inputsCount = 0;
    int thinkFunctionsCount = g_ThinkFunctionNames.size();
    int classesCount = g_EntityClassMetadata.size();

    for(auto& [className, metadata] : g_EntityClassMetadata)
    {
        membersCount += metadata.members.size();
        outputsCount += metadata.outputs.size();
        inputsCount += metadata.inputs.size();
    }

    printf("Dumped %d members, %d outputs, %d inputs, %d think functions across %d classes.\n", membersCount, outputsCount, inputsCount, thinkFunctionsCount, classesCount);

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

    nlohmann::json datamapJson = nlohmann::json::array();

    for(auto& [className, metadata] : g_EntityClassMetadata)
    {
        nlohmann::json classJson;
        classJson["class_name"] = className;

        nlohmann::json membersJson = nlohmann::json::array();
        for(auto& member : metadata.members)
        {
            nlohmann::json memberJson;
            memberJson["type"] = member.type;
            memberJson["name"] = member.name;
            memberJson["schema_name"] = member.schemaName;
            membersJson.push_back(memberJson);
        }
        classJson["members"] = membersJson;

        nlohmann::json inputsJson = nlohmann::json::array();
        for(auto& input : metadata.inputs)
        {
            nlohmann::json inputJson;
            inputJson["name"] = input.name;
            inputJson["raw_name"] = input.raw_name;
            inputJson["description"] = input.description;
            inputJson["return_type"] = input.return_type;
            inputJson["variant_count"] = input.variant_count;
            inputJson["parameter_count"] = input.parameter_count;
            inputJson["parameter_types"] = input.parameter_types;
            inputJson["parameter_names"] = input.parameter_names;
            inputsJson.push_back(inputJson);
        }
        classJson["inputs"] = inputsJson;

        nlohmann::json outputsJson = nlohmann::json::array();
        for(auto& output : metadata.outputs)
        {
            nlohmann::json outputJson;
            outputJson["name"] = output.name;
            outputJson["schema_name"] = output.schema_name;
            outputsJson.push_back(outputJson);
        }
        classJson["outputs"] = outputsJson;

        nlohmann::json thinkFunctionsJson = nlohmann::json::array();
        for(auto& thinkFunction : metadata.thinkFunctions)
        {
            thinkFunctionsJson.push_back(thinkFunction);
        }
        classJson["think_functions"] = thinkFunctionsJson;

        datamapJson.push_back(classJson);
    }

    WriteJSON(outputPath + "/datamaps.json", datamapJson);
}