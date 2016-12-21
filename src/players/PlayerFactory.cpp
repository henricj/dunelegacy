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
#include <players/SmartBot.h>
#include <players/QuantBot.h>

std::vector<PlayerFactory::PlayerData> PlayerFactory::playerDataList;

void PlayerFactory::registerAllPlayers() {

    playerDataList.push_back( PlayerData(  HUMANPLAYERCLASS,
                                            "Human Player",
                                            std::bind(HumanPlayer::create, std::placeholders::_1, std::placeholders::_2),
                                            std::bind(HumanPlayer::load, std::placeholders::_1, std::placeholders::_2) ));



    playerDataList.push_back( PlayerData(  "qBotVeryEasy",
                                            "qBotVeryEasy",
                                            std::bind(QuantBot::create, std::placeholders::_1, std::placeholders::_2, QuantBot::Difficulty::Defend),
                                            std::bind(QuantBot::load, std::placeholders::_1, std::placeholders::_2) ));

    playerDataList.push_back( PlayerData(  "qBotEasy",
                                            "qBotEasy",
                                            std::bind(QuantBot::create, std::placeholders::_1, std::placeholders::_2, QuantBot::Difficulty::Easy),
                                            std::bind(QuantBot::load, std::placeholders::_1, std::placeholders::_2) ));

    playerDataList.push_back( PlayerData(  "qBotMedium",
                                            "qBotMedium",
                                            std::bind(QuantBot::create, std::placeholders::_1, std::placeholders::_2, QuantBot::Difficulty::Medium),
                                            std::bind(QuantBot::load, std::placeholders::_1, std::placeholders::_2) ));

    playerDataList.push_back( PlayerData(  "qBotHard",
                                            "qBotHard",
                                            std::bind(QuantBot::create, std::placeholders::_1, std::placeholders::_2, QuantBot::Difficulty::Hard),
                                            std::bind(QuantBot::load, std::placeholders::_1, std::placeholders::_2) ));

    playerDataList.push_back( PlayerData(  "qBotBrutal",
                                            "qBotBrutal",
                                            std::bind(QuantBot::create, std::placeholders::_1, std::placeholders::_2, QuantBot::Difficulty::Brutal),
                                            std::bind(QuantBot::load, std::placeholders::_1, std::placeholders::_2) ));

    playerDataList.push_back( PlayerData(  "SmartBot",
                                            "SmartBot",
                                            std::bind(SmartBot::create, std::placeholders::_1, std::placeholders::_2, SmartBot::Difficulty::Normal),
                                            std::bind(SmartBot::load, std::placeholders::_1, std::placeholders::_2) ));

    playerDataList.push_back( PlayerData(  "AIPlayerEasy",
                                            "AI Player (easy)",
                                            std::bind(AIPlayer::create, std::placeholders::_1, std::placeholders::_2, AIPlayer::Difficulty::Easy),
                                            std::bind(AIPlayer::load, std::placeholders::_1, std::placeholders::_2) ));

    playerDataList.push_back( PlayerData(  "AIPlayerMedium",
                                            "AI Player (medium)",
                                            std::bind(AIPlayer::create, std::placeholders::_1, std::placeholders::_2, AIPlayer::Difficulty::Medium),
                                            std::bind(AIPlayer::load, std::placeholders::_1, std::placeholders::_2) ));

    playerDataList.push_back( PlayerData(  "AIPlayerHard",
                                            "AI Player (hard)",
                                            std::bind(AIPlayer::create, std::placeholders::_1, std::placeholders::_2, AIPlayer::Difficulty::Hard),
                                            std::bind(AIPlayer::load, std::placeholders::_1, std::placeholders::_2) ));

}
