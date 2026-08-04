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

#include "../app/application.h"
#include "shared.h"

extern Application app;

void DumpInterfaces(std::string outputPath)
{
    auto interfaces = app.GetQueriedInterfaces();
    
    std::string output = "";
    for (const auto& interfaceName : interfaces)
    {
        if(output != "")
            output += "\n";
        output += interfaceName;
    }
    
    std::string path = outputPath + "/interfaces.txt";
    auto f = std::fopen(path.c_str(), "w");
    if(f)
    {
        fprintf(f, "%s", output.c_str());
        fclose(f);
    }
}