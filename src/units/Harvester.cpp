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

#include <units/Harvester.h>

#include <globals.h>

#include <FileClasses/GFXManager.h>
#include <House.h>
#include <Game.h>
#include <Map.h>
#include <Explosion.h>
#include <SoundPlayer.h>
#include <ScreenBorder.h>

#include <players/HumanPlayer.h>
#include <players/QuantBot.h>

#include <structures/Refinery.h>

#include <misc/draw_util.h>

#include <algorithm>

/* how often is the same sandframe redrawn */
#define HARVESTERDELAY 30

/* how often to change harvester position while harvesting */
#define RANDOMHARVESTMOVE 500

/* how much is the harvester movement slowed down when full  */
#define MAXIMUMHARVESTERSLOWDOWN 0.4_fix

/* number spice output frames - 1 */
#define LASTSANDFRAME 2

Harvester::Harvester(House* newOwner) : TrackedUnit(newOwner)
{
    Harvester::init();

    setHealth(getMaxHealth());

    spice = 0;
    harvestingMode = false;
    returningToRefinery = false;
    spiceCheckCounter = 0;

    attackMode = GUARD;
}

Harvester::Harvester(InputStream& stream) : TrackedUnit(stream)
{
    Harvester::init();

    harvestingMode = stream.readBool();
    returningToRefinery = stream.readBool();
    spice = stream.readFixPoint();
    spiceCheckCounter = stream.readUint32();
}

void Harvester::init()
{
    itemID = Unit_Harvester;
    owner->incrementUnits(itemID);

    canAttackStuff = false;

    graphicID = ObjPic_Harvester;
    graphic = pGFXManager->getObjPic(graphicID,getOwner()->getHouseID());

    numImagesX = NUM_ANGLES;
    numImagesY = 1;
}

Harvester::~Harvester() = default;

void Harvester::save(OutputStream& stream) const
{
    TrackedUnit::save(stream);
    stream.writeBool(harvestingMode);
    stream.writeBool(returningToRefinery);
    stream.writeFixPoint(spice);
    stream.writeUint32(spiceCheckCounter);
}

void Harvester::blitToScreen()
{
    int x = screenborder->world2screenX(realX);
    int y = screenborder->world2screenY(realY);

    SDL_Texture* pUnitGraphic = graphic[currentZoomlevel];
    SDL_Rect source = calcSpriteSourceRect(pUnitGraphic, drawnAngle, numImagesX);
    SDL_Rect dest = calcSpriteDrawingRect( pUnitGraphic, x, y, numImagesX, 1, HAlign::Center, VAlign::Center);

    SDL_RenderCopy(renderer, pUnitGraphic, &source, &dest);

    if(isHarvesting() == true) {

        const Coord harvesterSandOffset[] = {   Coord(-56, 4),
                                                Coord(-28, 20),
                                                Coord(0, 24),
                                                Coord(28, 20),
                                                Coord(56, 4),
                                                Coord(40, -24),
                                                Coord(0, -36),
                                                Coord(-36, -24)
                                            };

        SDL_Texture* pSandGraphic = pGFXManager->getZoomedObjPic(ObjPic_Harvester_Sand, getOwner()->getHouseID(), currentZoomlevel);

        int frame = ((currentGame->getGameCycleCount() + (getObjectID() * 10)) / HARVESTERDELAY) % (2*LASTSANDFRAME);
        if(frame > LASTSANDFRAME) {
            frame -= LASTSANDFRAME;
        }

        SDL_Rect sandSource = calcSpriteSourceRect(pSandGraphic, drawnAngle, NUM_ANGLES, frame, LASTSANDFRAME+1);
        SDL_Rect sandDest = calcSpriteDrawingRect(  pSandGraphic,
                                                    screenborder->world2screenX(realX + harvesterSandOffset[drawnAngle].x),
                                                    screenborder->world2screenY(realY + harvesterSandOffset[drawnAngle].y),
                                                    NUM_ANGLES, LASTSANDFRAME+1,
                                                    HAlign::Center, VAlign::Center);

        SDL_RenderCopy(renderer, pSandGraphic, &sandSource, &sandDest);
    }

    if(isBadlyDamaged()) {
        drawSmoke(x, y);
    }
}

void Harvester::checkPos()
{
    TrackedUnit::checkPos();

    if(attackMode == STOP) {
        harvestingMode = false;

        if(getOwner()->isAI()){
            doSetAttackMode(HARVEST);
        } /*The AI doesn't like STOP*/
    }

    if(active)  {
        if (returningToRefinery) {
            if (target && (target.getObjPointer() != nullptr) && (target.getObjPointer()->getItemID() == Structure_Refinery)) {
                Refinery* pRefinery = static_cast<Refinery*>(target.getObjPointer());
                Tile* pTile = currentGameMap->getTile(location);
                ObjectBase *pObject = pTile->getGroundObject();

                if( justStoppedMoving
                    && (pObject != nullptr)
                    && (pObject->getObjectID() == target.getObjectID()) )
                {
                    if(pRefinery->isFree()) {
                        awaitingPickup = false;
                        setReturned();
                    } else {
                        // the repair yard is already in use by some other unit => move out
                        Coord newDestination = currentGameMap->findDeploySpot(this, target.getObjPointer()->getLocation(), currentGame->randomGen, getLocation(), pRefinery->getStructureSize());
                        doMove2Pos(newDestination, true);
                        requestCarryall();
                    }
                } else if(!awaitingPickup && owner->hasCarryalls() && pRefinery->isFree() && blockDistance(location, pRefinery->getClosestPoint(location)) >= MIN_CARRYALL_LIFT_DISTANCE) {
                    requestCarryall();
                }


            } else {
                int leastNumBookings = std::numeric_limits<int>::max(); //huge amount so refinery couldn't possibly compete with any refinery num bookings
                FixPoint closestLeastBookedRefineryDistance = FixPt32_MAX;
                Refinery* pBestRefinery = nullptr;

                for(StructureBase* pStructure : structureList) {
                    if((pStructure->getItemID() == Structure_Refinery) && (pStructure->getOwner() == owner)) {
                        Refinery* pRefinery = static_cast<Refinery*>(pStructure);
                        Coord closestPoint = pRefinery->getClosestPoint(location);
                        FixPoint refineryDistance = blockDistance(location, closestPoint);

                        if (pRefinery->getNumBookings() < leastNumBookings) {
                            leastNumBookings = pRefinery->getNumBookings();
                            closestLeastBookedRefineryDistance = refineryDistance;
                            pBestRefinery = pRefinery;
                        } else if (pRefinery->getNumBookings() == leastNumBookings) {
                            if (refineryDistance < closestLeastBookedRefineryDistance) {
                                closestLeastBookedRefineryDistance = refineryDistance;
                                pBestRefinery = pRefinery;
                            }
                        }
                    }
                }

                if (pBestRefinery) {
                    doMove2Object(pBestRefinery);
                    pBestRefinery->startAnimate();
                } else {
                    setDestination(location);
                }
            }
        } else if (harvestingMode && !hasBookedCarrier() && destination.isValid() && (blockDistance(location, destination) >= MIN_CARRYALL_LIFT_DISTANCE)) {
            requestCarryall();
        } else if(respondable && !harvestingMode && attackMode != STOP) {
            if(spiceCheckCounter == 0) {
                // Find harvest location nearest to our base
                Coord newDestination;
                if(currentGameMap->findSpice(newDestination, guardPoint)) {
                    setDestination(newDestination);
                    setGuardPoint(newDestination);
                    harvestingMode = true;
                } else {
                    setDestination(location);
                    setGuardPoint(location);
                    harvestingMode = false;
                }
                spiceCheckCounter = 100;
            } else {
                spiceCheckCounter--;
            }
        }
    }
}

void Harvester::deploy(const Coord& newLocation)
{
    if(currentGameMap->tileExists(newLocation)) {
        TrackedUnit::deploy(newLocation);
        if(spice == 0) {
            Coord newDestination;
            if((attackMode != STOP) && currentGameMap->findSpice(newDestination, guardPoint)) {
                harvestingMode = true;
                setDestination(newDestination);
                setGuardPoint(newDestination);

            } else {
                harvestingMode = false;
            }
        }
    }
}

void Harvester::destroy()
{
    if(currentGameMap->tileExists(location) && isVisible()) {
        int xpos = location.x;
        int ypos = location.y;

        if(currentGameMap->tileExists(xpos,ypos)) {
            const auto spiceSpreaded = spice * 0.75_fix;
            int availableSandPos = 0;

            int circleRadius = lround(spice / 210);

            /* how many regions have sand */
            for(int i = -circleRadius; i <= circleRadius; i++) {
                for(int j = -circleRadius; j <= circleRadius; j++) {
                    if(currentGameMap->tileExists(xpos + i, ypos + j)
                        && (distanceFrom(xpos, ypos, xpos + i, ypos + j) + 0.0005_fix <= circleRadius))
                    {
                        Tile *pTile = currentGameMap->getTile(xpos + i, ypos + j);
                        if((pTile != nullptr) & ((pTile->isSand()) || (pTile->isSpice()) )) {
                            availableSandPos++;
                        }
                    }
                }
            }

            /* now we can spread spice */
            for(int i = -circleRadius; i <= circleRadius; i++) {
                for(int j = -circleRadius; j <= circleRadius; j++) {
                    if(currentGameMap->tileExists(xpos + i, ypos + j)
                        && (distanceFrom(xpos, ypos, xpos + i, ypos + j) + 0.0005_fix  <= circleRadius))
                    {
                        Tile *pTile = currentGameMap->getTile(xpos + i, ypos + j);
                        if((pTile != nullptr) & ((pTile->isSand()) || (pTile->isSpice()) )) {
                            pTile->setSpice(pTile->getSpice() + spiceSpreaded / availableSandPos);
                        }
                    }
                }
            }
        }

        setTarget(nullptr);

        Coord realPos(lround(realX), lround(realY));
        Uint32 explosionID = currentGame->randomGen.getRandOf({Explosion_Medium1, Explosion_Medium2});
        currentGame->getExplosionList().push_back(new Explosion(explosionID, realPos, owner->getHouseID()));

        if(isVisible(getOwner()->getTeamID())) {
            screenborder->shakeScreen(18);
            soundPlayer->playSoundAt(Sound_ExplosionLarge,location);
        }
    }

    TrackedUnit::destroy();
}

void Harvester::drawSelectionBox()
{
    SDL_Texture* selectionBox = nullptr;

    switch(currentZoomlevel) {
        case 0:     selectionBox = pGFXManager->getUIGraphic(UI_SelectionBox_Zoomlevel0);   break;
        case 1:     selectionBox = pGFXManager->getUIGraphic(UI_SelectionBox_Zoomlevel1);   break;
        case 2:
        default:    selectionBox = pGFXManager->getUIGraphic(UI_SelectionBox_Zoomlevel2);   break;
    }

    SDL_Rect dest = calcDrawingRect(selectionBox, screenborder->world2screenX(realX), screenborder->world2screenY(realY), HAlign::Center, VAlign::Center);
    SDL_RenderCopy(renderer, selectionBox, nullptr, &dest);

    for(int i=1;i<=currentZoomlevel+1;i++) {
        renderDrawHLine(renderer, dest.x+1, dest.y-i, dest.x+1 + (lround((getHealth()/getMaxHealth())*(getWidth(selectionBox)-3))), getHealthColor());
    }

    if((getOwner() == pLocalHouse) && (spice > 0)) {
        for(int i=1;i<=currentZoomlevel+1;i++) {
            renderDrawHLine(renderer, dest.x+1, dest.y-i-(currentZoomlevel+1), dest.x+1 + (lround(((spice)/HARVESTERMAXSPICE)*(getWidth(selectionBox)-3))), COLOR_ORANGE);
        }
    }
}

void Harvester::handleDamage(int damage, Uint32 damagerID, House* damagerOwner)
{
    TrackedUnit::handleDamage(damage, damagerID, damagerOwner);

    ObjectBase* damager = currentGame->getObjectManager().getObject(damagerID);

    if(!target && !forced && damager && canAttack(damager) && (attackMode != STOP)) {
        setTarget(damager);
    }
}

void Harvester::handleReturnClick() {
    currentGame->getCommandManager().addCommand(Command(pLocalPlayer->getPlayerID(), CMD_HARVESTER_RETURN,objectID));
}

void Harvester::doReturn()
{
    if(!returningToRefinery && isActive()) {
        returningToRefinery = true;
        harvestingMode = false;

        if(getAttackMode() == STOP) {
            setGuardPoint(Coord::Invalid());
        }
    }
}

void Harvester::setAmountOfSpice(FixPoint newSpice)
{
    if((newSpice >= 0) && (newSpice <= HARVESTERMAXSPICE)) {
        spice = newSpice;
    }
}

void Harvester::setDestination(int newX, int newY)
{
    TrackedUnit::setDestination(newX, newY);

    harvestingMode =  (attackMode != STOP) && (currentGameMap->tileExists(newX, newY) && currentGameMap->getTile(newX,newY)->hasSpice());
}

void Harvester::setTarget(const ObjectBase* newTarget)
{
    if(returningToRefinery && target && (target.getObjPointer()!= nullptr)
        && (target.getObjPointer()->getItemID() == Structure_Refinery))
    {
        static_cast<Refinery*>(target.getObjPointer())->unBook();
        returningToRefinery = false;
    }

    TrackedUnit::setTarget(newTarget);

    if(target && (target.getObjPointer() != nullptr)
        && (target.getObjPointer()->getOwner() == getOwner())
        && (target.getObjPointer()->getItemID() == Structure_Refinery))
    {
        static_cast<Refinery*>(target.getObjPointer())->book();
        returningToRefinery = true;
    }

}

void Harvester::setReturned()
{
    if(selected) {
        removeFromSelectionLists();
    }

    currentGameMap->removeObjectFromMap(getObjectID());

    static_cast<Refinery*>(target.getObjPointer())->assignHarvester(this);

    returningToRefinery = false;
    moving = false;
    respondable = false;
    setActive(false);

    setLocation(INVALID_POS, INVALID_POS);
    setVisible(VIS_ALL, false);
}

void Harvester::move()
{
    TrackedUnit::move();

    if(active && !moving && !justStoppedMoving) {
        if(harvestingMode) {

            if(location == destination) {
                if(spice < HARVESTERMAXSPICE) {

                    Tile* tile = currentGameMap->getTile(location);

                    if(tile->hasSpice()) {

                        int beforeTileType = tile->getType();
                        spice += tile->harvestSpice();
                        int afterTileType = tile->getType();

                        if(beforeTileType != afterTileType) {
                            currentGameMap->spiceRemoved(location);
                            if(!currentGameMap->findSpice(destination, location)) {
                                doReturn();
                            } else {
                                doMove2Pos(destination, false);
                            }
                        }
                    } else if (!currentGameMap->findSpice(destination, location)) {
                        if(spice > 0) {
                            doReturn();
                        }
                    } else {
                        doMove2Pos(destination, false);
                    }
                } else {
                    doReturn();
                }
            }
        }
    }
}

bool Harvester::isHarvesting() const {
    return  harvestingMode
            && (spice < HARVESTERMAXSPICE)
            && (blockDistance(location, destination) <= FixPt_SQRT2)
            && currentGameMap->tileExists(location) && currentGameMap->getTile(location)->hasSpice();
}

bool Harvester::canAttack(const ObjectBase* object) const
{
    return((object != nullptr)
            && object->isInfantry()
            && (object->getOwner()->getTeamID() != owner->getTeamID())
            && object->isVisible(getOwner()->getTeamID()));
}

FixPoint Harvester::extractSpice(FixPoint extractionSpeed)
{
    FixPoint oldSpice = spice;

    if((spice - extractionSpeed) >= 0) {
        spice -= extractionSpeed;
    } else {
        spice = 0;
    }

    return (oldSpice - spice);
}

void Harvester::setSpeeds()
{
    FixPoint speed = getMaxSpeed();

    if(isBadlyDamaged()) {
        speed *= HEAVILYDAMAGEDSPEEDMULTIPLIER;
    }

    FixPoint percentFull = spice/HARVESTERMAXSPICE;
    speed = speed * (1 - MAXIMUMHARVESTERSLOWDOWN*percentFull);

    switch(drawnAngle){
        case LEFT:      xSpeed = -speed;                    ySpeed = 0;         break;
        case LEFTUP:    xSpeed = -speed*DIAGONALSPEEDCONST; ySpeed = xSpeed;    break;
        case UP:        xSpeed = 0;                         ySpeed = -speed;    break;
        case RIGHTUP:   xSpeed = speed*DIAGONALSPEEDCONST;  ySpeed = -xSpeed;   break;
        case RIGHT:     xSpeed = speed;                     ySpeed = 0;         break;
        case RIGHTDOWN: xSpeed = speed*DIAGONALSPEEDCONST;  ySpeed = xSpeed;    break;
        case DOWN:      xSpeed = 0;                         ySpeed = speed;     break;
        case LEFTDOWN:  xSpeed = -speed*DIAGONALSPEEDCONST; ySpeed = -xSpeed;   break;
    }
}
