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

#include <FileClasses/GFXManager.h>

#include <globals.h>

#include <FileClasses/SurfaceLoader.h>
#include <Renderer/DuneTextures.h>

#include <misc/draw_util.h>
#include <misc/exceptions.h>

GFXManager::GFXManager()
    : random_ {RandomFactory {}.create("UI")}, duneTextures {DuneTextures::create(renderer, &surfaceLoader)} { }

GFXManager::~GFXManager() = default;

const DuneTexture* GFXManager::getZoomedObjPic(unsigned int id, HOUSETYPE house, unsigned int z) const {
    return &duneTextures.get_object_picture(id, house, z);
#if 0
    auto* surface = surfaceLoader.getZoomedObjSurface(id, house, z);

    const auto idx = static_cast<int>(house);

    if(objPicTex[id][idx][z] == nullptr) {
        auto make_shadow_texture = [](SDL_Surface* surface, Uint32 oldColor, Uint32 newColor) {
            replaceColor(surface, oldColor, newColor);
            if(SDL_SetSurfaceBlendMode(surface, SDL_BlendMode::SDL_BLENDMODE_BLEND))
                THROW(std::runtime_error,
                      std::string("getObjPic(): SDL_SetSurfaceBlendMode() failed: ") + std::string(SDL_GetError()));
            auto tex = convertSurfaceToTexture(surface);
            if(SDL_SetTextureBlendMode(tex.get(), SDL_BlendMode::SDL_BLENDMODE_BLEND))
                THROW(std::runtime_error,
                      std::string("getObjPic(): SDL_SetTextureBlendMode() failed: ") + std::string(SDL_GetError()));

            return tex;
        };

        // now convert to display format
        if(id == ObjPic_Windtrap) {
            // Windtrap uses palette animation on PALCOLOR_WINDTRAP_COLORCYCLE; fake this
            objPicTex[id][idx][z] = convertSurfaceToTexture(surfaceLoader.generateWindtrapAnimationFrames(surface));
#    if 1
        } else if(id == ObjPic_Terrain_HiddenFog) {
            const auto pHiddenFog       = convertSurfaceToDisplayFormat(surface);
            objPicTex[id][idx][z] = make_shadow_texture(pHiddenFog.get(), COLOR_BLACK, COLOR_FOG_TRANSPARENT);
        } else if(id == ObjPic_CarryallShadow) {
            const auto pShadow          = convertSurfaceToDisplayFormat(surface);
            objPicTex[id][idx][z] = make_shadow_texture(pShadow.get(), COLOR_BLACK, COLOR_SHADOW_TRANSPARENT);
        } else if(id == ObjPic_FrigateShadow) {
            const auto pShadow          = convertSurfaceToDisplayFormat(surface);
            objPicTex[id][idx][z] = make_shadow_texture(pShadow.get(), COLOR_BLACK, COLOR_SHADOW_TRANSPARENT);
        } else if(id == ObjPic_OrnithopterShadow) {
            const auto pShadow          = convertSurfaceToDisplayFormat(surface);
            objPicTex[id][idx][z] = make_shadow_texture(pShadow.get(), COLOR_BLACK, COLOR_SHADOW_TRANSPARENT);
#    endif // 0
        } else if(id == ObjPic_Bullet_SonicTemp) {
            objPicTex[id][idx][z] = sdl2::texture_ptr{SDL_CreateTexture(
                renderer, SCREEN_FORMAT, SDL_TEXTUREACCESS_STREAMING, surface->w, surface->h)};
        } else if(id == ObjPic_SandwormShimmerTemp) {
            objPicTex[id][idx][z] = sdl2::texture_ptr{SDL_CreateTexture(
                renderer, SCREEN_FORMAT, SDL_TEXTUREACCESS_TARGET, surface->w, surface->h)};
        } else {
            objPicTex[id][idx][z] = convertSurfaceToTexture(surface);
        }
    }

    return objPicTex[id][idx][z].get();
#endif     // 0
}

zoomable_texture GFXManager::getObjPic(unsigned int id, HOUSETYPE house) const {
    if (id >= NUM_OBJPICS) {
        THROW(std::invalid_argument, "GFXManager::getObjPic(): Unit Picture with ID %u is not available!", id);
    }

    return zoomable_texture {&duneTextures.get_object_picture(id, house, 0),
                             &duneTextures.get_object_picture(id, house, 1),
                             &duneTextures.get_object_picture(id, house, 2)};
}

const DuneTexture* GFXManager::getSmallDetailPic(unsigned int id) const {
    if (id >= NUM_SMALLDETAILPICS) {
        return nullptr;
    }

    const auto& texture = duneTextures.get_small_object(id);

    if (!texture)
        sdl2::log_info("Unable to get small detail picture %u", id);

    return texture ? &texture : nullptr;
}

const DuneTexture* GFXManager::getTinyPicture(unsigned int id) const {
    if (id >= NUM_TINYPICTURE) {
        return nullptr;
    }

    const auto& texture = duneTextures.get_tiny_picture(id);

    if (!texture)
        sdl2::log_info("Unable to get tiny picture %u", id);

    return texture ? &texture : nullptr;
}

const DuneTexture* GFXManager::getGeneratedPicture(GeneratedPicture id) const {
    if (static_cast<int>(id) >= NUM_GENERATEDPICTURES) {
        THROW(std::invalid_argument,
              "GFXManager::getGeneratedPicture(): Generated pictured with ID %u is not available!", static_cast<unsigned int>(id));
    }

    const auto& texture = duneTextures.get_generated_picture(id);

    if (!texture)
        sdl2::log_info("Unable to get generated picture %u", static_cast<unsigned int>(id));

    return texture ? &texture : nullptr;
}

const DuneTexture* GFXManager::getUIGraphic(unsigned int id, HOUSETYPE house) const {
    if (id >= NUM_UIGRAPHICS) {
        THROW(std::invalid_argument, "GFXManager::getUIGraphic(): UI Graphic with ID %u is not available!", static_cast<unsigned int>(id));
    }

    const auto& texture = duneTextures.get_ui_graphic(id, house);

    if (!texture)
        sdl2::log_info("Unable to get ui graphic %u for house %d", id, static_cast<int>(house));

    return texture ? &texture : nullptr;
}

const DuneTexture* GFXManager::getMapChoicePiece(unsigned int num, HOUSETYPE house) const {
    if (num >= NUM_MAPCHOICEPIECES) {
        THROW(std::invalid_argument, "GFXManager::getMapChoicePiece(): Map Piece with number %u is not available!", num);
    }

    const auto& texture = duneTextures.get_ui_graphic(num, house);

    if (!texture)
        sdl2::log_info("Unable to get map choice piece %u for house %d", num, static_cast<int>(house));

    return texture ? &texture : nullptr;
}

SDL_Texture* GFXManager::getTempStreamingTexture(SDL_Renderer* renderer, int width, int height) {
    for (const auto& texture : streamingTextureCache_) {
        int w, h;
        SDL_QueryTexture(texture.get(), nullptr, nullptr, &w, &h);
        if (w == width && h == height)
            return texture.get();
    }

    { // Scope
        const auto& texture = streamingTextureCache_.emplace_back(
            SDL_CreateTexture(renderer, SCREEN_FORMAT, SDL_TEXTUREACCESS_STREAMING, width, height));

        return texture.get();
    }
}

sdl2::texture_ptr GFXManager::extractSmallDetailPicTex(const std::string& filename) const {
    const auto pSurface = surfaceLoader.extractSmallDetailPic(filename);

    return convertSurfaceToTexture(pSurface.get());
}
