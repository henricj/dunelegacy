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

#ifndef INIMAPLOADER_H
#define INIMAPLOADER_H

#include <FileClasses/INIFile.h>
#include <DataTypes.h>

#include <INIMap/INIMap.h>
#include <misc/SDL2pp.h>

#include <string>
#include <map>
#include <memory>


// forward declarations
class Game;
class House;

class INIMapLoader : public INIMap {
public:
    INIMapLoader(Game* pGame, const std::string& mapname, const std::string& mapdata = "");
    ~INIMapLoader();

private:
    void load();
    void loadMap();
    void loadHouses();
    void loadChoam();
    void loadUnits();
    void loadStructures();
    void loadReinforcements();
    void loadAITeams();
    void loadView();

    House* getOrCreateHouse(int house);
    HOUSETYPE getHouseID(const std::string& name);

    Game* pGame;
    std::map<std::string, HOUSETYPE> housename2house;
};

#endif //INIMAPLOADER_H
