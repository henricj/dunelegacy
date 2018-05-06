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

#include <globals.h>
#include <Game.h>
#include <House.h>
#include <SoundPlayer.h>

TimeoutTrigger::TimeoutTrigger(Uint32 triggerCycleNumber) : Trigger(triggerCycleNumber)
{
}

TimeoutTrigger::TimeoutTrigger(InputStream& stream) : Trigger(stream)
{
}

TimeoutTrigger::~TimeoutTrigger() = default;

void TimeoutTrigger::save(OutputStream& stream) const
{
    Trigger::save(stream);
}

void TimeoutTrigger::trigger()
{
    if((currentGame->loseFlags & WINLOSEFLAGS_TIMEOUT) != 0) {
        // player has won
        currentGame->setGameWon();
        soundPlayer->playVoice(YourMissionIsComplete, pLocalHouse->getHouseID());
    } else {
        // ai has won
        currentGame->setGameLost();
        soundPlayer->playVoice(YouHaveFailedYourMission, pLocalHouse->getHouseID());
    }
}
