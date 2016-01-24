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

#include <units/SandWorm.h>

#include <globals.h>

#include <FileClasses/GFXManager.h>
#include <House.h>
#include <Game.h>
#include <Map.h>
#include <ScreenBorder.h>
#include <SoundPlayer.h>

#include <misc/draw_util.h>

#include <units/InfantryBase.h>

#include <limits>

#define MAX_SANDWORMSLEEPTIME 50000
#define MIN_SANDWORMSLEEPTIME 10000

#define SANDWORM_ATTACKFRAMETIME 10

Sandworm::Sandworm(House* newOwner) : GroundUnit(newOwner) {

    Sandworm::init();

    setHealth(getMaxHealth());

    kills = 0;
	attackFrameTimer = 0;
	sleepTimer = 0;
	respondable = false;
}

Sandworm::Sandworm(InputStream& stream) : GroundUnit(stream) {

    Sandworm::init();

	kills = stream.readSint32();
	attackFrameTimer = stream.readSint32();
	sleepTimer = stream.readSint32();
}

void Sandworm::init() {
    itemID = Unit_Sandworm;
    owner->incrementUnits(itemID);

	numWeapons = 0;

	graphicID = ObjPic_Sandworm;
	graphic = pGFXManager->getObjPic(graphicID,getOwner()->getHouseID());

	numImagesX = 1;
	numImagesY = 9;

	drawnFrame = INVALID;

	for(int i = 0; i < SANDWORM_SEGMENTS; i++) {
	    for(int z = 0; z < NUM_ZOOMLEVEL; z++) {
            shimmerSurface[i][z] = copySurface(pGFXManager->getObjPic(ObjPic_SandwormShimmerMask,HOUSE_HARKONNEN)[z]);
            SDL_FillRect(shimmerSurface[i][z], NULL, COLOR_TRANSPARENT);
	    }
	}
}

Sandworm::~Sandworm() {
	for(int i = 0; i < SANDWORM_SEGMENTS; i++) {
	    for(int z = 0; z < NUM_ZOOMLEVEL; z++) {
            SDL_FreeSurface(shimmerSurface[i][z]);
	    }
	}
}

void Sandworm::save(OutputStream& stream) const {
	GroundUnit::save(stream);

	stream.writeSint32(kills);
	stream.writeSint32(attackFrameTimer);
	stream.writeSint32(sleepTimer);
}

void Sandworm::assignToMap(const Coord& pos) {
	if(currentGameMap->tileExists(pos)) {
		currentGameMap->getTile(pos)->assignUndergroundUnit(getObjectID());
		// do not unhide map cause this would give Fremen players an advantage
		// currentGameMap->viewMap(owner->getTeam(), location, getViewRange());
	}
}

void Sandworm::attack() {
	if(primaryWeaponTimer == 0) {
		if(target) {
			soundPlayer->playSoundAt(Sound_WormAttack, location);
			drawnFrame = 0;
			attackFrameTimer = SANDWORM_ATTACKFRAMETIME;
			primaryWeaponTimer = getWeaponReloadTime();
		}
	}
}

void Sandworm::deploy(const Coord& newLocation) {
	UnitBase::deploy(newLocation);

	respondable = false;
}

void Sandworm::blitToScreen() {
    int width = shimmerSurface[0][currentZoomlevel]->w;
    int height = shimmerSurface[0][currentZoomlevel]->h;

    if(moving && !justStoppedMoving && !currentGame->isGamePaused() && !currentGame->isGameFinished()) {
        //create worms shimmer
        if(shimmerSurface[0][currentZoomlevel]->format->BitsPerPixel == 8) {

            SDL_Surface *mask = pGFXManager->getObjPic(ObjPic_SandwormShimmerMask,HOUSE_HARKONNEN)[currentZoomlevel];
            if((!SDL_MUSTLOCK(screen) || (SDL_LockSurface(screen) >= 0))
                && (!SDL_MUSTLOCK(mask) || (SDL_LockSurface(mask) >= 0)))
            {
                unsigned char	*maskPixels = (unsigned char*)mask->pixels,
                                *screenPixels = (unsigned char*)screen->pixels,
                                *surfacePixels;
                int maxX = screenborder->getRight();

                Random randomGen(currentGame->getGameCycleCount());

                for(int count = 0; count < SANDWORM_SEGMENTS; count++) {
                    //for each segment of the worms length
                    if((!SDL_MUSTLOCK(shimmerSurface[count][currentZoomlevel]) || (SDL_LockSurface(shimmerSurface[count][currentZoomlevel]) >= 0))) {
                        surfacePixels = (unsigned char*)shimmerSurface[count][currentZoomlevel]->pixels;

                        int destX = screenborder->world2screenX(lastLocs[count*(SANDWORM_LENGTH/SANDWORM_SEGMENTS)].x) - width/2;
                        int destY = screenborder->world2screenY(lastLocs[count*(SANDWORM_LENGTH/SANDWORM_SEGMENTS)].y) - height/2;

                        for(int i = 0; i < width; i++) {
                            for(int j = 0; j < height; j++) {
                                int x,y;

                                if(maskPixels[i + j*mask->pitch] == 0) {
                                    //direct copy
                                    x = i;
                                    y = j;
                                } else {
                                    x = i + world2zoomedWorld(randomGen.rand(2,5)*4);
                                    y = j;
                                }

                                if(destX + x < 0) {
                                    destX = x = 0;
                                } else if(destX + x >= maxX) {
                                    destX = maxX - 1, x = 0;
                                }

                                if(destY + y < 0) {
                                    destY = y = 0;
                                } else if (destY + y >= screen->h) {
                                    destY = screen->h - 1, y = 0;
                                }

                                if((destX + x >= 0) && (destX + x < screen->w) && (destY + y >= 0) && (destY + y < screen->h)) {
                                    surfacePixels[i + j*shimmerSurface[count][currentZoomlevel]->pitch] = screenPixels[destX + x + (destY + y)*screen->pitch];
                                } else {
                                    surfacePixels[i + j*shimmerSurface[count][currentZoomlevel]->pitch] = 0;
                                }
                            }
                        }

                        if(SDL_MUSTLOCK(shimmerSurface[count][currentZoomlevel])) {
                            SDL_UnlockSurface(shimmerSurface[count][currentZoomlevel]);
                        }
                    }
                }

                if(SDL_MUSTLOCK(mask)) {
                    SDL_UnlockSurface(mask);
                }

                if(SDL_MUSTLOCK(screen)) {
                    SDL_UnlockSurface(screen);
                }
            }
        }
    }

    /////draw wormy shimmer segments
    for(int count = 0; count < SANDWORM_SEGMENTS; count++) {
        //draw all the shimmering images
        SDL_Rect dest = {   static_cast<Sint16>(screenborder->world2screenX(lastLocs[count*(SANDWORM_LENGTH/SANDWORM_SEGMENTS)].x) - width/2),
                            static_cast<Sint16>(screenborder->world2screenY(lastLocs[count*(SANDWORM_LENGTH/SANDWORM_SEGMENTS)].y) - height/2),
                            static_cast<Uint16>(width),
                            static_cast<Uint16>(height) };

        SDL_BlitSurface(shimmerSurface[count][currentZoomlevel], NULL, screen, &dest);
    }

    if(drawnFrame != INVALID) {
        int imageW = graphic[currentZoomlevel]->w/numImagesX;
        int imageH = graphic[currentZoomlevel]->h/numImagesY;

        SDL_Rect dest = {   static_cast<Sint16>(screenborder->world2screenX(realX) - imageW/2),
                            static_cast<Sint16>(screenborder->world2screenY(realY) - imageH/2),
                            static_cast<Uint16>(imageW),
                            static_cast<Uint16>(imageH) };
        SDL_Rect source = { 0, static_cast<Sint16>(drawnFrame*imageH), static_cast<Uint16>(imageW), static_cast<Uint16>(imageH) };
        SDL_BlitSurface(graphic[currentZoomlevel], &source, screen, &dest);
    }
}

void Sandworm::checkPos() {
	if(moving && !justStoppedMoving) {
        if((std::abs(lround(realX) - lastLocs[0].x) >= 4) || (std::abs(lround(realY) - lastLocs[0].y) >= 4)) {
			for(int i = (SANDWORM_LENGTH-1); i > 0 ; i--) {
				lastLocs[i] = lastLocs[i-1];
			}

			lastLocs[0].x = lround(realX);
			lastLocs[0].y = lround(realY);
		}
	}

	if(justStoppedMoving) {
		realX = location.x*TILESIZE + TILESIZE/2;
		realY = location.y*TILESIZE + TILESIZE/2;

		if(currentGameMap->tileExists(location)) {
			Tile* pTile = currentGameMap->getTile(location);
			if(pTile->hasInfantry() && (pTile->getInfantry()->getOwner() == pLocalHouse)) {
				soundPlayer->playSound(SomethingUnderTheSand);
			}
		}
	}
}

void Sandworm::engageTarget() {
    if(isEating()) {
        return;
    }

    UnitBase::engageTarget();

    if(target) {
        float maxDistance;

        if(forced) {
            maxDistance = std::numeric_limits<float>::max();
        } else {
            switch(attackMode) {
                case GUARD:
                case AREAGUARD:
                case AMBUSH: {
                    maxDistance = getViewRange();
                } break;

                case HUNT: {
                    maxDistance = std::numeric_limits<float>::max();
                } break;

                case STOP:
                case CAPTURE:
                default: {
                    maxDistance = 0;
                } break;
            }
        }

        if(targetDistance > maxDistance) {
            // give up
            setDestination(guardPoint);
            setTarget(NULL);
        }
    }
}

void Sandworm::setLocation(int xPos, int yPos) {
	if(currentGameMap->tileExists(xPos, yPos) || ((xPos == INVALID_POS) && (yPos == INVALID_POS))) {
		UnitBase::setLocation(xPos, yPos);

		for(int i = 0; i < SANDWORM_LENGTH; i++) {
			lastLocs[i].x = lround(realX);
			lastLocs[i].y = lround(realY);
		}
	}
}

/*
    Put sandworm to sleep for a while
*/
void Sandworm::sleep() {
	sleepTimer = currentGame->randomGen.rand(MIN_SANDWORMSLEEPTIME, MAX_SANDWORMSLEEPTIME);
	setActive(false);
	setVisible(VIS_ALL, false);
	setForced(false);
	currentGameMap->removeObjectFromMap(getObjectID());	//no map point will reference now
	setLocation(NONE, NONE);
	setHealth(getMaxHealth());
	kills = 0;
	drawnFrame = INVALID;
	attackFrameTimer = 0;
}

bool Sandworm::sleepOrDie() {

    /*
        Make sand worms always drop spice, even if they don't die
    */
    if(currentGame->getGameInitSettings().getGameOptions().killedSandwormsDropSpice) {
            currentGameMap->createSpiceField(location, 4);
    }

    if(currentGame->getGameInitSettings().getGameOptions().sandwormsRespawn) {
        sleep();
        return true;
    } else {
        destroy();
        return false;
    }
}

bool Sandworm::update() {
	if(getHealth() <= getMaxHealth()/2) {



		if(sleepOrDie() == false) {
            return false;
		}
	} else {
		if(GroundUnit::update() == false) {
		    return false;
		}

        if(attackFrameTimer > 0) {
            attackFrameTimer--;

            //death frame has started
            if(attackFrameTimer == 0) {
                drawnFrame++;
                if(drawnFrame >= 9) {
                    drawnFrame = INVALID;
                    if(kills >= 3) {
                        if(sleepOrDie() == false) {
                            return false;
                        }
                    }
                } else {
                    attackFrameTimer = SANDWORM_ATTACKFRAMETIME;
                    if(drawnFrame == 1) {
                        // the close mouth bit of graphic is currently shown => eat unit
                        bool wasAlive = ( target && target.getObjPointer()->isVisible(getOwner()->getTeam()));	//see if unit was alive before attack
                        Coord realPos = Coord(lround(realX), lround(realY));
                        currentGameMap->damage(objectID, getOwner(), realPos, Bullet_Sandworm, 5000, NONE, false);

                        if(wasAlive && (target.getObjPointer()->isVisible(getOwner()->getTeam()) == false)) {
                            kills++;
                        }
                    }
                }
            }
        }

		if(sleepTimer > 0) {
		    sleepTimer--;

			if(sleepTimer == 0) {
			    // awaken the worm!

			    for(int tries = 0 ; tries < 1000 ; tries++) {
                    int x = currentGame->randomGen.rand(0, currentGameMap->getSizeX() - 1);
                    int y = currentGame->randomGen.rand(0, currentGameMap->getSizeY() - 1);

                    if(canPass(x, y)) {
                        deploy(currentGameMap->getTile(x, y)->getLocation());
                        break;
                    }
			    }

                if(isActive() == false) {
                    // no room for sandworm on map => take another nap
                    if(sleepOrDie() == false) {
                        return false;
                    }
                }
			}
		}
	}

	return true;
}

bool Sandworm::canAttack(const ObjectBase* object) const {
	if((object != NULL)
		&& object->isAGroundUnit()
		&& (object->getItemID() != Unit_Sandworm)	//wont kill other sandworms
		//&& object->isVisible(getOwner()->getTeam())
		//&& (object->getOwner()->getTeam() != owner->getTeam())
		&& currentGameMap->tileExists(object->getLocation())
		&& canPass(object->getLocation().x, object->getLocation().y)
		&& (currentGameMap->getTile(object->getLocation())->getSandRegion() == currentGameMap->getTile(location)->getSandRegion())) {
		return true;
	} else {
		return false;
	}
}

bool Sandworm::canPass(int xPos, int yPos) const {
	return (currentGameMap->tileExists(xPos, yPos)
			&& !currentGameMap->getTile(xPos, yPos)->isRock()
			&& (!currentGameMap->getTile(xPos, yPos)->hasAnUndergroundUnit()
				|| (currentGameMap->getTile(xPos, yPos)->getUndergroundUnit() == this)));
}

const ObjectBase* Sandworm::findTarget() const {
    if(isEating()) {
        return NULL;
    }

	const ObjectBase* closestTarget = NULL;

	if(attackMode == HUNT) {
	    FixPoint closestDistance = FixPt_MAX;

        RobustList<UnitBase*>::const_iterator iter;
	    for(iter = unitList.begin(); iter != unitList.end(); ++iter) {
			UnitBase* tempUnit = *iter;
            if (canAttack(tempUnit)
				&& (blockDistance(location, tempUnit->getLocation()) < closestDistance)) {
                closestTarget = tempUnit;
                closestDistance = blockDistance(location, tempUnit->getLocation());
            }
		}
	} else {
		closestTarget = ObjectBase::findTarget();
	}

	return closestTarget;
}

int Sandworm::getCurrentAttackAngle() const {
    // we can always attack an target
    return targetAngle;
}

void Sandworm::playAttackSound() {
	soundPlayer->playSoundAt(Sound_WormAttack,location);
}
