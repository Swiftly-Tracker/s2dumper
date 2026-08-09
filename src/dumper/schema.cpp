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

#include "../app/application.h"
#include "../shared/jsonc.h"
#include "../shared/hash.h"
#include "../shared/string.h"

extern Application app;

std::set<uint64_t> g_sNetworkedFields;
std::set<std::string> g_ProcessedSchemaClassNames;

bool IsStandardLayoutClass(SchemaClassInfoData_t* classData) {
    {
        auto pClass = classData;
        int classesWithFields = 0;
        do {
            classesWithFields += ((pClass->m_nSize > 1) || (pClass->m_nFieldCount != 0)) ? 1 : 0;

            if (classesWithFields > 1) return false;

            pClass = (pClass->m_pBaseClasses == nullptr) ? nullptr : pClass->m_pBaseClasses->m_pClass;
        } while (pClass != nullptr);
    }

    auto fields = classData->m_pFields;
    auto fieldsCount = classData->m_nFieldCount;
    for (uint16_t i = 0; i < fieldsCount; i++) {
        auto fieldType = fields[i].m_pType;
        if (fieldType->m_eTypeCategory == SchemaTypeCategory_t::SCHEMA_TYPE_DECLARED_CLASS) {
            CSchemaType_DeclaredClass* fClass = reinterpret_cast<CSchemaType_DeclaredClass*>(fieldType);
            if (fClass->m_pClassInfo && !IsStandardLayoutClass(fClass->m_pClassInfo)) return false;
        }
    }

    return true;
}

std::string GetBuiltinTypeName(CSchemaType_Builtin* pType)
{
    switch (pType->m_eBuiltinType)
    {
    case SCHEMA_BUILTIN_TYPE_VOID: return "void";
    case SCHEMA_BUILTIN_TYPE_CHAR: return "char";
    case SCHEMA_BUILTIN_TYPE_INT8: return "int8";
    case SCHEMA_BUILTIN_TYPE_UINT8: return "uint8";
    case SCHEMA_BUILTIN_TYPE_INT16: return "int16";
    case SCHEMA_BUILTIN_TYPE_UINT16: return "uint16";
    case SCHEMA_BUILTIN_TYPE_INT32: return "int32";
    case SCHEMA_BUILTIN_TYPE_UINT32: return "uint32";
    case SCHEMA_BUILTIN_TYPE_INT64: return "int64";
    case SCHEMA_BUILTIN_TYPE_UINT64: return "uint64";
    case SCHEMA_BUILTIN_TYPE_FLOAT32: return "float32";
    case SCHEMA_BUILTIN_TYPE_FLOAT64: return "float64";
    case SCHEMA_BUILTIN_TYPE_BOOL: return "bool";
    default: return "";
    }
}

std::string ReadFieldType(CSchemaType* field)
{

    if (field->IsA<CSchemaType_Builtin>())
    {
        return GetBuiltinTypeName(field->ReinterpretAs<CSchemaType_Builtin>());
    }
    else if (field->IsA<CSchemaType_DeclaredClass>())
    {
        auto classInfo = field->ReinterpretAs<CSchemaType_DeclaredClass>()->m_pClassInfo;
        if (classInfo)
        {
            return classInfo->m_pszName;
        }
        else
        {
            return field->m_sTypeName.Get();
        }
    }
    else if (field->IsA<CSchemaType_DeclaredEnum>())
    {
        return field->ReinterpretAs<CSchemaType_DeclaredEnum>()->m_pEnumInfo->m_pszName;
    }
    else if (field->IsA<CSchemaType_Ptr>())
    {
        return ReadFieldType(field->ReinterpretAs<CSchemaType_Ptr>()->GetInnerType().Get());
    }
    else if (field->IsA<CSchemaType_Bitfield>())
    {
        return "bitfield";
    }
    else if (field->IsA<CSchemaType_FixedArray>())
    {
        auto fixed_array = field->ReinterpretAs<CSchemaType_FixedArray>();
        return ReadFieldType(fixed_array->m_pElementType) + "["+ std::to_string(fixed_array->m_nElementCount) +"]";
    }
    else if (field->IsA<CSchemaType_Atomic>())
    {
        return field->m_sTypeName.Get();
    }
    else return field->m_sTypeName.Get();
}

void FindChainer(bool& has_chainer, int& chainer_offset, CSchemaClassInfo* classInfo)
{
    for (int i = 0; i < classInfo->m_nBaseClassCount; i++)
    {
        auto baseClass = classInfo->m_pBaseClasses[i].m_pClass;
        if (baseClass)
        {
            for (int j = 0; j < baseClass->m_nFieldCount; j++)
            {
                if (baseClass->m_pFields[j].m_pszName == std::string("__m_pChainEntity"))
                {
                    has_chainer = true;
                    chainer_offset = baseClass->m_pFields[j].m_nSingleInheritanceOffset;
                    break;
                }
            }
        }
        if (has_chainer) break;
    }

    if (!has_chainer)
    {
        for (int i = 0; i < classInfo->m_nBaseClassCount; i++)
        {
            auto baseClass = classInfo->m_pBaseClasses[i].m_pClass;
            if (baseClass)
            {
                FindChainer(has_chainer, chainer_offset, baseClass);
                if (has_chainer) break;
            }
        }
    }
}

void ReadClasses(CSchemaType_DeclaredClass* declClass, nlohmann::json& outJson)
{
    auto classInfo = declClass->m_pClassInfo;

    if (!classInfo) return;
    if(g_ProcessedSchemaClassNames.contains(classInfo->m_pszName)) return;
    g_ProcessedSchemaClassNames.insert(classInfo->m_pszName);

    uint32_t class_hash = hash_32_fnv1a_const(classInfo->m_pszName);
    bool isStruct = IsStandardLayoutClass(classInfo);

    nlohmann::json classObject = {
        {"name", classInfo->m_pszName},
        {"name_hash", class_hash},
        {"is_struct", isStruct},
        {"project", classInfo->m_pszProjectName ? classInfo->m_pszProjectName : "default"},
        {"alignment", classInfo->m_nAlignment},
        {"size", classInfo->m_nSize},
        {"fields_count", classInfo->m_nFieldCount},
    };

    if (classInfo->m_nBaseClassCount) {
        classObject["base_classes_count"] = classInfo->m_nBaseClassCount;
        for (int i = 0; i < classInfo->m_nBaseClassCount; i++) {
            classObject["base_classes"].push_back(classInfo->m_pBaseClasses[i].m_pClass->m_pszName);
        }
    }

    auto field_size = classInfo->m_nFieldCount;
    auto fields = classInfo->m_pFields;

    bool has_chainer = false;
    int chainer_offset = 0;

    for (int i = 0; i < field_size; i++)
    {
        if (fields[i].m_pszName == std::string("__m_pChainEntity"))
        {
            has_chainer = true;
            chainer_offset = fields[i].m_nSingleInheritanceOffset;
            break;
        }
    }

    if (!has_chainer)
    {
        FindChainer(has_chainer, chainer_offset, classInfo);
    }

    classObject["has_chainer"] = has_chainer;

    for (int i = 0; i < field_size; i++)
    {
        auto field = fields[i];
        uint64_t fieldHash = ((uint64_t)(class_hash) << 32 | hash_32_fnv1a_const(field.m_pszName));

        int size;
        uint8_t alignment;

        field.m_pType->GetSizeAndAlignment(size, alignment);

        nlohmann::json fieldObject = {
            {"name", field.m_pszName},
            {"name_hash", fieldHash},
            {"networked", g_sNetworkedFields.contains(fieldHash)},
            {"offset", field.m_nSingleInheritanceOffset},
            {"size", size},
            {"alignment", alignment},
        };

        switch (field.m_pType->m_eTypeCategory)
        {
            case SCHEMA_TYPE_BUILTIN:
            case SCHEMA_TYPE_DECLARED_ENUM:
            case SCHEMA_TYPE_DECLARED_CLASS:
            {
                fieldObject["kind"] = "ref";
                fieldObject["type"] = ReadFieldType(field.m_pType);
                break;
            }
            case SCHEMA_TYPE_ATOMIC:
            {
                fieldObject["kind"] = "atomic";
                fieldObject["type"] = explode(ReadFieldType(field.m_pType), "<")[0];
                fieldObject["templated"] = ReadFieldType(field.m_pType);

                switch (field.m_pType->m_eAtomicCategory)
                {
                case SCHEMA_ATOMIC_T:
                {
                    auto atomic = field.m_pType->ReinterpretAs<CSchemaType_Atomic_T>();
                    fieldObject["template"].push_back(ReadFieldType(atomic->m_pTemplateType));
                    break;
                }
                case SCHEMA_ATOMIC_TT:
                {
                    auto atomic = field.m_pType->ReinterpretAs<CSchemaType_Atomic_TT>();
                    fieldObject["template"].push_back(ReadFieldType(atomic->m_pTemplateType));
                    fieldObject["template"].push_back(ReadFieldType(atomic->m_pTemplateType2));
                    break;
                }
                case SCHEMA_ATOMIC_COLLECTION_OF_T:
                {
                    auto atomic = field.m_pType->ReinterpretAs<CSchemaType_Atomic_CollectionOfT>();
                    fieldObject["template"].push_back(ReadFieldType(atomic->m_pTemplateType));
                    if (atomic->m_nFixedBufferCount > 0) {
                        fieldObject["template"].push_back({
                            {"type", "literal"},
                            {"value", atomic->m_nFixedBufferCount}
                            });
                    }
                    break;
                }
                case SCHEMA_ATOMIC_I:
                {
                    auto atomic = field.m_pType->ReinterpretAs<CSchemaType_Atomic_I>();
                    fieldObject["template"].push_back({
                        {"type", "literal"},
                        {"value", atomic->m_nInteger}
                        });
                    break;
                }
                }

                break;
            }
            case SCHEMA_TYPE_POINTER:
            {
                fieldObject["kind"] = "ptr";
                fieldObject["type"] = ReadFieldType(field.m_pType);

                break;
            }
            case SCHEMA_TYPE_BITFIELD:
            {
                auto bitfield = field.m_pType->ReinterpretAs<CSchemaType_Bitfield>();
                fieldObject["kind"] = "bitfield";
                fieldObject["type"] = ReadFieldType(field.m_pType);
                fieldObject["count"] = bitfield->m_nBitfieldCount;
                break;
            }
            case SCHEMA_TYPE_FIXED_ARRAY:
            {
                auto fixed_array = field.m_pType->ReinterpretAs<CSchemaType_FixedArray>();
                fieldObject["kind"] = "fixed_array";
                fieldObject["type"] = ReadFieldType(fixed_array->m_pElementType);
                fieldObject["element_size"] = fixed_array->m_nElementSize;
                fieldObject["element_count"] = fixed_array->m_nElementCount;
                fieldObject["element_alignment"] = fixed_array->m_nElementAlignment;
                break;
            }
        }

        classObject["fields"].push_back(fieldObject);
    }

    outJson["classes"].push_back(classObject);
}

void ReadEnums(CSchemaType_DeclaredEnum* declClass, nlohmann::json& outJson)
{
    auto enumInfo = declClass->m_pEnumInfo;

    nlohmann::json enumJson = {
        {"name", enumInfo->m_pszName},
        {"project", enumInfo->m_pszProjectName ? enumInfo->m_pszProjectName : "default"},
        {"alignment", enumInfo->m_nAlignment},
        {"size", enumInfo->m_nSize},
        {"fields_count", enumInfo->m_nEnumeratorCount},
    };

    auto field_size = enumInfo->m_nEnumeratorCount;
    auto fields = enumInfo->m_pEnumerators;

    for (int i = 0; i < field_size; i++)
    {
        auto field = fields[i];
        enumJson["fields"].push_back({
            {"name", field.m_pszName},
            {"value", field.m_nValue},
        });
    }

    outJson["enums"].push_back(enumJson);
}

void DumpSchema(std::string outputPath)
{
    auto codegenDatabase = app.GetCodeGenDatabase();

    uint32_t classHash = 0;
    uint64_t fieldHash = 0;

    FOR_EACH_MAP_FAST(codegenDatabase->m_ClassInfos, i)
    {
        auto className = codegenDatabase->m_ClassInfos.Key(i);
        auto classInfo = codegenDatabase->m_ClassInfos[i];
        classHash = hash_32_fnv1a_const(className);
        FOR_EACH_VEC(classInfo->m_Fields, j)
        {
            auto fieldInfo = classInfo->m_Fields[j];
            fieldHash = ((uint64_t)(classHash) << 32 | hash_32_fnv1a_const(fieldInfo->m_pszFieldName.Get()));
            g_sNetworkedFields.insert(fieldHash);
        }
    }

    CSchemaSystem* schemaSystem = (CSchemaSystem*)app.GetSchemaSystem();

    nlohmann::json sdkJson;

    auto globalTypeScope = schemaSystem->GlobalTypeScope();

    int classes_count = globalTypeScope->m_DeclaredClasses.m_Map.Count();
    FOR_EACH_MAP(globalTypeScope->m_DeclaredClasses.m_Map, iter)
    {
        ReadClasses(globalTypeScope->m_DeclaredClasses.m_Map.Element(iter), sdkJson);
    }

    for (int i = 0; i < schemaSystem->m_TypeScopes.GetNumStrings(); i++)
    {
        auto ts = schemaSystem->m_TypeScopes[i];

        classes_count += ts->m_DeclaredClasses.m_Map.Count();

        FOR_EACH_MAP(ts->m_DeclaredClasses.m_Map, iter)
        {
            ReadClasses(ts->m_DeclaredClasses.m_Map.Element(iter), sdkJson);
        }
    }

    printf("Dumped %d classes.\n", classes_count);

    int enums_count = globalTypeScope->m_DeclaredEnums.m_Map.Count();
    FOR_EACH_MAP(globalTypeScope->m_DeclaredEnums.m_Map, iter)
    {
        ReadEnums(globalTypeScope->m_DeclaredEnums.m_Map.Element(iter), sdkJson);
    }

    for (int i = 0; i < schemaSystem->m_TypeScopes.GetNumStrings(); i++)
    {
        auto ts = schemaSystem->m_TypeScopes[i];

        enums_count += ts->m_DeclaredEnums.m_Map.Count();

        FOR_EACH_MAP(ts->m_DeclaredEnums.m_Map, iter)
        {
            ReadEnums(ts->m_DeclaredEnums.m_Map.Element(iter), sdkJson);
        }
    }

    printf("Dumped %d enums.\n", enums_count);
    WriteJSON(outputPath + "/sdk.json", sdkJson);
}