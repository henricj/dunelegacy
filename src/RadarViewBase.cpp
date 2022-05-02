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

#include <RadarViewBase.h>

RadarViewBase::RadarViewBase() : Widget(), bRadarInteraction(false) {
    RadarViewBase::resize(RADARWIDTH + (2 * RADARVIEW_BORDERTHICKNESS), RADARHEIGHT + (2 * RADARVIEW_BORDERTHICKNESS));
}

RadarViewBase::~RadarViewBase() = default;
bool RadarViewBase::isOnRadar(int mouseX, int mouseY) const {
    int scale   = 1;
    int offsetX = 0;
    int offsetY = 0;

    calculateScaleAndOffsets(getMapSizeX(), getMapSizeY(), scale, offsetX, offsetY);

    const int offsetFromRightX  = 128 - getMapSizeX() * scale - offsetX;
    const int offsetFromBottomY = 128 - getMapSizeY() * scale - offsetY;

    return ((mouseX >= offsetX + RADARVIEW_BORDERTHICKNESS)
            && (mouseX < RADARWIDTH - offsetFromRightX + RADARVIEW_BORDERTHICKNESS)
            && (mouseY >= offsetY + RADARVIEW_BORDERTHICKNESS)
            && (mouseY < RADARHEIGHT - offsetFromBottomY + RADARVIEW_BORDERTHICKNESS));
}

Coord RadarViewBase::getWorldCoords(int mouseX, int mouseY) const {
    const Coord positionOnRadar(mouseX - RADARVIEW_BORDERTHICKNESS, mouseY - RADARVIEW_BORDERTHICKNESS);

    int scale   = 1;
    int offsetX = 0;
    int offsetY = 0;

    calculateScaleAndOffsets(getMapSizeX(), getMapSizeY(), scale, offsetX, offsetY);

    const auto x = ((positionOnRadar.x - offsetX) * getMapSizeX() * TILESIZE) / (getMapSizeX() * scale);
    const auto y = ((positionOnRadar.y - offsetY) * getMapSizeY() * TILESIZE) / (getMapSizeY() * scale);

    return {x, y};
}

void RadarViewBase::calculateScaleAndOffsets(int MapSizeX, int MapSizeY, int& scale, int& offsetX, int& offsetY) {
    scale   = 1;
    offsetX = 0;
    offsetY = 0;

    if (MapSizeX <= 32 && MapSizeY <= 32) {
        scale *= 2;
    }

    if (MapSizeX <= 64 && MapSizeY <= 64) {
        scale *= 2;
    }

    if (MapSizeX <= 21 && MapSizeY <= 21) {
        scale++;
    }

    offsetX = (128 - (MapSizeX * scale)) / 2;
    offsetY = (128 - (MapSizeY * scale)) / 2;
}

void RadarViewBase::handleMouseMovement(int32_t x, int32_t y, bool insideOverlay) {
    if (bRadarInteraction && isOnRadar(x, y)) {
        if (pOnRadarClick) {
            bRadarInteraction = pOnRadarClick(getWorldCoords(x, y), false, true);
        }
    }
}

bool RadarViewBase::handleMouseLeft(int32_t x, int32_t y, bool pressed) {
    if (pressed) {
        if (isOnRadar(x, y)) {
            if (pOnRadarClick) {
                bRadarInteraction = pOnRadarClick(getWorldCoords(x, y), false, false);
            }
            return true;
        }
        return false;
    }
    bRadarInteraction = false;
    return false;
}

bool RadarViewBase::handleMouseRight(int32_t x, int32_t y, bool pressed) {
    if (pressed) {
        if (isOnRadar(x, y)) {
            if (pOnRadarClick) {
                bRadarInteraction = pOnRadarClick(getWorldCoords(x, y), true, false);
            }
            return true;
        }
        return false;
    }
    bRadarInteraction = false;
    return false;
}
