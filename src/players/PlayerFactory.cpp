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

#include <players/HumanPlayer.h>
#include <players/AIPlayer.h>
#include <players/CampaignAIPlayer.h>
#include <players/SmartBot.h>
#include <players/QuantBot.h>

std::vector<PlayerFactory::PlayerData> PlayerFactory::playerDataList;

void PlayerFactory::registerAllPlayers() {

    playerDataList.emplace_back(  HUMANPLAYERCLASS,
                                            "Human Player",
                                            [](House* house,const std::string& playername) { return std::make_unique<HumanPlayer>(house, playername); },
                                            [](InputStream& inputStream, House* house) { return std::make_unique<HumanPlayer>(inputStream, house); } );

    playerDataList.emplace_back(  "qBotVeryEasy",
                                            "qBotVeryEasy",
        [](House* house, const std::string& playername) { return std::make_unique<QuantBot>(house, playername, QuantBot::Difficulty::Defend); },
        [](InputStream& inputStream, House* house) { return std::make_unique<QuantBot>(inputStream, house); } );

    playerDataList.emplace_back(  "qBotEasy",
                                            "qBotEasy",
        [](House* house, const std::string& playername) { return std::make_unique<QuantBot>(house, playername, QuantBot::Difficulty::Easy); },
        [](InputStream& inputStream, House* house) { return std::make_unique<QuantBot>(inputStream, house); });

    playerDataList.emplace_back(  "qBotMedium",
                                            "qBotMedium",
        [](House* house, const std::string& playername) { return std::make_unique<QuantBot>(house, playername, QuantBot::Difficulty::Medium); },
        [](InputStream& inputStream, House* house) { return std::make_unique<QuantBot>(inputStream, house); });

    playerDataList.emplace_back(  "qBotHard",
                                            "qBotHard",
        [](House* house, const std::string& playername) { return std::make_unique<QuantBot>(house, playername, QuantBot::Difficulty::Hard); },
        [](InputStream& inputStream, House* house) { return std::make_unique<QuantBot>(inputStream, house); });

    playerDataList.emplace_back(  "qBotBrutal",
                                            "qBotBrutal",
        [](House* house, const std::string& playername) { return std::make_unique<QuantBot>(house, playername, QuantBot::Difficulty::Brutal); },
        [](InputStream& inputStream, House* house) { return std::make_unique<QuantBot>(inputStream, house); });

    playerDataList.emplace_back(  "SmartBot",
                                            "SmartBot",
        [](House* house, const std::string& playername) { return std::make_unique<SmartBot>(house, playername, SmartBot::Difficulty::Normal); },
        [](InputStream& inputStream, House* house) { return std::make_unique<SmartBot>(inputStream, house); });

    playerDataList.emplace_back(  "AIPlayerEasy",
                                            "AI Player (easy)",
        [](House* house, const std::string& playername) { return std::make_unique<AIPlayer>(house, playername, AIPlayer::Difficulty::Easy); },
        [](InputStream& inputStream, House* house) { return std::make_unique<AIPlayer>(inputStream, house); });

    playerDataList.emplace_back(  "AIPlayerMedium",
                                            "AI Player (medium)",
        [](House* house, const std::string& playername) { return std::make_unique<AIPlayer>(house, playername, AIPlayer::Difficulty::Medium); },
        [](InputStream& inputStream, House* house) { return std::make_unique<AIPlayer>(inputStream, house); });

    playerDataList.emplace_back(  "AIPlayerHard",
                                            "AI Player (hard)",
        [](House* house, const std::string& playername) { return std::make_unique<AIPlayer>(house, playername, AIPlayer::Difficulty::Hard); },
        [](InputStream& inputStream, House* house) { return std::make_unique<AIPlayer>(inputStream, house); });

    playerDataList.emplace_back(  "CampaignAIPlayer",
                                            "CampaignAIPlayer",
        [](House* house, const std::string& playername) { return std::make_unique<CampaignAIPlayer>(house, playername); },
        [](InputStream& inputStream, House* house) { return std::make_unique<CampaignAIPlayer>(inputStream, house); } );

}
