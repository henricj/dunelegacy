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

#include <units/AirUnit.h>

#include <globals.h>

#include <House.h>
#include <Game.h>
#include <Explosion.h>
#include <SoundPlayer.h>
#include <Map.h>
#include <ScreenBorder.h>

#include <FileClasses/GFXManager.h>

#include <misc/draw_util.h>

#include <structures/RepairYard.h>

AirUnit::AirUnit(House* newOwner) : UnitBase(newOwner)
{
    AirUnit::init();

    currentMaxSpeed = 2;
}

AirUnit::AirUnit(InputStream& stream) : UnitBase(stream)
{
    AirUnit::init();

    currentMaxSpeed = stream.readFixPoint();
}

void AirUnit::init()
{
    aFlyingUnit = true;
}

AirUnit::~AirUnit() = default;

void AirUnit::save(OutputStream& stream) const
{
    UnitBase::save(stream);

    stream.writeFixPoint(currentMaxSpeed);
}

void AirUnit::destroy()
{
    if(isVisible()) {
        Coord position(lround(realX), lround(realY));
        currentGame->getExplosionList().push_back(new Explosion(Explosion_Medium2, position, owner->getHouseID()));

        if(isVisible(getOwner()->getTeamID()))
            soundPlayer->playSoundAt(Sound_ExplosionMedium,location);
    }

    UnitBase::destroy();
}

void AirUnit::assignToMap(const Coord& pos)
{
    if(currentGameMap->tileExists(pos)) {
        if(guardPoint.isInvalid()) {
            guardPoint = pos;
        }

        currentGameMap->getTile(pos)->assignAirUnit(getObjectID());
        // do not reveal map for air units
        // currentGameMap->viewMap(owner->getHouseID(), location, getViewRange());
    }
}

void AirUnit::checkPos()
{
    // do nothing
}

void AirUnit::blitToScreen()
{
    if(settings.video.rotateUnitGraphics) {
        double rotationAngleDeg = -angle.toDouble()*360.0/8.0;

        if(shadowGraphic[currentZoomlevel] != nullptr) {
            int x = screenborder->world2screenX(realX + 4);
            int y = screenborder->world2screenY(realY + 12);

            SDL_Rect source = calcSpriteSourceRect(shadowGraphic[currentZoomlevel], RIGHT, numImagesX, drawnFrame, numImagesY);
            SDL_Rect dest = calcSpriteDrawingRect(shadowGraphic[currentZoomlevel], x, y, numImagesX, numImagesY, HAlign::Center, VAlign::Center);

            SDL_RenderCopyEx(renderer, shadowGraphic[currentZoomlevel], &source, &dest, rotationAngleDeg, nullptr, SDL_FLIP_NONE);
        }

        int x = screenborder->world2screenX(realX);
        int y = screenborder->world2screenY(realY);

        SDL_Texture* pUnitGraphic = graphic[currentZoomlevel];
        SDL_Rect source = calcSpriteSourceRect(pUnitGraphic, RIGHT, numImagesX, drawnFrame, numImagesY);
        SDL_Rect dest = calcSpriteDrawingRect( pUnitGraphic, x, y, numImagesX, numImagesY, HAlign::Center, VAlign::Center);

        SDL_RenderCopyEx(renderer, pUnitGraphic, &source, &dest, rotationAngleDeg, nullptr, SDL_FLIP_NONE);
    } else {
        if(shadowGraphic[currentZoomlevel] != nullptr) {
            int x = screenborder->world2screenX(realX + 4);
            int y = screenborder->world2screenY(realY + 12);

            SDL_Rect source = calcSpriteSourceRect(shadowGraphic[currentZoomlevel], drawnAngle, numImagesX, drawnFrame, numImagesY);
            SDL_Rect dest = calcSpriteDrawingRect(shadowGraphic[currentZoomlevel], x, y, numImagesX, numImagesY, HAlign::Center, VAlign::Center);

            SDL_RenderCopy(renderer, shadowGraphic[currentZoomlevel], &source, &dest);
        }

        int x = screenborder->world2screenX(realX);
        int y = screenborder->world2screenY(realY);

        SDL_Texture* pUnitGraphic = graphic[currentZoomlevel];
        SDL_Rect source = calcSpriteSourceRect(pUnitGraphic, drawnAngle, numImagesX, drawnFrame, numImagesY);
        SDL_Rect dest = calcSpriteDrawingRect( pUnitGraphic, x, y, numImagesX, numImagesY, HAlign::Center, VAlign::Center);

        SDL_RenderCopy(renderer, pUnitGraphic, &source, &dest);
    }
}

bool AirUnit::canPass(int xPos, int yPos) const
{
    return true;
}

void AirUnit::navigate() {
    moving = true;
    justStoppedMoving = false;
}

void AirUnit::move() {
    FixPoint angleRad = (angle * (FixPt_PI << 1)) / 8;
    FixPoint speed = getMaxSpeed();

    realX += FixPoint::cos(angleRad)*speed;
    realY += -FixPoint::sin(angleRad)*speed;

    Coord newLocation = Coord(realX.lround()/TILESIZE, realY.lround()/TILESIZE);

    if(newLocation != location) {
        unassignFromMap(location);
        assignToMap(newLocation);
        location = newLocation;
    }

    checkPos();
}

FixPoint AirUnit::getDestinationAngle() const {
    return destinationAngleRad(realX, realY, destination.x*TILESIZE + TILESIZE/2, destination.y*TILESIZE + TILESIZE/2)*8 / (FixPt_PI << 1);;
}

void AirUnit::turn() {
    if(destination.isValid()) {
        FixPoint destinationAngle = getDestinationAngle();

        FixPoint angleLeft = 0;
        FixPoint angleRight = 0;

        if(angle > destinationAngle) {
            angleRight = angle - destinationAngle;
            angleLeft = FixPoint::abs(NUM_ANGLES-angle)+destinationAngle;
        } else if (angle < destinationAngle) {
            angleRight = FixPoint::abs(NUM_ANGLES-destinationAngle) + angle;
            angleLeft = destinationAngle - angle;
        }

        if(angleLeft <= angleRight) {
            angle += std::min(currentGame->objectData.data[itemID][originalHouseID].turnspeed, angleLeft);
            if(angle >= NUM_ANGLES) {
                angle -= NUM_ANGLES;
            }
            drawnAngle = lround(angle) % NUM_ANGLES;
        } else {
            angle -= std::min(currentGame->objectData.data[itemID][originalHouseID].turnspeed, angleRight);
            if(angle < 0) {
                angle += NUM_ANGLES;
            }
            drawnAngle = lround(angle) % NUM_ANGLES;
        }
    } else {
        angle -= currentGame->objectData.data[itemID][originalHouseID].turnspeed / 8;
        if(angle < 0) {
            angle += NUM_ANGLES;
        }
        drawnAngle = lround(angle) % NUM_ANGLES;
    }
}
