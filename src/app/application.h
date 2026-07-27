/************************************************************************************************
 *  S2Dumper is a dumper for various properties of Source2-based games.
 *  Copyright (C) 2026 Swiftly Solution SRL via Sava Andrei-Sebastian and it's contributors
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

#ifndef _src_app_application_h
#define _src_app_application_h

#include <string>

#include "../binary/binary.h"

#include "public/schemasystem/schemasystem.h"
#include "public/eiface.h"
#include "public/tier0/interface.h"

class Application
{
public:
    void Initialize(std::string gamePath, std::string game);
    void Shutdown();

    void *GetCVar();
    void *GetSchemaSystem();

private:
    Binary *tier0;
    Binary *schema;

    CSchemaSystem *m_pSchemaSystem;
    ICvar *m_pCVar;
};

#endif