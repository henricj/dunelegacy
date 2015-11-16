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

#include <Explosion.h>

#include <globals.h>

#include <FileClasses/GFXManager.h>
#include <Game.h>
#include <ScreenBorder.h>

#define CYCLES_PER_FRAME    5

Explosion::Explosion()
 : explosionID(NONE), house(HOUSE_HARKONNEN)
{
    graphic = NULL;
    numFrames = 0;
    frameTimer = CYCLES_PER_FRAME;
    currentFrame = 0;
}

Explosion::Explosion(Uint32 explosionID, const Coord& position, int house)
 : explosionID(explosionID), position(position) , house(house)
{
    init();

    frameTimer = CYCLES_PER_FRAME;
    currentFrame = 0;
}

Explosion::Explosion(InputStream& stream)
{
    explosionID = stream.readUint32();
    position.x = stream.readSint16();
	position.y = stream.readSint16();
	house = stream.readUint32();
    frameTimer = stream.readSint32();
    currentFrame = stream.readSint32();

    init();
}

Explosion::~Explosion()
{

}

void Explosion::init()
{
    switch(explosionID) {
        case Explosion_Small: {
            graphic = pGFXManager->getObjPic(ObjPic_ExplosionSmall);
            numFrames = 5;
        } break;

        case Explosion_Medium1: {
            graphic = pGFXManager->getObjPic(ObjPic_ExplosionMedium1);
            numFrames = 5;
        } break;

        case Explosion_Medium2: {
            graphic = pGFXManager->getObjPic(ObjPic_ExplosionMedium2);
            numFrames = 5;
        } break;

        case Explosion_Large1: {
            graphic = pGFXManager->getObjPic(ObjPic_ExplosionLarge1);
            numFrames = 5;
        } break;

        case Explosion_Large2: {
            graphic = pGFXManager->getObjPic(ObjPic_ExplosionLarge2);
            numFrames = 5;
        } break;

        case Explosion_Gas: {
            graphic = pGFXManager->getObjPic(ObjPic_Hit_Gas, house);
            numFrames = 5;
        } break;

        case Explosion_ShellSmall: {
            graphic = pGFXManager->getObjPic(ObjPic_Hit_ShellSmall);
            numFrames = 1;
        } break;

        case Explosion_ShellMedium: {
            graphic = pGFXManager->getObjPic(ObjPic_Hit_ShellMedium);
            numFrames = 1;
        } break;

        case Explosion_ShellLarge: {
            graphic = pGFXManager->getObjPic(ObjPic_Hit_ShellLarge);
            numFrames = 1;
        } break;

        case Explosion_SmallUnit: {
            graphic = pGFXManager->getObjPic(ObjPic_ExplosionSmallUnit);
            numFrames = 2;
        } break;

        case Explosion_Flames: {
            graphic = pGFXManager->getObjPic(ObjPic_ExplosionFlames);
            numFrames = 21;
        } break;

        case Explosion_SpiceBloom: {
            graphic = pGFXManager->getObjPic(ObjPic_ExplosionSpiceBloom);
            numFrames = 3;
        } break;

        default: {
            fprintf(stderr,"Explosion: Unknown explosion type %d.\n",explosionID);
            exit(EXIT_FAILURE);
        } break;
    }
}

void Explosion::save(OutputStream& stream) const
{
    stream.writeUint32(explosionID);
    stream.writeSint16(position.x);
	stream.writeSint16(position.y);
	stream.writeUint32(house);
    stream.writeSint32(frameTimer);
    stream.writeSint32(currentFrame);
}

void Explosion::blitToScreen() const
{
    Uint16 width = graphic[currentZoomlevel]->w/numFrames;
    Uint16 height = graphic[currentZoomlevel]->h;

    if(screenborder->isInsideScreen(position, Coord(width, height))) {
        SDL_Rect dest = {   static_cast<Sint16>(screenborder->world2screenX(position.x) - width/2),
                            static_cast<Sint16>(screenborder->world2screenY(position.y) - height/2),
                            width,
                            height };

        SDL_Rect source = { static_cast<Sint16>(width * currentFrame),
                            0,
                            width,
                            height };
        SDL_BlitSurface(graphic[currentZoomlevel], &source, screen, &dest);
    }
}

void Explosion::update()
{
    frameTimer--;

    if(frameTimer < 0) {
        frameTimer = CYCLES_PER_FRAME;
        currentFrame++;

        if(currentFrame >= numFrames) {
            //this explosion is finished
            currentGame->getExplosionList().remove(this);
            delete this;
        }
    }
}
