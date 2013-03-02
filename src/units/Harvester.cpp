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

#include <structures/Refinery.h>

#include <misc/draw_util.h>

#include <algorithm>

/* how often is the same sandframe redrawn */
#define HARVESTERDELAY 30

/* how often to change harvester position while harvesting */
#define RANDOMHARVESTMOVE 500

/* how much is the harvester movement slowed down when full  */
#define MAXIMUMHARVESTERSLOWDOWN 0.4f

Harvester::Harvester(House* newOwner) : TrackedUnit(newOwner)
{
    Harvester::init();

    setHealth(getMaxHealth());

    spice = 0.0f;
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
    spice = stream.readFloat();
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

Harvester::~Harvester()
{
	;
}

void Harvester::save(OutputStream& stream) const
{
	TrackedUnit::save(stream);
	stream.writeBool(harvestingMode);
	stream.writeBool(returningToRefinery);
    stream.writeFloat(spice);
    stream.writeUint32(spiceCheckCounter);
}

void Harvester::blitToScreen()
{
    SDL_Surface* pUnitGraphic = graphic[currentZoomlevel];
    int imageW = pUnitGraphic->w/numImagesX;
    int imageH = pUnitGraphic->h/numImagesY;
    int x = screenborder->world2screenX(realX);
    int y = screenborder->world2screenY(realY);

    SDL_Rect source = { drawnAngle * imageW, drawnFrame * imageH, imageW, imageH };
    SDL_Rect dest = { x - imageW/2, y - imageH/2, imageW, imageH };

    SDL_BlitSurface(pUnitGraphic, &source, screen, &dest);

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


        SDL_Surface** sand = pGFXManager->getObjPic(ObjPic_Harvester_Sand,getOwner()->getHouseID());

        SDL_Surface* pSandGraphic = sand[currentZoomlevel];
        int sandImageW = pSandGraphic->w/8;
        int sandImageH = pSandGraphic->h/3;
        int sandX = screenborder->world2screenX(realX + harvesterSandOffset[drawnAngle].x);
        int sandY = screenborder->world2screenY(realY + harvesterSandOffset[drawnAngle].y);

        int frame = ((currentGame->getGameCycleCount() + (getObjectID() * 10)) / HARVESTERDELAY) % (2*LASTSANDFRAME);
        if(frame > LASTSANDFRAME) {
            frame -= LASTSANDFRAME;
        }

        SDL_Rect sandSource = { drawnAngle* sandImageW, frame * sandImageH, sandImageW, sandImageH };
        SDL_Rect sandDest = { sandX - sandImageW/2, sandY - sandImageH/2, sandImageW, sandImageH };

        SDL_BlitSurface(pSandGraphic, &sandSource, screen, &sandDest);
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
	}

	if(active)	{
		if (returningToRefinery) {
			if (target && (target.getObjPointer() != NULL) && (target.getObjPointer()->getItemID() == Structure_Refinery)) {
				//find a refinery to return to
				Coord closestPoint = target.getObjPointer()->getClosestPoint(location);

				if(!moving && !justStoppedMoving && blockDistance(location, closestPoint) <= 1.5f)	{
					awaitingPickup = false;
					if (((Refinery*)target.getObjPointer())->isFree())
						setReturned();
				} else if(!awaitingPickup && blockDistance(location, closestPoint) >= 5.0f) {
					requestCarryall();
				}
			} else if (!structureList.empty()) {
				int	leastNumBookings = 1000000; //huge amount so refinery couldn't possibly compete with any refinery num bookings
				float	closestLeastBookedRefineryDistance = 1000000.0f;
				Refinery	*bestRefinery = NULL;

                RobustList<StructureBase*>::const_iterator iter;
                for(iter = structureList.begin(); iter != structureList.end(); ++iter) {
					StructureBase* tempStructure = *iter;

					if((tempStructure->getItemID() == Structure_Refinery) && (tempStructure->getOwner() == owner)) {
						Refinery* tempRefinery = static_cast<Refinery*>(tempStructure);
						Coord closestPoint = tempRefinery->getClosestPoint(location);
						float tempDistance = distanceFrom(location, closestPoint);
						int tempNumBookings = tempRefinery->getNumBookings();

						if (tempNumBookings < leastNumBookings)	{
							leastNumBookings = tempNumBookings;
							closestLeastBookedRefineryDistance = tempDistance;
							bestRefinery = tempRefinery;
						} else if (tempNumBookings == leastNumBookings) {
							if (tempDistance < closestLeastBookedRefineryDistance) {
								closestLeastBookedRefineryDistance = tempDistance;
								bestRefinery = tempRefinery;
							}
						}
					}
				}

				if (bestRefinery) {
					doMove2Object(bestRefinery);

					bestRefinery->startAnimate();
				}
			}
		} else if (harvestingMode && !hasBookedCarrier() && (blockDistance(location, destination) > 10.0f)) {
			requestCarryall();
        } else if(respondable && !harvestingMode && attackMode != STOP) {
            if(spiceCheckCounter == 0) {
                if(currentGameMap->findSpice(destination, guardPoint)) {
                    harvestingMode = true;
                    guardPoint = destination;
                } else {
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
		UnitBase::deploy(newLocation);
		if(spice == 0.0f) {
			if((attackMode != STOP) && currentGameMap->findSpice(destination, guardPoint)) {
				harvestingMode = true;
				guardPoint = destination;
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
            float spiceSpreaded = 0.75f * spice;
            int availableSandPos = 0;

            int circleRadius = lroundf(spice / 210);

            /* how many regions have sand */
            for(int i = -circleRadius; i <= circleRadius; i++) {
                for(int j = -circleRadius; j <= circleRadius; j++) {
                    if(currentGameMap->tileExists(xpos + i, ypos + j)
                        && (distanceFrom(xpos, ypos, xpos + i, ypos + j) + 0.0005 <= circleRadius))
                    {
                        Tile *pTile = currentGameMap->getTile(xpos + i, ypos + j);
                        if((pTile != NULL) & ((pTile->isSand()) || (pTile->isSpice()) )) {
                            availableSandPos++;
                        }
                    }
                }
            }

            /* now we can spread spice */
            for(int i = -circleRadius; i <= circleRadius; i++) {
                for(int j = -circleRadius; j <= circleRadius; j++) {
                    if(currentGameMap->tileExists(xpos + i, ypos + j)
                        && (distanceFrom(xpos, ypos, xpos + i, ypos + j) + 0.0005  <= circleRadius))
                    {
                        Tile *pTile = currentGameMap->getTile(xpos + i, ypos + j);
                        if((pTile != NULL) & ((pTile->isSand()) || (pTile->isSpice()) )) {
                            pTile->setSpice(pTile->getSpice() + spiceSpreaded / availableSandPos);
                        }
                    }
                }
            }
        }

        setTarget(NULL);

        Coord realPos(lround(realX), lround(realY));
        Uint32 explosionID = currentGame->randomGen.getRandOf(2,Explosion_Medium1, Explosion_Medium2);
        currentGame->getExplosionList().push_back(new Explosion(explosionID, realPos, owner->getHouseID()));

        if(isVisible(getOwner()->getTeam())) {
            screenborder->shakeScreen(18);
            soundPlayer->playSoundAt(Sound_ExplosionLarge,location);
        }
    }

	TrackedUnit::destroy();
}

void Harvester::drawSelectionBox()
{
    SDL_Surface* selectionBox = NULL;

    switch(currentZoomlevel) {
        case 0:     selectionBox = pGFXManager->getUIGraphic(UI_SelectionBox_Zoomlevel0);   break;
        case 1:     selectionBox = pGFXManager->getUIGraphic(UI_SelectionBox_Zoomlevel1);   break;
        case 2:
        default:    selectionBox = pGFXManager->getUIGraphic(UI_SelectionBox_Zoomlevel2);   break;
    }

    SDL_Rect dest = {   screenborder->world2screenX(realX) - selectionBox->w/2,
                        screenborder->world2screenY(realY) - selectionBox->h/2,
                        selectionBox->w,
                        selectionBox->h };

	SDL_BlitSurface(selectionBox, NULL, screen, &dest);

	for(int i=1;i<=currentZoomlevel+1;i++) {
        drawHLine(screen, dest.x+1, dest.y-i, dest.x+1 + ((int)((getHealth()/(float)getMaxHealth())*(selectionBox->w-3))), getHealthColor());
	}

	if((getOwner() == pLocalHouse) && (spice > 0.0f)) {
        for(int i=1;i<=currentZoomlevel+1;i++) {
            drawHLine(screen, dest.x+1, dest.y-i-(currentZoomlevel+1), dest.x+1 + ((int)((((float)spice)/HARVESTERMAXSPICE)*(selectionBox->w-3))), COLOR_ORANGE);
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
	returningToRefinery = true;
	harvestingMode = false;

	if(getAttackMode() == STOP) {
        setGuardPoint(Coord::Invalid());
	}
}

void Harvester::setAmountOfSpice(float newSpice)
{
	if((newSpice >= 0.0f) && (newSpice <= HARVESTERMAXSPICE)) {
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
	if(returningToRefinery && target && (target.getObjPointer()!= NULL)
		&& (target.getObjPointer()->getItemID() == Structure_Refinery))
	{
		((Refinery*)target.getObjPointer())->unBook();
		returningToRefinery = false;
	}

	TrackedUnit::setTarget(newTarget);

	if(target && (target.getObjPointer() != NULL)
		&& (target.getObjPointer()->getOwner() == getOwner())
		&& (target.getObjPointer()->getItemID() == Structure_Refinery))
	{
		((Refinery*)target.getObjPointer())->book();
		returningToRefinery = true;
	}

}

void Harvester::setReturned()
{
	if(selected) {
		removeFromSelectionLists();
    }

	currentGameMap->removeObjectFromMap(getObjectID());

	((Refinery*)target.getObjPointer())->assignHarvester(this);

	returningToRefinery = false;
	moving = false;
	respondable = false;
	setActive(false);

	setLocation(INVALID_POS, INVALID_POS);
	setVisible(VIS_ALL, false);
}

void Harvester::move()
{
	UnitBase::move();

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
						doReturn();
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
    return harvestingMode && (blockDistance(location, destination) <= DIAGONALCOST) && currentGameMap->tileExists(location) && currentGameMap->getTile(location)->hasSpice();
}

bool Harvester::canAttack(const ObjectBase* object) const
{
	return((object != NULL)
			&& object->isInfantry()
			&& (object->getOwner()->getTeam() != owner->getTeam())
			&& object->isVisible(getOwner()->getTeam()));
}

float Harvester::extractSpice(float extractionSpeed)
{
	float oldSpice = spice;

	if((spice - extractionSpeed) >= 0.0f) {
		spice -= extractionSpeed;
	} else {
		spice = 0.0f;
	}

	return (oldSpice - spice);
}

void Harvester::setSpeeds()
{
	float speed = getMaxSpeed();

	if(isBadlyDamaged()) {
        speed *= HEAVILYDAMAGEDSPEEDMULTIPLIER;
	}

    float percentFull = spice/HARVESTERMAXSPICE;
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
