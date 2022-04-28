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

#include <RadarView.h>

#include <globals.h>

#include <FileClasses/GFXManager.h>

#include <Game.h>
#include <House.h>
#include <Map.h>
#include <ScreenBorder.h>
#include <SoundPlayer.h>
#include <Tile.h>

#include <globals.h>

#include <misc/draw_util.h>

RadarView::RadarView()
    : currentRadarMode(RadarMode::RadarOff), animFrame(NUM_STATIC_FRAMES - 1), animCounter(NUM_STATIC_FRAME_TIME),
      radarStaticAnimation(dune::globals::pGFXManager->getUIGraphic(UI_RadarAnimation)) {

    radarSurface = sdl2::surface_ptr{SDL_CreateRGBSurfaceWithFormat(0, 128, 128, SCREEN_BPP, SCREEN_FORMAT)};
    if (radarSurface == nullptr) {
        THROW(std::runtime_error, "RadarView::RadarView(): Cannot create new surface!");
    }

    SDL_FillRect(radarSurface.get(), nullptr, SDL_MapRGB(radarSurface->format, 100, 50, 0));

    radarTexture = sdl2::texture_ptr{
        SDL_CreateTexture(dune::globals::renderer.get(), SCREEN_FORMAT, SDL_TEXTUREACCESS_STREAMING, 128, 128)};
}

RadarView::~RadarView() = default;

int RadarView::getMapSizeX() const {
    return dune::globals::currentGameMap->getSizeX();
}

int RadarView::getMapSizeY() const {
    return dune::globals::currentGameMap->getSizeY();
}

void RadarView::draw(Point position) {
    const SDL_Rect radarPosition = {position.x + RADARVIEW_BORDERTHICKNESS, position.y + RADARVIEW_BORDERTHICKNESS,
                                    RADARWIDTH, RADARHEIGHT};

    auto* const screenborder = dune::globals::screenborder.get();
    auto* const renderer     = dune::globals::renderer.get();

    switch (currentRadarMode) {
        case RadarMode::RadarOff:
        case RadarMode::RadarOn: {
            auto* const map = dune::globals::currentGameMap;

            const auto mapSizeX = map->getSizeX();
            const auto mapSizeY = map->getSizeY();

            int scale   = 1;
            int offsetX = 0;
            int offsetY = 0;

            calculateScaleAndOffsets(mapSizeX, mapSizeY, scale, offsetX, offsetY);

            updateRadarSurface(scale, offsetX, offsetY);

            SDL_UpdateTexture(radarTexture.get(), nullptr, radarSurface->pixels, radarSurface->pitch);

            const SDL_Rect dest = calcDrawingRect(radarTexture.get(), radarPosition.x, radarPosition.y);
            Dune_RenderCopy(renderer, radarTexture.get(), nullptr, &dest);

            SDL_Rect radarRect;
            radarRect.x = (screenborder->getLeft() * mapSizeX * scale) / (mapSizeX * TILESIZE) + offsetX;
            radarRect.y = (screenborder->getTop() * mapSizeY * scale) / (mapSizeY * TILESIZE) + offsetY;
            radarRect.w =
                ((screenborder->getRight() - screenborder->getLeft()) * mapSizeX * scale) / (mapSizeX * TILESIZE);
            radarRect.h =
                ((screenborder->getBottom() - screenborder->getTop()) * mapSizeY * scale) / (mapSizeY * TILESIZE);

            if (radarRect.x < offsetX) {
                radarRect.w -= radarRect.x;
                radarRect.x = offsetX;
            }

            if (radarRect.y < offsetY) {
                radarRect.h -= radarRect.y;
                radarRect.y = offsetY;
            }

            const auto offsetFromRightX = 128 - mapSizeX * scale - offsetX;
            if (radarRect.x + radarRect.w > radarPosition.w - offsetFromRightX) {
                radarRect.w = radarPosition.w - offsetFromRightX - radarRect.x - 1;
            }

            const auto offsetFromBottomY = 128 - mapSizeY * scale - offsetY;
            if (radarRect.y + radarRect.h > radarPosition.h - offsetFromBottomY) {
                radarRect.h = radarPosition.h - offsetFromBottomY - radarRect.y - 1;
            }

            renderDrawRect(renderer, radarPosition.x + radarRect.x, radarPosition.y + radarRect.y,
                           radarPosition.x + (radarRect.x + radarRect.w), radarPosition.y + (radarRect.y + radarRect.h),
                           COLOR_WHITE);

        } break;

        case RadarMode::AnimationRadarOff:
        case RadarMode::AnimationRadarOn: {
            const auto source = calcSpriteSourceRect(
                radarStaticAnimation, animFrame % NUM_STATIC_ANIMATIONS_PER_ROW, NUM_STATIC_ANIMATIONS_PER_ROW,
                animFrame / NUM_STATIC_ANIMATIONS_PER_ROW,
                (NUM_STATIC_FRAMES + NUM_STATIC_ANIMATIONS_PER_ROW - 1) / NUM_STATIC_ANIMATIONS_PER_ROW);
            const auto dest = calcSpriteDrawingRect(
                radarStaticAnimation, radarPosition.x, radarPosition.y, NUM_STATIC_ANIMATIONS_PER_ROW,
                (NUM_STATIC_FRAMES + NUM_STATIC_ANIMATIONS_PER_ROW - 1) / NUM_STATIC_ANIMATIONS_PER_ROW);
            Dune_RenderCopyF(renderer, radarStaticAnimation, &source, &dest);
        } break;
    }
}

void RadarView::update() {
    if (dune::globals::pLocalHouse->hasRadarOn()) {
        if (currentRadarMode != RadarMode::RadarOn && currentRadarMode != RadarMode::AnimationRadarOn
            && currentRadarMode != RadarMode::AnimationRadarOff) {
            switchRadarMode(true);
        }
    } else {
        if (currentRadarMode != RadarMode::RadarOff && currentRadarMode != RadarMode::AnimationRadarOn
            && currentRadarMode != RadarMode::AnimationRadarOff) {
            switchRadarMode(false);
        }
    }

    switch (currentRadarMode) {
        case RadarMode::RadarOff: {

        } break;

        case RadarMode::RadarOn: {

        } break;

        case RadarMode::AnimationRadarOff: {
            if (animFrame >= NUM_STATIC_FRAMES - 1) {
                currentRadarMode = RadarMode::RadarOff;
            } else {
                animCounter--;
                if (animCounter <= 0) {
                    animFrame++;
                    animCounter = NUM_STATIC_FRAME_TIME;
                }
            }
        } break;

        case RadarMode::AnimationRadarOn: {
            if (animFrame <= 0) {
                currentRadarMode = RadarMode::RadarOn;
            } else {
                animCounter--;
                if (animCounter <= 0) {
                    animFrame--;
                    animCounter = NUM_STATIC_FRAME_TIME;
                }
            }
        } break;
    }
}

void RadarView::switchRadarMode(bool bOn) {
    using dune::globals::soundPlayer;

    soundPlayer->playSound(Sound_enum::Sound_RadarNoise);

    const auto house_id = dune::globals::pLocalHouse->getHouseID();

    if (bOn) {
        soundPlayer->playVoice(Voice_enum::RadarActivated, house_id);
        currentRadarMode = RadarMode::AnimationRadarOn;
    } else {
        soundPlayer->playVoice(Voice_enum::RadarDeactivated, house_id);
        currentRadarMode = RadarMode::AnimationRadarOff;
    }
}

void RadarView::updateRadarSurface(int scale, int offsetX, int offsetY) {

    // Lock radarSurface for direct access to the pixels
    sdl2::surface_lock lock{radarSurface.get()};

    auto* map = dune::globals::currentGameMap;

    const auto radar_on =
        ((currentRadarMode == RadarMode::RadarOn) || (currentRadarMode == RadarMode::AnimationRadarOff));

    map->for_all([&](Tile& t) {
        auto color = t.getRadarColor(dune::globals::currentGame.get(), dune::globals::pLocalHouse, radar_on);
        color      = MapRGBA(radarSurface->format, color);

        uint8_t* const RESTRICT out =
            static_cast<uint8_t*>(radarSurface->pixels) + (offsetY + scale * t.getLocation().y) * radarSurface->pitch;
        const auto offset = (offsetX + scale * t.getLocation().x);

        for (auto j = 0; j < scale; j++) {
            auto* p = reinterpret_cast<uint32_t*>(out + j * radarSurface->pitch) + offset;

            for (auto i = 0; i < scale; ++i, ++p) {
                // Do not use putPixel here to avoid overhead
                *p = color;
            }
        }
    });
}
