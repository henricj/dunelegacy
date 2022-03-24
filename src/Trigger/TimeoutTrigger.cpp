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

#include <Trigger/TimeoutTrigger.h>

#include <Game.h>
#include <House.h>
#include <SoundPlayer.h>
#include <globals.h>

TimeoutTrigger::TimeoutTrigger(uint32_t triggerCycleNumber) : Trigger(triggerCycleNumber) { }

TimeoutTrigger::TimeoutTrigger(InputStream& stream) : Trigger(stream) { }

TimeoutTrigger::~TimeoutTrigger() = default;

void TimeoutTrigger::save(OutputStream& stream) const {
    Trigger::save(stream);
}

void TimeoutTrigger::trigger(const GameContext& context) {
    auto& game = context.game;

    if ((game.loseFlags & WINLOSEFLAGS_TIMEOUT) != 0) {
        // player has won
        game.setGameWon();
        soundPlayer->playVoice(Voice_enum::YourMissionIsComplete, pLocalHouse->getHouseID());
    } else {
        // ai has won
        game.setGameLost();
        soundPlayer->playVoice(Voice_enum::YouHaveFailedYourMission, pLocalHouse->getHouseID());
    }
}
