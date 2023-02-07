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

#include <FileClasses/SurfaceLoader.h>
#include <Renderer/DuneTextures.h>

#include <misc/draw_util.h>
#include <misc/exceptions.h>

#include <SDL2/SDL.h>

GFXManager::GFXManager(SDL_Renderer* renderer, int width, int height)
    : random_{RandomFactory{}.create("UI")}, surfaceLoader{width, height}, duneTextures{DuneTextures::create(
                                                                               renderer, &surfaceLoader)} {
    initialize_cursors();
}

GFXManager::~GFXManager() = default;

const DuneTexture* GFXManager::getZoomedObjPic(ObjPic_enum id, HOUSETYPE house, unsigned int z) const {
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

zoomable_texture GFXManager::getObjPic(ObjPic_enum id, HOUSETYPE house) const {
    if (id >= NUM_OBJPICS) {
        THROW(std::invalid_argument, "GFXManager::getObjPic(): Unit Picture with ID {} is not available!", id);
    }

    return {&duneTextures.get_object_picture(id, house, 0), &duneTextures.get_object_picture(id, house, 1),
            &duneTextures.get_object_picture(id, house, 2)};
}

const DuneTexture* GFXManager::getSmallDetailPic(SmallDetailPics_Enum id) const {
    if (id >= NUM_SMALLDETAILPICS) {
        return nullptr;
    }

    const auto& texture = duneTextures.get_small_object(id);

    if (!texture)
        sdl2::log_info("Unable to get small detail picture {}", id);

    return texture ? &texture : nullptr;
}

const DuneTexture* GFXManager::getTinyPicture(TinyPicture_Enum id) const {
    if (id >= NUM_TINYPICTURE) {
        return nullptr;
    }

    const auto& texture = duneTextures.get_tiny_picture(id);

    if (!texture)
        sdl2::log_info("Unable to get tiny picture {}", id);

    return texture ? &texture : nullptr;
}

const DuneTexture* GFXManager::getGeneratedPicture(GeneratedPicture id) const {
    if (static_cast<int>(id) >= NUM_GENERATEDPICTURES) {
        THROW(std::invalid_argument,
              "GFXManager::getGeneratedPicture(): Generated pictured with ID {} is not available!",
              static_cast<unsigned int>(id));
    }

    const auto& texture = duneTextures.get_generated_picture(id);

    if (!texture)
        sdl2::log_info("Unable to get generated picture {}", static_cast<unsigned int>(id));

    return texture ? &texture : nullptr;
}

SDL_Cursor* GFXManager::getCursor(UIGraphics_Enum id) const {
    const auto it = cursors_.find(id);
    if (it == cursors_.end()) {
        sdl2::log_info("Unable to get cursor {}", id);
        return nullptr;
    }

    return it->second.get();
}

const DuneTexture* GFXManager::getUIGraphic(UIGraphics_Enum id, HOUSETYPE house) const {
    if (id >= NUM_UIGRAPHICS) {
        THROW(std::invalid_argument, "GFXManager::getUIGraphic(): UI Graphic with ID {} is not available!", id);
    }

    const auto& texture = duneTextures.get_ui_graphic(id, house);

    if (!texture)
        sdl2::log_info("Unable to get ui graphic {} for house {}", id, static_cast<int>(house));

    return texture ? &texture : nullptr;
}

const DuneTexture* GFXManager::getMapChoicePiece(UIGraphics_Enum num, HOUSETYPE house) const {
    if (num >= NUM_MAPCHOICEPIECES) {
        THROW(std::invalid_argument, "GFXManager::getMapChoicePiece(): Map Piece with number {} is not available!",
              num);
    }

    const auto& texture = duneTextures.get_map_choice(num, house);

    if (!texture)
        sdl2::log_info("Unable to get map choice piece {} for house {}", num, static_cast<int>(house));

    return texture ? &texture : nullptr;
}

SDL_Texture* GFXManager::getTempStreamingTexture(SDL_Renderer* renderer, int width, int height) {
    for (const auto& texture : streamingTextureCache_) {
        int w = 0, h = 0;
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

namespace {
struct cursor_definition {
    UIGraphics_Enum id_;
    HAlign h_align_;
    VAlign v_align_;
};

} // namespace
void GFXManager::initialize_cursors() {
    static constexpr auto cursor_ids = std::to_array<cursor_definition>({
        {UI_CursorNormal, HAlign::Left, VAlign::Top},
        {UI_CursorUp, HAlign::Left, VAlign::Top},
        {UI_CursorRight, HAlign::Center, VAlign::Top},
        {UI_CursorDown, HAlign::Left, VAlign::Center},
        {UI_CursorLeft, HAlign::Left, VAlign::Top},
        {UI_CursorMove_Zoomlevel0, HAlign::Center, VAlign::Center},
        {UI_CursorMove_Zoomlevel1, HAlign::Center, VAlign::Center},
        {UI_CursorMove_Zoomlevel2, HAlign::Center, VAlign::Center},
        {UI_CursorAttack_Zoomlevel0, HAlign::Center, VAlign::Center},
        {UI_CursorAttack_Zoomlevel1, HAlign::Center, VAlign::Center},
        {UI_CursorAttack_Zoomlevel2, HAlign::Center, VAlign::Center},
        {UI_CursorCapture_Zoomlevel0, HAlign::Center, VAlign::Bottom},
        {UI_CursorCapture_Zoomlevel1, HAlign::Center, VAlign::Bottom},
        {UI_CursorCapture_Zoomlevel2, HAlign::Center, VAlign::Bottom},
        {UI_CursorCarryallDrop_Zoomlevel0, HAlign::Center, VAlign::Bottom},
        {UI_CursorCarryallDrop_Zoomlevel1, HAlign::Center, VAlign::Bottom},
        {UI_CursorCarryallDrop_Zoomlevel2, HAlign::Center, VAlign::Bottom},
    });

    default_cursor_.reset(SDL_CreateSystemCursor(SDL_SystemCursor::SDL_SYSTEM_CURSOR_ARROW));

    for (const auto& cd : cursor_ids) {
        auto* const surface = getUIGraphicSurface(cd.id_);

        if (!surface)
            continue;

        const auto x = [w = surface->w, align = cd.h_align_]() {
            switch (align) {
                default:
                case HAlign::Left: return 0;
                case HAlign::Center: return w / 2;
                case HAlign::Right: return w - 1;
            }
        }();

        const auto y = [h = surface->h, align = cd.v_align_]() {
            switch (align) {
                default:
                case VAlign::Top: return 0;
                case VAlign::Center: return h / 2;
                case VAlign::Bottom: return h - 1;
            }
        }();

        cursors_[cd.id_].reset(SDL_CreateColorCursor(surface, x, y));
    }
}
