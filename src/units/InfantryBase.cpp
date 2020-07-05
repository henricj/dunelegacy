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

#include <units/InfantryBase.h>

#include <globals.h>

#include <House.h>
#include <Game.h>
#include <Map.h>
#include <SoundPlayer.h>
#include <ScreenBorder.h>

#include <players/HumanPlayer.h>

#include <structures/StructureBase.h>
#include <structures/Refinery.h>
#include <structures/RepairYard.h>
#include <units/Harvester.h>

// the position on the tile
Coord tilePositionOffset[5] = { Coord(0,0), Coord(-TILESIZE/4,-TILESIZE/4), Coord(TILESIZE/4,-TILESIZE/4), Coord(-TILESIZE/4,TILESIZE/4), Coord(TILESIZE/4,TILESIZE/4)};


InfantryBase::InfantryBase(ItemID_enum itemID, Uint32 objectID, const ObjectInitializer& initializer) : GroundUnit(itemID, objectID, initializer) {

    InfantryBase::init();

    setHealth(getMaxHealth());

    tilePosition = INVALID_POS;
    oldTilePosition = INVALID_POS;
}

InfantryBase::InfantryBase(ItemID_enum itemID, Uint32 objectID, const ObjectStreamInitializer& initializer) : GroundUnit(itemID, objectID, initializer) {

    InfantryBase::init();

    auto& stream = initializer.Stream;

    tilePosition = stream.readSint8();
    oldTilePosition = stream.readSint8();
}

void InfantryBase::init() {
    infantry = true;
}

InfantryBase::~InfantryBase() = default;


void InfantryBase::save(OutputStream& stream) const {

    GroundUnit::save(stream);

    stream.writeSint8(tilePosition);
    stream.writeSint8(oldTilePosition);
}

void InfantryBase::handleCaptureClick(int xPos, int yPos) {
    if(respondable && ((getItemID() == Unit_Soldier) || (getItemID() == Unit_Trooper))) {
        const auto* const tempTarget = currentGameMap->tryGetObject(xPos, yPos);

        if(!tempTarget) return;

        // capture structure
        currentGame->getCommandManager().addCommand(Command(
            pLocalPlayer->getPlayerID(), CMDTYPE::CMD_INFANTRY_CAPTURE, objectID, tempTarget->getObjectID()));
    }
}

void InfantryBase::doCaptureStructure(Uint32 targetStructureID) {
    const StructureBase* pStructure = dynamic_cast<StructureBase*>(currentGame->getObjectManager().getObject(targetStructureID));
    doCaptureStructure(pStructure);
}

void InfantryBase::doCaptureStructure(const StructureBase* pStructure) {

    if((pStructure == nullptr) || (!pStructure->canBeCaptured()) || (pStructure->getOwner()->getTeamID() == getOwner()->getTeamID())) {
        // does not exist anymore, cannot be captured or is a friendly building
        return;
    }

    doAttackObject(pStructure, true);
    doSetAttackMode(CAPTURE);
}

void InfantryBase::assignToMap(const Coord& pos) {
    if(currentGameMap->tileExists(pos)) {
        oldTilePosition = tilePosition;
        tilePosition = currentGameMap->getTile(pos)->assignInfantry(currentGame->getObjectManager(), getObjectID());
        currentGameMap->viewMap(owner->getHouseID(), pos, getViewRange());
    }
}

void InfantryBase::blitToScreen() {
    SDL_Rect dest = calcSpriteDrawingRect(  graphic[currentZoomlevel],
                                            screenborder->world2screenX(realX),
                                            screenborder->world2screenY(realY),
                                            numImagesX, numImagesY,
                                            HAlign::Center, VAlign::Center);

    auto temp = drawnAngle;
    if(temp == ANGLETYPE::UP) {
        temp = ANGLETYPE::RIGHTUP;
    } else if(temp == ANGLETYPE::DOWN) {
        temp = ANGLETYPE::LEFTUP;
    } else if(temp == ANGLETYPE::LEFTUP || temp == ANGLETYPE::LEFTDOWN || temp == ANGLETYPE::LEFT) {
        temp = ANGLETYPE::UP;
    } else {
        temp = ANGLETYPE::RIGHT;
    }

    SDL_Rect source = calcSpriteSourceRect(graphic[currentZoomlevel], static_cast<int>(temp), numImagesX, (walkFrame/10 == 3) ? 1 : walkFrame/10, numImagesY);

    SDL_RenderCopy(renderer, graphic[currentZoomlevel], &source, &dest);
}

bool InfantryBase::canPassTile(const Tile* pTile) const {
    bool passable = false;

    if(!pTile->hasAGroundObject()) {
        if(pTile->getType() != Terrain_Mountain) {
            passable = true;
        } else {
            /* if this unit is infantry so can climb, and tile can take more infantry */
            if(pTile->infantryNotFull()) { passable = true; }
        }
    } else {
        auto* const object = pTile->getGroundObject(currentGame->getObjectManager());

        if((object != nullptr) && (object->getObjectID() == target.getObjectID()) && object->isAStructure() &&
           (object->getOwner()->getTeamID() != owner->getTeamID()) && object->isVisible(getOwner()->getTeamID())) {
            passable = true;
        } else {
            passable = (!pTile->hasANonInfantryGroundObject() &&
                        (pTile->infantryNotFull() && (pTile->getInfantryTeam(currentGame->getObjectManager()) == getOwner()->getTeamID())));
        }
    }

    return passable;
}

void InfantryBase::checkPos() {
    if(moving && !justStoppedMoving) {
        if(++walkFrame > 39) {
            walkFrame = 0;
        }
    }

    if(!justStoppedMoving)
        return;

    walkFrame = 0;

    if(currentGameMap->getTile(location)->isSpiceBloom()) {
        setHealth(0);
        currentGameMap->getTile(location)->triggerSpiceBloom(currentGame.get(), getOwner());
    } else if(currentGameMap->getTile(location)->isSpecialBloom()){
        currentGameMap->getTile(location)->triggerSpecialBloom(currentGame.get(), getOwner());
    }

    auto *const object = target.getObjPointer();

    if (!object || !object->isAStructure())
        return;

    //check to see if close enough to blow up target
    if (getOwner()->getTeamID() == object->getOwner()->getTeamID()) {
        const auto closestPoint = object->getClosestPoint(location);

        if (blockDistance(location, closestPoint) <= 0.5_fix) {
            StructureBase* pCapturedStructure = target.getStructurePointer();
            if (pCapturedStructure->getHealthColor() == COLOR_RED) {
                House* pOwner = pCapturedStructure->getOwner();
                auto targetID = pCapturedStructure->getItemID();
                int posX = pCapturedStructure->getX();
                int posY = pCapturedStructure->getY();
                const auto origHouse = pCapturedStructure->getOriginalHouseID();
                int oldHealth = lround(pCapturedStructure->getHealth());
                bool isSelected = pCapturedStructure->isSelected();
                bool isSelectedByOtherPlayer = pCapturedStructure->isSelectedByOtherPlayer();

                FixPoint capturedSpice = 0;

                UnitBase* pContainedUnit = nullptr;

                if (pCapturedStructure->getItemID() == Structure_Silo) {
                    capturedSpice = currentGame->objectData.data[Structure_Silo][static_cast<int>(originalHouseID)].capacity * (pOwner->getStoredCredits() / pOwner->getCapacity());
                }
                else if (pCapturedStructure->getItemID() == Structure_Refinery) {
                    capturedSpice = currentGame->objectData.data[Structure_Silo][static_cast<int>(originalHouseID)].capacity * (pOwner->getStoredCredits() / pOwner->getCapacity());
                    auto* pRefinery = static_cast<Refinery*>(pCapturedStructure);
                    if (!pRefinery->isFree()) {
                        pContainedUnit = pRefinery->getHarvester();
                    }
                }
                else if (pCapturedStructure->getItemID() == Structure_RepairYard) {
                    auto* pRepairYard = static_cast<RepairYard*>(pCapturedStructure);
                    if (!pRepairYard->isFree()) {
                        pContainedUnit = pRepairYard->getRepairUnit();
                    }
                }

                auto containedUnitID = ItemID_enum::ItemID_Invalid;
                FixPoint containedUnitHealth = 0;
                FixPoint containedHarvesterSpice = 0;
                if (pContainedUnit != nullptr) {
                    containedUnitID = pContainedUnit->getItemID();
                    containedUnitHealth = pContainedUnit->getHealth();
                    if (containedUnitID == Unit_Harvester) {
                        containedHarvesterSpice = static_cast<Harvester*>(pContainedUnit)->getAmountOfSpice();
                    }

                    // will be destroyed by the captured structure
                    pContainedUnit = nullptr;
                }

                // remove all other infantry units capturing this building
                const auto capturedStructureLocation = pCapturedStructure->getLocation();
                for (auto i = capturedStructureLocation.x; i < capturedStructureLocation.x + pCapturedStructure->getStructureSizeX(); i++) {
                    for (auto j = capturedStructureLocation.y; j < capturedStructureLocation.y + pCapturedStructure->getStructureSizeY(); j++) {

                        // make a copy of infantry list to avoid problems of modifying the list during iteration (!)
                        auto infantry_copy = currentGameMap->getTile(i, j)->getInfantryList();
                        std::vector<ObjectBase *> killObjects;
                        for (const auto infantryID : infantry_copy) {
                            if (infantryID != getObjectID()) {
                                auto *const pObject = currentGame->getObjectManager().getObject(infantryID);
                                if (pObject->getLocation() == Coord(i, j)) {
                                    killObjects.push_back(pObject);
                                }
                            }
                        }

                        for (auto *pObject : killObjects) {
                            pObject->destroy();
                        }
                    }
                }


                // destroy captured structure ...
                pCapturedStructure->setHealth(0);
                currentGame->getObjectManager().removeObject(pCapturedStructure->getObjectID());

                // ... and create a new one
                StructureBase* pNewStructure = owner->placeStructure(NONE_ID, targetID, posX, posY, false, true);

                pNewStructure->setOriginalHouseID(origHouse);
                pNewStructure->setHealth(oldHealth);
                if (isSelected) {
                    pNewStructure->setSelected(true);
                    currentGame->getSelectedList().insert(pNewStructure->getObjectID());
                    currentGame->selectionChanged();
                }

                if (isSelectedByOtherPlayer) {
                    pNewStructure->setSelectedByOtherPlayer(true);
                    currentGame->getSelectedByOtherPlayerList().insert(pNewStructure->getObjectID());
                }

                if (containedUnitID != NONE_ID) {
                    UnitBase * pNewUnit = owner->createUnit(containedUnitID);

                    pNewUnit->setRespondable(false);
                    pNewUnit->setActive(false);
                    pNewUnit->setVisible(VIS_ALL, false);
                    pNewUnit->setHealth(containedUnitHealth);

                    if (pNewUnit->getItemID() == Unit_Harvester) {
                        static_cast<Harvester*>(pNewUnit)->setAmountOfSpice(containedHarvesterSpice);
                    }

                    if (pNewStructure->getItemID() == Structure_Refinery) {
                        auto *pRefinery = static_cast<Refinery*>(pNewStructure);
                        pRefinery->book();
                        pRefinery->assignHarvester(static_cast<Harvester*>(pNewUnit));
                    }
                    else if (pNewStructure->getItemID() == Structure_RepairYard) {
                        auto *pRepairYard = static_cast<RepairYard*>(pNewStructure);
                        pRepairYard->book();
                        pRepairYard->assignUnit(pNewUnit);
                    }
                }

                // steal credits
                pOwner->takeCredits(capturedSpice);
                owner->addCredits(capturedSpice, false);
                owner->updateBuildLists();

            }
            else {
                int damage = lround(std::min(pCapturedStructure->getHealth() / 2, getHealth() * 2));
                pCapturedStructure->handleDamage(damage, NONE_ID, getOwner());
            }
            // destroy unit indirectly
            setTarget(nullptr);
            setHealth(0);
            return;
        }
    }
    else if (target.getObjPointer() != nullptr && target.getObjPointer()->isAStructure()) {
        Coord   closestPoint;
        closestPoint = target.getObjPointer()->getClosestPoint(location);

        if (blockDistance(location, closestPoint) <= 0.5_fix) {
            // destroy unit indirectly
            setTarget(nullptr);
            setHealth(0);
            return;
        }
    }
}

void InfantryBase::destroy() {
    if(currentGameMap->tileExists(location) && isVisible()) {
        auto *pTile = currentGameMap->getTile(location);

        if(pTile->hasANonInfantryGroundObject()) {
            if(pTile->getNonInfantryGroundObject(currentGame->getObjectManager())->isAUnit()) {
                // squashed
                pTile->assignDeadUnit( currentGame->randomGen.randBool() ? DeadUnit_Infantry_Squashed1 : DeadUnit_Infantry_Squashed2,
                                            owner->getHouseID(),
                                            Coord(lround(realX), lround(realY)) );

                if(isVisible(getOwner()->getTeamID())) {
                    soundPlayer->playSoundAt(Sound_Squashed,location);
                }
            } else {
                // this unit has captured a building
            }

        } else if(getItemID() != Unit_Saboteur) {
            // "normal" dead
            pTile->assignDeadUnit(DeadUnit_Infantry,
                                        owner->getHouseID(),
                                        Coord(lround(realX), lround(realY)));

            if(isVisible(getOwner()->getTeamID())) {
                soundPlayer->playSoundAt(getRandomOf(Sound_Scream1,Sound_Scream2,Sound_Scream3,Sound_Scream4,Sound_Scream5,Sound_Trumpet),location);
            }
        }
    }

    GroundUnit::destroy();
}

void InfantryBase::move() {
    if(!moving && !justStoppedMoving && (((currentGame->getGameCycleCount() + getObjectID()) % 512) == 0)) {
        currentGameMap->viewMap(owner->getHouseID(), location, getViewRange());
    }

    if(moving && !justStoppedMoving) {
        realX += xSpeed;
        realY += ySpeed;


        // check if unit is on the first half of the way
        FixPoint fromDistanceX;
        FixPoint fromDistanceY;
        FixPoint toDistanceX;
        FixPoint toDistanceY;

        const FixPoint epsilon = 3.75_fix;

        if(location != nextSpot) {
            const auto abstractDistanceX = FixPoint::abs(location.x*TILESIZE + TILESIZE/2 - (realX-bumpyOffsetX));
            const auto abstractDistanceY = FixPoint::abs(location.y*TILESIZE + TILESIZE/2 - (realY-bumpyOffsetY));

            fromDistanceX = FixPoint::abs(location.x*TILESIZE + TILESIZE/2 + tilePositionOffset[oldTilePosition].x - (realX-bumpyOffsetX));
            fromDistanceY = FixPoint::abs(location.y*TILESIZE + TILESIZE/2 + tilePositionOffset[oldTilePosition].y - (realY-bumpyOffsetY));
            toDistanceX = FixPoint::abs(nextSpot.x*TILESIZE + TILESIZE/2 + tilePositionOffset[tilePosition].x - (realX-bumpyOffsetX));
            toDistanceY = FixPoint::abs(nextSpot.y*TILESIZE + TILESIZE/2 + tilePositionOffset[tilePosition].y - (realY-bumpyOffsetY));

            // check if unit is half way out of old tile
            if((abstractDistanceX >= TILESIZE/2 + epsilon) || (abstractDistanceY >= TILESIZE/2 + epsilon)) {
                // let something else go in
                unassignFromMap(location);
                oldLocation = location;
                location = nextSpot;

                currentGameMap->viewMap(owner->getHouseID(), location, getViewRange());
            }

        } else {
            fromDistanceX = FixPoint::abs(oldLocation.x*TILESIZE + TILESIZE/2 + tilePositionOffset[oldTilePosition].x - (realX-bumpyOffsetX));
            fromDistanceY = FixPoint::abs(oldLocation.y*TILESIZE + TILESIZE/2 + tilePositionOffset[oldTilePosition].y - (realY-bumpyOffsetY));
            toDistanceX = FixPoint::abs(location.x*TILESIZE + TILESIZE/2 + tilePositionOffset[tilePosition].x - (realX-bumpyOffsetX));
            toDistanceY = FixPoint::abs(location.y*TILESIZE + TILESIZE/2 + tilePositionOffset[tilePosition].y - (realY-bumpyOffsetY));

            Coord   wantedReal;
            wantedReal.x = nextSpot.x*TILESIZE + TILESIZE/2 + tilePositionOffset[tilePosition].x;
            wantedReal.y = nextSpot.y*TILESIZE + TILESIZE/2 + tilePositionOffset[tilePosition].y;

            if( (FixPoint::abs(wantedReal.x - (realX-bumpyOffsetX)) <= FixPoint::abs(xSpeed)/2 + epsilon)
                && (FixPoint::abs(wantedReal.y - (realY-bumpyOffsetY)) <= FixPoint::abs(ySpeed)/2 + epsilon) ) {
                realX = wantedReal.x;
                realY = wantedReal.y;
                bumpyOffsetX = 0;
                bumpyOffsetY = 0;

                if(forced && (location == destination) && !target) {
                    setForced(false);
                }

                moving = false;
                justStoppedMoving = true;

                oldLocation.invalidate();
            }
        }

        bumpyMovementOnRock(fromDistanceX, fromDistanceY, toDistanceX, toDistanceY);

    } else {
        justStoppedMoving = false;
    }

    checkPos();
}


void InfantryBase::setLocation(int xPos, int yPos) {
    if(currentGameMap->tileExists(xPos, yPos) || ((xPos == INVALID_POS) && (yPos == INVALID_POS))) {
        oldTilePosition = tilePosition = INVALID_POS;
        GroundUnit::setLocation(xPos, yPos);

        if(tilePosition != INVALID_POS) {
            realX += tilePositionOffset[tilePosition].x;
            realY += tilePositionOffset[tilePosition].y;
        }
    }
}

void InfantryBase::setSpeeds() {
    if(oldTilePosition == INVALID_POS) {
        SDL_Log("Warning: InfantryBase::setSpeeds(): Infantry tile position == INVALID_POS.");
    } else if(tilePosition == oldTilePosition) {
        // havent changed infantry position
        GroundUnit::setSpeeds();
    } else {

        int sx = tilePositionOffset[oldTilePosition].x;
        int sy = tilePositionOffset[oldTilePosition].y;

        int dx = 0;
        int dy = 0;
        // clang-format off
        switch(drawnAngle) {
            case ANGLETYPE::RIGHT:     dx += TILESIZE;                 break;
            case ANGLETYPE::RIGHTUP:   dx += TILESIZE; dy -= TILESIZE; break;
            case ANGLETYPE::UP:                        dy -= TILESIZE; break;
            case ANGLETYPE::LEFTUP:    dx -= TILESIZE; dy -= TILESIZE; break;
            case ANGLETYPE::LEFT:      dx -= TILESIZE;                 break;
            case ANGLETYPE::LEFTDOWN:  dx -= TILESIZE; dy += TILESIZE; break;
            case ANGLETYPE::DOWN:                      dy += TILESIZE; break;
            case ANGLETYPE::RIGHTDOWN: dx += TILESIZE; dy += TILESIZE; break;
        }
        // clang-format on

        if(tilePosition != INVALID_POS) {
            dx += tilePositionOffset[tilePosition].x;
            dy += tilePositionOffset[tilePosition].y;
        }

        dx -= sx;
        dy -= sy;

        FixPoint scale = currentGame->objectData.data[itemID][static_cast<int>(originalHouseID)].maxspeed/FixPoint::sqrt((dx*dx + dy*dy));
        xSpeed = dx*scale;
        ySpeed = dy*scale;
    }
}

void InfantryBase::squash() {
    destroy();
    return;
}

void InfantryBase::playConfirmSound() {
    soundPlayer->playVoice(getRandomOf(MovingOut,InfantryOut), getOwner()->getHouseID());
}

void InfantryBase::playSelectSound() {
    soundPlayer->playVoice(YesSir, getOwner()->getHouseID());
}
