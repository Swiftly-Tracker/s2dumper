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
#include <public/icvar.h>

#include <s2binlib/s2binlib.h>
#include "../hooks/vfunction.h"
#include "../shared/jsonc.h"

#include <map>
#include <string>

class CStyleSymbol
{
public:
    uint8_t m_bSymbolID;
};

class CStyleProperty
{
public:
    virtual ~CStyleProperty() = 0;
    virtual void unk001() = 0;
    virtual void unk002() = 0;
    virtual void unk003() = 0;
    virtual void unk004() = 0;
    virtual void unk005() = 0;
    virtual void unk006() = 0;
    virtual void ToString(CBufferString* out) = 0;
    virtual void unk008() = 0;
    virtual void unk009() = 0;
    virtual void unk010() = 0;
    virtual void unk011() = 0;
    virtual const char* GetDescription() = 0;
};

void DumpPanorama(std::string outputPath)
{
    void* factoryWrapperVTable = nullptr;
    s2binlib_find_vtable_nested_2("panorama", "panorama", "CStyleFactoryWrapper", &factoryWrapperVTable);

    s2binlib_dump_xrefs("panorama");

    void* CStyleSymbolConstructor = nullptr;
    s2binlib_find_func_with_string("panorama", "g_iStylePropertyIndexNextFree < 255", &CStyleSymbolConstructor);

    std::map<std::string, std::string> propertyNameToDescription;

    CUtlVector<CUtlString>* propertyAndAliasNames = reinterpret_cast<CUtlVector<CUtlString>*(*)()>(((void**)factoryWrapperVTable)[6])();
    for(int i = 0; i < propertyAndAliasNames->Count(); i++)
    {
        std::string name = propertyAndAliasNames->Element(i).Get();

        CStyleSymbol symbol;
        reinterpret_cast<void*(*)(CStyleSymbol*, const char*)>(CStyleSymbolConstructor)(&symbol, name.c_str());

        auto property = reinterpret_cast<CStyleProperty*(*)(void*, CStyleSymbol)>(((void**)factoryWrapperVTable)[3])(factoryWrapperVTable, symbol);
        if(property)
        {
            propertyNameToDescription[name] = property->GetDescription();
            reinterpret_cast<void(*)(void*, CStyleProperty*)>(((void**)factoryWrapperVTable)[5])(factoryWrapperVTable, property);
        }
    }

    nlohmann::json panoramaJson;
    for(auto& [name, description] : propertyNameToDescription)
    {
        panoramaJson["properties"][name] = description;
    }

    printf("Dumped %zu panorama properties\n", propertyNameToDescription.size());

    WriteJSON(outputPath + "/panorama.json", panoramaJson);
}