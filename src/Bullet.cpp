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

#include <Bullet.h>

#include <globals.h>

#include <FileClasses/GFXManager.h>
#include <SoundPlayer.h>
#include <ObjectBase.h>
#include <Game.h>
#include <Map.h>
#include <House.h>
#include <Explosion.h>

#include <misc/draw_util.h>
#include <misc/strictmath.h>

#include <algorithm>


Bullet::Bullet(Uint32 shooterID, Coord* newRealLocation, Coord* newRealDestination, Uint32 bulletID, int damage, bool air)
{
    airAttack = air;

    this->shooterID = shooterID;

    ObjectBase* pShooter = currentGame->getObjectManager().getObject(shooterID);
    if(pShooter == NULL) {
        owner = NULL;
    } else {
        owner = pShooter->getOwner();
    }

	this->bulletID = bulletID;

    this->damage = damage;

    Bullet::init();

    destination = *newRealDestination;

	if(bulletID == Bullet_Sonic) {
		int diffX = destination.x - newRealLocation->x;
        int diffY = destination.y - newRealLocation->y;

		int weaponrange = currentGame->objectData.data[Unit_SonicTank][(owner == NULL) ? HOUSE_ATREIDES : owner->getHouseID()].weaponrange;

		if((diffX == 0) && (diffY == 0)) {
			diffY = weaponrange*TILESIZE;
		}

		float square_root = strictmath::sqrt((float)(diffX*diffX + diffY*diffY));
		float ratio = (weaponrange*TILESIZE)/square_root;
		destination.x = newRealLocation->x + (int)(((float)diffX)*ratio);
		destination.y = newRealLocation->y + (int)(((float)diffY)*ratio);
	} else if(bulletID == Bullet_Rocket || bulletID == Bullet_DRocket) {
	    float distance = distanceFrom(*newRealLocation, *newRealDestination);


        float randAngle = 2.0f * strictmath::pi * currentGame->randomGen.randFloat();
        int radius = currentGame->randomGen.rand(0,TILESIZE/2 + (distance/TILESIZE));

        destination.x += strictmath::cos(randAngle) * radius;
        destination.y -= strictmath::sin(randAngle) * radius;

	}

	realX = (float)newRealLocation->x;
	realY = (float)newRealLocation->y;
	source.x = newRealLocation->x;
	source.y = newRealLocation->y;
	location.x = newRealLocation->x/TILESIZE;
	location.y = newRealLocation->y/TILESIZE;

	angle = destinationAngle(*newRealLocation, *newRealDestination);
	drawnAngle = (int)((float)numFrames*angle/256.0);

    xSpeed = speed * strictmath::cos(angle * conv2char);
	ySpeed = speed * -strictmath::sin(angle * conv2char);
}

Bullet::Bullet(InputStream& stream)
{
	bulletID = stream.readUint32();

	airAttack = stream.readBool();
	damage = stream.readSint32();

	shooterID = stream.readUint32();
	Uint32 x = stream.readUint32();
	if(x < NUM_HOUSES) {
		owner = currentGame->getHouse(x);
	} else {
		owner = currentGame->getHouse(0);
	}

    source.x = stream.readSint32();
	source.y = stream.readSint32();
	destination.x = stream.readSint32();
	destination.y = stream.readSint32();
    location.x = stream.readSint32();
	location.y = stream.readSint32();
	realX = stream.readFloat();
	realY = stream.readFloat();

    xSpeed = stream.readFloat();
	ySpeed = stream.readFloat();

	drawnAngle = stream.readSint8();
    angle = stream.readFloat();

	Bullet::init();

	detonationTimer = stream.readSint8();
}

void Bullet::init()
{
    explodesAtGroundObjects = false;

	int houseID = (owner == NULL) ? HOUSE_HARKONNEN : owner->getHouseID();

	switch(bulletID) {
        case Bullet_DRocket: {
            damageRadius = TILESIZE/2;
            speed = 20.0f;
            detonationTimer = 19;
            numFrames = 16;
            graphic = pGFXManager->getObjPic(ObjPic_Bullet_MediumRocket, houseID);
        } break;

        case Bullet_LargeRocket: {
            damageRadius = TILESIZE;
            speed = 20.0f;
            detonationTimer = -1;
            numFrames = 16;
            graphic = pGFXManager->getObjPic(ObjPic_Bullet_LargeRocket, houseID);
        } break;

        case Bullet_Rocket: {
            damageRadius = TILESIZE/2;
            speed = 17.5f;
            detonationTimer = 22;
            numFrames = 16;
            graphic = pGFXManager->getObjPic(ObjPic_Bullet_MediumRocket, houseID);
        } break;

        case Bullet_TurretRocket: {
            damageRadius = TILESIZE/2;
            speed = 20.0f;
            detonationTimer = -1;
            numFrames = 16;
            graphic = pGFXManager->getObjPic(ObjPic_Bullet_MediumRocket, houseID);
        } break;

        case Bullet_ShellSmall: {
            damageRadius = TILESIZE/2;
            explodesAtGroundObjects = true;
            speed = 20.0f;
            detonationTimer = -1;
            numFrames = 1;
            graphic = pGFXManager->getObjPic(ObjPic_Bullet_Small, houseID);
        } break;

        case Bullet_ShellMedium: {
            damageRadius = TILESIZE/2;
            explodesAtGroundObjects = true;
            speed = 20.0f;
            detonationTimer = -1;
            numFrames = 1;
            graphic = pGFXManager->getObjPic(ObjPic_Bullet_Medium, houseID);
        } break;

        case Bullet_ShellLarge: {
            damageRadius = TILESIZE/2;
            explodesAtGroundObjects = true;
            speed = 20.0f;
            detonationTimer = -1;
            numFrames = 1;
            graphic = pGFXManager->getObjPic(ObjPic_Bullet_Large, houseID);
        } break;

        case Bullet_SmallRocket: {
            damageRadius = TILESIZE/2;
            speed = 20.0f;
            detonationTimer = 7;
            numFrames = 16;
            graphic = pGFXManager->getObjPic(ObjPic_Bullet_SmallRocket, houseID);
        } break;

        case Bullet_Sonic: {
            damageRadius = (TILESIZE*3)/4;
            speed = 9.0f;
            numFrames = 1;
            detonationTimer = 28;
            SDL_Surface** tmpSurfaceStack = pGFXManager->getObjPic(ObjPic_Bullet_Sonic, houseID);
            graphic = new SDL_Surface*[NUM_ZOOMLEVEL];
            for(int z = 0; z < NUM_ZOOMLEVEL; z++) {
                graphic[z] =  copySurface(tmpSurfaceStack[z]);	//make a copy of the image
            }
        } break;

        case Bullet_Sandworm: {
            fprintf(stderr,"Bullet::init(): Unknown Bullet_Sandworm not allowed.\n");
            graphic = NULL;
        } break;

        default: {
            fprintf(stderr,"Bullet::init(): Unknown Bullet type %d.\n",bulletID);
            graphic = NULL;
        } break;
	}
}


Bullet::~Bullet()
{
    if((bulletID == Bullet_Sonic) && (graphic != NULL)) {
        for(int z = 0; z < NUM_ZOOMLEVEL; z++) {
            SDL_FreeSurface(graphic[z]);
        }
        delete [] graphic;
        graphic = NULL;
    }
}

void Bullet::save(OutputStream& stream) const
{
	stream.writeUint32(bulletID);

	stream.writeBool(airAttack);
    stream.writeSint32(damage);

    stream.writeUint32(shooterID);
    stream.writeUint32(owner->getHouseID());

	stream.writeSint32(source.x);
	stream.writeSint32(source.y);
	stream.writeSint32(destination.x);
	stream.writeSint32(destination.y);
    stream.writeSint32(location.x);
	stream.writeSint32(location.y);
	stream.writeFloat(realX);
	stream.writeFloat(realY);

    stream.writeFloat(xSpeed);
	stream.writeFloat(ySpeed);

	stream.writeSint8(drawnAngle);
    stream.writeFloat(angle);

    stream.writeSint8(detonationTimer);
}


void Bullet::blitToScreen()
{
    int imageW = graphic[currentZoomlevel]->w/numFrames;
    int imageH = graphic[currentZoomlevel]->h;

	if(screenborder->isInsideScreen( Coord(lround(realX), lround(realY)), Coord(imageW, imageH)) == false) {
        return;
	}

    SDL_Rect source = { static_cast<Sint16>((numFrames > 1) ? drawnAngle * imageW : 0), 0, static_cast<Uint16>(imageW), static_cast<Uint16>(imageH) };
	SDL_Rect dest = {   static_cast<Sint16>(screenborder->world2screenX(realX) - imageW/2),
                        static_cast<Sint16>(screenborder->world2screenY(realY) - imageH/2),
                        static_cast<Uint16>(imageW), static_cast<Uint16>(imageH) };

    if(bulletID == Bullet_Sonic && !currentGame->isGamePaused()) {
        SDL_Surface** mask = pGFXManager->getObjPic(ObjPic_Bullet_Sonic,(owner == NULL) ? HOUSE_HARKONNEN : owner->getHouseID());

        if(mask[currentZoomlevel]->format->BitsPerPixel == 8) {
            if ((!SDL_MUSTLOCK(screen) || (SDL_LockSurface(screen) == 0))
                && (!SDL_MUSTLOCK(mask[currentZoomlevel]) || (SDL_LockSurface(mask[currentZoomlevel]) == 0))
                && (!SDL_MUSTLOCK(graphic[currentZoomlevel]) || (SDL_LockSurface(graphic[currentZoomlevel]) == 0)))
            {
                unsigned char *maskPixels = (unsigned char*)mask[currentZoomlevel]->pixels;
                unsigned char *screenPixels = (unsigned char*)screen->pixels;
                unsigned char *surfacePixels = (unsigned char*)graphic[currentZoomlevel]->pixels;
                int	maxX = world2zoomedWorld(screenborder->getRight());
                int	maxY = world2zoomedWorld(screenborder->getBottom());

                for(int i = 0; i < dest.w; i++) {
                    for (int j = 0; j < dest.h; j++) {
                        int x;
                        int y;

                        if(maskPixels[i + j*mask[currentZoomlevel]->pitch] == 0) {
                            //not masked, direct copy
                            x = i;
                            y = j;
                        } else {
                            x = i + world2zoomedWorld(12);
                            y = j;
                        }

                        dest.x = std::max(0,std::min( (int) dest.x, maxX-1));
                        dest.y = std::max(0,std::min( (int) dest.y, maxY-1));

                        if(x < 0) {
                            x = 0;
                        } else if (dest.x + x >= maxX) {
                            x = maxX - dest.x - 1;
                        }

                        if(y < 0) {
                            y = 0;
                        } else if (dest.y + y >= screen->h) {
                            x = screen->h - dest.y - 1;
                        }

                        if((dest.x + x >= 0) && (dest.x + x < screen->w) && (dest.y + y >= 0) && (dest.y + y < screen->h)) {
                            surfacePixels[i + j*graphic[currentZoomlevel]->pitch] = screenPixels[dest.x + x + (dest.y + y)*screen->pitch];
                        } else {
                            surfacePixels[i + j*graphic[currentZoomlevel]->pitch] = 0;
                        }
                    }
                }

                if (SDL_MUSTLOCK(graphic[currentZoomlevel]))
                    SDL_UnlockSurface(graphic[currentZoomlevel]);

                if (SDL_MUSTLOCK(mask[currentZoomlevel]))
                    SDL_UnlockSurface(mask[currentZoomlevel]);

                if (SDL_MUSTLOCK(screen))
                    SDL_UnlockSurface(screen);
            }
        }
    } //end of if sonic

    SDL_BlitSurface(graphic[currentZoomlevel], &source, screen, &dest);
}


void Bullet::update()
{
	if(bulletID == Bullet_Rocket || bulletID == Bullet_DRocket) {

        float angleToDestination = destinationAngle(Coord(realX, realY), destination);

        float angleDiff = angleToDestination - angle;
        if(angleDiff > 128.0f) {
            angleDiff -= 256.0f;
        } else if(angleDiff < -128.0f) {
            angleDiff += 256.0f;
        }

        static const float turnSpeed = 4.5f;

        if(angleDiff >= turnSpeed) {
            angleDiff = turnSpeed;
        } else if(angleDiff <= -turnSpeed) {
            angleDiff = -turnSpeed;
        }

        angle += angleDiff;

        if(angle < 0.0f) {
            angle += 256.0f;
        } else if(angle >= 256.0f) {
            angle -= 256.0f;
        }

        xSpeed = speed * strictmath::cos(angle * conv2char);
        ySpeed = speed * -strictmath::sin(angle * conv2char);

        drawnAngle = (int)((float)numFrames*angle/256.0);
    }


	float oldDistanceToDestination = distanceFrom(realX, realY, destination.x, destination.y);

	realX += xSpeed;  //keep the bullet moving by its current speeds
	realY += ySpeed;
	location.x = (int)(realX/TILESIZE);
	location.y = (int)(realY/TILESIZE);

	if((location.x < -5) || (location.x >= currentGameMap->getSizeX() + 5) || (location.y < -5) || (location.y >= currentGameMap->getSizeY() + 5)) {
        // it's off the map => delete it
        bulletList.remove(this);
        delete this;
        return;
	} else {
        float newDistanceToDestination = distanceFrom(realX, realY, destination.x, destination.y);

        if(detonationTimer > 0) {
            detonationTimer--;
        }

	    if(bulletID == Bullet_Sonic) {

            if(detonationTimer == 0) {
                destroy();
                return;
            }

            float weaponDamage = currentGame->objectData.data[Unit_SonicTank][(owner == NULL) ? HOUSE_ATREIDES : owner->getHouseID()].weapondamage;

	        float startDamage = (weaponDamage / 4.0f + 1.0f) / 3.0f;
	        float endDamage = ((weaponDamage-9) / 4.0f + 1.0f) / 3.0f;

		    float damageDecrease = - (startDamage-endDamage)/(30.0f * 2.0f * speed);
		    float dist = distanceFrom(source.x, source.y, realX, realY);

		    float currentDamage = dist*damageDecrease + startDamage;

            Coord realPos = Coord(lround(realX), lround(realY));
            currentGameMap->damage(shooterID, owner, realPos, bulletID, currentDamage/2.0f, damageRadius, false);

            realX += xSpeed;  //keep the bullet moving by its current speeds
            realY += ySpeed;

            realPos = Coord(lround(realX), lround(realY));
            currentGameMap->damage(shooterID, owner, realPos, bulletID, currentDamage/2.0f, damageRadius, false);
		} else if( explodesAtGroundObjects
                    && currentGameMap->tileExists(location)
                    && currentGameMap->getTile(location)->hasAGroundObject()
                    && currentGameMap->getTile(location)->getGroundObject()->isAStructure()) {
			destroy();
            return;
		} else if(oldDistanceToDestination < newDistanceToDestination || newDistanceToDestination < 4)	{

            if(bulletID == Bullet_Rocket || bulletID == Bullet_DRocket) {
                if(detonationTimer == 0) {
                    destroy();
                return;
                }
            } else {
                realX = destination.x;
                realY = destination.y;
                destroy();
                return;
            }
		}
	}
}


void Bullet::destroy()
{
    Coord position = Coord(lround(realX), lround(realY));

    int houseID = (owner == NULL) ? HOUSE_HARKONNEN : owner->getHouseID();

    switch(bulletID) {
        case Bullet_DRocket: {
            currentGameMap->damage(shooterID, owner, position, bulletID, damage, damageRadius, airAttack);
            soundPlayer->playSoundAt(Sound_ExplosionGas, position);
            currentGame->getExplosionList().push_back(new Explosion(Explosion_Gas,position,houseID));
        } break;

        case Bullet_LargeRocket: {
            soundPlayer->playSoundAt(Sound_ExplosionLarge, position);

            for(int i = 0; i < 5; i++) {
                for(int j = 0; j < 5; j++) {
                    if (((i != 0) && (i != 4)) || ((j != 0) && (j != 4))) {
                        position.x = lround(realX) + (i - 2)*TILESIZE;
                        position.y = lround(realY) + (j - 2)*TILESIZE;

                        currentGameMap->damage(shooterID, owner, position, bulletID, damage, damageRadius, airAttack);

                        Uint32 explosionID = currentGame->randomGen.getRandOf(2,Explosion_Large1,Explosion_Large2);
                        currentGame->getExplosionList().push_back(new Explosion(explosionID,position,houseID));
                        screenborder->shakeScreen(22);
                    }
                }
            }
        } break;

        case Bullet_Rocket:
        case Bullet_TurretRocket:
        case Bullet_SmallRocket: {
            currentGameMap->damage(shooterID, owner, position, bulletID, damage, damageRadius, airAttack);
            currentGame->getExplosionList().push_back(new Explosion(Explosion_Small,position,houseID));
        } break;

        case Bullet_ShellSmall: {
            currentGameMap->damage(shooterID, owner, position, bulletID, damage, damageRadius, airAttack);
            currentGame->getExplosionList().push_back(new Explosion(Explosion_ShellSmall,position,houseID));
        } break;

        case Bullet_ShellMedium: {
            currentGameMap->damage(shooterID, owner, position, bulletID, damage, damageRadius, airAttack);
            currentGame->getExplosionList().push_back(new Explosion(Explosion_ShellMedium,position,houseID));
        } break;

        case Bullet_ShellLarge: {
            currentGameMap->damage(shooterID, owner, position, bulletID, damage, damageRadius, airAttack);
            currentGame->getExplosionList().push_back(new Explosion(Explosion_ShellLarge,position,houseID));
        } break;

        case Bullet_Sonic:
        case Bullet_Sandworm:
        default: {
            // do nothing
        } break;
    }

    bulletList.remove(this);
    delete this;
}

