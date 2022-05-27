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

#include "mmath.h"
#include <FileClasses/GFXManager.h>
#include <FileClasses/SFXManager.h>
#include <Game.h>
#include <House.h>
#include <Map.h>

namespace {
constexpr TurretBaseConstants gun_turret_constants{RocketTurret::item_id, Bullet_TurretRocket};
}

RocketTurret::RocketTurret(uint32_t objectID, const ObjectInitializer& initializer)
    : TurretBase(gun_turret_constants, objectID, initializer) {
    RocketTurret::init();

    RocketTurret::setHealth(getMaxHealth());
}

RocketTurret::RocketTurret(uint32_t objectID, const ObjectStreamInitializer& initializer)
    : TurretBase(gun_turret_constants, objectID, initializer) {
    RocketTurret::init();
}

void RocketTurret::init() {
    assert(itemID_ == Structure_RocketTurret);
    owner_->incrementStructures(itemID_);

    attackSound = Sound_enum::Sound_Rocket;

    graphicID_   = ObjPic_RocketTurret;
    graphic_     = dune::globals::pGFXManager->getObjPic(graphicID_, getOwner()->getHouseID());
    numImagesX_  = 10;
    numImagesY_  = 1;
    curAnimFrame = firstAnimFrame = lastAnimFrame = ((10 - static_cast<int>(drawnAngle_)) % 8) + 2;
}

RocketTurret::~RocketTurret() = default;

void RocketTurret::updateStructureSpecificStuff(const GameContext& context) {
    const auto& game = context.game;

    if ((!game.getGameInitSettings().getGameOptions().rocketTurretsNeedPower || getOwner()->hasPower())
        || (((game.gameType == GameType::Campaign) || (game.gameType == GameType::Skirmish)) && getOwner()->isAI())) {
        parent::updateStructureSpecificStuff(context);
    }
}

bool RocketTurret::canAttack(const ObjectBase* object) const {
    return object != nullptr
        && ((object->getOwner()->getTeamID() != owner_->getTeamID()) || object->getItemID() == Unit_Sandworm)
        && object->isVisible(getOwner()->getTeamID());
}

void RocketTurret::attack(const GameContext& context) {
    if ((weaponTimer != 0) || (target_.getObjPointer() == nullptr))
        return;

    const auto centerPoint       = getCenterPoint();
    auto* const pObject          = target_.getObjPointer();
    const auto targetCenterPoint = pObject->getClosestCenterPoint(location_);

    auto& game = context.game;
    auto& map  = context.map;

    if (distanceFrom(centerPoint, targetCenterPoint) < 3 * TILESIZE) {
        // we are just shooting a bullet as a gun turret would do
        // for air units do nothing
        if (!pObject->isAFlyingUnit()) {
            const auto& turret_data = game.objectData.data[Structure_GunTurret][static_cast<int>(originalHouseID_)];

            map.add_bullet(objectID_, &centerPoint, &targetCenterPoint, Bullet_ShellTurret, turret_data.weapondamage,
                           false, pObject);

            map.viewMap(static_cast<HOUSETYPE>(pObject->getOwner()->getTeamID()), location_, 2);
            dune::globals::soundPlayer->playSoundAt(Sound_enum::Sound_ExplosionSmall, location_);
            weaponTimer = turret_data.weaponreloadtime;
        }
    } else {
        // we are in normal shooting mode
        map.add_bullet(objectID_, &centerPoint, &targetCenterPoint, turret_constants().bulletType(),
                       game.objectData.data[itemID_][static_cast<int>(originalHouseID_)].weapondamage,
                       pObject->isAFlyingUnit(), nullptr);

        map.viewMap(static_cast<HOUSETYPE>(pObject->getOwner()->getTeamID()), location_, 2);
        dune::globals::soundPlayer->playSoundAt(attackSound, location_);
        weaponTimer = getWeaponReloadTime();
    }
}
