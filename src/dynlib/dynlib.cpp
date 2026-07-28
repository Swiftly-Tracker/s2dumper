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

#include "dynlib.h"

#ifdef _WIN32
void *load_library(std::string_view path)
{
    HMODULE h = ::LoadLibraryA(path.data());
    return (void *)h;
}

void *get_export(void *h, std::string_view name)
{
    void *f = ::GetProcAddress((HMODULE)h, name.data());
    return f;
}

void unload_library(void *lib)
{
    if (!lib)
        return;
    ::FreeLibrary((HMODULE)lib);
}
#else
void *load_library(std::string_view path)
{
    void *h = dlopen(path.data(), RTLD_LAZY | RTLD_LOCAL);
    return h;
}
void *get_export(void *h, std::string_view name)
{
    void *f = dlsym(h, name.data());
    return f;
}
void unload_library(void *lib)
{
    if (!lib)
        return;
    dlclose(lib);
}
#endif