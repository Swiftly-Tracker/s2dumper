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

#ifndef _src_dumper_shared_h
#define _src_dumper_shared_h

#include <vector>
#include <string>

std::vector<std::string> ParseFlags(uint64_t flags);
void DumpCommands(std::string outputPath);
void DumpConVars(std::string outputPath);
void DumpSchema(std::string outputPath);
void DumpInterfaces(std::string outputPath);
void DumpEntitySystem(std::string outputPath);
void DumpDatamaps(std::string outputPath);
void PopulateConStuff(std::string module_name);
void InterfaceQueried(std::string interfaceName);

#endif