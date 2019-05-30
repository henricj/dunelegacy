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

#include <structures/StructureBase.h>

#include <globals.h>

#include <House.h>
#include <Game.h>
#include <Map.h>
#include <ScreenBorder.h>
#include <Explosion.h>
#include <SoundPlayer.h>

#include <players/HumanPlayer.h>

#include <misc/draw_util.h>

#include <units/UnitBase.h>

#include <GUI/ObjectInterfaces/DefaultStructureInterface.h>

StructureBase::StructureBase(House* newOwner) : ObjectBase(newOwner) {
    StructureBase::init();

    repairing = false;
    fogged = false;
    degradeTimer = MILLI2CYCLES(15*1000);
}

StructureBase::StructureBase(InputStream& stream): ObjectBase(stream) {
    StructureBase::init();

    repairing = stream.readBool();
    fogged = stream.readBool();
    lastVisibleFrame = stream.readUint32();

    degradeTimer = stream.readSint32();

    size_t numSmoke = stream.readUint32();
    for(size_t i=0;i<numSmoke; i++) {
        smoke.emplace_back(stream);
    }
}

void StructureBase::init() {
    aStructure = true;

    structureSize.x = 0;
    structureSize.y = 0;

    justPlacedTimer = 0;

    lastVisibleFrame = firstAnimFrame = lastAnimFrame = curAnimFrame = 2;
    animationCounter = 0;

    structureList.push_back(this);
}

StructureBase::~StructureBase() {
    try {
        currentGameMap->removeObjectFromMap(getObjectID()); //no map point will reference now
        currentGame->getObjectManager().removeObject(getObjectID());
        structureList.remove(this);
        owner->decrementStructures(itemID, location);

        removeFromSelectionLists();

        for(int i=0; i < NUMSELECTEDLISTS; i++) {
            pLocalPlayer->getGroupList(i).erase(getObjectID());
        }
    } catch(std::exception& e) {
        SDL_Log("StructureBase::~StructureBase(): %s", e.what());
    }
}

void StructureBase::save(OutputStream& stream) const {
    ObjectBase::save(stream);

    stream.writeBool(repairing);
    stream.writeBool(fogged);
    stream.writeUint32(lastVisibleFrame);

    stream.writeSint32(degradeTimer);

    stream.writeUint32(smoke.size());
    for(const StructureSmoke& structureSmoke : smoke) {
        structureSmoke.save(stream);
    }
}

void StructureBase::assignToMap(const Coord& pos) {
    bool bFoundNonConcreteTile = false;

    Coord temp;
    for(int i = pos.x; i < pos.x + structureSize.x; i++) {
        for(int j = pos.y; j < pos.y + structureSize.y; j++) {
            if(currentGameMap->tileExists(i, j)) {
                Tile* pTile = currentGameMap->getTile(i,j);
                pTile->assignNonInfantryGroundObject(getObjectID());
                if(!pTile->isConcrete() && currentGame->getGameInitSettings().getGameOptions().concreteRequired && (currentGame->gameState != GameState::Start)) {
                    bFoundNonConcreteTile = true;

                    if((itemID != Structure_Wall) && (itemID != Structure_ConstructionYard)) {
                        setHealth(getHealth() - FixPoint(getMaxHealth())/(2*structureSize.x*structureSize.y));
                    }
                }
                pTile->setType(Terrain_Rock);
                pTile->setOwner(getOwner()->getHouseID());

                setVisible(VIS_ALL, true);
                setActive(true);
                setRespondable(true);
            }
        }
    }

    currentGameMap->viewMap(getOwner()->getHouseID(), pos, getViewRange());

    if(!bFoundNonConcreteTile && !currentGame->getGameInitSettings().getGameOptions().structuresDegradeOnConcrete) {
        degradeTimer = -1;
    }
}

void StructureBase::blitToScreen() {
    int index = fogged ? lastVisibleFrame : curAnimFrame;
    int indexX = index % numImagesX;
    int indexY = index / numImagesX;

    SDL_Rect dest = calcSpriteDrawingRect(  graphic[currentZoomlevel],
                                            screenborder->world2screenX(lround(realX)),
                                            screenborder->world2screenY(lround(realY)),
                                            numImagesX, numImagesY);
    SDL_Rect source = calcSpriteSourceRect(graphic[currentZoomlevel],indexX,numImagesX,indexY,numImagesY);

    SDL_RenderCopy(renderer, graphic[currentZoomlevel], &source, &dest);

    if(!fogged) {
        SDL_Texture* pSmokeTex = pGFXManager->getZoomedObjPic(ObjPic_Smoke, getOwner()->getHouseID(), currentZoomlevel);
        SDL_Rect smokeSource = calcSpriteSourceRect(pSmokeTex, 0, 3);
        for(const StructureSmoke& structureSmoke : smoke) {
            SDL_Rect smokeDest = calcSpriteDrawingRect( pSmokeTex,
                                                        screenborder->world2screenX(structureSmoke.realPos.x),
                                                        screenborder->world2screenY(structureSmoke.realPos.y),
                                                        3, 1, HAlign::Center, VAlign::Bottom);
            Uint32 cycleDiff = currentGame->getGameCycleCount() - structureSmoke.startGameCycle;

            Uint32 smokeFrame = (cycleDiff/25) % 4;
            if(smokeFrame == 3) {
                smokeFrame = 1;
            }

            smokeSource.x = smokeFrame * smokeSource.w;
            SDL_RenderCopy(renderer, pSmokeTex, &smokeSource, &smokeDest);
        }
    }
}

ObjectInterface* StructureBase::getInterfaceContainer() {
    if((pLocalHouse == owner) || (debug == true)) {
        return DefaultStructureInterface::create(objectID);
    } else {
        return DefaultObjectInterface::create(objectID);
    }
}

void StructureBase::drawSelectionBox() {
    SDL_Rect dest;
    dest.x = screenborder->world2screenX(realX);
    dest.y = screenborder->world2screenY(realY);
    dest.w = getWidth(graphic[currentZoomlevel])/numImagesX;
    dest.h = getHeight(graphic[currentZoomlevel])/numImagesY;

    //now draw the selection box thing, with parts at all corners of structure

    // top left bit
    for(int i=0;i<=currentZoomlevel;i++) {
        renderDrawHLine(renderer, dest.x+i, dest.y+i, dest.x+(currentZoomlevel+1)*3, COLOR_WHITE);
        renderDrawVLine(renderer, dest.x+i, dest.y+i, dest.y+(currentZoomlevel+1)*3, COLOR_WHITE);
    }

    // top right bit
    for(int i=0;i<=currentZoomlevel;i++) {
        renderDrawHLine(renderer, dest.x + dest.w-1 - i, dest.y+i, dest.x + dest.w-1 - (currentZoomlevel+1)*3, COLOR_WHITE);
        renderDrawVLine(renderer, dest.x + dest.w-1 - i, dest.y+i, dest.y+(currentZoomlevel+1)*3, COLOR_WHITE);
    }

    // bottom left bit
    for(int i=0;i<=currentZoomlevel;i++) {
        renderDrawHLine(renderer, dest.x+i, dest.y + dest.h-1 - i, dest.x+(currentZoomlevel+1)*3, COLOR_WHITE);
        renderDrawVLine(renderer, dest.x+i, dest.y + dest.h-1 - i, dest.y + dest.h-1 - (currentZoomlevel+1)*3, COLOR_WHITE);
    }

    // bottom right bit
    for(int i=0;i<=currentZoomlevel;i++) {
        renderDrawHLine(renderer, dest.x + dest.w-1 - i, dest.y + dest.h-1 - i, dest.x + dest.w-1 - (currentZoomlevel+1)*3, COLOR_WHITE);
        renderDrawVLine(renderer, dest.x + dest.w-1 - i, dest.y + dest.h-1 - i, dest.y + dest.h-1 - (currentZoomlevel+1)*3, COLOR_WHITE);
    }

    // health bar
    for(int i=1;i<=currentZoomlevel+1;i++) {
        renderDrawHLine(renderer, dest.x, dest.y-i-1, dest.x + (lround((getHealth()/getMaxHealth())*(world2zoomedWorld(TILESIZE)*structureSize.x - 1))), getHealthColor());
    }
}

void StructureBase::drawOtherPlayerSelectionBox() {
    SDL_Rect dest;
    dest.x = screenborder->world2screenX(realX) + (currentZoomlevel+1);
    dest.y = screenborder->world2screenY(realY) + (currentZoomlevel+1);
    dest.w = getWidth(graphic[currentZoomlevel])/numImagesX - 2*(currentZoomlevel+1);
    dest.h = getHeight(graphic[currentZoomlevel])/numImagesY - 2*(currentZoomlevel+1);

    //now draw the selection box thing, with parts at all corners of structure

    // top left bit
    for(int i=0;i<=currentZoomlevel;i++) {
        renderDrawHLine(renderer, dest.x+i, dest.y+i, dest.x+(currentZoomlevel+1)*2, COLOR_LIGHTBLUE);
        renderDrawVLine(renderer, dest.x+i, dest.y+i, dest.y+(currentZoomlevel+1)*2, COLOR_LIGHTBLUE);
    }

    // top right bit
    for(int i=0;i<=currentZoomlevel;i++) {
        renderDrawHLine(renderer, dest.x + dest.w-1 - i, dest.y+i, dest.x + dest.w-1 - (currentZoomlevel+1)*2, COLOR_LIGHTBLUE);
        renderDrawVLine(renderer, dest.x + dest.w-1 - i, dest.y+i, dest.y+(currentZoomlevel+1)*2, COLOR_LIGHTBLUE);
    }

    // bottom left bit
    for(int i=0;i<=currentZoomlevel;i++) {
        renderDrawHLine(renderer, dest.x+i, dest.y + dest.h-1 - i, dest.x+(currentZoomlevel+1)*2, COLOR_LIGHTBLUE);
        renderDrawVLine(renderer, dest.x+i, dest.y + dest.h-1 - i, dest.y + dest.h-1 - (currentZoomlevel+1)*2, COLOR_LIGHTBLUE);
    }

    // bottom right bit
    for(int i=0;i<=currentZoomlevel;i++) {
        renderDrawHLine(renderer, dest.x + dest.w-1 - i, dest.y + dest.h-1 - i, dest.x + dest.w-1 - (currentZoomlevel+1)*2, COLOR_LIGHTBLUE);
        renderDrawVLine(renderer, dest.x + dest.w-1 - i, dest.y + dest.h-1 - i, dest.y + dest.h-1 - (currentZoomlevel+1)*2, COLOR_LIGHTBLUE);
    }
}

void StructureBase::drawGatheringPointLine() {
    if(isABuilder() && (getItemID() != Structure_ConstructionYard) && destination.isValid() && (getOwner() == pLocalHouse)) {
        Coord indicatorPosition = destination*TILESIZE + Coord(TILESIZE/2, TILESIZE/2);
        Coord structurePosition = getCenterPoint();

        renderDrawLine( renderer,
                        screenborder->world2screenX(structurePosition.x), screenborder->world2screenY(structurePosition.y),
                        screenborder->world2screenX(indicatorPosition.x), screenborder->world2screenY(indicatorPosition.y),
                        COLOR_HALF_TRANSPARENT);


        SDL_Texture* pUIIndicator = pGFXManager->getUIGraphic(UI_Indicator);
        SDL_Rect source = calcSpriteSourceRect(pUIIndicator, 0, 3);
        SDL_Rect drawLocation = calcSpriteDrawingRect(  pUIIndicator,
                                                        screenborder->world2screenX(indicatorPosition.x),
                                                        screenborder->world2screenY(indicatorPosition.y),
                                                        3, 1,
                                                        HAlign::Center, VAlign::Center);

        // Render twice
        SDL_RenderCopy(renderer, pUIIndicator, &source, &drawLocation);
        SDL_RenderCopy(renderer, pUIIndicator, &source, &drawLocation);
    }
}

/**
    Returns the center point of this structure
    \return the center point in world coordinates
*/
Coord StructureBase::getCenterPoint() const {
    return Coord( lround(realX + structureSize.x*TILESIZE/2),
                  lround(realY + structureSize.y*TILESIZE/2));
}

Coord StructureBase::getClosestCenterPoint(const Coord& objectLocation) const {
    return getClosestPoint(objectLocation) * TILESIZE + Coord(TILESIZE/2, TILESIZE/2);
}

void StructureBase::handleActionClick(int xPos, int yPos) {
    if ((xPos < location.x) || (xPos >= (location.x + structureSize.x)) || (yPos < location.y) || (yPos >= (location.y + structureSize.y))) {
        currentGame->getCommandManager().addCommand(Command(pLocalPlayer->getPlayerID(), CMD_STRUCTURE_SETDEPLOYPOSITION,objectID, (Uint32) xPos, (Uint32) yPos));
    } else {
        currentGame->getCommandManager().addCommand(Command(pLocalPlayer->getPlayerID(), CMD_STRUCTURE_SETDEPLOYPOSITION,objectID, (Uint32) NONE_ID, (Uint32) NONE_ID));
    }
}

void StructureBase::handleRepairClick() {
    currentGame->getCommandManager().addCommand(Command(pLocalPlayer->getPlayerID(), CMD_STRUCTURE_REPAIR,objectID));
}

void StructureBase::doSetDeployPosition(int xPos, int yPos) {
    setTarget(nullptr);
    setDestination(xPos,yPos);
    setForced(true);
}


void StructureBase::doRepair() {
    repairing = true;
}

void StructureBase::setDestination(int newX, int newY) {
    if(currentGameMap->tileExists(newX, newY) || ((newX == INVALID_POS) && (newY == INVALID_POS))) {
        destination.x = newX;
        destination.y = newY;
    }
}

void StructureBase::setJustPlaced() {
    justPlacedTimer = 6;
    curAnimFrame = 0;
    animationCounter = -STRUCTURE_ANIMATIONTIMER; // make first build animation double as long
}

bool StructureBase::update() {
    if(((currentGame->getGameCycleCount() + getObjectID()) % 512) == 0) {
        currentGameMap->viewMap(owner->getHouseID(), location, getViewRange());
    }

    if(!fogged) {
        lastVisibleFrame = curAnimFrame;
    }

    // degrade
    if((degradeTimer >= 0) && currentGame->getGameInitSettings().getGameOptions().concreteRequired && (owner->getPowerRequirement() > owner->getProducedPower())) {
        degradeTimer--;
        if(degradeTimer <= 0) {
            degradeTimer = MILLI2CYCLES(15*1000);

            int damageMultiplyer = 1;
            if(owner->getHouseID() == HOUSE_HARKONNEN || owner->getHouseID() == HOUSE_SARDAUKAR) {
                damageMultiplyer = 3;
            } else if(owner->getHouseID() == HOUSE_ORDOS) {
                damageMultiplyer = 2;
            } else if(owner->getHouseID() == HOUSE_MERCENARY) {
                damageMultiplyer = 5;
            }

            if(getHealth() > getMaxHealth() / 2) {
                setHealth( getHealth() - FixPoint(damageMultiplyer * getMaxHealth())/100);
            }
        }
    }

    updateStructureSpecificStuff();

    if(getHealth() <= 0) {
        destroy();
        return false;
    }

    if(repairing) {
        if(owner->getCredits() >= 5) {
            // Original dune 2 is doing the repair calculation with fix-point math (multiply everything with 256).
            // It is calculating what fraction 2 hitpoints of the maximum health would be.
            int fraction = (2*256)/getMaxHealth();
            FixPoint repairprice = FixPoint(fraction * currentGame->objectData.data[itemID][originalHouseID].price) / 256;

            // Original dune is always repairing 5 hitpoints (for the costs of 2) but we are only repairing 1/30th of that
            const auto repairHealth = 5_fix/30_fix;
            owner->takeCredits(repairprice/30);
            FixPoint newHealth = getHealth() + repairHealth;
            if(newHealth >= getMaxHealth()) {
                setHealth(getMaxHealth());
                repairing = false;
            } else {
                setHealth(newHealth);
            }
        } else {
            repairing = false;
        }
    } else if(owner->isAI() && (getHealth() < getMaxHealth()/2)) {
        doRepair();
    }

    // check smoke
    std::list<StructureSmoke>::iterator iter = smoke.begin();
    while(iter != smoke.end()) {
        if(currentGame->getGameCycleCount() - iter->startGameCycle >= MILLI2CYCLES(8*1000)) {
            smoke.erase(iter++);
        } else {
            ++iter;
        }
    }

    // update animations
    animationCounter++;
    if(animationCounter > STRUCTURE_ANIMATIONTIMER) {
        animationCounter = 0;
        curAnimFrame++;
        if((curAnimFrame < firstAnimFrame) || (curAnimFrame > lastAnimFrame)) {
            curAnimFrame = firstAnimFrame;
        }

        justPlacedTimer--;
        if((justPlacedTimer > 0) && (justPlacedTimer % 2 == 0)) {
            curAnimFrame = 0;
        }
    }

    return true;
}

void StructureBase::destroy() {
    int*    pDestroyedStructureTiles = nullptr;
    int     DestroyedStructureTilesSizeY = 0;
    static int DestroyedStructureTilesWall[] = { DestroyedStructure_Wall };
    static int DestroyedStructureTiles1x1[] = { Destroyed1x1Structure };
    static int DestroyedStructureTiles2x2[] = { Destroyed2x2Structure_TopLeft, Destroyed2x2Structure_TopRight,
                                                Destroyed2x2Structure_BottomLeft, Destroyed2x2Structure_BottomRight };
    static int DestroyedStructureTiles3x2[] = { Destroyed3x2Structure_TopLeft, Destroyed3x2Structure_TopCenter, Destroyed3x2Structure_TopRight,
                                                Destroyed3x2Structure_BottomLeft, Destroyed3x2Structure_BottomCenter, Destroyed3x2Structure_BottomRight};
    static int DestroyedStructureTiles3x3[] = { Destroyed3x3Structure_TopLeft, Destroyed3x3Structure_TopCenter, Destroyed3x3Structure_TopRight,
                                                Destroyed3x3Structure_CenterLeft, Destroyed3x3Structure_CenterCenter, Destroyed3x3Structure_CenterRight,
                                                Destroyed3x3Structure_BottomLeft, Destroyed3x3Structure_BottomCenter, Destroyed3x3Structure_BottomRight};


    if(itemID == Structure_Wall) {
        pDestroyedStructureTiles = DestroyedStructureTilesWall;
        DestroyedStructureTilesSizeY = 1;
    } else {
        switch(structureSize.y) {
            case 1: {
                pDestroyedStructureTiles = DestroyedStructureTiles1x1;
                DestroyedStructureTilesSizeY = 1;
            } break;

            case 2: {
                if(structureSize.x == 2) {
                    pDestroyedStructureTiles = DestroyedStructureTiles2x2;
                    DestroyedStructureTilesSizeY = 2;
                } else if(structureSize.x == 3) {
                    pDestroyedStructureTiles = DestroyedStructureTiles3x2;
                    DestroyedStructureTilesSizeY = 3;
                } else {
                    THROW(std::runtime_error, "StructureBase::destroy(): Invalid structure size");
                }
            } break;

            case 3: {
                pDestroyedStructureTiles = DestroyedStructureTiles3x3;
                DestroyedStructureTilesSizeY = 3;
            } break;

            default: {
                THROW(std::runtime_error, "StructureBase::destroy(): Invalid structure size");
            } break;
        }
    }

    if(itemID != Structure_Wall) {
        for(int j = 0; j < structureSize.y; j++) {
            for(int i = 0; i < structureSize.x; i++) {
                Tile* pTile = currentGameMap->getTile(location.x + i, location.y + j);
                pTile->setDestroyedStructureTile(pDestroyedStructureTiles[DestroyedStructureTilesSizeY*j + i]);

                Coord position((location.x+i)*TILESIZE + TILESIZE/2, (location.y+j)*TILESIZE + TILESIZE/2);
                Uint32 explosionID = currentGame->randomGen.getRandOf({Explosion_Large1,Explosion_Large2});
                currentGame->getExplosionList().push_back(new Explosion(explosionID, position, owner->getHouseID()) );

                if(currentGame->randomGen.rand(1,100) <= getInfSpawnProp()) {
                    UnitBase* pNewUnit = owner->createUnit(Unit_Soldier);
                    pNewUnit->setHealth(pNewUnit->getMaxHealth()/2);
                    pNewUnit->deploy(location + Coord(i,j));
                }
            }
        }
    }

    if(isVisible(pLocalHouse->getTeamID()))
        soundPlayer->playSoundAt(Sound_ExplosionStructure, location);


    delete this;
}

Coord StructureBase::getClosestPoint(const Coord& objectLocation) const {
    Coord closestPoint;

    // find the closest tile of a structure from a location
    if(objectLocation.x <= location.x) {
        // if we are left of the structure
        // set destination, left most point
        closestPoint.x = location.x;
    } else if(objectLocation.x >= (location.x + structureSize.x-1)) {
        //vica versa
        closestPoint.x = location.x + structureSize.x-1;
    } else {
        //we are above or below at least one tile of the structure, closest path is straight
        closestPoint.x = objectLocation.x;
    }

    //same deal but with y
    if(objectLocation.y <= location.y) {
        closestPoint.y = location.y;
    } else if(objectLocation.y >= (location.y + structureSize.y-1)) {
        closestPoint.y = location.y + structureSize.y-1;
    } else {
        closestPoint.y = objectLocation.y;
    }

    return closestPoint;
}
