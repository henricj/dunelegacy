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


InfantryBase::InfantryBase(House* newOwner) : GroundUnit(newOwner) {

    InfantryBase::init();

    setHealth(getMaxHealth());

    tilePosition = INVALID_POS;
    oldTilePosition = INVALID_POS;
}

InfantryBase::InfantryBase(InputStream& stream) : GroundUnit(stream) {

    InfantryBase::init();

    tilePosition = stream.readSint8();
    oldTilePosition = stream.readSint8();
}

void InfantryBase::init() {
    infantry = true;
    walkFrame = 0;
}

InfantryBase::~InfantryBase() = default;


void InfantryBase::save(OutputStream& stream) const {

    GroundUnit::save(stream);

    stream.writeSint8(tilePosition);
    stream.writeSint8(oldTilePosition);
}

void InfantryBase::handleCaptureClick(int xPos, int yPos) {
    if(respondable && ((getItemID() == Unit_Soldier) || (getItemID() == Unit_Trooper))) {
        if (currentGameMap->tileExists(xPos, yPos)) {
            if (currentGameMap->getTile(xPos,yPos)->hasAnObject()) {
                // capture structure
                ObjectBase* tempTarget = currentGameMap->getTile(xPos,yPos)->getObject();

                currentGame->getCommandManager().addCommand(Command(pLocalPlayer->getPlayerID(), CMD_INFANTRY_CAPTURE,objectID,tempTarget->getObjectID()));
            }
        }
    }

}

void InfantryBase::doCaptureStructure(Uint32 targetStructureID) {
    const StructureBase* pStructure = dynamic_cast<StructureBase*>(currentGame->getObjectManager().getObject(targetStructureID));
    doCaptureStructure(pStructure);
}

void InfantryBase::doCaptureStructure(const StructureBase* pStructure) {

    if((pStructure == nullptr) || (pStructure->canBeCaptured() == false) || (pStructure->getOwner()->getTeamID() == getOwner()->getTeamID())) {
        // does not exist anymore, cannot be captured or is a friendly building
        return;
    }

    doAttackObject(pStructure, true);
    doSetAttackMode(CAPTURE);
}

void InfantryBase::assignToMap(const Coord& pos) {
    if(currentGameMap->tileExists(pos)) {
        oldTilePosition = tilePosition;
        tilePosition = currentGameMap->getTile(pos)->assignInfantry(getObjectID());
        currentGameMap->viewMap(owner->getHouseID(), pos, getViewRange());
    }
}

void InfantryBase::blitToScreen() {
    SDL_Rect dest = calcSpriteDrawingRect(  graphic[currentZoomlevel],
                                            screenborder->world2screenX(realX),
                                            screenborder->world2screenY(realY),
                                            numImagesX, numImagesY,
                                            HAlign::Center, VAlign::Center);

    int temp = drawnAngle;
    if(temp == UP) {
        temp = 1;
    } else if (temp == DOWN) {
        temp = 3;
    } else if (temp == LEFTUP || temp == LEFTDOWN || temp == LEFT) {
        temp = 2;
    } else {
        //RIGHT
        temp = 0;
    }

    SDL_Rect source = calcSpriteSourceRect(graphic[currentZoomlevel], temp, numImagesX, (walkFrame/10 == 3) ? 1 : walkFrame/10, numImagesY);

    SDL_RenderCopy(renderer, graphic[currentZoomlevel], &source, &dest);
}

bool InfantryBase::canPass(int xPos, int yPos) const {
    bool passable = false;
    if(currentGameMap->tileExists(xPos, yPos)) {
        Tile* pTile = currentGameMap->getTile(xPos, yPos);
        if(!pTile->hasAGroundObject()) {
            if(pTile->getType() != Terrain_Mountain) {
                passable = true;
            } else {
                /* if this unit is infantry so can climb, and tile can take more infantry */
                if(pTile->infantryNotFull()) {
                    passable = true;
                }
            }
        } else {
            ObjectBase *object = pTile->getGroundObject();

            if((object != nullptr) && (object->getObjectID() == target.getObjectID())
                && object->isAStructure()
                && (object->getOwner()->getTeamID() != owner->getTeamID())
                && object->isVisible(getOwner()->getTeamID())) {
                passable = true;
            } else {
                passable = (!pTile->hasANonInfantryGroundObject()
                            && (pTile->infantryNotFull()
                            && (pTile->getInfantryTeam() == getOwner()->getTeamID())));
            }
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

    if(justStoppedMoving) {
        walkFrame = 0;

        if(currentGameMap->getTile(location)->isSpiceBloom()) {
            setHealth(0);
            currentGameMap->getTile(location)->triggerSpiceBloom(getOwner());
        } else if(currentGameMap->getTile(location)->isSpecialBloom()){
            currentGameMap->getTile(location)->triggerSpecialBloom(getOwner());
        }

        //check to see if close enough to blow up target
        if(target.getObjPointer() != nullptr
            && target.getObjPointer()->isAStructure()
            && (getOwner()->getTeamID() != target.getObjPointer()->getOwner()->getTeamID()))
        {
            Coord   closestPoint;

            closestPoint = target.getObjPointer()->getClosestPoint(location);

            if(blockDistance(location, closestPoint) <= 0.5_fix) {
                StructureBase* pCapturedStructure = target.getStructurePointer();
                if(pCapturedStructure->getHealthColor() == COLOR_RED) {
                    House* pOwner = pCapturedStructure->getOwner();
                    int targetID = pCapturedStructure->getItemID();
                    int posX = pCapturedStructure->getX();
                    int posY = pCapturedStructure->getY();
                    int origHouse = pCapturedStructure->getOriginalHouseID();
                    int oldHealth = lround(pCapturedStructure->getHealth());
                    bool isSelected = pCapturedStructure->isSelected();
                    bool isSelectedByOtherPlayer = pCapturedStructure->isSelectedByOtherPlayer();

                    FixPoint capturedSpice = 0;

                    UnitBase* pContainedUnit = nullptr;

                    if(pCapturedStructure->getItemID() == Structure_Silo) {
                        capturedSpice = currentGame->objectData.data[Structure_Silo][originalHouseID].capacity * (pOwner->getStoredCredits() / pOwner->getCapacity());
                    } else if(pCapturedStructure->getItemID() == Structure_Refinery) {
                        capturedSpice = currentGame->objectData.data[Structure_Silo][originalHouseID].capacity * (pOwner->getStoredCredits() / pOwner->getCapacity());
                        Refinery* pRefinery = static_cast<Refinery*>(pCapturedStructure);
                        if(pRefinery->isFree() == false) {
                            pContainedUnit = pRefinery->getHarvester();
                        }
                    } else if(pCapturedStructure->getItemID() == Structure_RepairYard) {
                        RepairYard* pRepairYard = static_cast<RepairYard*>(pCapturedStructure);
                        if(pRepairYard->isFree() == false) {
                            pContainedUnit = pRepairYard->getRepairUnit();
                        }
                    }

                    Uint32 containedUnitID = NONE_ID;
                    FixPoint containedUnitHealth = 0;
                    FixPoint containedHarvesterSpice = 0;
                    if(pContainedUnit != nullptr) {
                        containedUnitID = pContainedUnit->getItemID();
                        containedUnitHealth = pContainedUnit->getHealth();
                        if(containedUnitID == Unit_Harvester) {
                            containedHarvesterSpice = static_cast<Harvester*>(pContainedUnit)->getAmountOfSpice();
                        }

                        // will be destroyed by the captured structure
                        pContainedUnit = nullptr;
                    }

                    // remove all other infantry units capturing this building
                    Coord capturedStructureLocation = pCapturedStructure->getLocation();
                    for(int i = capturedStructureLocation.x; i < capturedStructureLocation.x + pCapturedStructure->getStructureSizeX(); i++) {
                        for(int j = capturedStructureLocation.y; j < capturedStructureLocation.y + pCapturedStructure->getStructureSizeY(); j++) {

                            // make a copy of infantry list to avoid problems of modifying the list during iteration (!)
                            const std::list<Uint32> infantryList = currentGameMap->getTile(i,j)->getInfantryList();
                            for(const Uint32& infantryID : infantryList) {
                                if(infantryID != getObjectID()) {
                                    ObjectBase* pObject = currentGame->getObjectManager().getObject(infantryID);
                                    if(pObject->getLocation() == Coord(i,j)) {
                                        pObject->destroy();
                                    }
                                }
                            }

                        }
                    }


                    // destroy captured structure ...
                    pCapturedStructure->setHealth(0);
                    delete pCapturedStructure;

                    // ... and create a new one
                    StructureBase* pNewStructure = owner->placeStructure(NONE_ID, targetID, posX, posY, false, true);

                    pNewStructure->setOriginalHouseID(origHouse);
                    pNewStructure->setHealth(oldHealth);
                    if(isSelected == true) {
                        pNewStructure->setSelected(true);
                        currentGame->getSelectedList().insert(pNewStructure->getObjectID());
                        currentGame->selectionChanged();
                    }

                    if(isSelectedByOtherPlayer == true) {
                        pNewStructure->setSelectedByOtherPlayer(true);
                        currentGame->getSelectedByOtherPlayerList().insert(pNewStructure->getObjectID());
                    }

                    if(containedUnitID != NONE_ID) {
                        UnitBase* pNewUnit = owner->createUnit(containedUnitID);

                        pNewUnit->setRespondable(false);
                        pNewUnit->setActive(false);
                        pNewUnit->setVisible(VIS_ALL, false);
                        pNewUnit->setHealth(containedUnitHealth);

                        if(pNewUnit->getItemID() == Unit_Harvester) {
                            static_cast<Harvester*>(pNewUnit)->setAmountOfSpice(containedHarvesterSpice);
                        }

                        if(pNewStructure->getItemID() == Structure_Refinery) {
                            Refinery* pRefinery = static_cast<Refinery*>(pNewStructure);
                            pRefinery->book();
                            pRefinery->assignHarvester(static_cast<Harvester*>(pNewUnit));
                        } else if(pNewStructure->getItemID() == Structure_RepairYard) {
                            RepairYard* pRepairYard = static_cast<RepairYard*>(pNewStructure);
                            pRepairYard->book();
                            pRepairYard->assignUnit(pNewUnit);
                        }
                    }

                    // steal credits
                    pOwner->takeCredits(capturedSpice);
                    owner->addCredits(capturedSpice, false);
                    owner->updateBuildLists();

                } else {
                    int damage = lround(std::min(pCapturedStructure->getHealth()/2, getHealth()*2));
                    pCapturedStructure->handleDamage(damage, NONE_ID, getOwner());
                }
                // destroy unit indirectly
                setTarget(nullptr);
                setHealth(0);
                return;
            }
        } else if(target.getObjPointer() != nullptr && target.getObjPointer()->isAStructure())  {
            Coord   closestPoint;
            closestPoint = target.getObjPointer()->getClosestPoint(location);

            if(blockDistance(location, closestPoint) <= 0.5_fix) {
                // destroy unit indirectly
                setTarget(nullptr);
                setHealth(0);
                return;
            }
        }
    }
}

void InfantryBase::destroy() {
    if(currentGameMap->tileExists(location) && isVisible()) {
        Tile* pTile = currentGameMap->getTile(location);

        if(pTile->hasANonInfantryGroundObject() == true) {
            if(pTile->getNonInfantryGroundObject()->isAUnit()) {
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
            pTile->assignDeadUnit( DeadUnit_Infantry,
                                        owner->getHouseID(),
                                        Coord(lround(realX), lround(realY)));

            if(isVisible(getOwner()->getTeamID())) {
                soundPlayer->playSoundAt(getRandomOf({Sound_Scream1,Sound_Scream2,Sound_Scream3,Sound_Scream4,Sound_Scream5,Sound_Trumpet}),location);
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
            FixPoint abstractDistanceX = FixPoint::abs(location.x*TILESIZE + TILESIZE/2 - (realX-bumpyOffsetX));
            FixPoint abstractDistanceY = FixPoint::abs(location.y*TILESIZE + TILESIZE/2 - (realY-bumpyOffsetY));

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
        switch(drawnAngle) {
            case RIGHT:     dx += TILESIZE;                 break;
            case RIGHTUP:   dx += TILESIZE; dy -= TILESIZE; break;
            case UP:                        dy -= TILESIZE; break;
            case LEFTUP:    dx -= TILESIZE; dy -= TILESIZE; break;
            case LEFT:      dx -= TILESIZE;                 break;
            case LEFTDOWN:  dx -= TILESIZE; dy += TILESIZE; break;
            case DOWN:                      dy += TILESIZE; break;
            case RIGHTDOWN: dx += TILESIZE; dy += TILESIZE; break;
        }

        if(tilePosition != INVALID_POS) {
            dx += tilePositionOffset[tilePosition].x;
            dy += tilePositionOffset[tilePosition].y;
        }

        dx -= sx;
        dy -= sy;

        FixPoint scale = currentGame->objectData.data[itemID][originalHouseID].maxspeed/FixPoint::sqrt((dx*dx + dy*dy));
        xSpeed = dx*scale;
        ySpeed = dy*scale;
    }
}

void InfantryBase::squash() {
    destroy();
    return;
}

void InfantryBase::playConfirmSound() {
    soundPlayer->playVoice(getRandomOf({MovingOut,InfantryOut}), getOwner()->getHouseID());
}

void InfantryBase::playSelectSound() {
    soundPlayer->playVoice(YesSir, getOwner()->getHouseID());
}
