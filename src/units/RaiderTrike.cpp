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

namespace {
constexpr GroundUnitConstants raider_trike_constants{RaiderTrike::item_id, 2, Bullet_ShellSmall};
}

RaiderTrike::RaiderTrike(Uint32 objectID, const ObjectInitializer& initializer)
    : GroundUnit(raider_trike_constants, objectID, initializer) {
    RaiderTrike::init();

    setHealth(getMaxHealth());
}

RaiderTrike::RaiderTrike(Uint32 objectID, const ObjectStreamInitializer& initializer)
    : GroundUnit(raider_trike_constants, objectID, initializer) {
    RaiderTrike::init();
}

void RaiderTrike::init() {
    assert(itemID == Unit_RaiderTrike);
    owner->incrementUnits(itemID);

    graphicID = ObjPic_Trike;
    graphic = pGFXManager->getObjPic(graphicID,getOwner()->getHouseID());

    numImagesX = static_cast<int>(ANGLETYPE::NUM_ANGLES);
    numImagesY = 1;
}

RaiderTrike::~RaiderTrike() = default;

void RaiderTrike::destroy(const GameContext& context) {
    if(currentGameMap->tileExists(location) && isVisible()) {
        Coord realPos(lround(realX), lround(realY));
        currentGame->addExplosion(Explosion_SmallUnit, realPos, owner->getHouseID());

        if(isVisible(getOwner()->getTeamID()))
            soundPlayer->playSoundAt(Sound_ExplosionSmall,location);
    }

    GroundUnit::destroy(context);
}

void RaiderTrike::playAttackSound() {
    soundPlayer->playSoundAt(Sound_MachineGun,location);
}
