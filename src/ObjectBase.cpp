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

#include <ObjectBase.h>

#include <globals.h>

#include <FileClasses/GFXManager.h>
#include <FileClasses/music/MusicPlayer.h>

#include <GUI/ObjectInterfaces/DefaultObjectInterface.h>
#include <Game.h>
#include <House.h>
#include <Map.h>
#include <ScreenBorder.h>
#include <SoundPlayer.h>
#include <players/HumanPlayer.h>

// structures
#include <structures/Barracks.h>
#include <structures/ConstructionYard.h>
#include <structures/GunTurret.h>
#include <structures/HeavyFactory.h>
#include <structures/HighTechFactory.h>
#include <structures/IX.h>
#include <structures/LightFactory.h>
#include <structures/Palace.h>
#include <structures/Radar.h>
#include <structures/Refinery.h>
#include <structures/RepairYard.h>
#include <structures/RocketTurret.h>
#include <structures/Silo.h>
#include <structures/StarPort.h>
#include <structures/WOR.h>
#include <structures/Wall.h>
#include <structures/WindTrap.h>

// units
#include <units/Carryall.h>
#include <units/Devastator.h>
#include <units/Deviator.h>
#include <units/Frigate.h>
#include <units/Harvester.h>
#include <units/Launcher.h>
#include <units/MCV.h>
#include <units/Ornithopter.h>
#include <units/Quad.h>
#include <units/RaiderTrike.h>
#include <units/Saboteur.h>
#include <units/SandWorm.h>
#include <units/SiegeTank.h>
#include <units/Soldier.h>
#include <units/SonicTank.h>
#include <units/Tank.h>
#include <units/Trike.h>
#include <units/Trooper.h>

#include <array>

ObjectBase::ObjectBase(const ObjectBaseConstants& object_constants, uint32_t objectID,
                       const ObjectInitializer& initializer)
    : ObjectBase(object_constants, objectID) {
    originalHouseID_ = initializer.owner()->getHouseID();
    owner_           = initializer.owner();
    byScenario_      = initializer.byScenario();

    health_       = 0;
    badlyDamaged_ = false;

    location_    = Coord::Invalid();
    oldLocation_ = Coord::Invalid();
    destination_ = Coord::Invalid();
    realX_       = 0;
    realY_       = 0;

    drawnAngle_ = static_cast<ANGLETYPE>(0);
    angle_      = static_cast<int>(drawnAngle_);

    active_                = false;
    respondable_           = true;
    byScenario_            = false;
    selected_              = false;
    selectedByOtherPlayer_ = false;

    forced_ = false;
    ObjectBase::setTarget(nullptr);
    targetFriendly_ = false;
    attackMode_     = GUARD;

    setVisible(VIS_ALL, false);
}

ObjectBase::ObjectBase(const ObjectBaseConstants& object_constants, uint32_t objectID,
                       const ObjectStreamInitializer& initializer)
    : ObjectBase(object_constants, objectID) {
    const auto* const game = dune::globals::currentGame.get();

    auto& stream     = initializer.stream();
    originalHouseID_ = static_cast<HOUSETYPE>(stream.readUint32());
    owner_           = game->getHouse(static_cast<HOUSETYPE>(stream.readUint32()));

    health_       = stream.readFixPoint();
    badlyDamaged_ = stream.readBool();

    location_.x    = stream.readSint32();
    location_.y    = stream.readSint32();
    oldLocation_.x = stream.readSint32();
    oldLocation_.y = stream.readSint32();
    destination_.x = stream.readSint32();
    destination_.y = stream.readSint32();
    realX_         = stream.readFixPoint();
    realY_         = stream.readFixPoint();

    angle_      = stream.readFixPoint();
    drawnAngle_ = static_cast<ANGLETYPE>(stream.readSint8());

    active_      = stream.readBool();
    respondable_ = stream.readBool();
    byScenario_  = stream.readBool();

    if (game->getGameInitSettings().getGameType() != GameType::CustomMultiplayer) {
        selected_              = stream.readBool();
        selectedByOtherPlayer_ = stream.readBool();
    } else {
        selected_              = false;
        selectedByOtherPlayer_ = false;
    }

    forced_ = stream.readBool();
    target_.load(stream);
    targetFriendly_ = stream.readBool();
    attackMode_     = static_cast<ATTACKMODE>(stream.readUint32());

    std::array<bool, 7> b{false, false, false, false, false, false, false};

    stream.readBools(&b[0], &b[1], &b[2], &b[3], &b[4], &b[5], &b[6]);

    for (decltype(visible_.size()) i = 0; i < visible_.size(); ++i)
        visible_[i] = b[i];
}

ObjectBase::ObjectBase(const ObjectBaseConstants& object_constants, uint32_t objectID)
    : constants_{object_constants}, itemID_{object_constants.itemID}, objectID_{objectID} {
    graphicID_  = static_cast<ObjPic_enum>(-1);
    numImagesX_ = 0;
    numImagesY_ = 0;
}

ObjectBase::~ObjectBase() = default;

void ObjectBase::destroy(const GameContext& context) {
    context.objectManager.removeObject(getObjectID());
}

void ObjectBase::cleanup([[maybe_unused]] const GameContext& context, [[maybe_unused]] HumanPlayer* humanPlayer) { }

void ObjectBase::save(OutputStream& stream) const {
    stream.writeUint32(static_cast<uint32_t>(originalHouseID_));
    stream.writeUint32(static_cast<uint32_t>(owner_->getHouseID()));

    stream.writeFixPoint(health_);
    stream.writeBool(badlyDamaged_);

    stream.writeSint32(location_.x);
    stream.writeSint32(location_.y);
    stream.writeSint32(oldLocation_.x);
    stream.writeSint32(oldLocation_.y);
    stream.writeSint32(destination_.x);
    stream.writeSint32(destination_.y);
    stream.writeFixPoint(realX_);
    stream.writeFixPoint(realY_);

    stream.writeFixPoint(angle_);
    stream.writeSint8(static_cast<int8_t>(drawnAngle_));

    stream.writeBool(active_);
    stream.writeBool(respondable_);
    stream.writeBool(byScenario_);

    if (dune::globals::currentGame->getGameInitSettings().getGameType() != GameType::CustomMultiplayer) {
        stream.writeBool(selected_);
        stream.writeBool(selectedByOtherPlayer_);
    }

    stream.writeBool(forced_);
    target_.save(stream);
    stream.writeBool(targetFriendly_);
    stream.writeUint32(attackMode_);

    stream.writeBools(visible_[0], visible_[1], visible_[2], visible_[3], visible_[4], visible_[5], visible_[6]);
}

/**
    Returns the center point of this object
    \return the center point in world coordinates
*/
Coord ObjectBase::getCenterPoint() const {
    return {lround(realX_), lround(realY_)};
}

Coord ObjectBase::getClosestCenterPoint([[maybe_unused]] const Coord& objectLocation) const {
    return getCenterPoint();
}

int ObjectBase::getMaxHealth() const {
    return dune::globals::currentGame->objectData.data[itemID_][static_cast<int>(originalHouseID_)].hitpoints;
}

void ObjectBase::handleDamage([[maybe_unused]] const GameContext& context, int damage, uint32_t damagerID,
                              House* damagerOwner) {
    if (damage >= 0) {
        FixPoint newHealth = getHealth();

        newHealth -= damage;

        if (newHealth <= 0) {
            setHealth(0);

            if (damagerOwner != nullptr) {
                damagerOwner->informHasKilled(itemID_);
            }
        } else {
            setHealth(newHealth);
        }
    }

    if (getOwner() == dune::globals::pLocalHouse) {
        dune::globals::musicPlayer->changeMusic(MUSIC_ATTACK);
    }

    getOwner()->noteDamageLocation(this, damage, damagerID);
}

std::unique_ptr<ObjectInterface> ObjectBase::getInterfaceContainer(const GameContext& context) {
    return Widget::create<DefaultObjectInterface>(context, objectID_);
}

void ObjectBase::setDestination(int newX, int newY) {
    if (dune::globals::currentGameMap->tileExists(newX, newY) || (newX == INVALID_POS && newY == INVALID_POS)) {
        destination_.x = newX;
        destination_.y = newY;
    }
}

void ObjectBase::setHealth(FixPoint newHealth) {
    if (newHealth >= 0 && newHealth <= getMaxHealth()) {
        health_       = newHealth;
        badlyDamaged_ = health_ < BADLYDAMAGEDRATIO * getMaxHealth();
    }
}

void ObjectBase::setLocation(const GameContext& context, int xPos, int yPos) {
    if (xPos == INVALID_POS && yPos == INVALID_POS) {
        location_.invalidate();
    } else if (context.map.tileExists(xPos, yPos)) {
        location_.x = xPos;
        location_.y = yPos;
        realX_      = location_.x * TILESIZE;
        realY_      = location_.y * TILESIZE;

        assignToMap(context, location_);
    }
}

void ObjectBase::setVisible(int teamID, bool status) {
    if (teamID == VIS_ALL) {
        if (status) {
            visible_.set();
        } else {
            visible_.reset();
        }
    } else if (teamID >= 0 && teamID < NUM_TEAMS) {
        visible_[teamID] = status;
    }
}

void ObjectBase::setTarget(const ObjectBase* newTarget) {
    target_.pointTo(newTarget);

    auto friendly = false;

    if (target_) {
        if (const auto* targetPtr = target_.getObjPointer()) {
            friendly = targetPtr->getOwner()->getTeamID() == owner_->getTeamID() && getItemID() != Unit_Sandworm
                    && targetPtr->getItemID() != Unit_Sandworm;
        }
    }

    targetFriendly_ = friendly;
}

void ObjectBase::unassignFromMap(const Coord& location) const {
    auto* const map = dune::globals::currentGameMap;

    if (map->tileExists(location)) {
        map->getTile(location)->unassignObject(getObjectID());
    }
}

bool ObjectBase::canAttack(const ObjectBase* object) const {
    return canAttack() && object != nullptr && (object->isAStructure() || !object->isAFlyingUnit())
        && ((object->getOwner()->getTeamID() != owner_->getTeamID() && object->isVisible(getOwner()->getTeamID()))
            || object->getItemID() == Unit_Sandworm);
}

bool ObjectBase::isOnScreen() const {
    const Coord position{lround(getRealX()), lround(getRealY())};

    auto* const texture = graphic_[dune::globals::currentZoomlevel];

    const Coord size{static_cast<int>(std::ceil(getWidth(texture) / numImagesX_)),
                     static_cast<int>(std::ceil(getHeight(texture) / numImagesY_))};

    return dune::globals::screenborder->isInsideScreen(position, size);
}

bool ObjectBase::isVisible(int teamID) const {
    if (visible_.all())
        return true;

    if (teamID >= 0 && teamID < NUM_TEAMS) {
        return visible_[teamID];
    }
    return false;
}

bool ObjectBase::isVisible() const {
    return visible_.any();
}

uint32_t ObjectBase::getHealthColor() const {
    const FixPoint healthPercent = health_ / getMaxHealth();

    if (healthPercent >= BADLYDAMAGEDRATIO) {
        return COLOR_LIGHTGREEN;
    }
    if (healthPercent >= HEAVILYDAMAGEDRATIO) {

        return COLOR_YELLOW;
    }
    return COLOR_RED;
}

Coord ObjectBase::getClosestPoint([[maybe_unused]] const Coord& point) const {
    return location_;
}

const StructureBase* ObjectBase::findClosestTargetStructure() const {
    const StructureBase* pClosestStructure = nullptr;
    auto closestDistance                   = FixPt_MAX;
    for (const StructureBase* pStructure : dune::globals::structureList) {
        if (canAttack(pStructure)) {
            const auto closestPoint = pStructure->getClosestPoint(getLocation());
            auto structureDistance  = blockDistance(getLocation(), closestPoint);

            if (pStructure->getItemID() == Structure_Wall) {
                structureDistance += 20000000; // so that walls are targeted very last
            }

            if (structureDistance < closestDistance) {
                closestDistance   = structureDistance;
                pClosestStructure = pStructure;
            }
        }
    }

    return pClosestStructure;
}

const UnitBase* ObjectBase::findClosestTargetUnit() const {
    const UnitBase* pClosestUnit = nullptr;
    auto closestDistance         = FixPt_MAX;
    for (const UnitBase* pUnit : dune::globals::unitList) {
        if (canAttack(pUnit)) {
            const auto closestPoint = pUnit->getClosestPoint(getLocation());
            const auto unitDistance = blockDistance(getLocation(), closestPoint);

            if (unitDistance < closestDistance) {
                closestDistance = unitDistance;
                pClosestUnit    = pUnit;
            }
        }
    }

    return pClosestUnit;
}

const ObjectBase* ObjectBase::findClosestTarget() const {
    const ObjectBase* pClosestObject = nullptr;
    FixPoint closestDistance         = FixPt_MAX;
    for (const StructureBase* pStructure : dune::globals::structureList) {
        if (canAttack(pStructure)) {
            const auto closestPoint = pStructure->getClosestPoint(getLocation());
            auto structureDistance  = blockDistance(getLocation(), closestPoint);

            if (pStructure->getItemID() == Structure_Wall) {
                structureDistance += 20000000; // so that walls are targeted very last
            }

            if (structureDistance < closestDistance) {
                closestDistance = structureDistance;
                pClosestObject  = pStructure;
            }
        }
    }

    for (const UnitBase* pUnit : dune::globals::unitList) {
        if (canAttack(pUnit)) {
            const auto closestPoint = pUnit->getClosestPoint(getLocation());
            const auto unitDistance = blockDistance(getLocation(), closestPoint);

            if (unitDistance < closestDistance) {
                closestDistance = unitDistance;
                pClosestObject  = pUnit;
            }
        }
    }

    return pClosestObject;
}

const ObjectBase* ObjectBase::findTarget() const {
    // searches for a target in an area like as shown below
    //
    //                     *
    //                   *****
    //                   *****
    //                  ***T***
    //                   *****
    //                   *****
    //                     *

    auto checkRange = 0;
    switch (attackMode_) {
        case GUARD: {
            checkRange = getWeaponRange();
        } break;

        case AREAGUARD: {
            checkRange = getAreaGuardRange();
        } break;

        case AMBUSH: {
            checkRange = getViewRange();
        } break;

        case HUNT: {
            // check whole map
            return findClosestTarget();
        }

        case STOP:
        default: {
            return nullptr;
        }
    }

    if (getItemID() == Unit_Sandworm) {
        checkRange = getViewRange();
    }

    const ObjectBase* pClosestTarget = nullptr;
    auto closestTargetDistance       = FixPt_MAX;

    auto* const map = dune::globals::currentGameMap;

    Coord coord;
    const auto startY = std::max(0, location_.y - checkRange);
    const auto endY   = std::min(map->getSizeY() - 1, location_.y + checkRange);
    for (coord.y = startY; coord.y <= endY; coord.y++) {
        const auto startX = std::max(0, location_.x - checkRange);
        const auto endX   = std::min(map->getSizeX() - 1, location_.x + checkRange);
        for (coord.x = startX; coord.x <= endX; coord.x++) {

            const auto targetDistance = blockDistance(location_, coord);
            if (targetDistance <= checkRange) {
                auto* const game = dune::globals::currentGame.get();

                const Tile* pTile = map->getTile(coord);

                if (pTile->isExploredByTeam(game, getOwner()->getTeamID())
                    && !pTile->isFoggedByTeam(game, getOwner()->getTeamID()) && pTile->hasAnObject()) {

                    const auto* const pNewTarget = pTile->getObject(game->getObjectManager());
                    if (!pNewTarget)
                        continue;

                    if (((pNewTarget->getItemID() != Structure_Wall && pNewTarget->getItemID() != Unit_Carryall)
                         || pClosestTarget == nullptr)
                        && canAttack(pNewTarget)) {
                        if (targetDistance < closestTargetDistance) {
                            pClosestTarget        = pNewTarget;
                            closestTargetDistance = targetDistance;
                        }
                    }
                }
            }
        }
    }

    return pClosestTarget;
}

int ObjectBase::getViewRange() const {
    return dune::globals::currentGame->objectData.data[itemID_][static_cast<int>(originalHouseID_)].viewrange;
}

int ObjectBase::getAreaGuardRange() const {
    return 2 * getWeaponRange();
}

int ObjectBase::getWeaponRange() const {
    return dune::globals::currentGame->objectData.data[itemID_][static_cast<int>(originalHouseID_)].weaponrange;
}

int ObjectBase::getWeaponReloadTime() const {
    return dune::globals::currentGame->objectData.data[itemID_][static_cast<int>(originalHouseID_)].weaponreloadtime;
}

int ObjectBase::getInfSpawnProp() const {
    return dune::globals::currentGame->objectData.data[itemID_][static_cast<int>(originalHouseID_)].infspawnprop;
}

namespace {
template<typename ObjectType, typename... Args>
std::unique_ptr<ObjectBase> makeObject(Args&&... args) {
    static_assert(std::is_constructible_v<ObjectType, Args...>, "ObjectType is not constructible");
    static_assert(std::is_base_of_v<ObjectBase, ObjectType>, "ObjectType not derived from ObjectBase");
    static_assert(std::is_base_of_v<ObjectBase, typename ObjectType::parent>,
                  "ObjectType's parent is not derived from ObjectBase");
    static_assert(std::is_base_of_v<typename ObjectType::parent, ObjectType>,
                  "ObjectType's parent is not a base class");

    return std::make_unique<ObjectType>(std::forward<Args>(args)...);
}

template<typename... Args>
auto objectFactory(ItemID_enum itemID, Args&&... args) {
    // clang-format off
    switch(itemID) {
        case Barracks::item_id:            return makeObject<Barracks>(args...);
        case ConstructionYard::item_id:    return makeObject<ConstructionYard>(args...);
        case GunTurret::item_id:           return makeObject<GunTurret>(args...);
        case HeavyFactory::item_id:        return makeObject<HeavyFactory>(args...);
        case HighTechFactory::item_id:     return makeObject<HighTechFactory>(args...);
        case IX::item_id:                  return makeObject<IX>(args...);
        case LightFactory::item_id:        return makeObject<LightFactory>(args...);
        case Palace::item_id:              return makeObject<Palace>(args...);
        case Radar::item_id:               return makeObject<Radar>(args...);
        case Refinery::item_id:            return makeObject<Refinery>(args...);
        case RepairYard::item_id:          return makeObject<RepairYard>(args...);
        case RocketTurret::item_id:        return makeObject<RocketTurret>(args...);
        case Silo::item_id:                return makeObject<Silo>(args...);
        case StarPort::item_id:            return makeObject<StarPort>(args...);
        case Wall::item_id:                return makeObject<Wall>(args...);
        case WindTrap::item_id:            return makeObject<WindTrap>(args...);
        case WOR::item_id:                 return makeObject<WOR>(args...);

        case Carryall::item_id:            return makeObject<Carryall>(args...);
        case Devastator::item_id:          return makeObject<Devastator>(args...);
        case Deviator::item_id:            return makeObject<Deviator>(args...);
        case Frigate::item_id:             return makeObject<Frigate>(args...);
        case Harvester::item_id:           return makeObject<Harvester>(args...);
        case Soldier::item_id:             return makeObject<Soldier>(args...);
        case Launcher::item_id:            return makeObject<Launcher>(args...);
        case MCV::item_id:                 return makeObject<MCV>(args...);
        case Ornithopter::item_id:         return makeObject<Ornithopter>(args...);
        case Quad::item_id:                return makeObject<Quad>(args...);
        case Saboteur::item_id:            return makeObject<Saboteur>(args...);
        case Sandworm::item_id:            return makeObject<Sandworm>(args...);
        case SiegeTank::item_id:           return makeObject<SiegeTank>(args...);
        case SonicTank::item_id:           return makeObject<SonicTank>(args...);
        case Tank::item_id:                return makeObject<Tank>(args...);
        case Trike::item_id:               return makeObject<Trike>(args...);
        case RaiderTrike::item_id:         return makeObject<RaiderTrike>(args...);
        case Trooper::item_id:             return makeObject<Trooper>(args...);

        default:                            sdl2::log_info("ObjectBase::makeObject(): %d is no valid ItemID!",itemID);
                                            return std::unique_ptr<ObjectBase>{};
    }
    // clang-format on
}

} // anonymous namespace

std::unique_ptr<ObjectBase>
ObjectBase::createObject(ItemID_enum itemID, uint32_t objectID, const ObjectInitializer& initializer) {
    return objectFactory(itemID, objectID, initializer);
}

std::unique_ptr<ObjectBase>
ObjectBase::loadObject(ItemID_enum itemID, uint32_t objectID, const ObjectStreamInitializer& initializer) {
    return objectFactory(itemID, objectID, initializer);
}

bool ObjectBase::targetInWeaponRange() const {
    const auto coord = target_.getObjPointer()->getClosestPoint(location_);
    const auto dist  = blockDistance(location_, coord);

    return dist <= dune::globals::currentGame->objectData.data[itemID_][static_cast<int>(originalHouseID_)].weaponrange;
}
