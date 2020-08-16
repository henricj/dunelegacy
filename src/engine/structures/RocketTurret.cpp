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

#include "engine_mmath.h"

#include <structures/RocketTurret.h>

#include <Bullet.h>

#include <House.h>
#include <Game.h>
#include <Map.h>

namespace {
using namespace Dune::Engine;

constexpr TurretBaseConstants gun_turret_constants{RocketTurret::item_id, Bullet_TurretRocket};
} // namespace

namespace Dune::Engine {

RocketTurret::RocketTurret(uint32_t objectID, const ObjectInitializer& initializer)
    : TurretBase(gun_turret_constants, objectID, initializer) {
    RocketTurret::init();

    RocketTurret::setHealth(initializer.game(), getMaxHealth(initializer.game()));
}

RocketTurret::RocketTurret(uint32_t objectID, const ObjectStreamInitializer& initializer)
    : TurretBase(gun_turret_constants, objectID, initializer) {
    RocketTurret::init();
}

void RocketTurret::init() {
    assert(itemID == Structure_RocketTurret);
    owner->incrementStructures(itemID);
}

RocketTurret::~RocketTurret() = default;

void RocketTurret::updateStructureSpecificStuff(const GameContext& context) {
    auto& game = context.game;

    if((!game.getGameInitSettings().getGameOptions().rocketTurretsNeedPower || getOwner()->hasPower()) ||
       (((game.gameType == GameType::Campaign) || (game.gameType == GameType::Skirmish)) && getOwner()->isAI())) {
        parent::updateStructureSpecificStuff(context);
    }
}

bool RocketTurret::canAttack(const GameContext& context, const ObjectBase* object) const {
    return object != nullptr &&
           ((object->getOwner()->getTeamID() != owner->getTeamID()) || object->getItemID() == Unit_Sandworm) &&
           object->isVisible(getOwner()->getTeamID());
}

void RocketTurret::attack(const GameContext& context) {
    if((weaponTimer != 0)) return;

    auto* const pObject = target.getObjPointer(context.objectManager);
    if(!pObject) return;

    const auto  centerPoint       = getCenterPoint();
    const auto  targetCenterPoint = pObject->getClosestCenterPoint(location);

    auto& game = context.game;
    auto& map  = context.map;

    if(distanceFrom(centerPoint, targetCenterPoint) < 3 * TILESIZE) {
        // we are just shooting a bullet as a gun turret would do
        // for air units do nothing
        if(!pObject->isAFlyingUnit()) {
            const auto& turret_data = game.getObjectData(Structure_GunTurret, originalHouseID);

            game.add_bullet(context, objectID, &centerPoint, &targetCenterPoint, Bullet_ShellTurret,
                            turret_data.weapondamage, false, pObject);

            map.viewMap(static_cast<HOUSETYPE>(pObject->getOwner()->getTeamID()), location, 2);
            weaponTimer = turret_data.weaponreloadtime;
        }
    } else {
        // we are in normal shooting mode
        game.add_bullet(context, objectID, &centerPoint, &targetCenterPoint, turret_constants().bulletType(),
                        game.getObjectData(itemID, originalHouseID).weapondamage,
                        pObject->isAFlyingUnit(), nullptr);

        map.viewMap(static_cast<HOUSETYPE>(pObject->getOwner()->getTeamID()), location, 2);
        weaponTimer = getWeaponReloadTime(game);
    }
}

} // namespace Dune::Engine
