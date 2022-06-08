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

#ifndef SURFACELOADER_H
#define SURFACELOADER_H

#include "GFXConstants.h"

#include "Animation.h"
#include "Shpfile.h"
#include "Wsafile.h"
#include <DataTypes.h>

#include <misc/SDL2pp.h>

#include <array>
#include <memory>
#include <string>

#include "PictureFactory.h"

class SurfaceLoader final {
public:
    SurfaceLoader(int width, int height);
    ~SurfaceLoader();

    SurfaceLoader(const SurfaceLoader&)            = delete;
    SurfaceLoader(SurfaceLoader&&)                 = delete;
    SurfaceLoader& operator=(const SurfaceLoader&) = delete;
    SurfaceLoader& operator=(SurfaceLoader&&)      = delete;

    SDL_Surface* getZoomedObjSurface(unsigned int id, HOUSETYPE house, unsigned int z);
    SDL_Surface* getZoomedObjSurface(unsigned int id, unsigned int z) {
        return getZoomedObjSurface(id, HOUSETYPE::HOUSE_HARKONNEN, z);
    }

    SDL_Surface* getSmallDetailSurface(unsigned int id);
    SDL_Surface* getTinyPictureSurface(unsigned int id);

    SDL_Surface* getUIGraphicSurface(unsigned int id, HOUSETYPE house = HOUSETYPE::HOUSE_HARKONNEN);
    SDL_Surface* getMapChoicePieceSurface(unsigned int num, HOUSETYPE house);

    Animation* getAnimation(unsigned int id);

    sdl2::surface_ptr generateWindtrapAnimationFrames(SDL_Surface* windtrapPic) const;
    static sdl2::surface_ptr
    generateMapChoiceArrowFrames(SDL_Surface* arrowPic, HOUSETYPE house = HOUSETYPE::HOUSE_HARKONNEN);
    [[nodiscard]] sdl2::surface_ptr extractSmallDetailPic(const std::string& filename) const;

    [[nodiscard]] SDL_Surface* getBackgroundSurface() const { return pBackgroundSurface.get(); }

    struct DecorationBorderType {
        SDL_Surface* ball;
        SDL_Surface* hspacer;
        SDL_Surface* vspacer;
        SDL_Surface* hborder;
        SDL_Surface* vborder;
    };

    struct BorderStyle {
        SDL_Surface* leftUpperCorner;
        SDL_Surface* rightUpperCorner;
        SDL_Surface* leftLowerCorner;
        SDL_Surface* rightLowerCorner;
        SDL_Surface* hborder;
        SDL_Surface* vborder;
    };

    [[nodiscard]] DecorationBorderType getDecorationBorder() const;
    [[nodiscard]] BorderStyle getBorderStyle(DecorationFrame type) const;

private:
    [[nodiscard]] std::unique_ptr<Animation> loadAnimationFromWsa(const std::string& filename) const;

    [[nodiscard]] std::unique_ptr<Shpfile> loadShpfile(const std::string& filename) const;
    [[nodiscard]] std::unique_ptr<Wsafile> loadWsafile(const std::string& filename) const;

    [[nodiscard]] sdl2::surface_ptr generateDoubledObjPic(unsigned int id, int h) const;
    [[nodiscard]] sdl2::surface_ptr generateTripledObjPic(unsigned int id, int h) const;

    // 8-bit surfaces kept in main memory for processing as needed, e.g. color remapping
    std::array<std::array<std::array<sdl2::surface_ptr, NUM_ZOOMLEVEL>, NUM_HOUSES>, NUM_OBJPICS> objPic;
    std::array<std::array<sdl2::surface_ptr, NUM_HOUSES>, NUM_UIGRAPHICS> uiGraphic;
    std::array<std::array<sdl2::surface_ptr, NUM_HOUSES>, NUM_MAPCHOICEPIECES> mapChoicePieces;
    std::array<std::unique_ptr<Animation>, NUM_ANIMATION> animation{};

    std::array<sdl2::surface_ptr, NUM_SMALLDETAILPICS> smallDetailPic;
    std::array<sdl2::surface_ptr, NUM_TINYPICTURE> tinyPicture;

    // 32-bit surfaces
    sdl2::surface_ptr pBackgroundSurface;

    PictureFactory ::DecorationBorderType decorationBorder_;
    std::array<PictureFactory ::BorderStyle, static_cast<int>(DecorationFrame::NUM_DECORATIONFRAMES)> frame_;
};

#endif // SURFACELOADER_H
