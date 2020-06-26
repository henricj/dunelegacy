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
#include <Map.h>

RocketTurret::RocketTurret(House* newOwner) : TurretBase(newOwner) {
    RocketTurret::init();

    ObjectBase::setHealth(getMaxHealth());
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
    curAnimFrame = firstAnimFrame = lastAnimFrame = ((10 - static_cast<int>(drawnAngle)) % 8) + 2;
}

RocketTurret::~RocketTurret() = default;

void RocketTurret::updateStructureSpecificStuff() {
    if( ( !currentGame->getGameInitSettings().getGameOptions().rocketTurretsNeedPower || getOwner()->hasPower() )
        || ( ((currentGame->gameType == GameType::Campaign) || (currentGame->gameType == GameType::Skirmish)) && getOwner()->isAI()) ) {
        TurretBase::updateStructureSpecificStuff();
    }
}

bool RocketTurret::canAttack(const ObjectBase* object) const {
    return object != nullptr
        && ((object->getOwner()->getTeamID() != owner->getTeamID()) || object->getItemID() == Unit_Sandworm)
        && object->isVisible(getOwner()->getTeamID());
}

void RocketTurret::attack() {
    if ((weaponTimer != 0) || (target.getObjPointer() == nullptr)) return;

    const auto centerPoint = getCenterPoint();
    const auto pObject = target.getObjPointer();
    const auto targetCenterPoint = pObject->getClosestCenterPoint(location);

    auto game = currentGame;
    auto map = currentGameMap;

    if(distanceFrom(centerPoint, targetCenterPoint) < 3 * TILESIZE) {
        // we are just shooting a bullet as a gun turret would do
        // for air units do nothing
        if (!pObject->isAFlyingUnit()) {
            const auto& turret_data = game->objectData.data[Structure_GunTurret][static_cast<int>(originalHouseID)];


            map->add_bullet(objectID, &centerPoint, &targetCenterPoint, Bullet_ShellTurret,
                turret_data.weapondamage, false, pObject);

            map->viewMap(static_cast<HOUSETYPE>(pObject->getOwner()->getTeamID()), location, 2);
            soundPlayer->playSoundAt(Sound_ExplosionSmall, location);
            weaponTimer = turret_data.weaponreloadtime;
        }
    } else {
        // we are in normal shooting mode
        map->add_bullet(objectID, &centerPoint, &targetCenterPoint, bulletType,
                                          game->objectData.data[itemID][static_cast<int>(originalHouseID)].weapondamage,
                                          pObject->isAFlyingUnit(), nullptr);

        map->viewMap(static_cast<HOUSETYPE>(pObject->getOwner()->getTeamID()), location, 2);
        soundPlayer->playSoundAt(attackSound, location);
        weaponTimer = getWeaponReloadTime();
    }
}
