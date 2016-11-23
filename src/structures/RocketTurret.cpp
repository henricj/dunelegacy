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

#include <structures/RocketTurret.h>

#include <globals.h>

#include <Bullet.h>
#include <SoundPlayer.h>

#include <FileClasses/GFXManager.h>
#include <FileClasses/SFXManager.h>
#include <House.h>
#include <Game.h>

RocketTurret::RocketTurret(House* newOwner) : TurretBase(newOwner) {
    RocketTurret::init();

    setHealth(getMaxHealth());
}

RocketTurret::RocketTurret(InputStream& stream) : TurretBase(stream) {
    RocketTurret::init();
}

void RocketTurret::init() {
    itemID = Structure_RocketTurret;
    owner->incrementStructures(itemID);

    attackSound = Sound_Rocket;
    bulletType = Bullet_TurretRocket;

    graphicID = ObjPic_RocketTurret;
    graphic = pGFXManager->getObjPic(graphicID,getOwner()->getHouseID());
    numImagesX = 10;
    numImagesY = 1;
    curAnimFrame = firstAnimFrame = lastAnimFrame = ((10-drawnAngle) % 8) + 2;
}

RocketTurret::~RocketTurret() {
}

void RocketTurret::updateStructureSpecificStuff() {
    if( ( !currentGame->getGameInitSettings().getGameOptions().rocketTurretsNeedPower || getOwner()->hasPower() )
        || ( ((currentGame->gameType == GameType::Campaign) || (currentGame->gameType == GameType::Skirmish)) && getOwner()->isAI()) ) {
        TurretBase::updateStructureSpecificStuff();
    }
}

bool RocketTurret::canAttack(const ObjectBase* object) const {
    if((object != nullptr)
        && ((object->getOwner()->getTeam() != owner->getTeam()) || object->getItemID() == Unit_Sandworm)
        && object->isVisible(getOwner()->getTeam())) {
        return true;
    } else {
        return false;
    }
}

void RocketTurret::attack() {
    if((weaponTimer == 0) && (target.getObjPointer() != nullptr)) {
        Coord centerPoint = getCenterPoint();
        Coord targetCenterPoint = target.getObjPointer()->getClosestCenterPoint(location);

        if(distanceFrom(centerPoint, targetCenterPoint) < 3 * TILESIZE) {
            // we are just shooting a bullet as a gun turret would do
            bulletList.push_back( new Bullet( objectID, &centerPoint, &targetCenterPoint, Bullet_ShellTurret,
                                                   currentGame->objectData.data[Structure_GunTurret][originalHouseID].weapondamage,
                                                   target.getObjPointer()->isAFlyingUnit() ) );

            soundPlayer->playSoundAt(Sound_Gun, location);
            weaponTimer = currentGame->objectData.data[Structure_GunTurret][originalHouseID].weaponreloadtime;
        } else {
            // we are in normal shooting mode
            bulletList.push_back( new Bullet( objectID, &centerPoint, &targetCenterPoint, bulletType,
                                                   currentGame->objectData.data[itemID][originalHouseID].weapondamage,
                                                   target.getObjPointer()->isAFlyingUnit() ) );

            soundPlayer->playSoundAt(attackSound, location);
            weaponTimer = getWeaponReloadTime();
        }

    }
}
