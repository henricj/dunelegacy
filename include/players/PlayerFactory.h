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

#ifndef PLAYERFACTORY_H
#define PLAYERFACTORY_H

#include <players/Player.h>

#include <functional>
#include <memory>

class PlayerFactory {
public:
    class PlayerData {
    public:
        PlayerData( const std::string& playerclass, const std::string& name,
                    const std::function<std::unique_ptr<Player> (House*, const std::string&)>& pCreate,
                    const std::function<std::unique_ptr<Player> (InputStream&, House*)>& pLoad)
         : playerclass(playerclass), name(name), pCreate(pCreate), pLoad(pLoad) {
        }

        const std::string& getPlayerClass() const {
            return playerclass;
        }

        const std::string& getName() const {
            return name;
        }

        std::unique_ptr<Player> create(House* associatedHouse, const std::string& playername) const {
            auto pPlayer = pCreate(associatedHouse, playername);
            pPlayer->setPlayerclass(playerclass);
            return pPlayer;
        }

        std::unique_ptr<Player> load(InputStream& stream, House* associatedHouse) const {
            auto pPlayer = pLoad(stream, associatedHouse);
            pPlayer->setPlayerclass(playerclass);
            return pPlayer;
        }

    private:
        std::string playerclass;
        std::string name;
        std::function<std::unique_ptr<Player>(House*, const std::string&)> pCreate;
        std::function<std::unique_ptr<Player>(InputStream&, House*)> pLoad;
    };

    static const std::vector<PlayerData>& getList() {
        if(playerDataList.empty()) {
            registerAllPlayers();
        }
        return playerDataList;
    }

    static const PlayerData* getByIndex(int index) {
        if(playerDataList.empty()) {
            registerAllPlayers();
        }

        if(index < 0 || index >= static_cast<int>(playerDataList.size())) {
            return nullptr;
        }

        return &playerDataList[index];
    }

    static const PlayerData* getByPlayerClass(const std::string& playerclass) {
        if(playerDataList.empty()) {
            registerAllPlayers();
        }

        for(auto& player_data : playerDataList) {
            if(player_data.getPlayerClass() == playerclass) {
                return &player_data;
            }
        }

        return nullptr;
    }

    static int getIndexByPlayerClass(const std::string& playerclass) {
        if(playerDataList.empty()) {
            registerAllPlayers();
        }

        for(unsigned int i = 0; i < playerDataList.size(); i++) {
            if(playerDataList[i].getPlayerClass() == playerclass) {
                return i;
            }
        }

        return -1;
    }

private:

    static void registerAllPlayers();

    static std::vector<PlayerData> playerDataList;
};

#endif //PLAYERFACTORY_H
