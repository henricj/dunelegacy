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

#include <units/UnitBase.h>

#include <globals.h>

#include <FileClasses/GFXManager.h>

#include <Bullet.h>
#include <House.h>
#include <Map.h>
#include <ScreenBorder.h>
#include <SoundPlayer.h>

#include <players/HumanPlayer.h>

#include <misc/draw_util.h>

#include <GUI/ObjectInterfaces/UnitInterface.h>

#include <misc/reverse.h>
#include <structures/Refinery.h>
#include <structures/RepairYard.h>
#include <units/Harvester.h>

inline constexpr auto SMOKEDELAY    = 30;
inline constexpr auto UNITIDLETIMER = GAMESPEED_DEFAULT * 315; // about every 5s

UnitBase::UnitBase(const UnitBaseConstants& constants, uint32_t objectID, const ObjectInitializer& initializer)
    : ObjectBase(constants, objectID, initializer), targetAngle(ANGLETYPE::INVALID_ANGLE), nextSpotAngle(drawnAngle_) {

    UnitBase::init();

    drawnAngle_ = static_cast<ANGLETYPE>(initializer.game().randomGen.rand(0, 7));
    angle_      = static_cast<int>(drawnAngle_);

    guardPoint = Coord::Invalid();
    attackPos  = Coord::Invalid();

    nextSpot = Coord::Invalid();
}

UnitBase::UnitBase(const UnitBaseConstants& constants, uint32_t objectID, const ObjectStreamInitializer& initializer)
    : ObjectBase(constants, objectID, initializer) {

    UnitBase::init();

    auto& stream = initializer.stream();

    stream.readBools(&goingToRepairYard, &pickedUp, &bFollow);
    guardPoint.x = stream.readSint32();
    guardPoint.y = stream.readSint32();
    attackPos.x  = stream.readSint32();
    attackPos.y  = stream.readSint32();

    stream.readBools(&moving, &turning, &justStoppedMoving);
    xSpeed       = stream.readFixPoint();
    ySpeed       = stream.readFixPoint();
    bumpyOffsetX = stream.readFixPoint();
    bumpyOffsetY = stream.readFixPoint();

    targetDistance = stream.readFixPoint();
    targetAngle    = static_cast<ANGLETYPE>(stream.readSint8());

    noCloserPointCount      = stream.readUint8();
    nextSpotFound           = stream.readBool();
    nextSpotAngle           = static_cast<ANGLETYPE>(stream.readSint8());
    recalculatePathTimer    = stream.readSint32();
    nextSpot.x              = stream.readSint32();
    nextSpot.y              = stream.readSint32();
    const auto numPathNodes = stream.readUint32();
    pathList.resize(numPathNodes);
    for (auto i = 0u; i < numPathNodes; ++i) {
        auto x                         = stream.readSint32();
        auto y                         = stream.readSint32();
        pathList[numPathNodes - 1 - i] = {x, y};
    }

    findTargetTimer      = stream.readSint32();
    primaryWeaponTimer   = stream.readSint32();
    secondaryWeaponTimer = stream.readSint32();

    deviationTimer = stream.readSint32();
}

void UnitBase::init() {
    drawnFrame = 0;

    dune::globals::unitList.push_back(this);
}

UnitBase::~UnitBase() = default;

void UnitBase::cleanup(const GameContext& context, HumanPlayer* humanPlayer) {
    try {
        const auto& [game, map, objectManager] = context;

        map.removeObjectFromMap(getObjectID()); // no map point will reference now

        game.getHouse(originalHouseID_)->decrementUnits(itemID_);

        dune::globals::unitList.remove(this);
    } catch (std::exception& e) {
        sdl2::log_info("UnitBase::cleanup(): %s", e.what());
    }

    parent::cleanup(context, humanPlayer);
}

void UnitBase::save(OutputStream& stream) const {

    ObjectBase::save(stream);

    stream.writeBools(goingToRepairYard, pickedUp, bFollow);
    stream.writeSint32(guardPoint.x);
    stream.writeSint32(guardPoint.y);
    stream.writeSint32(attackPos.x);
    stream.writeSint32(attackPos.y);

    stream.writeBools(moving, turning, justStoppedMoving);
    stream.writeFixPoint(xSpeed);
    stream.writeFixPoint(ySpeed);
    stream.writeFixPoint(bumpyOffsetX);
    stream.writeFixPoint(bumpyOffsetY);

    stream.writeFixPoint(targetDistance);
    stream.writeSint8(static_cast<int8_t>(targetAngle));

    stream.writeUint8(noCloserPointCount);
    stream.writeBool(nextSpotFound);
    stream.writeSint8(static_cast<int8_t>(nextSpotAngle));
    stream.writeSint32(recalculatePathTimer);
    stream.writeSint32(nextSpot.x);
    stream.writeSint32(nextSpot.y);
    stream.writeUint32(pathList.size());
    for (const auto& coord : dune::reverse(pathList)) {
        stream.writeSint32(coord.x);
        stream.writeSint32(coord.y);
    }

    stream.writeSint32(findTargetTimer);
    stream.writeSint32(primaryWeaponTimer);
    stream.writeSint32(secondaryWeaponTimer);

    stream.writeSint32(deviationTimer);
}

bool UnitBase::attack(const GameContext& context) {

    if (!numWeapons())
        return false;

    if (primaryWeaponTimer != 0 && (numWeapons() != 2 || secondaryWeaponTimer != 0 || isBadlyDamaged()))
        return false;

    Coord targetCenterPoint;
    Coord centerPoint = getCenterPoint();
    bool bAirBullet   = false;

    auto* pObject = target_.getObjPointer();
    if (pObject != nullptr) {
        targetCenterPoint = pObject->getClosestCenterPoint(location_);
        bAirBullet        = pObject->isAFlyingUnit();
    } else {
        targetCenterPoint = context.map.getTile(attackPos)->getCenterPoint();
        bAirBullet        = false;
    }

    int currentBulletType = bulletType();
    int32_t currentWeaponDamage =
        context.game.objectData.data[itemID_][static_cast<int>(originalHouseID_)].weapondamage;

    if (getItemID() == Unit_Trooper && !bAirBullet) {
        // Troopers change weapon type depending on distance

        const FixPoint distance = distanceFrom(centerPoint, targetCenterPoint);
        if (distance <= 2 * TILESIZE) {
            currentBulletType = Bullet_ShellSmall;
            currentWeaponDamage -= currentWeaponDamage / 4;
        }
    } else if (getItemID() == Unit_Launcher && bAirBullet) {
        // Launchers change weapon type when targeting flying units
        currentBulletType = Bullet_TurretRocket;
    }

    if (primaryWeaponTimer == 0) {
        dune::globals::bulletList.push_back(std::make_unique<Bullet>(
            objectID_, &centerPoint, &targetCenterPoint, currentBulletType, currentWeaponDamage, bAirBullet, pObject));
        if (pObject != nullptr) {
            context.map.viewMap(pObject->getOwner()->getHouseID(), location_, 2);
        }
        playAttackSound();
        primaryWeaponTimer = getWeaponReloadTime();

        secondaryWeaponTimer = 15;

        if (attackPos && getItemID() != Unit_SonicTank && context.map.getTile(attackPos)->isSpiceBloom()) {
            setDestination(location_);
            forced_ = false;
            attackPos.invalidate();
        }

        // shorten deviation time
        if (deviationTimer > 0) {
            deviationTimer = std::max(0, deviationTimer - MILLI2CYCLES(20 * 1000));
        }
    }

    if (numWeapons() == 2 && secondaryWeaponTimer == 0 && !isBadlyDamaged()) {
        dune::globals::bulletList.push_back(std::make_unique<Bullet>(
            objectID_, &centerPoint, &targetCenterPoint, currentBulletType, currentWeaponDamage, bAirBullet, pObject));
        if (pObject != nullptr) {
            context.map.viewMap(pObject->getOwner()->getHouseID(), location_, 2);
        }
        playAttackSound();
        secondaryWeaponTimer = -1;

        if (attackPos && getItemID() != Unit_SonicTank && context.map.getTile(attackPos)->isSpiceBloom()) {
            setDestination(location_);
            forced_ = false;
            attackPos.invalidate();
        }

        // shorten deviation time
        if (deviationTimer > 0) {
            deviationTimer = std::max(0, deviationTimer - MILLI2CYCLES(20 * 1000));
        }
    }

    return true;
}

void UnitBase::blitToScreen() {
    const auto* const screenborder = dune::globals::screenborder.get();

    const auto x = screenborder->world2screenX(realX_);
    const auto y = screenborder->world2screenY(realY_);

    const auto* const pUnitGraphic = graphic_[dune::globals::currentZoomlevel];
    const auto source =
        calcSpriteSourceRect(pUnitGraphic, static_cast<int>(drawnAngle_), numImagesX_, drawnFrame, numImagesY_);
    const auto dest =
        calcSpriteDrawingRect(pUnitGraphic, x, y, numImagesX_, numImagesY_, HAlign::Center, VAlign::Center);

    Dune_RenderCopyF(dune::globals::renderer.get(), pUnitGraphic, &source, &dest);

    if (isBadlyDamaged()) {
        drawSmoke(x, y);
    }
}

std::unique_ptr<ObjectInterface> UnitBase::getInterfaceContainer(const GameContext& context) {
    if ((dune::globals::pLocalHouse == owner_ && isRespondable()) || dune::globals::debug) {
        return Widget::create<UnitInterface>(context, objectID_);
    }
    return Widget::create<DefaultObjectInterface>(context, objectID_);
}

ANGLETYPE UnitBase::getCurrentAttackAngle() const {
    return drawnAngle_;
}

void UnitBase::deploy(const GameContext& context, const Coord& newLocation) {
    const auto& [game, map, objectManager] = context;

    if (map.tileExists(newLocation)) {
        setLocation(context, newLocation);

        if (guardPoint.isInvalid())
            guardPoint = location_;
        setDestination(guardPoint);
        pickedUp = false;
        setRespondable(true);
        setActive(true);
        setVisible(VIS_ALL, true);
        setForced(false);

        // Deployment logic to hopefully stop units freezing
        if (getAttackMode() == CARRYALLREQUESTED || getAttackMode() == HUNT) {
            if (getItemID() == Unit_Harvester) {
                doSetAttackMode(context, HARVEST);
            } else {
                doSetAttackMode(context, GUARD);
            }
        }

        if (isAGroundUnit() && getItemID() != Unit_Sandworm) {
            if (map.getTile(location_)->isSpiceBloom()) {
                setHealth(0);
                setVisible(VIS_ALL, false);
                map.getTile(location_)->triggerSpiceBloom(context, getOwner());
            } else if (map.getTile(location_)->isSpecialBloom()) {
                map.getTile(location_)->triggerSpecialBloom(context, getOwner());
            }
        }

        if (dune::globals::pLocalHouse == getOwner()) {
            dune::globals::pLocalPlayer->onUnitDeployed(this);
        }
    }
}

void UnitBase::destroy(const GameContext& context) {

    setTarget(nullptr);

    const auto& [game, map, objectManager] = context;

    objectManager.removeObject(getObjectID());

    if (isVisible()) {
        if (game.randomGen.rand(1, 100) <= getInfSpawnProp()) {
            auto* pNewUnit = game.getHouse(originalHouseID_)->createUnit(Unit_Soldier);
            pNewUnit->setHealth(pNewUnit->getMaxHealth() / 2);
            pNewUnit->deploy(context, location_);

            if (owner_->getHouseID() != originalHouseID_) {
                // deviation is inherited
                pNewUnit->owner_   = owner_;
                pNewUnit->graphic_ = dune::globals::pGFXManager->getObjPic(pNewUnit->graphicID_, owner_->getHouseID());
                pNewUnit->deviationTimer = deviationTimer;
            }
        }
    }
}

void UnitBase::deviate(const GameContext& context, House* newOwner) {
    const auto& [game, map, objectManager] = context;

    if (newOwner->getHouseID() == originalHouseID_) {
        quitDeviation(context);
    } else {
        map.removeSelection(objectID_);
        setTarget(nullptr);
        setGuardPoint(location_);
        setDestination(location_);
        clearPath();
        doSetAttackMode(context, GUARD);
        owner_ = newOwner;

        graphic_ = dune::globals::pGFXManager->getObjPic(graphicID_, getOwner()->getHouseID());

        deviationTimer = DEVIATIONTIME;
    }

    // Adding this in as a surrogate for damage inflicted upon deviation.. Still not sure what the best value
    // should be... going in with a 25% of the units value unless its a devastator which we can destruct or an
    // ornithopter which is likely to get killed
    if (getItemID() == Unit_Devastator || getItemID() == Unit_Ornithopter) {
        newOwner->informHasDamaged(Unit_Deviator,
                                   game.objectData.data[getItemID()][static_cast<int>(newOwner->getHouseID())].price);
    } else {
        newOwner->informHasDamaged(
            Unit_Deviator, game.objectData.data[getItemID()][static_cast<int>(newOwner->getHouseID())].price / 5);
    }
}

void UnitBase::drawSelectionBox() {
    const auto* const screenborder = dune::globals::screenborder.get();
    auto* const renderer           = dune::globals::renderer.get();
    const auto zoom                = dune::globals::currentZoomlevel;
    const auto* const gfx          = dune::globals::pGFXManager.get();

    const DuneTexture* selectionBox = nullptr;

    switch (zoom) {
        case 0: selectionBox = gfx->getUIGraphic(UI_SelectionBox_Zoomlevel0); break;
        case 1: selectionBox = gfx->getUIGraphic(UI_SelectionBox_Zoomlevel1); break;
        case 2:
        default: selectionBox = gfx->getUIGraphic(UI_SelectionBox_Zoomlevel2); break;
    }

    const auto screenX = screenborder->world2screenX(realX_);
    const auto screenY = screenborder->world2screenY(realY_);

    const auto dest = calcDrawingRect(selectionBox, screenX, screenY, HAlign::Center, VAlign::Center);
    Dune_RenderCopyF(renderer, selectionBox, nullptr, &dest);

    const auto width = getWidth(selectionBox);
    const auto x     = screenX - width / 2;
    const auto y     = screenY - getHeight(selectionBox) / 2;

    const auto healthWidth = (getHealth() / getMaxHealth()).toFloat() * (width - 3);
    renderFillRectF(renderer, x + 1, y - zoom - 1, x + 1 + healthWidth, y - 1, getHealthColor());
}

void UnitBase::drawOtherPlayerSelectionBox() {
    const auto* const screenborder = dune::globals::screenborder.get();
    auto* const renderer           = dune::globals::renderer.get();
    const auto zoom                = dune::globals::currentZoomlevel;

    const DuneTexture* selectionBox = nullptr;

    const auto* const gfx = dune::globals::pGFXManager.get();
    switch (zoom) {
        case 0: selectionBox = gfx->getUIGraphic(UI_OtherPlayerSelectionBox_Zoomlevel0); break;
        case 1: selectionBox = gfx->getUIGraphic(UI_OtherPlayerSelectionBox_Zoomlevel1); break;
        case 2:
        default: selectionBox = gfx->getUIGraphic(UI_OtherPlayerSelectionBox_Zoomlevel2); break;
    }

    const auto dest = calcDrawingRect(selectionBox, screenborder->world2screenX(realX_),
                                      screenborder->world2screenY(realY_), HAlign::Center, VAlign::Center);
    Dune_RenderCopyF(renderer, selectionBox, nullptr, &dest);
}

void UnitBase::releaseTarget() {
    if (forced_) {
        guardPoint = location_;
    }
    setDestination(guardPoint);

    findTargetTimer = 0;
    setForced(false);
    setTarget(nullptr);
}

void UnitBase::engageTarget(const GameContext& context) {

    if (target_ && target_.getObjPointer() == nullptr) {
        // the target does not exist anymore
        releaseTarget();
        return;
    }

    if (target_ && !target_.getObjPointer()->isActive()) {
        // the target changed its state to inactive
        releaseTarget();
        return;
    }

    if (target_ && !targetFriendly_ && !canAttack(target_.getObjPointer())) {
        // the (non-friendly) target cannot be attacked anymore
        releaseTarget();
        return;
    }

    if (target_ && !targetFriendly_ && !forced_ && !isInAttackRange(target_.getObjPointer())) {
        // the (non-friendly) target left the attack mode range (and we were not forced to attack it)
        releaseTarget();
        return;
    }

    if (target_) {
        // we have a target unit or structure

        const Coord targetLocation = target_.getObjPointer()->getClosestPoint(location_);

        if (destination_ != targetLocation) {
            // the location of the target has moved
            // => recalculate path
            clearPath();
        }

        targetDistance = blockDistance(location_, targetLocation);

        const auto newTargetAngle = destinationDrawnAngle(location_, targetLocation);

        if (bFollow) {
            // we are following someone
            setDestination(targetLocation);
            return;
        }

        if (targetDistance > getWeaponRange()) {
            // we are not in attack range
            if (target_.getObjPointer()->isAFlyingUnit()) {
                // we are not following this air unit
                releaseTarget();
                return;
            } // follow the target

            setDestination(targetLocation);

            return;
        }

        // we are in attack range

        if (targetFriendly_ && !forced_) {
            // the target is friendly and we only attack these if were forced to do so
            return;
        }

        if (goingToRepairYard) {
            // we are going to the repair yard
            // => we do not need to change the destination
            targetAngle = ANGLETYPE::INVALID_ANGLE;
        } else if (attackMode_ == CAPTURE) {
            // we want to capture the target building
            setDestination(targetLocation);
            targetAngle = ANGLETYPE::INVALID_ANGLE;
        } else if (isTracked() && target_.getObjPointer()->isInfantry() && !targetFriendly_
                   && context.map.tileExists(targetLocation) && !context.map.getTile(targetLocation)->isMountain()
                   && forced_) {
            // we squash the infantry unit because we are forced to
            setDestination(targetLocation);
            targetAngle = ANGLETYPE::INVALID_ANGLE;
        } else if (!isAFlyingUnit()) {
            // we decide to fire on the target thus we can stop moving
            setDestination(location_);
            targetAngle = newTargetAngle;
        }

        if (getCurrentAttackAngle() == newTargetAngle) {
            attack(context);
        }

    } else if (attackPos) {
        // we attack a position

        targetDistance = blockDistance(location_, attackPos);

        const auto newTargetAngle = destinationDrawnAngle(location_, attackPos);

        if (targetDistance <= getWeaponRange()) {
            if (!isAFlyingUnit()) {
                // we are in weapon range thus we can stop moving
                setDestination(location_);
                targetAngle = newTargetAngle;
            }

            if (getCurrentAttackAngle() == newTargetAngle) {
                attack(context);
            }
        } else {
            targetAngle = ANGLETYPE::INVALID_ANGLE;
        }
    }
}

void UnitBase::move(const GameContext& context) {

    if (moving && !justStoppedMoving) {
        if (!isBadlyDamaged() || isAFlyingUnit()) {
            realX_ += xSpeed;
            realY_ += ySpeed;
        } else {
            realX_ += xSpeed / 2;
            realY_ += ySpeed / 2;
        }

        // check if vehicle is on the first half of the way
        FixPoint fromDistanceX;
        FixPoint fromDistanceY;
        FixPoint toDistanceX;
        FixPoint toDistanceY;
        if (location_ != nextSpot) {
            // check if vehicle is half way out of old tile

            fromDistanceX = FixPoint::abs(location_.x * TILESIZE - (realX_ - bumpyOffsetX) + TILESIZE / 2);
            fromDistanceY = FixPoint::abs(location_.y * TILESIZE - (realY_ - bumpyOffsetY) + TILESIZE / 2);
            toDistanceX   = FixPoint::abs(nextSpot.x * TILESIZE - (realX_ - bumpyOffsetX) + TILESIZE / 2);
            toDistanceY   = FixPoint::abs(nextSpot.y * TILESIZE - (realY_ - bumpyOffsetY) + TILESIZE / 2);

            if (fromDistanceX >= TILESIZE / 2 || fromDistanceY >= TILESIZE / 2) {
                // let something else go in
                unassignFromMap(location_);
                oldLocation_ = location_;
                location_    = nextSpot;

                if (!isAFlyingUnit() && itemID_ != Unit_Sandworm) {
                    context.map.viewMap(owner_->getHouseID(), location_, getViewRange());
                }
            }

        } else {
            // if vehicle is out of old tile

            fromDistanceX = FixPoint::abs(oldLocation_.x * TILESIZE - (realX_ - bumpyOffsetX) + TILESIZE / 2);
            fromDistanceY = FixPoint::abs(oldLocation_.y * TILESIZE - (realY_ - bumpyOffsetY) + TILESIZE / 2);
            toDistanceX   = FixPoint::abs(location_.x * TILESIZE - (realX_ - bumpyOffsetX) + TILESIZE / 2);
            toDistanceY   = FixPoint::abs(location_.y * TILESIZE - (realY_ - bumpyOffsetY) + TILESIZE / 2);

            if (fromDistanceX >= TILESIZE || fromDistanceY >= TILESIZE) {

                if (forced_ && location_ == destination_ && !target_) {
                    setForced(false);
                    if (getAttackMode() == CARRYALLREQUESTED) {
                        doSetAttackMode(context, GUARD);
                    }
                }

                moving            = false;
                justStoppedMoving = true;
                realX_            = location_.x * TILESIZE + TILESIZE / 2;
                realY_            = location_.y * TILESIZE + TILESIZE / 2;
                bumpyOffsetX      = 0;
                bumpyOffsetY      = 0;

                oldLocation_.invalidate();
            }
        }

        bumpyMovementOnRock(fromDistanceX, fromDistanceY, toDistanceX, toDistanceY);

    } else {
        justStoppedMoving = false;
    }

    checkPos(context);
}

void UnitBase::bumpyMovementOnRock(FixPoint fromDistanceX, FixPoint fromDistanceY, FixPoint toDistanceX,
                                   FixPoint toDistanceY) {

    auto* const map = dune::globals::currentGameMap;

    if (hasBumpyMovementOnRock()
        && (map->getTile(location_)->getType() == TERRAINTYPE::Terrain_Rock
            || map->getTile(location_)->getType() == TERRAINTYPE::Terrain_Mountain
            || map->getTile(location_)->getType() == TERRAINTYPE::Terrain_ThickSpice)) {
        // bumping effect

        const FixPoint epsilon     = 0.005_fix;
        const FixPoint bumpyOffset = 2.5_fix;
        const FixPoint absXSpeed   = FixPoint::abs(xSpeed);
        const FixPoint absYSpeed   = FixPoint::abs(ySpeed);

        if (FixPoint::abs(xSpeed) >= epsilon && FixPoint::abs(fromDistanceX - absXSpeed) < absXSpeed / 2) {
            realY_ -= bumpyOffset;
            bumpyOffsetY -= bumpyOffset;
        }
        if (FixPoint::abs(ySpeed) >= epsilon && FixPoint::abs(fromDistanceY - absYSpeed) < absYSpeed / 2) {
            realX_ += bumpyOffset;
            bumpyOffsetX += bumpyOffset;
        }

        if (FixPoint::abs(xSpeed) >= epsilon && FixPoint::abs(fromDistanceX - 4 * absXSpeed) < absXSpeed / 2) {
            realY_ += bumpyOffset;
            bumpyOffsetY += bumpyOffset;
        }
        if (FixPoint::abs(ySpeed) >= epsilon && FixPoint::abs(fromDistanceY - 4 * absYSpeed) < absYSpeed / 2) {
            realX_ -= bumpyOffset;
            bumpyOffsetX -= bumpyOffset;
        }

        if (FixPoint::abs(xSpeed) >= epsilon && FixPoint::abs(fromDistanceX - 10 * absXSpeed) < absXSpeed / 2) {
            realY_ -= bumpyOffset;
            bumpyOffsetY -= bumpyOffset;
        }
        if (FixPoint::abs(ySpeed) >= epsilon && FixPoint::abs(fromDistanceY - 20 * absYSpeed) < absYSpeed / 2) {
            realX_ += bumpyOffset;
            bumpyOffsetX += bumpyOffset;
        }

        if (FixPoint::abs(xSpeed) >= epsilon && FixPoint::abs(fromDistanceX - 14 * absXSpeed) < absXSpeed / 2) {
            realY_ += bumpyOffset;
            bumpyOffsetY += bumpyOffset;
        }
        if (FixPoint::abs(ySpeed) >= epsilon && FixPoint::abs(fromDistanceY - 14 * absYSpeed) < absYSpeed / 2) {
            realX_ -= bumpyOffset;
            bumpyOffsetX -= bumpyOffset;
        }

        if (FixPoint::abs(xSpeed) >= epsilon && FixPoint::abs(toDistanceX - absXSpeed) < absXSpeed / 2) {
            realY_ -= bumpyOffset;
            bumpyOffsetY -= bumpyOffset;
        }
        if (FixPoint::abs(ySpeed) >= epsilon && FixPoint::abs(toDistanceY - absYSpeed) < absYSpeed / 2) {
            realX_ += bumpyOffset;
            bumpyOffsetX += bumpyOffset;
        }

        if (FixPoint::abs(xSpeed) >= epsilon && FixPoint::abs(toDistanceX - 4 * absXSpeed) < absXSpeed / 2) {
            realY_ += bumpyOffset;
            bumpyOffsetY += bumpyOffset;
        }
        if (FixPoint::abs(ySpeed) >= epsilon && FixPoint::abs(toDistanceY - 4 * absYSpeed) < absYSpeed / 2) {
            realX_ -= bumpyOffset;
            bumpyOffsetX -= bumpyOffset;
        }

        if (FixPoint::abs(xSpeed) >= epsilon && FixPoint::abs(toDistanceX - 10 * absXSpeed) < absXSpeed / 2) {
            realY_ -= bumpyOffset;
            bumpyOffsetY -= bumpyOffset;
        }
        if (FixPoint::abs(ySpeed) >= epsilon && FixPoint::abs(toDistanceY - 10 * absYSpeed) < absYSpeed / 2) {
            realX_ += bumpyOffset;
            bumpyOffsetX += bumpyOffset;
        }

        if (FixPoint::abs(xSpeed) >= epsilon && FixPoint::abs(toDistanceX - 14 * absXSpeed) < absXSpeed / 2) {
            realY_ += bumpyOffset;
            bumpyOffsetY += bumpyOffset;
        }
        if (FixPoint::abs(ySpeed) >= epsilon && FixPoint::abs(toDistanceY - 14 * absYSpeed) < absYSpeed / 2) {
            realX_ -= bumpyOffset;
            bumpyOffsetX -= bumpyOffset;
        }
    }
}

void UnitBase::navigate_fallback(const GameContext& context) {
    if (const auto* const targetPtr = target_.getObjPointer()) {
        if (targetFriendly_ && targetPtr->getItemID() != Structure_RepairYard
            && (targetPtr->getItemID() != Structure_Refinery || getItemID() != Unit_Harvester)) {
            setTarget(nullptr);
        }
    }

    const auto* const house = getOwner();

    /// This method will transport units if they get stuck inside a base
    /// This often happens after an AI get nuked and has a hole in their base
    if (house->hasCarryalls()) {
        if (auto* const unit = dune_cast<GroundUnit>(this)) {
            const auto& game = context.game;
            auto& options    = game.getGameInitSettings().getGameOptions();

            if ((options.manualCarryallDrops || house->isAI())
                && blockDistance(location_, destination_) >= MIN_CARRYALL_LIFT_DISTANCE) {

                unit->requestCarryall(context);
                return;
            }
        }
    }

    if (house->isAI()) {
        if (auto* const harvester = dune_cast<Harvester>(this)) {
            if (!harvester->isReturning() && blockDistance(location_, destination_) >= 2) {
                // try getting back to a refinery
                harvester->doReturn();

                return;
            }
        }
    }

    ObjectBase::setDestination(location_); // can't get any closer, give up
    forced_ = false;
}

void UnitBase::navigate(const GameContext& context) {

    const auto& game = context.game;

    // navigation is only performed every 5th frame
    if (!isAFlyingUnit() && (game.getGameCycleCount() + getObjectID() * 1337) % 5 != 0)
        return;

    if (moving || justStoppedMoving)
        return;

    if (location_ != destination_) {
        if (!nextSpotFound) {

            if (pathList.empty() && recalculatePathTimer == 0) {
                recalculatePathTimer = 100;

                // try searching for a path a number of times then give up
                if (!SearchPathWithAStar() && ++noCloserPointCount >= 3 && location_ != oldLocation_) {

                    navigate_fallback(context);
                }
            }

            if (!pathList.empty()) {
                nextSpot = pathList.back();
                pathList.pop_back();
                nextSpotFound        = true;
                recalculatePathTimer = 0;
                noCloserPointCount   = 0;
            }
        } else {
            const auto tempAngle = Map::getPosAngle(location_, nextSpot);
            if (tempAngle != ANGLETYPE::INVALID_ANGLE) {
                nextSpotAngle = tempAngle;
            }

            if (!canPass(nextSpot.x, nextSpot.y)) {
                clearPath();
            } else {
                if (drawnAngle_ == nextSpotAngle) {
                    moving        = true;
                    nextSpotFound = false;

                    assignToMap(context, nextSpot);
                    angle_ = static_cast<int>(drawnAngle_);
                    setSpeeds(context);
                }
            }
        }
    } else if (!target_ && attackPos.isInvalid()) {
        if ((game.getGameCycleCount() + getObjectID() * 1337) % MILLI2CYCLES(UNITIDLETIMER) == 0) {
            idleAction(context);
        }
    }
}

void UnitBase::idleAction(const GameContext& context) {
    // not moving and not wanting to go anywhere, do some random turning
    if (isAGroundUnit() && getItemID() != Unit_Harvester && getAttackMode() == GUARD) {
        // we might turn this cycle with 20% chance
        if (context.game.randomGen.rand(0, 4) == 0) {
            // choose a random one of the eight possible angles
            nextSpotAngle = static_cast<ANGLETYPE>(context.game.randomGen.rand(0, 7));
        }
    }
}

void UnitBase::handleActionClick(const GameContext& context, int xPos, int yPos) {
    const auto& map = context.map;

    if (!respondable_ || !map.tileExists(xPos, yPos))
        return;

    auto& game = context.game;

    const auto* const tempTarget = map.tryGetObject(context, xPos, yPos);

    if (tempTarget) {
        // attack unit/structure or move to structure

        const auto is_owner = tempTarget->getOwner()->getTeamID() == getOwner()->getTeamID();
        const auto cmd_type = is_owner ? CMDTYPE::CMD_UNIT_MOVE2OBJECT : CMDTYPE::CMD_UNIT_ATTACKOBJECT;

        game.getCommandManager().addCommand(
            Command{dune::globals::pLocalPlayer->getPlayerID(), cmd_type, objectID_, tempTarget->getObjectID()});
    } else {
        // move this unit
        game.getCommandManager().addCommand(Command{dune::globals::pLocalPlayer->getPlayerID(),
                                                    CMDTYPE::CMD_UNIT_MOVE2POS, objectID_, static_cast<uint32_t>(xPos),
                                                    static_cast<uint32_t>(yPos), static_cast<uint32_t>(true)});
    }
}

void UnitBase::handleAttackClick(const GameContext& context, int xPos, int yPos) {
    if (!respondable_)
        return;

    const auto& map = context.map;

    if (map.tileExists(xPos, yPos)) {
        auto& game = context.game;

        const auto* const tempTarget = map.tryGetObject(context, xPos, yPos);
        if (tempTarget) {
            // attack unit/structure or move to structure

            game.getCommandManager().addCommand(Command(dune::globals::pLocalPlayer->getPlayerID(),
                                                        CMDTYPE::CMD_UNIT_ATTACKOBJECT, objectID_,
                                                        tempTarget->getObjectID()));
        } else {
            // attack pos
            game.getCommandManager().addCommand(
                Command(dune::globals::pLocalPlayer->getPlayerID(), CMDTYPE::CMD_UNIT_ATTACKPOS, objectID_,
                        static_cast<uint32_t>(xPos), static_cast<uint32_t>(yPos), static_cast<uint32_t>(true)));
        }
    }
}

void UnitBase::handleMoveClick(const GameContext& context, int xPos, int yPos) {
    if (!respondable_)
        return;

    if (context.map.tileExists(xPos, yPos)) {
        // move to pos
        context.game.getCommandManager().addCommand(
            Command(dune::globals::pLocalPlayer->getPlayerID(), CMDTYPE::CMD_UNIT_MOVE2POS, objectID_,
                    static_cast<uint32_t>(xPos), static_cast<uint32_t>(yPos), static_cast<uint32_t>(true)));
    }
}

void UnitBase::handleSetAttackModeClick(const GameContext& context, ATTACKMODE newAttackMode) {
    context.game.getCommandManager().addCommand(Command(dune::globals::pLocalPlayer->getPlayerID(),
                                                        CMDTYPE::CMD_UNIT_SETMODE, objectID_,
                                                        static_cast<uint32_t>(newAttackMode)));
}

/**
    User action
    Request a Carryall to drop at target location
**/
void UnitBase::handleRequestCarryallDropClick(const GameContext& context, int xPos, int yPos) {
    if (!respondable_)
        return;

    if (context.map.tileExists(xPos, yPos)) {
        context.game.getCommandManager().addCommand(Command(dune::globals::pLocalPlayer->getPlayerID(),
                                                            CMDTYPE::CMD_UNIT_REQUESTCARRYALLDROP, objectID_,
                                                            static_cast<uint32_t>(xPos), static_cast<uint32_t>(yPos)));
    }
}

void UnitBase::doMove2Pos(const GameContext& context, int xPos, int yPos, bool bForced) {
    if (attackMode_ == CAPTURE || attackMode_ == HUNT) {
        doSetAttackMode(context, GUARD);
    }

    if (context.map.tileExists(xPos, yPos)) {
        if (xPos != destination_.x || yPos != destination_.y) {
            clearPath();
            findTargetTimer = 0;
        }

        setTarget(nullptr);
        setDestination(xPos, yPos);
        setForced(bForced);
        setGuardPoint(xPos, yPos);
    } else {
        setTarget(nullptr);
        setDestination(location_);
        setForced(bForced);
        setGuardPoint(location_);
    }
}

void UnitBase::doMove2Pos(const GameContext& context, const Coord& coord, bool bForced) {
    doMove2Pos(context, coord.x, coord.y, bForced);
}

void UnitBase::doMove2Object(const GameContext& context, const ObjectBase* pTargetObject) {
    if (pTargetObject->getObjectID() == getObjectID()) {
        return;
    }

    if (attackMode_ == CAPTURE || attackMode_ == HUNT) {
        doSetAttackMode(context, GUARD);
    }

    setDestination(INVALID_POS, INVALID_POS);
    setTarget(pTargetObject);
    setForced(true);

    bFollow = true;

    clearPath();
    findTargetTimer = 0;
}

void UnitBase::doMove2Object(const GameContext& context, uint32_t targetObjectID) {
    const ObjectBase* pObject = context.objectManager.getObject(targetObjectID);

    if (pObject == nullptr) {
        return;
    }

    doMove2Object(context, pObject);
}

void UnitBase::doAttackPos(const GameContext& context, int xPos, int yPos, bool bForced) {
    if (!context.map.tileExists(xPos, yPos)) {
        return;
    }

    if (attackMode_ == CAPTURE) {
        doSetAttackMode(context, GUARD);
    }

    setDestination(xPos, yPos);
    setTarget(nullptr);
    setForced(bForced);
    attackPos.x = xPos;
    attackPos.y = yPos;

    clearPath();
    findTargetTimer = 0;
}

void UnitBase::doAttackObject(const GameContext& context, const ObjectBase* pTargetObject, bool bForced) {
    if (pTargetObject->getObjectID() == getObjectID() || (!canAttack() && getItemID() != Unit_Harvester))
        return;

    if (attackMode_ == CAPTURE)
        doSetAttackMode(context, GUARD);

    setDestination(INVALID_POS, INVALID_POS);

    setTarget(pTargetObject);
    // hack to make it possible to attack own repair yard
    if (goingToRepairYard && target_) {
        if (auto* const repair_yard = dune_cast<RepairYard>(target_.getObjPointer())) {
            repair_yard->unBook();
            goingToRepairYard = false;
        }
    }

    setForced(bForced);

    clearPath();
    findTargetTimer = 0;
}

void UnitBase::doAttackObject(const GameContext& context, uint32_t TargetObjectID, bool bForced) {
    const auto* pObject = context.objectManager.getObject(TargetObjectID);

    if (pObject == nullptr)
        return;

    doAttackObject(context, pObject, bForced);
}

void UnitBase::doSetAttackMode(const GameContext& context, ATTACKMODE newAttackMode) {
    if (newAttackMode >= 0 && newAttackMode < ATTACKMODE_MAX)
        attackMode_ = newAttackMode;

    if (attackMode_ == GUARD || attackMode_ == STOP) {
        if (moving && !justStoppedMoving) {
            doMove2Pos(context, nextSpot, false);
        } else {
            doMove2Pos(context, location_, false);
        }
    }
}

void UnitBase::handleDamage(const GameContext& context, int damage, uint32_t damagerID, House* damagerOwner) {
    // shorten deviation time
    if (deviationTimer > 0)
        deviationTimer = std::max(0, deviationTimer - MILLI2CYCLES(damage * 20 * 1000));

    parent::handleDamage(context, damage, damagerID, damagerOwner);

    const auto* pDamager = context.objectManager.getObject(damagerID);

    if (pDamager != nullptr) {

        if (attackMode_ == HUNT && !forced_) {
            if (canAttack(pDamager)) {
                if (!target_ || target_.getObjPointer() == nullptr || !isInWeaponRange(target_.getObjPointer())) {
                    // no target or target not on weapon range => switch target
                    doAttackObject(context, pDamager, false);
                }
            }
        }

        /*
         This method records the damage taken so that QuantBot can use it to know how effective different unit
         classes are during the current game so that it can adjust its unit build ratios
        */

        // If you damaged your own unit then, the damage should be treated as negative.
        if (damagerOwner == getOwner()) {
            damage *= -1;
        }

        damagerOwner->informHasDamaged(pDamager->getItemID(), damage);
    }
}

bool UnitBase::isInGuardRange(const ObjectBase* pObject) const {
    int checkRange = 0;
    switch (attackMode_) {
        case GUARD: {
            checkRange = getItemID() == Unit_Sandworm ? getViewRange() : getWeaponRange();
        } break;

        case AREAGUARD: {
            if (getItemID() == Unit_Sandworm) {
                return true;
            }
            checkRange = getAreaGuardRange();
        } break;

        case AMBUSH: {
            checkRange = getViewRange();
        } break;

        case HUNT: {
            return true;
        }

        case CARRYALLREQUESTED: {
            return false;
        }

        case RETREAT: {
            return false;
        }

        case STOP:
        default: {
            return false;
        }
    }

    return blockDistance(guardPoint * TILESIZE + Coord(TILESIZE / 2, TILESIZE / 2), pObject->getCenterPoint())
        <= checkRange * TILESIZE;
}

bool UnitBase::isInAttackRange(const ObjectBase* object) const {
    int checkRange = 0;
    switch (attackMode_) {
        case GUARD: {
            checkRange = getWeaponRange();
        } break;

        case AREAGUARD: {
            checkRange = getAreaGuardRange() + getWeaponRange() + 1;
        } break;

        case AMBUSH: {
            checkRange = getViewRange() + 1;
        } break;

        case HUNT: {
            return true;
        }

        case CARRYALLREQUESTED: {
            return false;
        }

        case RETREAT: {
            return false;
        }

        case STOP:
        default: {
            return false;
        }
    }

    if (getItemID() == Unit_Sandworm) {
        checkRange = getViewRange() + 1;
    }

    return blockDistance(guardPoint * TILESIZE + Coord(TILESIZE / 2, TILESIZE / 2), object->getCenterPoint())
        <= checkRange * TILESIZE;
}

bool UnitBase::isInWeaponRange(const ObjectBase* object) const {
    if (object == nullptr)
        return false;

    const Coord targetLocation = target_.getObjPointer()->getClosestPoint(location_);

    return blockDistance(location_, targetLocation) <= getWeaponRange();
}

void UnitBase::setAngle(ANGLETYPE newAngle) {
    if (!moving && !justStoppedMoving) {
        newAngle    = normalizeAngle(newAngle);
        drawnAngle_ = newAngle;
        angle_      = static_cast<int>(newAngle);
        clearPath();
    }
}

void UnitBase::setGettingRepaired() {
    auto* const repair_yard = dune_cast<RepairYard>(target_.getObjPointer());

    if (!repair_yard)
        return;

    dune::globals::currentGameMap->removeObjectFromMap(getObjectID());

    repair_yard->assignUnit(this);

    respondable_ = false;
    setActive(false);
    setVisible(VIS_ALL, false);
    goingToRepairYard = false;
    badlyDamaged_     = false;

    setTarget(nullptr);
    // setLocation(INVALID_POS, INVALID_POS);
    setDestination(location_);
    nextSpotAngle = ANGLETYPE::DOWN;
}

void UnitBase::setGuardPoint(int newX, int newY) {
    if (dune::globals::currentGameMap->tileExists(newX, newY) || (newX == INVALID_POS && newY == INVALID_POS)) {
        guardPoint.x = newX;
        guardPoint.y = newY;

        if (getItemID() == Unit_Harvester && guardPoint.isValid()) {
            if (dune::globals::currentGameMap->getTile(newX, newY)->hasSpice()) {
                if (attackMode_ == STOP) {
                    attackMode_ = GUARD;
                }
            } else {
                if (attackMode_ != STOP) {
                    attackMode_ = STOP;
                }
            }
        }
    }
}

void UnitBase::setLocation(const GameContext& context, int xPos, int yPos) {

    if (xPos == INVALID_POS && yPos == INVALID_POS) {
        parent::setLocation(context, xPos, yPos);
    } else if (dune::globals::currentGameMap->tileExists(xPos, yPos)) {
        parent::setLocation(context, xPos, yPos);
        realX_ += TILESIZE / 2;
        realY_ += TILESIZE / 2;
        bumpyOffsetX = 0;
        bumpyOffsetY = 0;
    }

    moving   = false;
    pickedUp = false;
    setTarget(nullptr);

    clearPath();
}

void UnitBase::setPickedUp(const GameContext& context, UnitBase* newCarrier) {
    context.map.removeObjectFromMap(getObjectID());

    if (goingToRepairYard) {
        if (auto* const repair_yard = dune_cast<RepairYard>(target_.getObjPointer()))
            repair_yard->unBook();
    }

    if (const auto* const harvester = dune_cast<Harvester>(this)) {
        if (harvester->isReturning()) {
            if (auto* const refinery = dune_cast<Refinery>(target_.getObjPointer()))
                refinery->unBook();
        }
    }

    target_.pointTo(newCarrier);

    goingToRepairYard = false;
    forced_           = false;
    moving            = false;
    pickedUp          = true;
    respondable_      = false;
    setActive(false);
    setVisible(VIS_ALL, false);

    clearPath();
}

FixPoint UnitBase::getMaxSpeed(const GameContext& context) const {
    return context.game.objectData.data[itemID_][static_cast<int>(originalHouseID_)].maxspeed;
}

void UnitBase::setSpeeds(const GameContext& context) {
    FixPoint speed = getMaxSpeed(context);

    if (!isAFlyingUnit()) {
        speed += speed * (1 - getTerrainDifficulty(context.map.getTile(location_)->getType()));
        if (isBadlyDamaged()) {
            speed *= HEAVILYDAMAGEDSPEEDMULTIPLIER;
        }
    }

    // clang-format off
    switch(drawnAngle_){
        case ANGLETYPE::LEFT:      xSpeed = -speed;                    ySpeed = 0;         break;
        case ANGLETYPE::LEFTUP:    xSpeed = -speed*DIAGONALSPEEDCONST; ySpeed = xSpeed;    break;
        case ANGLETYPE::UP:        xSpeed = 0;                         ySpeed = -speed;    break;
        case ANGLETYPE::RIGHTUP:   xSpeed = speed*DIAGONALSPEEDCONST;  ySpeed = -xSpeed;   break;
        case ANGLETYPE::RIGHT:     xSpeed = speed;                     ySpeed = 0;         break;
        case ANGLETYPE::RIGHTDOWN: xSpeed = speed*DIAGONALSPEEDCONST;  ySpeed = xSpeed;    break;
        case ANGLETYPE::DOWN:      xSpeed = 0;                         ySpeed = speed;     break;
        case ANGLETYPE::LEFTDOWN:  xSpeed = -speed*DIAGONALSPEEDCONST; ySpeed = -xSpeed;   break;
        default: {
            assert(0);
        } break;
    }
    // clang-format on
}

void UnitBase::setTarget(const ObjectBase* newTarget) {
    attackPos.invalidate();
    bFollow     = false;
    targetAngle = ANGLETYPE::INVALID_ANGLE;

    if (goingToRepairYard) {
        if (auto* repairYard = dune_cast<RepairYard>(target_.getObjPointer()))
            repairYard->unBook();

        goingToRepairYard = false;
    }

    parent::setTarget(newTarget);

    if (auto* const currentTarget = dune_cast<RepairYard>(target_.getObjPointer())) {
        if (currentTarget->getOwner() == getOwner()) {
            currentTarget->book();
            goingToRepairYard = true;
        }
    }
}

void UnitBase::targeting(const GameContext& context) {
    if (findTargetTimer == 0) {

        if (attackMode_ != STOP && attackMode_ != CARRYALLREQUESTED) {

            // lets add a bit of logic to make units recalibrate their nearest target if the target isn't in weapon
            // range
            if (target_ && !attackPos && !forced_
                && (attackMode_ == GUARD || attackMode_ == AREAGUARD || attackMode_ == HUNT)) {
                if (!isInWeaponRange(target_.getObjPointer())) {
                    const auto* pNewTarget = findTarget();

                    if (pNewTarget != nullptr) {

                        doAttackObject(context, pNewTarget, false);

                        findTargetTimer = 500;
                    }
                }
            }

            if (!target_ && !attackPos && !moving && !justStoppedMoving && !forced_) {
                // we have no target, we have stopped moving and we weren't forced to do anything else

                const ObjectBase* pNewTarget = findTarget();

                if (pNewTarget != nullptr && isInGuardRange(pNewTarget)) {
                    // we have found a new target => attack it
                    if (attackMode_ == AMBUSH) {
                        doSetAttackMode(context, HUNT);
                    }
                    doAttackObject(context, pNewTarget, false);

                    if (getItemID() == Unit_Sandworm) {
                        doSetAttackMode(context, HUNT);
                    }
                } else if (attackMode_ == HUNT) {
                    setGuardPoint(location_);
                    doSetAttackMode(context, GUARD);
                }

                // reset target timer
                findTargetTimer = MILLI2CYCLES(2 * 1000);
            }
        }
    }

    engageTarget(context);
}

void UnitBase::turn(const GameContext& context) {
    if (!moving && !justStoppedMoving) {
        auto wantedAngle = ANGLETYPE::INVALID_ANGLE;

        // if we have to decide between moving and shooting we opt for moving
        if (nextSpotAngle != ANGLETYPE::INVALID_ANGLE) {
            wantedAngle = nextSpotAngle;
        } else if (targetAngle != ANGLETYPE::INVALID_ANGLE) {
            wantedAngle = targetAngle;
        }

        if (wantedAngle != ANGLETYPE::INVALID_ANGLE) {
            FixPoint angleLeft  = 0;
            FixPoint angleRight = 0;

            if (angle_ > static_cast<int>(wantedAngle)) {
                angleRight = angle_ - static_cast<int>(wantedAngle);
                angleLeft  = FixPoint::abs(8 - angle_) + static_cast<int>(wantedAngle);
            } else if (angle_ < static_cast<int>(wantedAngle)) {
                angleRight = FixPoint::abs(8 - static_cast<int>(wantedAngle)) + angle_;
                angleLeft  = static_cast<int>(wantedAngle) - angle_;
            }

            if (angleLeft <= angleRight) {
                turnLeft(context);
            } else {
                turnRight(context);
            }
        }
    }
}

void UnitBase::turnLeft(const GameContext& context) {
    angle_ += context.game.objectData.data[itemID_][static_cast<int>(originalHouseID_)].turnspeed;
    if (angle_ >= 7.5_fix) {
        angle_ -= NUM_ANGLES;
    }
    drawnAngle_ = normalizeAngle(static_cast<ANGLETYPE>(lround(angle_)));
}

void UnitBase::turnRight(const GameContext& context) {
    angle_ -= context.game.objectData.data[itemID_][static_cast<int>(originalHouseID_)].turnspeed;
    if (angle_ <= -0.5_fix) {
        angle_ += NUM_ANGLES;
    }
    drawnAngle_ = normalizeAngle(static_cast<ANGLETYPE>(lround(angle_)));
}

void UnitBase::quitDeviation(const GameContext& context) {
    if (wasDeviated()) {
        // revert back to real owner
        setTarget(nullptr);
        setGuardPoint(location_);
        setDestination(location_);
        owner_         = context.game.getHouse(originalHouseID_);
        graphic_       = dune::globals::pGFXManager->getObjPic(graphicID_, getOwner()->getHouseID());
        deviationTimer = INVALID;
    }
}

bool UnitBase::update(const GameContext& context) {
    if (active_) {
        targeting(context);
        navigate(context);
        move(context);
        if (active_) {
            turn(context);
            updateVisibleUnits(context);
        }
    }

    if (getHealth() <= 0) {
        destroy(context);
        return false;
    }

    if (recalculatePathTimer > 0)
        recalculatePathTimer--;
    if (findTargetTimer > 0)
        findTargetTimer--;
    if (primaryWeaponTimer > 0)
        primaryWeaponTimer--;
    if (secondaryWeaponTimer > 0)
        secondaryWeaponTimer--;
    if (deviationTimer != INVALID) {
        if (--deviationTimer <= 0) {
            quitDeviation(context);
        }
    }

    return true;
}

void UnitBase::updateVisibleUnits(const GameContext& context) {
    if (isAFlyingUnit()) {
        return;
    }

    const auto* pTile = context.map.tryGetTile(location_.x, location_.y);
    if (!pTile)
        return;

    const auto& game = context.game;

    game.for_each_house([&](auto& house) {
        if (pTile->isExploredByHouse(house.getHouseID()) && house.getTeamID() != getOwner()->getTeamID()
            && &house != getOwner()) {
            house.informDirectContactWithEnemy();
            getOwner()->informDirectContactWithEnemy();
        }

        if (pTile->isExploredByTeam(&context.game, house.getTeamID())) {
            if (house.getTeamID() == getOwner()->getTeamID()) {
                house.informVisibleFriendlyUnit();
            } else {
                house.informVisibleEnemyUnit();
                house.informContactWithEnemy();
                getOwner()->informContactWithEnemy();
            }
        }
    });
}

bool UnitBase::canPassTile(const Tile* pTile) const {
    if (!pTile || pTile->isMountain())
        return false;

    const auto ground_object_result = pTile->getGroundObjectID();

    if (!ground_object_result.first)
        return true;

    if (ground_object_result.second == target_.getObjectID()) {
        const auto* const pObject =
            dune::globals::currentGame->getObjectManager().getObject(ground_object_result.second);

        if (pObject != nullptr && pObject->getObjectID() == target_.getObjectID() && targetFriendly_
            && pObject->isAStructure() && pObject->getOwner()->getTeamID() == owner_->getTeamID()
            && pObject->isVisible(getOwner()->getTeamID())) {
            // are we entering a repair yard?
            return goingToRepairYard && pObject->getItemID() == Structure_RepairYard
                && static_cast<const RepairYard*>(pObject)->isFree();
        }
    }
    return false;
}

bool UnitBase::SearchPathWithAStar() {
    Coord destinationCoord;

    if (target_) {
        const auto* const obj_pointer = target_.getObjPointer();

        if (obj_pointer != nullptr) {
            if (itemID_ == Unit_Carryall && obj_pointer->getItemID() == Structure_Refinery) {
                destinationCoord = obj_pointer->getLocation() + Coord(2, 0);
            } else if (itemID_ == Unit_Frigate && obj_pointer->getItemID() == Structure_StarPort) {
                destinationCoord = obj_pointer->getLocation() + Coord(1, 1);
            } else {
                destinationCoord = obj_pointer->getClosestPoint(location_);
            }
        } else
            destinationCoord = destination_;
    } else {
        destinationCoord = destination_;
    }

    dune::globals::currentGameMap->find_path(this, location_, destinationCoord, pathList);

    if (pathList.empty()) {
        nextSpotFound = false;
        return false;
    }

    return true;
}

void UnitBase::drawSmoke(float x, float y) const {
    auto frame =
        static_cast<int>(dune::globals::currentGame->getGameCycleCount() + getObjectID() * 10) / SMOKEDELAY % (2 * 2);
    if (frame == 3)
        frame = 1;

    const auto* const pSmokeTex = dune::globals::pGFXManager->getZoomedObjPic(ObjPic_Smoke, getOwner()->getHouseID(),
                                                                              dune::globals::currentZoomlevel);

    const auto dest   = calcSpriteDrawingRect(pSmokeTex, x, y, 3, 1, HAlign::Center, VAlign::Bottom);
    const auto source = calcSpriteSourceRect(pSmokeTex, frame, 3);

    Dune_RenderCopyF(dune::globals::renderer.get(), pSmokeTex, &source, &dest);
}

void UnitBase::playAttackSound() { }
