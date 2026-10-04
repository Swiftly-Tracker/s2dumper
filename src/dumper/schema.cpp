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

#include <optional>

extern Application app;

struct NetworkData
{
    std::string serializer;
    std::string encoder;
    std::string sendProxyRecipientsFilter;
    std::string changePointerCallback;
    std::vector<std::string> changeCallbacks;
    std::string typeOverride;
    int bitCount;
    float min;
    float max;
};

std::map<uint64_t, NetworkData> g_NetworkedFields;

struct TemplateArgData
{
    bool literal = false;
    std::string type;
    int64_t value = 0;
};

struct FieldData
{
    std::string name;
    uint64_t nameHash;
    std::optional<NetworkData> network;
    int offset;
    int size;
    int alignment;

    std::string kind;
    std::string type;
    std::string templated;
    std::vector<TemplateArgData> templateArgs;

    std::optional<int> count;
    std::optional<int> elementSize;
    std::optional<int> elementCount;
    std::optional<int> elementAlignment;
};

struct ClassData
{
    std::string name;
    uint32_t nameHash;
    bool isStruct;
    std::string project;
    int alignment;
    int size;
    int fieldsCount;
    bool hasChainer;

    std::vector<std::string> baseClasses;
    std::vector<FieldData> fields;
};

struct EnumeratorData
{
    std::string name;
    int64_t value;
};

struct EnumData
{
    std::string name;
    std::string project;
    int alignment;
    int size;
    int fieldsCount;

    std::vector<EnumeratorData> fields;
};

std::vector<ClassData> g_ClassData;
std::vector<EnumData> g_EnumData;

std::string SafeString(const char* str)
{
    return str ? str : "";
}

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

void CollectField(const SchemaClassFieldData_t& field, uint32_t classHash, ClassData& classData)
{
    FieldData data;
    data.name = field.m_pszName;
    data.nameHash = ((uint64_t)(classHash) << 32 | hash_32_fnv1a_const(field.m_pszName));
    auto networkIt = g_NetworkedFields.find(data.nameHash);
    if (networkIt != g_NetworkedFields.end())
        data.network = networkIt->second;
    data.offset = field.m_nSingleInheritanceOffset;

    int size;
    uint8_t alignment;
    field.m_pType->GetSizeAndAlignment(size, alignment);
    data.size = size;
    data.alignment = alignment;

    switch (field.m_pType->m_eTypeCategory)
    {
        case SCHEMA_TYPE_BUILTIN:
        case SCHEMA_TYPE_DECLARED_ENUM:
        case SCHEMA_TYPE_DECLARED_CLASS:
        {
            data.kind = "ref";
            data.type = ReadFieldType(field.m_pType);
            break;
        }
        case SCHEMA_TYPE_ATOMIC:
        {
            data.kind = "atomic";
            data.templated = ReadFieldType(field.m_pType);
            data.type = explode(data.templated, "<")[0];

            switch (field.m_pType->m_eAtomicCategory)
            {
            case SCHEMA_ATOMIC_T:
            {
                auto atomic = field.m_pType->ReinterpretAs<CSchemaType_Atomic_T>();
                data.templateArgs.push_back({false, ReadFieldType(atomic->m_pTemplateType), 0});
                break;
            }
            case SCHEMA_ATOMIC_TT:
            {
                auto atomic = field.m_pType->ReinterpretAs<CSchemaType_Atomic_TT>();
                data.templateArgs.push_back({false, ReadFieldType(atomic->m_pTemplateType), 0});
                data.templateArgs.push_back({false, ReadFieldType(atomic->m_pTemplateType2), 0});
                break;
            }
            case SCHEMA_ATOMIC_COLLECTION_OF_T:
            {
                auto atomic = field.m_pType->ReinterpretAs<CSchemaType_Atomic_CollectionOfT>();
                data.templateArgs.push_back({false, ReadFieldType(atomic->m_pTemplateType), 0});
                if (atomic->m_nFixedBufferCount > 0)
                    data.templateArgs.push_back({true, "", (int64_t)atomic->m_nFixedBufferCount});
                break;
            }
            case SCHEMA_ATOMIC_I:
            {
                auto atomic = field.m_pType->ReinterpretAs<CSchemaType_Atomic_I>();
                data.templateArgs.push_back({true, "", atomic->m_nInteger});
                break;
            }
            }

            break;
        }
        case SCHEMA_TYPE_POINTER:
        {
            data.kind = "ptr";
            data.type = ReadFieldType(field.m_pType);
            break;
        }
        case SCHEMA_TYPE_BITFIELD:
        {
            auto bitfield = field.m_pType->ReinterpretAs<CSchemaType_Bitfield>();
            data.kind = "bitfield";
            data.type = ReadFieldType(field.m_pType);
            data.count = bitfield->m_nBitfieldCount;
            break;
        }
        case SCHEMA_TYPE_FIXED_ARRAY:
        {
            auto fixedArray = field.m_pType->ReinterpretAs<CSchemaType_FixedArray>();
            data.kind = "fixed_array";
            data.type = ReadFieldType(fixedArray->m_pElementType);
            data.elementSize = fixedArray->m_nElementSize;
            data.elementCount = fixedArray->m_nElementCount;
            data.elementAlignment = fixedArray->m_nElementAlignment;
            break;
        }
        default:
            break;
    }

    classData.fields.push_back(std::move(data));
}

void CollectClass(CSchemaType_DeclaredClass* declClass)
{
    auto classInfo = declClass->m_pClassInfo;
    if (!classInfo) return;

    ClassData data;
    data.name = classInfo->m_pszName;
    data.nameHash = hash_32_fnv1a_const(classInfo->m_pszName);
    data.isStruct = IsStandardLayoutClass(classInfo);
    data.project = classInfo->m_pszProjectName ? classInfo->m_pszProjectName : "default";
    data.alignment = classInfo->m_nAlignment;
    data.size = classInfo->m_nSize;
    data.fieldsCount = classInfo->m_nFieldCount;

    for (int i = 0; i < classInfo->m_nBaseClassCount; i++)
        data.baseClasses.push_back(classInfo->m_pBaseClasses[i].m_pClass->m_pszName);

    bool hasChainer = false;
    int chainerOffset = 0;
    for (int i = 0; i < classInfo->m_nFieldCount; i++)
    {
        if (classInfo->m_pFields[i].m_pszName == std::string("__m_pChainEntity"))
        {
            hasChainer = true;
            chainerOffset = classInfo->m_pFields[i].m_nSingleInheritanceOffset;
            break;
        }
    }
    if (!hasChainer)
        FindChainer(hasChainer, chainerOffset, classInfo);
    data.hasChainer = hasChainer;

    for (int i = 0; i < classInfo->m_nFieldCount; i++)
        CollectField(classInfo->m_pFields[i], data.nameHash, data);

    g_ClassData.push_back(std::move(data));
}

void CollectEnum(CSchemaType_DeclaredEnum* declEnum)
{
    auto enumInfo = declEnum->m_pEnumInfo;

    EnumData data;
    data.name = enumInfo->m_pszName;
    data.project = enumInfo->m_pszProjectName ? enumInfo->m_pszProjectName : "default";
    data.alignment = enumInfo->m_nAlignment;
    data.size = enumInfo->m_nSize;
    data.fieldsCount = enumInfo->m_nEnumeratorCount;

    for (int i = 0; i < enumInfo->m_nEnumeratorCount; i++)
    {
        auto& enumerator = enumInfo->m_pEnumerators[i];
        data.fields.push_back({enumerator.m_pszName, enumerator.m_nValue});
    }

    g_EnumData.push_back(std::move(data));
}

void CollectNetworkedFields()
{
    auto codegenDatabase = app.GetCodeGenDatabase();

    printf("%p\n", codegenDatabase);

    FOR_EACH_DICT_FAST(codegenDatabase->m_ClassInfos, i)
    {
        auto className = codegenDatabase->m_ClassInfos.GetElementName(i);
        auto classInfo = codegenDatabase->m_ClassInfos[i];
        uint32_t classHash = hash_32_fnv1a_const(className);
        FOR_EACH_VEC(classInfo->m_Fields, j)
        {
            auto fieldInfo = classInfo->m_Fields[j];
            uint64_t fieldHash = ((uint64_t)(classHash) << 32 | hash_32_fnv1a_const(fieldInfo->m_pszFieldName.Get()));

            NetworkData network;
            network.serializer = SafeString(fieldInfo->m_NetworkSerializer.Get());
            network.encoder = SafeString(fieldInfo->m_NetworkEncoder.Get());
            if (fieldInfo->m_NetworkSendProxyRecipientsFilter)
                network.sendProxyRecipientsFilter = SafeString(fieldInfo->m_NetworkSendProxyRecipientsFilter->m_FilterName.Get());
            if (fieldInfo->m_NetworkChangePointerCallback)
                network.changePointerCallback = SafeString(fieldInfo->m_NetworkChangePointerCallback->m_CallbackName.Get());
            FOR_EACH_VEC(fieldInfo->m_NetworkChangeCb, k)
            {
                network.changeCallbacks.push_back(SafeString(fieldInfo->m_NetworkChangeCb[k].Get()));
            }
            network.typeOverride = SafeString(fieldInfo->m_TypeOverride.Get());
            network.bitCount = fieldInfo->m_NetworkBitCount;
            network.min = fieldInfo->m_NetworkMin;
            network.max = fieldInfo->m_NetworkMax;

            g_NetworkedFields[fieldHash] = std::move(network);
        }
    }
}

void CollectClassesAndEnums()
{
    CSchemaSystem* schemaSystem = (CSchemaSystem*)app.GetSchemaSystem();

    auto globalTypeScope = schemaSystem->GlobalTypeScope();

    FOR_EACH_MAP(globalTypeScope->m_DeclaredClasses.m_Map, iter)
    {
        CollectClass(globalTypeScope->m_DeclaredClasses.m_Map.Element(iter));
    }

    FOR_EACH_MAP(globalTypeScope->m_DeclaredEnums.m_Map, iter)
    {
        CollectEnum(globalTypeScope->m_DeclaredEnums.m_Map.Element(iter));
    }

    for (int i = 0; i < schemaSystem->m_TypeScopes.GetNumStrings(); i++)
    {
        auto ts = schemaSystem->m_TypeScopes[i];

        FOR_EACH_MAP(ts->m_DeclaredClasses.m_Map, iter)
        {
            CollectClass(ts->m_DeclaredClasses.m_Map.Element(iter));
        }

        FOR_EACH_MAP(ts->m_DeclaredEnums.m_Map, iter)
        {
            CollectEnum(ts->m_DeclaredEnums.m_Map.Element(iter));
        }
    }
}

void DumpSchema(std::string outputPath)
{
    CollectNetworkedFields();
    CollectClassesAndEnums();

    printf("Dumped %zu classes.\n", g_ClassData.size());
    printf("Dumped %zu enums.\n", g_EnumData.size());

    nlohmann::json sdkJson;

    for (auto& classData : g_ClassData)
    {
        nlohmann::json classJson = {
            {"name", classData.name},
            {"name_hash", classData.nameHash},
            {"is_struct", classData.isStruct},
            {"project", classData.project},
            {"alignment", classData.alignment},
            {"size", classData.size},
            {"fields_count", classData.fieldsCount},
            {"has_chainer", classData.hasChainer},
        };

        if (!classData.baseClasses.empty())
        {
            classJson["base_classes_count"] = classData.baseClasses.size();
            classJson["base_classes"] = classData.baseClasses;
        }

        for (auto& field : classData.fields)
        {
            nlohmann::json fieldJson = {
                {"name", field.name},
                {"name_hash", field.nameHash},
                {"networked", field.network.has_value()},
                {"offset", field.offset},
                {"size", field.size},
                {"alignment", field.alignment},
            };

            if (!field.kind.empty())
            {
                fieldJson["kind"] = field.kind;
                fieldJson["type"] = field.type;
            }

            if (field.network)
            {
                auto& network = *field.network;
                fieldJson["network"] = {
                    {"serializer", network.serializer},
                    {"encoder", network.encoder},
                    {"send_proxy_recipients_filter", network.sendProxyRecipientsFilter},
                    {"change_pointer_callback", network.changePointerCallback},
                    {"change_callbacks", network.changeCallbacks},
                    {"type_override", network.typeOverride},
                    {"bit_count", network.bitCount},
                    {"min", network.min},
                    {"max", network.max},
                };
            }

            if (field.kind == "atomic")
                fieldJson["templated"] = field.templated;

            for (auto& arg : field.templateArgs)
            {
                if (arg.literal)
                    fieldJson["template"].push_back({{"type", "literal"}, {"value", arg.value}});
                else
                    fieldJson["template"].push_back(arg.type);
            }

            if (field.count) fieldJson["count"] = *field.count;
            if (field.elementSize) fieldJson["element_size"] = *field.elementSize;
            if (field.elementCount) fieldJson["element_count"] = *field.elementCount;
            if (field.elementAlignment) fieldJson["element_alignment"] = *field.elementAlignment;

            classJson["fields"].push_back(fieldJson);
        }

        sdkJson["classes"].push_back(classJson);
    }

    for (auto& enumData : g_EnumData)
    {
        nlohmann::json enumJson = {
            {"name", enumData.name},
            {"project", enumData.project},
            {"alignment", enumData.alignment},
            {"size", enumData.size},
            {"fields_count", enumData.fieldsCount},
        };

        for (auto& enumerator : enumData.fields)
            enumJson["fields"].push_back({{"name", enumerator.name}, {"value", enumerator.value}});

        sdkJson["enums"].push_back(enumJson);
    }

    WriteJSON(outputPath + "/sdk.json", sdkJson);
}
