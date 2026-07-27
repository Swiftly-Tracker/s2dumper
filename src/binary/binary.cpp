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

#include "binary.h"
#include "../dynlib/dynlib.h"

Binary::Binary(std::string game_path, bool exec_folder, std::string binary_name, std::string game)
    : m_sBinaryName(binary_name), m_pBinaryHandle(nullptr)
{
    std::string binary_path = game_path + (exec_folder ? "/bin" : "/" + game + "/bin") + "/" + WIN_LIN("win64/", "linuxsteamrt64/lib") + binary_name + WIN_LIN(".dll", ".so");
    m_pBinaryHandle = load_library(binary_path);
    if (m_pBinaryHandle == nullptr)
    {
        printf("[Binary] Failed to load binary: %.*s\n", (int)m_sBinaryName.size(), m_sBinaryName.data());
    }
}

Binary::~Binary()
{
    if (m_pBinaryHandle != nullptr)
        unload_library(m_pBinaryHandle);
}

bool Binary::IsValid()
{
    return m_pBinaryHandle != nullptr;
}

void *Binary::GetInterface(const char *interface_name)
{
    if (m_pBinaryHandle == nullptr)
        return nullptr;

    using CreateInterfaceFn = void *(*)(const char *, int *);
    CreateInterfaceFn create_interface = (CreateInterfaceFn)get_export(m_pBinaryHandle, "CreateInterface");
    if (create_interface == nullptr)
        return nullptr;

    return create_interface(interface_name, nullptr);
}