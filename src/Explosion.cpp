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
#include <misc/exceptions.h>

namespace {
inline constexpr auto CYCLES_PER_FRAME = 5;
}

Explosion::Explosion()
    : explosionID(NONE_ID), house(HOUSETYPE::HOUSE_HARKONNEN), currentFrame(0), frameTimer(CYCLES_PER_FRAME) { }

Explosion::Explosion(uint32_t explosionID, const Coord& position, HOUSETYPE house)
    : explosionID(explosionID), position(position), house(house), currentFrame(0), frameTimer(CYCLES_PER_FRAME) {
    init();
}

Explosion::Explosion(InputStream& stream) {
    explosionID  = stream.readUint32();
    position.x   = stream.readSint16();
    position.y   = stream.readSint16();
    house        = static_cast<HOUSETYPE>(stream.readUint32());
    frameTimer   = stream.readSint32();
    currentFrame = stream.readSint32();

    init();
}

Explosion::~Explosion() = default;

void Explosion::init() {
    auto* const gfx = dune::globals::pGFXManager.get();

    switch (explosionID) {
        case Explosion_Small: {
            graphic   = gfx->getObjPic(ObjPic_ExplosionSmall);
            numFrames = 5;
        } break;

        case Explosion_Medium1: {
            graphic   = gfx->getObjPic(ObjPic_ExplosionMedium1);
            numFrames = 5;
        } break;

        case Explosion_Medium2: {
            graphic   = gfx->getObjPic(ObjPic_ExplosionMedium2);
            numFrames = 5;
        } break;

        case Explosion_Large1: {
            graphic   = gfx->getObjPic(ObjPic_ExplosionLarge1);
            numFrames = 5;
        } break;

        case Explosion_Large2: {
            graphic   = gfx->getObjPic(ObjPic_ExplosionLarge2);
            numFrames = 5;
        } break;

        case Explosion_Gas: {
            graphic   = gfx->getObjPic(ObjPic_Hit_Gas, house);
            numFrames = 5;
        } break;

        case Explosion_ShellSmall: {
            graphic   = gfx->getObjPic(ObjPic_Hit_ShellSmall);
            numFrames = 1;
        } break;

        case Explosion_ShellMedium: {
            graphic   = gfx->getObjPic(ObjPic_Hit_ShellMedium);
            numFrames = 1;
        } break;

        case Explosion_ShellLarge: {
            graphic   = gfx->getObjPic(ObjPic_Hit_ShellLarge);
            numFrames = 1;
        } break;

        case Explosion_SmallUnit: {
            graphic   = gfx->getObjPic(ObjPic_ExplosionSmallUnit);
            numFrames = 2;
        } break;

        case Explosion_Flames: {
            graphic   = gfx->getObjPic(ObjPic_ExplosionFlames);
            numFrames = 21;
        } break;

        case Explosion_SpiceBloom: {
            graphic   = gfx->getObjPic(ObjPic_ExplosionSpiceBloom);
            numFrames = 3;
        } break;

        default: {
            THROW(std::invalid_argument, "Unknown explosion type %d", explosionID);
        }
    }
}

void Explosion::save(OutputStream& stream) const {
    stream.writeUint32(explosionID);
    stream.writeSint16(position.x);
    stream.writeSint16(position.y);
    stream.writeUint32(static_cast<uint32_t>(house));
    stream.writeSint32(frameTimer);
    stream.writeSint32(currentFrame);
}

void Explosion::blitToScreen() const {
    const auto zoom          = dune::globals::currentZoomlevel;
    auto* const screenborder = dune::globals::screenborder.get();

    const uint16_t width  = getWidth(graphic[zoom]) / numFrames;
    const uint16_t height = getHeight(graphic[zoom]);

    if (screenborder->isInsideScreen(position, Coord(width, height))) {
        const auto dest   = calcSpriteDrawingRect(graphic[zoom], screenborder->world2screenX(position.x),
                                                  screenborder->world2screenY(position.y), numFrames, 1, HAlign::Center,
                                                  VAlign::Center);
        const auto source = calcSpriteSourceRect(graphic[zoom], currentFrame, numFrames);
        Dune_RenderCopyF(dune::globals::renderer.get(), graphic[zoom], &source, &dest);
    }
}

bool Explosion::update() {
    frameTimer--;

    if (frameTimer < 0) {
        frameTimer = CYCLES_PER_FRAME;
        currentFrame++;

        if (currentFrame >= numFrames) {
            // this explosion is finished
            return true;
        }
    }

    return false;
}
