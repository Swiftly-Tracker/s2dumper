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

#ifndef _src_binary_binary_h
#define _src_binary_binary_h

#include <string>
#include "public/tier0/interface.h"

class Binary
{
public:
    Binary(std::string binary_name, std::string game_name);
    ~Binary();

    bool IsValid();
    void *GetInterface(const char *interface_name);
    CreateInterfaceFn GetFactory();
    void *GetExport(const char *export_name);

private:
    std::string m_sBinaryName;
    void *m_pBinaryHandle;
};

#endif