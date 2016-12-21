/*
 *  This file is part of Dune Legacy.
 *
 *  Dune Legacy is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  Dune Legacy is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with Dune Legacy.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef MAPINFO_H
#define MAPINFO_H

#include <DataTypes.h>

#include <string>

class MapInfo {
public:
    MapInfo(int mapSeed = INVALID, const std::string& author = "", const std::string& license = "", const std::string& losePicture = "LOSTVEHC.WSA",
            const std::string& winPicture = "WIN2.WSA", const std::string& briefPicture = "SARDUKAR.WSA", int techLevel = INVALID)
    : mapSeed(mapSeed), author(author), license(license), losePicture(losePicture),
      winPicture(winPicture), briefPicture(briefPicture), techLevel(techLevel) {
    }

    int mapSeed;
    std::string author;
    std::string license;
    std::string losePicture;
    std::string winPicture;
    std::string briefPicture;
    int timeout = 0;
    Coord   cursorPos = Coord(10,10);
    Coord   tacticalPos = Coord(10,10);
    int techLevel;
    int loseFlags = 1;
    int winFlags = 3;

};

#endif // MAPINFO_H
