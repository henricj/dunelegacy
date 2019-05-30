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

#define MAX_SANDWORMSLEEPTIME 50000
#define MIN_SANDWORMSLEEPTIME 10000

#define SANDWORM_ATTACKFRAMETIME 10

Sandworm::Sandworm(House* newOwner) : GroundUnit(newOwner) {

    Sandworm::init();

    setHealth(getMaxHealth());

    kills = 0;
    attackFrameTimer = 0;
    sleepTimer = 0;
    warningWormSignPlayedFlags = 0;
    respondable = false;

    for(int i = 0; i < SANDWORM_SEGMENTS; i++) {
        lastLocs[i].invalidate();
    }
    shimmerOffsetIndex = -1;
}

Sandworm::Sandworm(InputStream& stream) : GroundUnit(stream) {

    Sandworm::init();

    kills = stream.readSint32();
    attackFrameTimer = stream.readSint32();
    sleepTimer = stream.readSint32();
    warningWormSignPlayedFlags = stream.readUint8();
    shimmerOffsetIndex = stream.readSint32();
    for(int i = 0; i < SANDWORM_SEGMENTS; i++) {
        lastLocs[i].x = stream.readSint32();
        lastLocs[i].y = stream.readSint32();
    }
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
}

Sandworm::~Sandworm() = default;

void Sandworm::save(OutputStream& stream) const {
    GroundUnit::save(stream);

    stream.writeSint32(kills);
    stream.writeSint32(attackFrameTimer);
    stream.writeSint32(sleepTimer);
    stream.writeUint8(warningWormSignPlayedFlags);
    stream.writeSint32(shimmerOffsetIndex);
    for(int i = 0; i < SANDWORM_SEGMENTS; i++) {
        stream.writeSint32(lastLocs[i].x);
        stream.writeSint32(lastLocs[i].y);
    }
}

void Sandworm::assignToMap(const Coord& pos) {
    if(currentGameMap->tileExists(pos)) {
        currentGameMap->getTile(pos)->assignUndergroundUnit(getObjectID());
        // do not unhide map cause this would give Fremen players an advantage
        // currentGameMap->viewMap(owner->getHouseID(), location, getViewRange());
    }
}

bool Sandworm::attack() {
    if(primaryWeaponTimer == 0) {
        if(target) {
            soundPlayer->playSoundAt(Sound_WormAttack, location);
            drawnFrame = 0;
            attackFrameTimer = SANDWORM_ATTACKFRAMETIME;
            primaryWeaponTimer = getWeaponReloadTime();
            return true;
        }
    }
    return false;
}

void Sandworm::deploy(const Coord& newLocation) {
    UnitBase::deploy(newLocation);

    respondable = false;
}

void Sandworm::blitToScreen() {
    static const int shimmerOffset[]  = { 1, 3, 2, 5, 4, 3, 2, 1 };

    if(shimmerOffsetIndex >= 0) {
        // render sandworm's shimmer

        SDL_Texture* shimmerTex = pGFXManager->getZoomedObjPic(ObjPic_SandwormShimmerTemp, currentZoomlevel);
        SDL_Texture* shimmerMaskTex = pGFXManager->getZoomedObjPic(ObjPic_SandwormShimmerMask, currentZoomlevel);

        for(int i = 0; i < SANDWORM_SEGMENTS; i++) {
            if(lastLocs[i].isInvalid()) {
                continue;
            }

            SDL_Rect dest = calcDrawingRect(shimmerMaskTex, screenborder->world2screenX(lastLocs[i].x), screenborder->world2screenY(lastLocs[i].y), HAlign::Center, VAlign::Center);

            // switch to texture 'shimmerTex' for rendering
            SDL_Texture* oldRenderTarget = SDL_GetRenderTarget(renderer);
            SDL_SetRenderTarget(renderer, shimmerTex);

            // copy complete mask
            // contains solid black (0,0,0,255) for pixels to take from screen
            // and transparent (0,0,0,0) for pixels that should not be copied over
            SDL_SetTextureBlendMode(shimmerMaskTex, SDL_BLENDMODE_NONE);
            SDL_RenderCopy(renderer, shimmerMaskTex, nullptr, nullptr);
            SDL_SetTextureBlendMode(shimmerMaskTex, SDL_BLENDMODE_BLEND);

            // now copy r,g,b colors from screen but don't change alpha values in mask
            SDL_SetTextureBlendMode(screenTexture, SDL_BLENDMODE_ADD);
            SDL_Rect source = dest;
            source.x += shimmerOffset[(shimmerOffsetIndex+i)%8]*2;
            SDL_RenderCopy(renderer, screenTexture, &source, nullptr);
            SDL_SetTextureBlendMode(screenTexture, SDL_BLENDMODE_NONE);

            // switch back to old rendering target (from texture 'shimmerTex')
            SDL_SetRenderTarget(renderer, oldRenderTarget);

            // now blend shimmerTex to screen (= make use of alpha values in mask)
            SDL_SetTextureBlendMode(shimmerTex, SDL_BLENDMODE_BLEND);
            SDL_RenderCopy(renderer, shimmerTex, nullptr, &dest);

        }
    }

    if(drawnFrame != INVALID) {
        SDL_Rect dest = calcSpriteDrawingRect(  graphic[currentZoomlevel],
                                                screenborder->world2screenX(realX),
                                                screenborder->world2screenY(realY),
                                                numImagesX, numImagesY,
                                                HAlign::Center, VAlign::Center);
        SDL_Rect source = calcSpriteSourceRect(graphic[currentZoomlevel], 0, numImagesX, drawnFrame, numImagesY);
        SDL_RenderCopy(renderer, graphic[currentZoomlevel], &source, &dest);
    }
}

void Sandworm::checkPos() {
    if(justStoppedMoving) {
        realX = location.x*TILESIZE + TILESIZE/2;
        realY = location.y*TILESIZE + TILESIZE/2;

        if(currentGameMap->tileExists(location)) {
            Tile* pTile = currentGameMap->getTile(location);
            if(pTile->hasInfantry() && (pTile->getInfantry()->getOwner() == pLocalHouse)) {
                soundPlayer->playVoice(SomethingUnderTheSand, pTile->getInfantry()->getOwner()->getHouseID());
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
        FixPoint maxDistance;

        if(forced) {
            maxDistance = FixPt_MAX;
        } else {
            switch(attackMode) {
                case GUARD:
                case AMBUSH: {
                    maxDistance = getViewRange();
                } break;

                case AREAGUARD:
                case HUNT: {
                    maxDistance = FixPt_MAX;
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
            setTarget(nullptr);
        }
    }
}

void Sandworm::setLocation(int xPos, int yPos) {
    if(currentGameMap->tileExists(xPos, yPos) || ((xPos == INVALID_POS) && (yPos == INVALID_POS))) {
        UnitBase::setLocation(xPos, yPos);
    }
}

/**
    Put sandworm to sleep for a while
*/
void Sandworm::sleep() {
    sleepTimer = currentGame->randomGen.rand(MIN_SANDWORMSLEEPTIME, MAX_SANDWORMSLEEPTIME);
    setActive(false);
    setVisible(VIS_ALL, false);
    setForced(false);
    currentGameMap->removeObjectFromMap(getObjectID()); //no map point will reference now
    setLocation(INVALID_POS, INVALID_POS);
    setHealth(getMaxHealth());
    kills = 0;
    warningWormSignPlayedFlags = 0;
    drawnFrame = INVALID;
    attackFrameTimer = 0;
    shimmerOffsetIndex = -1;
    for(int i = 0; i < SANDWORM_SEGMENTS; i++) {
        lastLocs[i].invalidate();
    }
}

bool Sandworm::sleepOrDie() {

    // Make sand worms always drop spice, even if they don't die
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

void Sandworm::setTarget(const ObjectBase* newTarget) {
    GroundUnit::setTarget(newTarget);

    if( (newTarget != nullptr) && (newTarget->getOwner() == pLocalHouse)
        && ((warningWormSignPlayedFlags & (1 << pLocalHouse->getHouseID())) == 0) ) {
        soundPlayer->playVoice(WarningWormSign, pLocalHouse->getHouseID());
        warningWormSignPlayedFlags |= (1 << pLocalHouse->getHouseID());
    }
}

void Sandworm::handleDamage(int damage, Uint32 damagerID, House* damagerOwner) {
    if(damage > 0) {
        attackMode = HUNT;
    }
    GroundUnit::handleDamage(damage, damagerID, damagerOwner);
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

        if(isActive() && (moving || justStoppedMoving) && !currentGame->isGamePaused() && !currentGame->isGameFinished()) {
            Coord realLocation = getLocation()*TILESIZE + Coord(TILESIZE/2, TILESIZE/2);
            if(lastLocs[1] != realLocation) {
                for(int i = (SANDWORM_SEGMENTS-1); i > 0 ; i--) {
                    lastLocs[i] = lastLocs[i-1];
                }
                lastLocs[1] = realLocation;
            }
            lastLocs[0].x = lround(realX);
            lastLocs[0].y = lround(realY);
            shimmerOffsetIndex = ((currentGame->getGameCycleCount() + getObjectID()) % 48)/6;
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
                        if(target && target.getObjPointer() != nullptr) {
                            bool wasAlive = target.getObjPointer()->isVisible(getOwner()->getTeamID());  //see if unit was alive before attack
                            Coord realPos = Coord(lround(realX), lround(realY));
                            currentGameMap->damage(objectID, getOwner(), realPos, Bullet_Sandworm, 5000, NONE_ID, false);

                            if(wasAlive && target && (target.getObjPointer()->isVisible(getOwner()->getTeamID()) == false)) {
                                kills++;
                            }
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
    if((object != nullptr)
        && object->isAGroundUnit()
        && (object->getItemID() != Unit_Sandworm)   //wont kill other sandworms
        //&& object->isVisible(getOwner()->getTeamID())
        //&& (object->getOwner()->getTeamID() != owner->getTeamID())
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
        return nullptr;
    }

    const ObjectBase* closestTarget = nullptr;

    if((attackMode == HUNT) || (attackMode == AREAGUARD)) {
        FixPoint closestDistance = FixPt_MAX;

        for(UnitBase* pUnit : unitList) {
            if (canAttack(pUnit)
                && (blockDistance(location, pUnit->getLocation()) < closestDistance)) {
                closestTarget = pUnit;
                closestDistance = blockDistance(location, pUnit->getLocation());
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
