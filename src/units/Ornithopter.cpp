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

#include <units/Ornithopter.h>

#include <globals.h>

#include <FileClasses/GFXManager.h>
#include <Game.h>
#include <House.h>
#include <Map.h>
#include <SoundPlayer.h>

#include "mmath.h"

namespace {
inline constexpr auto ORNITHOPTER_FRAMETIME = 3;

constexpr AirUnitConstants ornithopter_constants{Ornithopter::item_id, 1, Bullet_SmallRocket};
} // namespace

Ornithopter::Ornithopter(uint32_t objectID, const ObjectInitializer& initializer)
    : AirUnit(ornithopter_constants, objectID, initializer) {

    Ornithopter::init();

    Ornithopter::setHealth(getMaxHealth());
}

Ornithopter::Ornithopter(uint32_t objectID, const ObjectStreamInitializer& initializer)
    : AirUnit(ornithopter_constants, objectID, initializer) {
    Ornithopter::init();

    auto& stream = initializer.stream();

    timeLastShot = stream.readUint32();
}

void Ornithopter::init() {
    assert(itemID_ == Unit_Ornithopter);
    owner_->incrementUnits(itemID_);

    const auto* const gfx = dune::globals::pGFXManager.get();

    graphicID_    = ObjPic_Ornithopter;
    graphic_      = gfx->getObjPic(graphicID_, getOwner()->getHouseID());
    shadowGraphic = gfx->getObjPic(ObjPic_OrnithopterShadow, getOwner()->getHouseID());

    numImagesX_ = NUM_ANGLES;
    numImagesY_ = 3;

    currentMaxSpeed = dune::globals::currentGame->objectData.data[itemID_][static_cast<int>(originalHouseID_)].maxspeed;
}

Ornithopter::~Ornithopter() = default;

void Ornithopter::save(OutputStream& stream) const {
    parent::save(stream);

    stream.writeUint32(timeLastShot);
}

void Ornithopter::checkPos(const GameContext& context) {
    parent::checkPos(context);

    if (!target_) {
        if (destination_.isValid()) {
            if (blockDistance(location_, destination_) <= 2) {
                destination_.invalidate();
            }
        } else {
            if (blockDistance(location_, guardPoint) > 17) {
                setDestination(guardPoint);
            }
        }
    }

    drawnFrame = ((context.game.getGameCycleCount() + getObjectID()) / ORNITHOPTER_FRAMETIME) % numImagesY_;
}

bool Ornithopter::canAttack(const ObjectBase* object) const {
    return (object != nullptr) && !object->isAFlyingUnit()
        && ((object->getOwner()->getTeamID() != owner_->getTeamID()) || object->getItemID() == Unit_Sandworm)
        && object->isVisible(getOwner()->getTeamID());
}

void Ornithopter::destroy(const GameContext& context) {
    // place wreck
    auto& map = context.map;

    if (auto* pTile = map.tryGetTile(location_.x, location_.y))
        pTile->assignDeadUnit(DeadUnit_Ornithopter, owner_->getHouseID(), {realX_.toFloat(), realY_.toFloat()});

    parent::destroy(context);
}

void Ornithopter::playAttackSound() {
    dune::globals::soundPlayer->playSoundAt(Sound_enum::Sound_Rocket, location_);
}

bool Ornithopter::canPassTile(const Tile* pTile) const {
    return pTile && (!pTile->hasAnAirUnit());
}

FixPoint Ornithopter::getDestinationAngle() const {
    FixPoint angle;

    auto fly_away = false;
    if (timeLastShot > 0) {
        const auto now = dune::globals::currentGame->getGameCycleCount();

        if (now >= timeLastShot && static_cast<int>(now - timeLastShot) < MILLI2CYCLES(1000))
            fly_away = true;
    }

    if (fly_away) {
        // we already shot at target and now want to fly in the opposite direction
        angle = destinationAngleRad(destination_.x * TILESIZE + TILESIZE / 2, destination_.y * TILESIZE + TILESIZE / 2,
                                    realX_, realY_);
    } else {
        angle = destinationAngleRad(realX_, realY_, destination_.x * TILESIZE + TILESIZE / 2,
                                    destination_.y * TILESIZE + TILESIZE / 2);
    }

    return angle * (8 / (FixPt_PI << 1));
}

bool Ornithopter::attack(const GameContext& context) {
    const auto bAttacked = parent::attack(context);

    if (bAttacked) {
        timeLastShot = context.game.getGameCycleCount();
    }
    return bAttacked;
}
