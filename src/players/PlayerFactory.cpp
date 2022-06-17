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

#include <players/PlayerFactory.h>

#include "Game.h"
#include "ObjectBase.h"
#include "misc/Random.h"

#include <players/AIPlayer.h>
#include <players/CampaignAIPlayer.h>
#include <players/HumanPlayer.h>
#include <players/QuantBot.h>
#include <players/SmartBot.h>

std::vector<PlayerFactory::PlayerData> PlayerFactory::playerDataList;

Random
PlayerFactory::PlayerData::create_random(const GameContext& context, House* house, std::string_view playername) const {
    const auto random_name =
        fmt::format("player {} {} {} {}", name, static_cast<int>(house->getHouseID()), playerclass, playername);

    return context.game.randomFactory.create(random_name);
}

void PlayerFactory::registerAllPlayers() {
    add<HumanPlayer>(HUMANPLAYERCLASS, "Human Player");
    add<QuantBot>("qBotVeryEasy", "qBotVeryEasy", QuantBot::Difficulty::Defend);
    add<QuantBot>("qBotEasy", "qBotEasy", QuantBot::Difficulty::Easy);
    add<QuantBot>("qBotMedium", "qBotMedium", QuantBot::Difficulty::Medium);
    add<QuantBot>("qBotHard", "qBotHard", QuantBot::Difficulty::Hard);
    add<QuantBot>("qBotBrutal", "qBotBrutal", QuantBot::Difficulty::Brutal);
    add<SmartBot>("SmartBot", "SmartBot", SmartBot::Difficulty::Normal);
    add<AIPlayer>("AIPlayerEasy", "AI Player (easy)", AIPlayer::Difficulty::Easy);
    add<AIPlayer>("AIPlayerMedium", "AI Player (medium)", AIPlayer::Difficulty::Medium);
    add<AIPlayer>("AIPlayerHard", "AI Player (hard)", AIPlayer::Difficulty::Hard);
    add<CampaignAIPlayer>("CampaignAIPlayer", "CampaignAIPlayer");
}
