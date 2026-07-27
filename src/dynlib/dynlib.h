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

#ifndef _src_dynlib_dynlib_h
#define _src_dynlib_dynlib_h

#include <string_view>

#ifdef _WIN32
#include <Windows.h>
#include <direct.h>

#define STR(s) L##s
#define CH(c) L##c
#define DIR_SEPARATOR L'\\'
#define WIN_LIN(win, lin) win
#else
#include <dlfcn.h>

#define STR(s) s
#define CH(c) c
#define DIR_SEPARATOR '/'
#define WIN_LIN(win, lin) lin
#endif

void *load_library(std::string_view path);
void *get_export(void *address, std::string_view name);
void unload_library(void *lib);

#endif