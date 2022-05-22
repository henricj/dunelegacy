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

#ifndef GFXMANAGER_H
#define GFXMANAGER_H

#include "GFXConstants.h"

#include "SurfaceLoader.h"
#include <Renderer/DuneTextures.h>

#include "Animation.h"
#include "misc/Random.h"

#include <DataTypes.h>

#include <misc/SDL2pp.h>

#include <array>
#include <memory>
#include <string>

class GFXManager final {
public:
    GFXManager();
    ~GFXManager();

    GFXManager(const GFXManager&)            = delete;
    GFXManager(GFXManager&&)                 = delete;
    GFXManager& operator=(const GFXManager&) = delete;
    GFXManager& operator=(GFXManager&&)      = delete;

    [[nodiscard]] const DuneTexture* getZoomedObjPic(ObjPic_enum id, HOUSETYPE house, unsigned int z) const;

    [[nodiscard]] const DuneTexture* getZoomedObjPic(ObjPic_enum id, unsigned int z) const {
        return getZoomedObjPic(id, HOUSETYPE::HOUSE_HARKONNEN, z);
    }
    zoomable_texture getObjPic(ObjPic_enum id, HOUSETYPE house = HOUSETYPE::HOUSE_HARKONNEN) const;

    [[nodiscard]] const DuneTexture* getSmallDetailPic(SmallDetailPics_Enum id) const;
    [[nodiscard]] const DuneTexture* getTinyPicture(TinyPicture_Enum id) const;
    [[nodiscard]] const DuneTexture*
    getUIGraphic(UIGraphics_Enum id, HOUSETYPE house = HOUSETYPE::HOUSE_HARKONNEN) const;
    [[nodiscard]] const DuneTexture* getMapChoicePiece(UIGraphics_Enum num, HOUSETYPE house) const;
    [[nodiscard]] const DuneTexture* getGeneratedPicture(GeneratedPicture id) const;

    [[nodiscard]] SDL_Cursor* getCursor(UIGraphics_Enum id) const;
    [[nodiscard]] SDL_Cursor* getDefaultCursor() const { return default_cursor_.get(); }

    SDL_Surface* getZoomedObjSurface(ObjPic_enum id, HOUSETYPE house, unsigned int z) {
        return surfaceLoader.getZoomedObjSurface(id, house, z);
    }
    SDL_Surface* getZoomedObjSurface(ObjPic_enum id, unsigned int z) {
        return surfaceLoader.getZoomedObjSurface(id, z);
    }

    SDL_Surface* getUIGraphicSurface(UIGraphics_Enum id, HOUSETYPE house = HOUSETYPE::HOUSE_HARKONNEN) {
        return surfaceLoader.getUIGraphicSurface(id, house);
    }
    SDL_Surface* getMapChoicePieceSurface(UIGraphics_Enum num, HOUSETYPE house) {
        return surfaceLoader.getMapChoicePieceSurface(num, house);
    }

    Animation* getAnimation(unsigned int id) { return surfaceLoader.getAnimation(id); }

    [[nodiscard]] SDL_Surface* getBackgroundSurface() const { return surfaceLoader.getBackgroundSurface(); }

    [[nodiscard]] SDL_Texture* getTempStreamingTexture(SDL_Renderer* renderer, int width, int height);

    [[nodiscard]] DuneTextures::DecorationBorderType getDecorationBorder() const {
        return duneTextures.getDecorationBorder();
    }

    [[nodiscard]] DuneTextures::BorderStyle getBorderStyle(DecorationFrame type) const {
        return duneTextures.getBorderStyle(type);
    }

    Random& random() noexcept { return random_; }

private:
    Random random_; ///< This random number generator is for use by the UI so that the game RNGs will not be disrupted.

    [[nodiscard]] sdl2::texture_ptr extractSmallDetailPicTex(const std::string& filename) const;

    void initialize_cursors();

    SurfaceLoader surfaceLoader;
    DuneTextures duneTextures;

    // Textures
    std::array<std::array<std::array<sdl2::texture_ptr, NUM_ZOOMLEVEL>, NUM_HOUSES>, NUM_OBJPICS> objPicTex;
    std::array<sdl2::texture_ptr, NUM_SMALLDETAILPICS> smallDetailPicTex;
    std::array<sdl2::texture_ptr, NUM_TINYPICTURE> tinyPictureTex;

    std::array<std::array<sdl2::texture_ptr, NUM_HOUSES>, NUM_UIGRAPHICS> uiGraphicTex;
    std::array<std::array<sdl2::texture_ptr, NUM_HOUSES>, NUM_MAPCHOICEPIECES> mapChoicePiecesTex;

    sdl2::cursor_ptr default_cursor_;
    std::unordered_map<UIGraphics_Enum, sdl2::cursor_ptr> cursors_;

    std::vector<sdl2::texture_ptr> streamingTextureCache_;

    DuneTextureOwned mainBackground_;
};

#endif // GFXMANAGER_H
