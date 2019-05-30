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

#include <units/RaiderTrike.h>

#include <globals.h>

#include <FileClasses/GFXManager.h>
#include <House.h>
#include <Game.h>
#include <Map.h>
#include <Explosion.h>
#include <SoundPlayer.h>

RaiderTrike::RaiderTrike(House* newOwner) : GroundUnit(newOwner) {
    RaiderTrike::init();

    setHealth(getMaxHealth());
}

RaiderTrike::RaiderTrike(InputStream& stream) : GroundUnit(stream) {
    RaiderTrike::init();
}

void RaiderTrike::init() {
    itemID = Unit_RaiderTrike;
    owner->incrementUnits(itemID);

    numWeapons = 2;
    bulletType = Bullet_ShellSmall;

    graphicID = ObjPic_Trike;
    graphic = pGFXManager->getObjPic(graphicID,getOwner()->getHouseID());

    numImagesX = NUM_ANGLES;
    numImagesY = 1;
}

RaiderTrike::~RaiderTrike() = default;

void RaiderTrike::destroy() {
    if(currentGameMap->tileExists(location) && isVisible()) {
        Coord realPos(lround(realX), lround(realY));
        currentGame->getExplosionList().push_back(new Explosion(Explosion_SmallUnit, realPos, owner->getHouseID()));

        if(isVisible(getOwner()->getTeamID()))
            soundPlayer->playSoundAt(Sound_ExplosionSmall,location);
    }

    GroundUnit::destroy();
}

void RaiderTrike::playAttackSound() {
    soundPlayer->playSoundAt(Sound_MachineGun,location);
}
