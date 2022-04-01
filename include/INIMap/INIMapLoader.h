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

#include <DataTypes.h>

#include <INIMap/INIMap.h>

#include <filesystem>
#include <map>
#include <memory>
#include <string>

// forward declarations
class Game;
class GameContext;
class House;

class INIMapLoader final : public INIMap {
public:
    INIMapLoader(Game* pGame, const std::filesystem::path& mapname, const std::string& mapdata = "");
    ~INIMapLoader() override;

    std::unique_ptr<Map> load();

private:
    void loadMap();
    void loadHouses(const GameContext& context);
    void loadChoam();
    void loadUnits(const GameContext& context);
    void loadStructures(const GameContext& context);
    void loadReinforcements(const GameContext& context);
    void loadAITeams(const GameContext& context);
    void loadView(const GameContext& context);

    House* getOrCreateHouse(const GameContext& context, HOUSETYPE house);
    HOUSETYPE getHouseID(std::string_view name);

    Game* pGame;
    std::unique_ptr<Map> map;

    std::unordered_map<std::string, HOUSETYPE> housename2house;
};

#endif // INIMAPLOADER_H
