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
    assert(itemID == Unit_Ornithopter);
    owner->incrementUnits(itemID);

    graphicID     = ObjPic_Ornithopter;
    graphic       = pGFXManager->getObjPic(graphicID, getOwner()->getHouseID());
    shadowGraphic = pGFXManager->getObjPic(ObjPic_OrnithopterShadow, getOwner()->getHouseID());

    numImagesX = static_cast<int>(ANGLETYPE::NUM_ANGLES);
    numImagesY = 3;

    currentMaxSpeed = currentGame->objectData.data[itemID][static_cast<int>(originalHouseID)].maxspeed;
}

Ornithopter::~Ornithopter() = default;

void Ornithopter::save(OutputStream& stream) const {
    parent::save(stream);

    stream.writeUint32(timeLastShot);
}

void Ornithopter::checkPos(const GameContext& context) {
    parent::checkPos(context);

    if (!target) {
        if (destination.isValid()) {
            if (blockDistance(location, destination) <= 2) {
                destination.invalidate();
            }
        } else {
            if (blockDistance(location, guardPoint) > 17) {
                setDestination(guardPoint);
            }
        }
    }

    drawnFrame = ((context.game.getGameCycleCount() + getObjectID()) / ORNITHOPTER_FRAMETIME) % numImagesY;
}

bool Ornithopter::canAttack(const ObjectBase* object) const {
    return (object != nullptr) && !object->isAFlyingUnit()
        && ((object->getOwner()->getTeamID() != owner->getTeamID()) || object->getItemID() == Unit_Sandworm)
        && object->isVisible(getOwner()->getTeamID());
}

void Ornithopter::destroy(const GameContext& context) {
    // place wreck
    if (currentGameMap->tileExists(location)) {
        auto* pTile = currentGameMap->getTile(location);
        pTile->assignDeadUnit(DeadUnit_Ornithopter, owner->getHouseID(), {realX.toFloat(), realY.toFloat()});
    }

    parent::destroy(context);
}

void Ornithopter::playAttackSound() {
    soundPlayer->playSoundAt(Sound_enum::Sound_Rocket, location);
}

bool Ornithopter::canPassTile(const Tile* pTile) const {
    return pTile && (!pTile->hasAnAirUnit());
}

FixPoint Ornithopter::getDestinationAngle() const {
    FixPoint angle;

    if (timeLastShot > 0 && (currentGame->getGameCycleCount() - timeLastShot) < MILLI2CYCLES(1000)) {
        // we already shot at target and now want to fly in the opposite direction
        angle = destinationAngleRad(destination.x * TILESIZE + TILESIZE / 2, destination.y * TILESIZE + TILESIZE / 2,
                                    realX, realY);
    } else {
        angle = destinationAngleRad(realX, realY, destination.x * TILESIZE + TILESIZE / 2,
                                    destination.y * TILESIZE + TILESIZE / 2);
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
