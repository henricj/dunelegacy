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

#include <Explosion.h>
#include <Game.h>
#include <House.h>
#include <Map.h>
#include <ScreenBorder.h>
#include <SoundPlayer.h>

#include <FileClasses/GFXManager.h>

#include <misc/draw_util.h>

AirUnit::AirUnit(const AirUnitConstants& constants, uint32_t objectID, const ObjectInitializer& initializer)
    : UnitBase(constants, objectID, initializer) { }

AirUnit::AirUnit(const AirUnitConstants& constants, uint32_t objectID, const ObjectStreamInitializer& initializer)
    : UnitBase(constants, objectID, initializer) {
    auto& stream = initializer.stream();

    currentMaxSpeed = stream.readFixPoint();
}

AirUnit::~AirUnit() = default;

void AirUnit::save(OutputStream& stream) const {
    UnitBase::save(stream);

    stream.writeFixPoint(currentMaxSpeed);
}

void AirUnit::destroy(const GameContext& context) {
    if (isVisible()) {
        Coord position(lround(realX), lround(realY));
        context.game.addExplosion(Explosion_Medium2, position, owner->getHouseID());

        if (isVisible(getOwner()->getTeamID()))
            dune::globals::soundPlayer->playSoundAt(Sound_enum::Sound_ExplosionMedium, location);
    }

    UnitBase::destroy(context);
}

void AirUnit::assignToMap(const GameContext& context, const Coord& pos) {
    auto& map = context.map;

    if (map.tileExists(pos)) {
        if (guardPoint.isInvalid()) {
            guardPoint = pos;
        }

        map.getTile(pos)->assignAirUnit(getObjectID());
        // do not reveal map for air units
        // currentGameMap->viewMap(owner->getHouseID(), location, getViewRange());
    }
}

void AirUnit::checkPos(const GameContext& context) {
    // do nothing
}

void AirUnit::blitToScreen() {
    const auto* const screenborder = dune::globals::screenborder.get();
    auto* const renderer           = dune::globals::renderer.get();
    const auto zoom                = dune::globals::currentZoomlevel;

    const auto* const shadow       = shadowGraphic[zoom];
    const auto* const pUnitGraphic = graphic[zoom];

    if (dune::globals::settings.video.rotateUnitGraphics) {
        const auto rotationAngleDeg = -angle.toDouble() * 360.0 / 8.0;

        if (shadow != nullptr) {
            const auto x = screenborder->world2screenX(realX + 4);
            const auto y = screenborder->world2screenY(realY + 12);

            const auto source =
                calcSpriteSourceRect(shadow, static_cast<int>(ANGLETYPE::RIGHT), numImagesX, drawnFrame, numImagesY);
            const auto dest =
                calcSpriteDrawingRect(shadow, x, y, numImagesX, numImagesY, HAlign::Center, VAlign::Center);

            Dune_RenderCopyExF(renderer, shadow, &source, &dest, rotationAngleDeg, nullptr, SDL_FLIP_NONE);
        }

        { // Scope
            const auto x = screenborder->world2screenX(realX);
            const auto y = screenborder->world2screenY(realY);

            const auto source = calcSpriteSourceRect(pUnitGraphic, static_cast<int>(ANGLETYPE::RIGHT), numImagesX,
                                                     drawnFrame, numImagesY);
            const auto dest =
                calcSpriteDrawingRect(pUnitGraphic, x, y, numImagesX, numImagesY, HAlign::Center, VAlign::Center);

            Dune_RenderCopyExF(renderer, pUnitGraphic, &source, &dest, rotationAngleDeg, nullptr, SDL_FLIP_NONE);
        }
    } else {
        if (shadow != nullptr) {
            const auto x = screenborder->world2screenX(realX + 4);
            const auto y = screenborder->world2screenY(realY + 12);

            const auto source =
                calcSpriteSourceRect(shadow, static_cast<int>(drawnAngle), numImagesX, drawnFrame, numImagesY);
            const auto dest =
                calcSpriteDrawingRect(shadow, x, y, numImagesX, numImagesY, HAlign::Center, VAlign::Center);

            Dune_RenderCopyF(renderer, shadow, &source, &dest);
        }

        const auto x = screenborder->world2screenX(realX);
        const auto y = screenborder->world2screenY(realY);

        const auto source =
            calcSpriteSourceRect(pUnitGraphic, static_cast<int>(drawnAngle), numImagesX, drawnFrame, numImagesY);
        const auto dest =
            calcSpriteDrawingRect(pUnitGraphic, x, y, numImagesX, numImagesY, HAlign::Center, VAlign::Center);

        Dune_RenderCopyF(renderer, pUnitGraphic, &source, &dest);
    }
}

void AirUnit::navigate(const GameContext& context) {
    moving            = true;
    justStoppedMoving = false;
}

void AirUnit::move(const GameContext& context) {
    const FixPoint angleRad = (angle * (FixPt_PI << 1)) / 8;
    const FixPoint speed    = getMaxSpeed(context);

    realX += FixPoint::cos(angleRad) * speed;
    realY += -FixPoint::sin(angleRad) * speed;

    const Coord newLocation = Coord(realX.lround() / TILESIZE, realY.lround() / TILESIZE);

    if (newLocation != location) {
        unassignFromMap(location);
        assignToMap(context, newLocation);
        location = newLocation;
    }

    checkPos(context);
}

FixPoint AirUnit::getDestinationAngle() const {
    return destinationAngleRad(realX, realY, destination.x * TILESIZE + TILESIZE / 2,
                               destination.y * TILESIZE + TILESIZE / 2)
         * 8 / (FixPt_PI << 1);
}

void AirUnit::turn(const GameContext& context) {
    const auto turn_speed = context.game.objectData.data[itemID][static_cast<int>(originalHouseID)].turnspeed;

    if (destination.isValid()) {
        const auto destinationAngle = getDestinationAngle();

        FixPoint angleLeft  = 0;
        FixPoint angleRight = 0;

        if (angle > destinationAngle) {
            angleRight = angle - destinationAngle;
            angleLeft  = FixPoint::abs(NUM_ANGLES - angle) + destinationAngle;
        } else if (angle < destinationAngle) {
            angleRight = FixPoint::abs(NUM_ANGLES - destinationAngle) + angle;
            angleLeft  = destinationAngle - angle;
        }

        if (angleLeft <= angleRight) {
            angle += std::min(turn_speed, angleLeft);
            if (angle > NUM_ANGLES) {
                angle -= NUM_ANGLES;
            }
            drawnAngle = normalizeAngle(static_cast<ANGLETYPE>(lround(angle)));
        } else {
            angle -= std::min(turn_speed, angleRight);
            if (angle < 0) {
                angle += NUM_ANGLES;
            }
            drawnAngle = normalizeAngle(static_cast<ANGLETYPE>(lround(angle)));
        }
    } else {
        angle -= turn_speed / 8;
        if (angle < 0) {
            angle += NUM_ANGLES;
        }
        drawnAngle = normalizeAngle(static_cast<ANGLETYPE>(lround(angle)));
    }
}

bool AirUnit::canPassTile(const Tile* pTile) const {
    return pTile;
}
