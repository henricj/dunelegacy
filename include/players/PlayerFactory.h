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
#include <utility>


class PlayerFactory {
public:
    class PlayerData {
    public:
        using create_functor = std::function<std::unique_ptr<Player>(const GameContext&, House*, const std::string&, Random&&)>;
        using load_functor = std::function<std::unique_ptr<Player>(const GameContext&, InputStream&, House*, Random&&)>;

        PlayerData(std::string&& playerclass, std::string&& name, create_functor&& pCreate, load_functor&& pLoad)
         : playerclass(std::move(playerclass)), name(std::move(name)), pCreate(std::move(pCreate)), pLoad(std::move(pLoad)) {
        }

        [[nodiscard]] const std::string& getPlayerClass() const {
            return playerclass;
        }

        [[nodiscard]] const std::string& getName() const {
            return name;
        }

        std::unique_ptr<Player> create(const GameContext& context, House* associatedHouse, const std::string& playername) const {
            auto pPlayer = pCreate(context, associatedHouse, playername, create_random(context, associatedHouse, playername));
            pPlayer->setPlayerclass(playerclass);
            return pPlayer;
        }

        std::unique_ptr<Player> load(const GameContext& context, InputStream& stream, House* associatedHouse) const {
            auto pPlayer = pLoad(context, stream, associatedHouse, create_random(context, associatedHouse,  "Default"));
            pPlayer->setPlayerclass(playerclass);
            return pPlayer;
        }

    private:
        const std::string playerclass;
        const std::string name;
        const create_functor pCreate;
        const load_functor pLoad;

        Random create_random(const GameContext& context, House* house, std::string_view playername) const {
            auto random_name = fmt::format("player {} {} {} {}", name, house->getHouseID(), playerclass, playername);

            return context.game.randomFactory.create(random_name);
        }
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

    template<typename PlayerType, typename... Args>
    static void add(std::string&& playerclass, std::string&& name, Args&&... args) {
        static_assert(std::is_base_of<Player, PlayerType>::value, "The PlayerType must be derived from Player");

        playerDataList.emplace_back(
            std::move(playerclass), std::move(name),
            [=](const GameContext& context, House* house, const std::string& playername, Random&& random) {
                return std::make_unique<PlayerType>(context, house, playername, std::move(random), args...);
            },
            [](const GameContext& context, InputStream& inputStream, House* house, Random&& random) {
                return std::make_unique<PlayerType>(context, inputStream, house, std::move(random));
            });
    }

    static std::vector<PlayerData> playerDataList;
};

#endif //PLAYERFACTORY_H
