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

#include <Renderer/DuneTextures.h>

#include <FileClasses/SaveTextureAsBmp.h>
#include <FileClasses/SurfaceLoader.h>
#include <GUI/ObjectInterfaces/PalaceInterface.h>
#include <rectpack2D/finders_interface.h>

#include <misc/dune_sdl2to3.h> // Include compatibility header
#include <misc/fnkdat.h>

#include <fmt/format.h> // For diagnostic logging

#include <algorithm>
#include <array>
#include <chrono>
#include <cstddef>
#include <filesystem>
#include <functional>
#include <future>
#include <limits>
#include <map>
#include <thread>
#include <tuple>
#include <type_traits>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <vector>

DuneTextures::DuneTextures() = default;

// clang-format off

DuneTextures::DuneTextures(std::vector<sdl2::texture_ptr>&& textures, object_pictures_type&& object_pictures,
                           small_details_type&& small_details, tiny_pictures_type&& tiny_pictures,
                           ui_graphics_type&& ui_graphics, map_choice_type&& map_choice,
                           generated_type&& generated_pictures,
                 decoration_border_type&& decoration_border, border_style_type&& border_style)
    : object_pictures_{object_pictures}, small_details_{small_details}, tiny_pictures_{tiny_pictures},
      ui_graphics_{ui_graphics}, map_choice_{map_choice}, generated_pictures_{generated_pictures},
      decoration_border_{decoration_border}, border_style_{std::move(border_style)},
      textures_{std::move(textures)} { }

// clang-format on

DuneTextures::~DuneTextures() = default;

namespace {
inline constexpr bool allow_flip            = false;
inline constexpr auto runtime_flipping_mode = rectpack2D::flipping_option::DISABLED;
inline constexpr bool export_sprite_sheets  = false;

inline constexpr auto guard = 1;

using spaces_type = rectpack2D::empty_spaces<allow_flip, rectpack2D::default_empty_spaces>;
using rect_type   = rectpack2D::output_rect_t<spaces_type>;

[[nodiscard]] constexpr auto makeFlipDedupTable() {
    std::array<bool, NUM_OBJPICS> table{};

    table[ObjPic_Tank_Base]           = true;
    table[ObjPic_Tank_Gun]            = true;
    table[ObjPic_Siegetank_Base]      = true;
    table[ObjPic_Siegetank_Gun]       = true;
    table[ObjPic_Devastator_Base]     = true;
    table[ObjPic_Devastator_Gun]      = true;
    table[ObjPic_Sonictank_Gun]       = true;
    table[ObjPic_Launcher_Gun]        = true;
    table[ObjPic_Quad]                = true;
    table[ObjPic_Trike]               = true;
    table[ObjPic_Harvester]           = true;
    table[ObjPic_Harvester_Sand]      = true;
    table[ObjPic_MCV]                 = true;
    table[ObjPic_Carryall]            = true;
    table[ObjPic_CarryallShadow]      = true;
    table[ObjPic_Frigate]             = true;
    table[ObjPic_FrigateShadow]       = true;
    table[ObjPic_Ornithopter]         = true;
    table[ObjPic_OrnithopterShadow]   = true;
    table[ObjPic_Trooper]             = true;
    table[ObjPic_Troopers]            = true;
    table[ObjPic_Soldier]             = true;
    table[ObjPic_Infantry]            = true;
    table[ObjPic_Saboteur]            = true;
    table[ObjPic_Bullet_SmallRocket]  = true;
    table[ObjPic_Bullet_MediumRocket] = true;
    table[ObjPic_Bullet_LargeRocket]  = true;
    table[ObjPic_Bullet_Sonic]        = true;

    return table;
}

inline constexpr auto flip_dedup_allowed = makeFlipDedupTable();

void save_texture_atlases(SDL_Renderer* renderer, const std::vector<sdl2::texture_ptr>& textures) {
    if (!export_sprite_sheets)
        return;

    const auto [ok, atlas_path] = fnkdat("cache/atlas/", FNKDAT_USER | FNKDAT_CREAT);
    if (!ok)
        THROW(std::runtime_error, "Unable to create atlas export directory");

    auto count = 0;
    for (const auto& texture : textures) {
        auto path = atlas_path / fmt::format("texture_{}.png", count++);
        path      = path.lexically_normal().make_preferred();

        SaveTextureAsPng(renderer, texture.get(), path);
    }
}

// Helper function to convert an indexed surface with a color key to ARGB8888 with proper alpha
// SDL3's SDL_ConvertSurface doesn't automatically convert color key pixels to alpha=0
sdl2::surface_ptr convertIndexedToARGBWithAlpha(SDL_Surface* source, SDL_PixelFormat destFormat) {
    if (!source)
        return nullptr;

    // Get source format details
    const SDL_PixelFormatDetails* srcDetails = SDL_GetPixelFormatDetails(source->format);
    if (!srcDetails) {
        return sdl2::surface_ptr{SDL_ConvertSurface(source, destFormat)};
    }

    // Check if source is an indexed/palette format (8-bit)
    const bool isIndexed = (srcDetails->bits_per_pixel == 8);

    // For non-indexed surfaces without color keys, just convert normally but check for implicit magenta transparency
    if (!isIndexed && !SDL_SurfaceHasColorKey(source)) {
        sdl2::surface_ptr converted{SDL_ConvertSurface(source, destFormat)};
        if (!converted)
            return nullptr;

        // Check if we need to fix implicit magenta transparency (common in legacy assets)
        const SDL_PixelFormatDetails* dstDetails = SDL_GetPixelFormatDetails(converted->format);
        if (dstDetails && dstDetails->bytes_per_pixel == 4) {
            // Calculate Magenta in destination format
            // Use full alpha 255 for the key color comparison
            Uint32 dstMagenta = SDL_MapRGBA(dstDetails, nullptr, 255, 0, 255, 255);

            // Mask for comparing RGB only
            const Uint32 rgbMask = dstDetails->Rmask | dstDetails->Gmask | dstDetails->Bmask;
            const Uint32 keyRGB  = dstMagenta & rgbMask;

            sdl2::surface_lock dstLock{converted.get()};
            auto* dstPixels     = static_cast<Uint32*>(converted->pixels);
            const auto dstPitch = converted->pitch / sizeof(Uint32);

            int fixedCount = 0;
            for (int y = 0; y < converted->h; ++y) {
                auto* row = dstPixels + y * dstPitch;
                for (int x = 0; x < converted->w; ++x) {
                    // Check for magenta (ignoring alpha in the check)
                    if ((row[x] & rgbMask) == keyRGB) {
                        row[x] = 0; // Make transparent
                        ++fixedCount;
                    }
                }
            }

            (void)fixedCount;
        }

        return converted;
    }

    // Get the color key from the source (default to 0 for indexed surfaces)
    Uint32 colorKey  = 0;
    bool hasColorKey = SDL_SurfaceHasColorKey(source);
    if (hasColorKey) {
        SDL_GetSurfaceColorKey(source, &colorKey);
    }

    // For indexed surfaces, always treat index 0 as transparent even if no explicit color key is set
    // This is the convention used in this game's graphics
    if (isIndexed && !hasColorKey) {
        colorKey    = 0;    // Default transparent color for indexed surfaces
        hasColorKey = true; // Treat it as having a color key
    }

    if (isIndexed) {
        // For indexed surfaces, manually convert to avoid SDL3 palette conversion issues
        // Create destination surface in the requested format
        sdl2::surface_ptr converted{SDL_CreateSurface(source->w, source->h, destFormat)};
        if (!converted)
            return nullptr;

        // Get the palette from the source surface
        SDL_Palette* srcPalette = SDL_GetSurfacePalette(source);
        if (!srcPalette) {
            // No palette, fall back to standard conversion
            return sdl2::surface_ptr{SDL_ConvertSurface(source, destFormat)};
        }

        // Get destination format details
        const SDL_PixelFormatDetails* dstDetails = SDL_GetPixelFormatDetails(converted->format);
        if (!dstDetails)
            return sdl2::surface_ptr{SDL_ConvertSurface(source, destFormat)};

        // Lock both surfaces for pixel access
        sdl2::surface_lock srcLock{source};
        sdl2::surface_lock dstLock{converted.get()};

        const auto* srcPixels = static_cast<const Uint8*>(source->pixels);
        auto* dstPixels       = static_cast<Uint32*>(converted->pixels);
        const auto srcPitch   = source->pitch;
        const auto dstPitch   = converted->pitch / sizeof(Uint32);
        const Uint8 keyIndex  = static_cast<Uint8>(colorKey);

        // Convert pixel by pixel
        for (int y = 0; y < source->h; ++y) {
            const auto* srcRow = srcPixels + y * srcPitch;
            auto* dstRow       = dstPixels + y * dstPitch;
            for (int x = 0; x < source->w; ++x) {
                const Uint8 paletteIndex = srcRow[x];

                // Check if this pixel matches the color key palette index
                if (hasColorKey && paletteIndex == keyIndex) {
                    // Set entire pixel to 0 (transparent black) to prevent color bleeding
                    dstRow[x] = 0;
                } else {
                    // Look up the color from the palette and convert
                    const SDL_Color& color = srcPalette->colors[paletteIndex];

                    // Check for implicit magenta transparency (255, 0, 255) in indexed surfaces
                    if (color.r == 255 && color.g == 0 && color.b == 255) {
                        dstRow[x] = 0;
                    } else {
                        dstRow[x] = SDL_MapRGBA(dstDetails, nullptr, color.r, color.g, color.b, color.a);
                    }
                }
            }
        }

        return converted;
    }

    // For non-indexed surfaces with a color key, use standard conversion then fix up
    sdl2::surface_ptr converted{SDL_ConvertSurface(source, destFormat)};
    if (!converted)
        return nullptr;

    // If no color key to process, return the converted surface
    if (!hasColorKey)
        return converted;

    // Get destination format details
    const SDL_PixelFormatDetails* dstDetails = SDL_GetPixelFormatDetails(converted->format);
    if (!dstDetails)
        return converted;

    // For non-indexed surfaces with a color key, we need to find and zero matching pixels
    // Get the RGBA value of the color key in the source format
    Uint8 keyR = 0, keyG = 0, keyB = 0, keyA = 0;
    SDL_Palette* srcPalette = SDL_GetSurfacePalette(source);
    SDL_GetRGBA(colorKey, srcDetails, srcPalette, &keyR, &keyG, &keyB, &keyA);

    // Calculate what the color key looks like in the destination format (with full alpha)
    Uint32 dstColorKey = SDL_MapRGBA(dstDetails, nullptr, keyR, keyG, keyB, 255);

    // Create a mask for comparing RGB only (ignoring alpha)
    const Uint32 rgbMask = dstDetails->Rmask | dstDetails->Gmask | dstDetails->Bmask;
    const Uint32 keyRGB  = dstColorKey & rgbMask;

    // Calculate Magenta in destination format
    Uint32 dstMagenta       = SDL_MapRGBA(dstDetails, nullptr, 255, 0, 255, 255);
    const Uint32 magentaRGB = dstMagenta & rgbMask; // Should be equivalent to dstMagenta if alpha is not in mask

    // Lock destination surface for pixel access
    sdl2::surface_lock dstLock{converted.get()};
    auto* dstPixels     = static_cast<Uint32*>(converted->pixels);
    const auto dstPitch = converted->pitch / sizeof(Uint32);

    for (int y = 0; y < converted->h; ++y) {
        auto* row = dstPixels + y * dstPitch;
        for (int x = 0; x < converted->w; ++x) {
            // Compare RGB components only
            Uint32 pixelRGB = row[x] & rgbMask;
            if (pixelRGB == keyRGB || pixelRGB == magentaRGB) {
                // Set entire pixel to 0 (transparent black) to prevent color bleeding
                row[x] = 0;
            }
        }
    }

    return converted;
}

std::tuple<bool, rectpack2D::rect_wh> packRectangles(const int max_side, std::vector<rect_type>& rectangles) {
    constexpr auto discard_step = 1;

    auto failed = false;

    const auto total_pixels = [&]() {
        auto sum = 0;
        for (const auto& r : rectangles)
            sum += r.w * r.h;
        return sum;
    }();

    const auto result_size = rectpack2D::find_best_packing<spaces_type>(
        rectangles,
        make_finder_input(
            max_side,
            discard_step,
            []([[maybe_unused]] const auto& rect) { return rectpack2D::callback_result::CONTINUE_PACKING; },
            [&failed]([[maybe_unused]] const auto& rect) {
                failed = true;
                return rectpack2D::callback_result::ABORT_PACKING;
            },
            runtime_flipping_mode));

    if (failed) {
        sdl2::log_info("Packing failed ");
        return {false, rectpack2D::rect_wh{}};
    }

    sdl2::log_info("Packed in {}x{}", result_size.w, result_size.h);

    const auto side = static_cast<int>(ceil(sqrt(total_pixels)));

    sdl2::log_info("Pixels {0} ({1}x{1}) for efficiency {2:.1f}",
                   total_pixels,
                   side,
                   100 * static_cast<double>(total_pixels) / (result_size.w * result_size.h));

    // for(const auto& r : rectangles) {
    //    sdl2::log_info("   {}x{} at {}x{}", r.w, r.h, r.x, r.y);
    //}

#if _DEBUG
    for (auto i = 0; i < rectangles.size() - 1; ++i) {
        const auto& a = rectangles[i];
        SDL_Rect sa{a.x, a.y, a.w, a.h};

        for (auto j = i + 1; j < rectangles.size(); ++j) {
            const auto& b = rectangles[j];
            SDL_Rect sb{b.x, b.y, b.w, b.h};

            if (SDL_HasIntersection(&sa, &sb)) {
                sdl2::log_info("Failed packing");
            }
        }
    }
#endif // _DEBUG

    return {true, result_size};
}

bool compare_surfaces(SDL_Surface* a, SDL_Surface* b) {
    if (a->w != b->w || a->h != b->h)
        return false;

    if (a == b)
        return true;

    const sdl2::surface_lock lock_a{a};
    const sdl2::surface_lock lock_b{b};

    const auto* RESTRICT pa = static_cast<const char*>(lock_a.pixels());
    const auto* RESTRICT pb = static_cast<const char*>(lock_b.pixels());

    if (lock_a.pitch() != lock_b.pitch())
        return false;

    const auto pitch = lock_a.pitch();

    for (auto i = 0; i < a->h; ++i, pa += pitch, pb += pitch) {
        if (0 != memcmp(pa, pb, lock_a.pitch()))
            return false;
    }

    return true;
}

bool compare_surface_rects(SDL_Surface* surface, const SDL_Rect& a, const SDL_Rect& b, SDL_FlipMode flip) {
    if (!surface || a.w != b.w || a.h != b.h || a.w <= 0 || a.h <= 0) {
        return false;
    }

    const auto* const details = SDL_GetPixelFormatDetails(surface->format);
    if (!details || details->bytes_per_pixel <= 0) {
        return false;
    }
    const auto bpp = details->bytes_per_pixel;

    const sdl2::surface_lock lock{surface};
    const auto* const pixels = static_cast<const uint8_t*>(lock.pixels());

    const auto flip_bits = static_cast<int>(flip);
    for (int y = 0; y < a.h; ++y) {
        const auto sy = ((flip_bits & static_cast<int>(SDL_FlipMode::SDL_FLIP_VERTICAL)) != 0) ? (a.h - 1 - y) : y;
        for (int x = 0; x < a.w; ++x) {
            const auto sx =
                ((flip_bits & static_cast<int>(SDL_FlipMode::SDL_FLIP_HORIZONTAL)) != 0) ? (a.w - 1 - x) : x;

            const auto* const pa = pixels + (a.y + y) * lock.pitch() + (a.x + x) * bpp;
            const auto* const pb = pixels + (b.y + sy) * lock.pitch() + (b.x + sx) * bpp;

            if (0 != memcmp(pa, pb, static_cast<size_t>(bpp))) {
                return false;
            }
        }
    }

    return true;
}

class Packer final {
    std::vector<rect_type> rectangles_;

    int w_{};
    int h_{};

public:
    void clear() { rectangles_.clear(); }

    int add(int w, int h) {
        const auto ret = static_cast<int>(rectangles_.size());

        rectangles_.emplace_back(0, 0, w + 2 * guard, h + 2 * guard);

        return ret;
    }

    bool pack(int max_side) {
        const auto& [ok, size] = packRectangles(max_side, rectangles_);

        if (ok) {
            w_ = size.w;
            h_ = size.h;
        } else {
            w_ = 0;
            h_ = 0;
        }

        return ok;
    }

    const rect_type& operator[](int index) const { return rectangles_.at(index); }

    [[nodiscard]] int width() const noexcept { return w_; }
    [[nodiscard]] int height() const noexcept { return h_; }

    [[nodiscard]] bool empty() const noexcept { return rectangles_.empty(); }
};

class PackableSet {
public:
    using packer_set_type = std::vector<std::tuple<int, int, SDL_Surface*, SDL_Rect>>;

    PackableSet(packer_set_type&& set) : set_{std::move(set)} { }

    template<typename F>
    void for_each(const Packer& packer, F&& f) {
        for (const auto& s : set_) {
            const auto& [rect_idx, s_idx, surface, source] = s;

            const auto& r = packer[rect_idx];

            f(r, s_idx, surface, source);
        }
    }

    template<typename F>
    void for_each(const Packer& packer, F&& f) const {
        for (const auto& s : set_) {
            const auto& [rect_idx, s_idx, surface, source] = s;

            const auto& r = packer[rect_idx];

            f(r, s_idx, surface, source);
        }
    }

private:
    packer_set_type set_;
};

template<typename Identifier>
class PackableSurfaces {
    struct record {
        Identifier identifier;
        SDL_Surface* surface;
        SDL_Rect source;
    };

    std::vector<record> surfaces_;
    std::vector<std::tuple<int, Identifier>> duplicates_;

public:
    using identifier_type = Identifier;

    int add(Identifier identifier, SDL_Surface* surface) {
        if (nullptr == surface)
            THROW(std::invalid_argument, "PackerSurfaces: Cannot use an invalid surface!");

        const auto idx = static_cast<int>(surfaces_.size());

        surfaces_.push_back({identifier, surface, SDL_Rect{0, 0, surface->w, surface->h}});

        return idx;
    }

    int add(Identifier identifier, SDL_Surface* surface, const SDL_Rect& source) {
        if (nullptr == surface)
            THROW(std::invalid_argument, "PackerSurfaces: Cannot use an invalid surface!");
        if (source.x < 0 || source.y < 0 || source.w <= 0 || source.h <= 0 || source.x + source.w > surface->w
            || source.y + source.h > surface->h) {
            THROW(std::invalid_argument, "PackerSurfaces: Invalid source rectangle!");
        }

        const auto idx = static_cast<int>(surfaces_.size());

        surfaces_.push_back({identifier, surface, source});

        return idx;
    }

    PackableSet create_packer_set(Packer& packer) {
        PackableSet::packer_set_type output;
        for (auto i = 0u; i < surfaces_.size(); ++i) {
            const auto& record = surfaces_[i];

            const auto idx = packer.add(record.source.w, record.source.h);

            output.emplace_back(static_cast<int>(idx), i, record.surface, record.source);
        }

        return PackableSet{std::move(output)};
    }

    template<typename Predicate>
    PackableSet create_packer_set(Packer& packer, Predicate&& predicate) {
        static_assert(std::is_invocable_r_v<bool, Predicate, Identifier, SDL_Surface*>);

        PackableSet::packer_set_type output;
        for (auto i = 0u; i < surfaces_.size(); ++i) {
            const auto& record = surfaces_[i];

            if (!predicate(record.identifier, record.surface))
                continue;

            const auto idx = packer.add(record.source.w, record.source.h);

            output.emplace_back(static_cast<int>(idx), i, record.surface, record.source);
        }

        return PackableSet{std::move(output)};
    }

    void add_duplicate(int key, Identifier identifier) {
        assert(key >= 0 && key < surfaces_.size());

        duplicates_.emplace_back(key, identifier);
    }

    template<typename F>
    void for_each_duplicate(F&& f) const {
        for (const auto& duplicate : duplicates_) {
            const auto& [s_idx, identifier] = duplicate;

            f(surfaces_[s_idx].identifier, identifier);
        }
    }

    template<typename Lookup>
    void update_duplicates(Lookup&& lookup) {
        static_assert(std::is_invocable_r_v<DuneTexture&, Lookup, const Identifier&>);
        for (const auto& duplicate : duplicates_) {
            const auto& [s_idx, identifier] = duplicate;

            lookup(identifier) = lookup(surfaces_.at(s_idx).identifier);
        }
    }

    Identifier operator[](int key) const { return surfaces_.at(key).identifier; }
};

class AtlasFactory23 final {
public:
    template<typename Identifier>
    int add(PackableSurfaces<Identifier>& packable) {
        auto set = packable.create_packer_set(packer_);

        const auto ret = static_cast<int>(surface_sets_.size());
        surface_sets_.emplace_back(std::move(set));

        return ret;
    }

    template<typename Identifier, typename Predicate>
    int add(PackableSurfaces<Identifier>& packable, Predicate&& predicate) {
        auto set = packable.create_packer_set(packer_, predicate);

        const auto ret = static_cast<int>(surface_sets_.size());
        surface_sets_.emplace_back(std::move(set));

        return ret;
    }

    sdl2::texture_ptr pack(SDL_Renderer* renderer, SDL_PixelFormat format, int max_side,
                           [[maybe_unused]] const char* atlasName = nullptr) {
        if (!packer_.pack(max_side))
            return nullptr;

        std::unordered_map<SDL_Surface*, sdl2::surface_ptr> converted_surface_cache;

        const sdl2::surface_ptr atlas_surface{
            SDL_CreateRGBSurfaceWithFormat(0, packer_.width(), packer_.height(), SDL_BITSPERPIXEL(format), format)};

        // SDL3: Clear atlas surface to transparent
        SDL_FillSurfaceRect(atlas_surface.get(), nullptr, 0);

        const auto draw = [&](const auto& r, [[maybe_unused]] int s_idx, SDL_Surface* surface, const SDL_Rect& source) {
            SDL_Rect atlas_rect{r.x + guard, r.y + guard, r.w - 2 * guard, r.h - 2 * guard};

            SDL_Surface* surface_to_blit = surface;
            if (const auto it = converted_surface_cache.find(surface); it != converted_surface_cache.end()) {
                if (it->second)
                    surface_to_blit = it->second.get();
            } else {
                auto converted_surface = convertIndexedToARGBWithAlpha(surface, format);

                if (converted_surface) {
                    surface_to_blit = converted_surface.get();
                    converted_surface_cache.emplace(surface, std::move(converted_surface));
                }
            }

            // Use SDL_BLENDMODE_NONE to copy pixels directly including alpha values.
            // The converted surface has proper alpha embedded, so we want a direct copy,
            // not alpha blending (which would blend with the cleared transparent atlas).
            if (!drawSurface(surface_to_blit, &source, atlas_surface.get(), &atlas_rect, SDL_BLENDMODE_NONE)) {
                // Retry after converting from palette to 32-bit surface...
                const sdl2::surface_ptr copy{SDL_ConvertSurface(surface_to_blit, format)};

                if (!copy) {
                    sdl2::log_warn("Unable to copy surface: {}", SDL_GetError());
                    return false;
                }

                if (!drawSurface(copy.get(), &source, atlas_surface.get(), &atlas_rect, SDL_BLENDMODE_NONE)) {
                    sdl2::log_warn("Unable to draw object");
                    return false;
                }
            }

            // Copy the edge pixels to the guard (use the converted surface if available)
            // Use SDL_BLENDMODE_NONE for consistent direct pixel copying
            SDL_Surface* edge_surface = surface_to_blit;

            { // Top
                const SDL_Rect src{source.x, source.y, source.w, 1};
                SDL_Rect dst{atlas_rect.x, atlas_rect.y - 1, src.w, 1};

                drawSurface(edge_surface, &src, atlas_surface.get(), &dst, SDL_BLENDMODE_NONE);
            }

            { // Left
                const SDL_Rect src{source.x, source.y, 1, source.h};
                SDL_Rect dst{atlas_rect.x - 1, atlas_rect.y, 1, src.h};

                drawSurface(edge_surface, &src, atlas_surface.get(), &dst, SDL_BLENDMODE_NONE);
            }

            { // Bottom
                const SDL_Rect src{source.x, source.y + source.h - 1, source.w, 1};
                SDL_Rect dst{atlas_rect.x, atlas_rect.y + source.h, src.w, 1};

                drawSurface(edge_surface, &src, atlas_surface.get(), &dst, SDL_BLENDMODE_NONE);
            }

            { // Right
                const SDL_Rect src{source.x + source.w - 1, source.y, 1, source.h};
                SDL_Rect dst{atlas_rect.x + source.w, atlas_rect.y, 1, src.h};

                drawSurface(edge_surface, &src, atlas_surface.get(), &dst, SDL_BLENDMODE_NONE);
            }

            // Fill in the corners

            return true;
        };

        for (const auto& set : surface_sets_) {
            set.for_each(packer_, draw);
        }

        // auto [ok, cache_path] = fnkdat("cache/", FNKDAT_USER | FNKDAT_CREAT);

        // auto path = cache_path / fmt::format("f23_{}.bmp", texture_identifier);
        // path      = path.lexical_normal().make_preferred();

        // SDL_SaveBMP(atlas_surface.get(), path.u8string().c_str());

        auto texture = sdl2::texture_ptr{SDL_CreateTextureFromSurface(renderer, atlas_surface.get())};

        if (texture && SDL_SetTextureBlendMode(texture.get(), SDL_BLENDMODE_BLEND)) {
            sdl2::log_warn("Unable to set texture atlas blend mode");
        }

        return texture;
    }

    template<typename Identifier, typename Lookup>
    void update(int key, SDL_Texture* texture, Lookup&& lookup) {
        static_assert(std::is_invocable_r_v<const DuneTexture&, Lookup, int>);

        const auto& set = surface_sets_.at(key);

        set.for_each(packer_,
                     [&](const auto& r, auto s_idx, [[maybe_unused]] auto* surface, [[maybe_unused]] const SDL_Rect&) {
                         const SDL_Rect rect{r.x + guard, r.y + guard, r.w - 2 * guard, r.h - 2 * guard};

                         lookup(s_idx) = DuneTexture{texture, rect};
                     });
    }

    template<typename F>
    void for_each_rect(int key, F&& f) {
        const auto& set = surface_sets_.at(key);
        set.for_each(packer_,
                     [&](const auto& r, auto s_idx, [[maybe_unused]] auto* surface, [[maybe_unused]] const SDL_Rect&) {
                         const SDL_Rect rect{r.x + guard, r.y + guard, r.w - 2 * guard, r.h - 2 * guard};
                         f(s_idx, rect);
                     });
    }

    void clear() {
        packer_.clear();
        surface_sets_.clear();
    }

    [[nodiscard]] bool empty() const noexcept { return packer_.empty(); }

private:
    Packer packer_;

    std::vector<PackableSet> surface_sets_;
};

class ObjectPicturePacker final {
public:
    using identifier_type = std::tuple<uint32_t, HOUSETYPE, int, int>;
    using textures_type   = DuneTextures::object_pictures_type;
    using object_key_type = std::tuple<uint32_t, HOUSETYPE, int>;

    void initialize(SurfaceLoader* surfaceLoader) {

        for (auto id = 0u; id < NUM_OBJPICS; ++id) {
            if (id == ObjPic_Bullet_SonicTemp || id == ObjPic_SandwormShimmerTemp)
                continue;

            const auto harkonnen_only = harkonnen_only_.contains(id);

            for (auto zoom = 0; zoom < NUM_ZOOMLEVEL; ++zoom) {
                for_each_housetype([&](auto house) {
                    if (harkonnen_only && house != HOUSETYPE::HOUSE_HARKONNEN)
                        return;

                    auto* const surface = surfaceLoader->getZoomedObjSurface(id, house, zoom);

                    if (!surface)
                        return;

                    const auto tiles        = surfaceLoader->getZoomedObjSurfaceTiles(id, house, zoom);
                    const auto frames_x     = std::max(1, tiles.x);
                    const auto frames_y     = std::max(1, tiles.y);
                    const auto frame_width  = surface->w / frames_x;
                    const auto frame_height = surface->h / frames_y;
                    const auto frame_count  = frames_x * frames_y;

                    const object_key_type object_key{id, house, zoom};
                    auto& meta          = object_meta_[object_key];
                    meta.frames_x       = static_cast<short>(frames_x);
                    meta.frames_y       = static_cast<short>(frames_y);
                    meta.frame_width    = static_cast<short>(frame_width);
                    meta.frame_height   = static_cast<short>(frame_height);
                    meta.logical_frames = std::vector<ObjectMeta::LogicalFrame>(static_cast<size_t>(frame_count));

                    struct CanonicalFrame {
                        int frame_index{};
                        SDL_Rect source{};
                    };
                    std::vector<CanonicalFrame> canonical_frames;
                    const bool allow_flips = flip_dedup_allowed[id];

                    for (auto row = 0; row < frames_y; ++row) {
                        for (auto col = 0; col < frames_x; ++col) {
                            const auto frame_index = row * frames_x + col;
                            const SDL_Rect source{col * frame_width, row * frame_height, frame_width, frame_height};
                            auto mapped       = false;
                            auto mapped_flip  = SDL_FlipMode::SDL_FLIP_NONE;
                            auto mapped_frame = frame_index;

                            for (const auto& canonical : canonical_frames) {
                                if (compare_surface_rects(
                                        surface, source, canonical.source, SDL_FlipMode::SDL_FLIP_NONE)) {
                                    mapped       = true;
                                    mapped_flip  = SDL_FlipMode::SDL_FLIP_NONE;
                                    mapped_frame = canonical.frame_index;
                                    break;
                                }

                                if (allow_flips) {
                                    if (compare_surface_rects(
                                            surface, source, canonical.source, SDL_FlipMode::SDL_FLIP_HORIZONTAL)) {
                                        mapped       = true;
                                        mapped_flip  = SDL_FlipMode::SDL_FLIP_HORIZONTAL;
                                        mapped_frame = canonical.frame_index;
                                        break;
                                    }

                                    if (compare_surface_rects(
                                            surface, source, canonical.source, SDL_FlipMode::SDL_FLIP_VERTICAL)) {
                                        mapped       = true;
                                        mapped_flip  = SDL_FlipMode::SDL_FLIP_VERTICAL;
                                        mapped_frame = canonical.frame_index;
                                        break;
                                    }

                                    constexpr auto both_flips =
                                        static_cast<SDL_FlipMode>(static_cast<int>(SDL_FlipMode::SDL_FLIP_HORIZONTAL)
                                                                  | static_cast<int>(SDL_FlipMode::SDL_FLIP_VERTICAL));
                                    if (compare_surface_rects(surface, source, canonical.source, both_flips)) {
                                        mapped       = true;
                                        mapped_flip  = both_flips;
                                        mapped_frame = canonical.frame_index;
                                        break;
                                    }
                                }
                            }

                            meta.logical_frames.at(frame_index) =
                                ObjectMeta::LogicalFrame{static_cast<short>(mapped_frame), mapped_flip};

                            if (!mapped) {
                                canonical_frames.push_back({frame_index, source});
                                surfaces_.add({id, house, zoom, frame_index}, surface, source);
                            }
                        }
                    }
                });
            }
        }
    }

    template<typename Predicate>
    int add(AtlasFactory23& factory23, Predicate&& predicate) {
        return factory23.add(surfaces_, predicate);
    }

    void update(AtlasFactory23& factory23, int key, SDL_Texture* texture) {
        struct BuildEntry {
            short frames_x{};
            short frames_y{};
            short frame_width{};
            short frame_height{};
            std::vector<DuneTextureSpriteFrame> frames{};
        };

        std::map<object_key_type, BuildEntry> build_map;

        factory23.for_each_rect(key, [&](int n, const SDL_Rect& atlas_rect) {
            const auto& [id, house, zoom, frame_index] = surfaces_[n];
            const object_key_type object_key{id, house, zoom};

            const auto meta_it = object_meta_.find(object_key);
            if (meta_it == object_meta_.end()) {
                return;
            }

            auto& entry = build_map[object_key];
            if (entry.frames.empty()) {
                entry.frames_x     = meta_it->second.frames_x;
                entry.frames_y     = meta_it->second.frames_y;
                entry.frame_width  = meta_it->second.frame_width;
                entry.frame_height = meta_it->second.frame_height;
                entry.frames.resize(static_cast<size_t>(entry.frames_x) * static_cast<size_t>(entry.frames_y));
            }

            entry.frames.at(frame_index).source = DuneTextureRect{atlas_rect};
        });

        for (const auto& [object_key, entry] : build_map) {
            const auto& [id, house, zoom] = object_key;
            const auto& remap             = object_meta_.at(object_key).logical_frames;

            std::vector<DuneTextureSpriteFrame> final_frames(remap.size());
            for (size_t logical = 0; logical < remap.size(); ++logical) {
                const auto& map       = remap[logical];
                auto frame            = entry.frames.at(map.packed_frame);
                frame.flip            = map.flip;
                final_frames[logical] = frame;
            }

            auto min_x = std::numeric_limits<int>::max();
            auto min_y = std::numeric_limits<int>::max();
            auto max_x = 0;
            auto max_y = 0;
            for (const auto& frame : final_frames) {
                const auto rect = frame.source.as_sdl();
                min_x           = std::min(min_x, rect.x);
                min_y           = std::min(min_y, rect.y);
                max_x           = std::max(max_x, rect.x + rect.w);
                max_y           = std::max(max_y, rect.y + rect.h);
            }

            auto& target  = dune_textures_.at(zoom).at(id).at(static_cast<int>(house));
            target        = DuneTexture{texture, SDL_Rect{min_x, min_y, max_x - min_x, max_y - min_y}};
            target.width_ = static_cast<float>(static_cast<int>(entry.frames_x) * static_cast<int>(entry.frame_width));
            target.height_ =
                static_cast<float>(static_cast<int>(entry.frames_y) * static_cast<int>(entry.frame_height));
            target.set_sprite_frames(
                entry.frames_x,
                entry.frames_y,
                std::make_shared<const std::vector<DuneTextureSpriteFrame>>(std::move(final_frames)));
        }
    }

    void update_duplicates() { }

    [[nodiscard]] DuneTextures::object_pictures_type object_pictures2() const { return dune_textures_; }

private:
    struct ObjectMeta {
        struct LogicalFrame {
            short packed_frame{};
            SDL_FlipMode flip{SDL_FlipMode::SDL_FLIP_NONE};
        };

        short frames_x{};
        short frames_y{};
        short frame_width{};
        short frame_height{};
        std::vector<LogicalFrame> logical_frames{};
    };

    PackableSurfaces<identifier_type> surfaces_;

    std::map<object_key_type, ObjectMeta> object_meta_;

    textures_type dune_textures_;

    // There is only one kind of these items, stored in the Harkonnen slot.
    static const std::unordered_set<uint32_t> harkonnen_only_;
};

// There is only one kind of these items, stored in the Harkonnen slot.
const std::unordered_set<uint32_t> ObjectPicturePacker::harkonnen_only_ = {
    ObjPic_ExplosionSmall,
    ObjPic_ExplosionMedium1,
    ObjPic_ExplosionMedium2,
    ObjPic_ExplosionLarge1,
    ObjPic_ExplosionLarge2,
    ObjPic_ExplosionSmallUnit,
    ObjPic_ExplosionFlames,
    ObjPic_ExplosionSpiceBloom,
    ObjPic_SandwormSegment,
    ObjPic_Terrain,
    ObjPic_DestroyedStructure,
    ObjPic_RockDamage,
    ObjPic_SandDamage,
    ObjPic_Terrain_Hidden,
    ObjPic_Terrain_HiddenFog,
    ObjPic_Terrain_Tracks,
    ObjPic_Star,
};

class UiGraphicPacker final {
public:
    using identifier_type = std::tuple<uint32_t, HOUSETYPE>;
    using textures_type   = DuneTextures::ui_graphics_type;

    void initialize(SurfaceLoader* surfaceLoader) {

        for (auto id = 0u; id < NUM_UIGRAPHICS; ++id) {
            SDL_Surface* harkonnen = nullptr;
            auto harkonnen_key     = 0;

            for_each_housetype([&](auto house) {
                auto* surface = surfaceLoader->getUIGraphicSurface(id, house);

                if (!surface)
                    return;

                const auto is_harkonnen = house == HOUSETYPE::HOUSE_HARKONNEN;

                if (is_harkonnen) {
                    harkonnen     = surface;
                    harkonnen_key = surfaces_.add({id, house}, surface);
                } else if (harkonnen && compare_surfaces(harkonnen, surface)) {
                    // We are identical to the Harkonnen image, so let it find the Harkonnen version.
                    surfaces_.add_duplicate(harkonnen_key, {id, house});
                } else {
                    surfaces_.add({id, house}, surface);
                }
            });
        }
    }

    template<typename Predicate>
    int add(AtlasFactory23& factory23, Predicate&& predicate) {
        return factory23.add(surfaces_, predicate);
    }

    void update(AtlasFactory23& factory23, int key, SDL_Texture* texture) {
        factory23.update<identifier_type>(key, texture, [&](auto n) -> DuneTexture& { return lookup_dune_texture(n); });
    }

    void update_duplicates() {
        surfaces_.update_duplicates([&](const auto& identifier) -> DuneTexture& {
            const auto& [id, house] = identifier;

            return dune_textures_.at(static_cast<int>(house)).at(id);
        });
    }

    [[nodiscard]] textures_type dune_textures() const { return dune_textures_; }

private:
    [[nodiscard]] DuneTexture& lookup_dune_texture(int n) {
        const auto identifier = surfaces_[n];

        const auto& [id, house] = identifier;

        return dune_textures_.at(static_cast<int>(house)).at(id);
    }

    PackableSurfaces<identifier_type> surfaces_;

    textures_type dune_textures_;
};

class MapChoicePacker final {
public:
    using identifier_type = std::tuple<uint32_t, HOUSETYPE>;
    using textures_type   = DuneTextures::map_choice_type;

    void initialize(SurfaceLoader* surfaceLoader) {

        for (auto id = 0u; id < NUM_MAPCHOICEPIECES; ++id) {

            for_each_housetype([&](auto house) {
                auto* surface = surfaceLoader->getMapChoicePieceSurface(id, house);

                if (!surface)
                    return;

                surfaces_.add({id, house}, surface);
            });
        }
    }

    int add(AtlasFactory23& factory23) { return factory23.add(surfaces_); }

    void update(AtlasFactory23& factory23, int key, SDL_Texture* texture) {
        factory23.update<identifier_type>(key, texture, [&](auto n) -> DuneTexture& { return lookup_dune_texture(n); });
    }

    void update_duplicates() {
        surfaces_.update_duplicates([&](const auto& identifier) -> DuneTexture& {
            const auto& [id, house] = identifier;

            return dune_textures_.at(static_cast<int>(house)).at(id);
        });
    }

    [[nodiscard]] textures_type dune_textures() const { return dune_textures_; }

private:
    [[nodiscard]] DuneTexture& lookup_dune_texture(int n) {
        const auto identifier = surfaces_[n];

        const auto& [id, house] = identifier;

        return dune_textures_.at(static_cast<int>(house)).at(id);
    }

    PackableSurfaces<identifier_type> surfaces_;

    textures_type dune_textures_;
};

template<typename TexturesType, typename IdentifierType>
class PackerBase {
public:
    using textures_type   = TexturesType;
    using identifier_type = IdentifierType;

    int add(AtlasFactory23& factory23) { return factory23.add(surfaces_); }

    template<typename Predicate>
    int add(AtlasFactory23& factory23, Predicate&& predicate) {
        return factory23.add(surfaces_, predicate);
    }

    void update(AtlasFactory23& factory23, int key, SDL_Texture* texture) {
        factory23.update<identifier_type>(
            key, texture, [&](auto n) -> DuneTexture& { return textures_.at(surfaces_[n]); });
    }

    void update_duplicates() {
        surfaces_.update_duplicates([&](const auto& identifier) -> DuneTexture& { return textures_.at(identifier); });
    }

    [[nodiscard]] textures_type dune_textures() const { return textures_; }

protected:
    PackerBase() = default;

    PackableSurfaces<identifier_type> surfaces_;

    textures_type textures_;
};

class TinyPicturePacker final : public PackerBase<DuneTextures::tiny_pictures_type, uint32_t> {
public:
    void initialize(SurfaceLoader* surfaceLoader) {
        for (auto id = 0; id < textures_.size(); ++id) {
            auto* surface = surfaceLoader->getTinyPictureSurface(id);

            if (!surface) {
                sdl2::log_warn("No surface available for tiny picture {}", id);
                continue;
            }

            surfaces_.add(id, surface);
        }
    }
};

class SmallDetailPicsPacker final : public PackerBase<DuneTextures::small_details_type, uint32_t> {
public:
    void initialize(SurfaceLoader* surfaceLoader) {
        for (auto id = 0; id < textures_.size(); ++id) {
            auto* surface = surfaceLoader->getSmallDetailSurface(id);

            if (!surface) {
                sdl2::log_warn("No surface available for small detail {}", id);
                continue;
            }

            surfaces_.add(id, surface);
        }
    }
};

class GeneratedPicturesPacker final : public PackerBase<DuneTextures::generated_type, uint32_t> {
public:
    void initialize(SurfaceLoader* surfaceLoader) {
        if (auto palaceReady = PalaceInterface::createSurface(surfaceLoader, GeneratedPicture::PalaceReadyText))
            generated_.at(static_cast<int>(GeneratedPicture::PalaceReadyText)) = std::move(palaceReady);

        for (auto id = 0u; id < textures_.size(); ++id) {
            auto* surface = generated_.at(id).get();

            if (!surface) {
                sdl2::log_warn("No surface available for generated picture {}", id);
                continue;
            }

            surfaces_.add(id, surface);
        }
    }

private:
    std::array<sdl2::surface_ptr, std::tuple_size_v<textures_type>> generated_;
};

class DecorationBorderPicturesPacker final : public PackerBase<DuneTextures::decoration_border_type, int> {
public:
    void initialize(const SurfaceLoader* surfaceLoader) {
        const auto border = surfaceLoader->getDecorationBorder();

        surfaces_.add(0, border.ball);
        surfaces_.add(1, border.hspacer);
        surfaces_.add(2, border.vspacer);
        surfaces_.add(3, border.hborder);
        surfaces_.add(4, border.vborder);
    }

    void update(AtlasFactory23& factory23, int key, SDL_Texture* texture) {
        factory23.update<identifier_type>(
            key, texture, [&](const auto& identifier) -> DuneTexture& { return lookup(identifier); });
    }

    void update_duplicates() {
        surfaces_.update_duplicates([&](const auto& identifier) -> DuneTexture& { return lookup(identifier); });
    }

private:
    DuneTexture& lookup(identifier_type n) {
        switch (n) {
            case 0: return textures_.ball;
            case 1: return textures_.hspacer;
            case 2: return textures_.vspacer;
            case 3: return textures_.hborder;
            case 4: return textures_.vborder;
            default: THROW(std::out_of_range, "Invalid texture identifier!");
        }
    }
};

std::vector<SDL_Color> get_colors_horizontal(SDL_Surface* surface) {
    std::vector<SDL_Color> colors;
    colors.reserve(surface->w);

    const sdl2::surface_lock lock{surface};

    // SDL3: SDL_GetRGBA requires SDL_PixelFormatDetails and SDL_Palette
    const SDL_PixelFormatDetails* details = SDL_GetPixelFormatDetails(surface->format);
    SDL_Palette* palette                  = SDL_GetSurfacePalette(surface);

    const bool isIndexed = (details && details->bits_per_pixel == 8);

    if (isIndexed && palette) {
        // 8-bit indexed surface - read palette indices and look up colors
        const auto* pixels = static_cast<const Uint8*>(surface->pixels);

        for (auto x = 0; x < surface->w; ++x) {
            const Uint8 paletteIndex = pixels[x];

            // For transparent pixels (index 0), use transparent black
            if (paletteIndex == 0) {
                colors.push_back({0, 0, 0, 0});
            } else if (paletteIndex < palette->ncolors) {
                const SDL_Color& c = palette->colors[paletteIndex];
                // Check for implicit magenta transparency
                if (c.r == 255 && c.g == 0 && c.b == 255) {
                    colors.push_back({0, 0, 0, 0});
                } else {
                    colors.push_back({c.r, c.g, c.b, c.a});
                }
            } else {
                colors.push_back({0, 0, 0, 255}); // Fallback
            }
        }
    } else {
        // 32-bit surface - read RGBA values directly
        const auto* pixels = static_cast<const Uint32*>(surface->pixels);

        for (auto x = 0; x < surface->w; ++x) {
            Uint8 r = 0, g = 0, b = 0, a = 0;
            SDL_GetRGBA(pixels[x], details, palette, &r, &g, &b, &a);

            // Check for implicit magenta transparency
            if (r == 255 && g == 0 && b == 255) {
                r = 0;
                g = 0;
                b = 0;
                a = 0;
            }

#ifdef HAVE_PARENTHESIZED_INITIALIZATION_OF_AGGREGATES
            colors.emplace_back(r, g, b, a);
#else
            colors.push_back({r, g, b, a});
#endif
        }
    }

    return colors;
}

std::vector<SDL_Color> get_colors_vertical(SDL_Surface* surface) {
    std::vector<SDL_Color> colors;
    colors.reserve(surface->h);

    sdl2::surface_lock lock{surface};

    // SDL3: SDL_GetRGBA requires SDL_PixelFormatDetails and SDL_Palette
    const SDL_PixelFormatDetails* details = SDL_GetPixelFormatDetails(surface->format);
    SDL_Palette* palette                  = SDL_GetSurfacePalette(surface);

    const bool isIndexed = (details && details->bits_per_pixel == 8);

    if (isIndexed && palette) {
        // 8-bit indexed surface - read palette indices and look up colors
        const auto* pixels = static_cast<const Uint8*>(surface->pixels);
        const auto pitch   = surface->pitch;

        for (auto y = 0; y < surface->h; ++y) {
            const Uint8 paletteIndex = pixels[y * pitch];

            // For transparent pixels (index 0), use transparent black
            if (paletteIndex == 0) {
                colors.push_back({0, 0, 0, 0});
            } else if (paletteIndex < palette->ncolors) {
                const SDL_Color& c = palette->colors[paletteIndex];
                colors.push_back({c.r, c.g, c.b, c.a});
            } else {
                colors.push_back({0, 0, 0, 255}); // Fallback
            }
        }
    } else {
        // 32-bit surface - read RGBA values directly
        const auto* pixels = static_cast<const Uint32*>(surface->pixels);
        const auto pitch   = surface->pitch / sizeof(Uint32);

        for (auto y = 0; y < surface->h; ++y) {
            Uint8 r = 0, g = 0, b = 0, a = 0;
            SDL_GetRGBA(pixels[y * pitch], details, palette, &r, &g, &b, &a);

            // Check for implicit magenta transparency
            if (r == 255 && g == 0 && b == 255) {
                r = 0;
                g = 0;
                b = 0;
                a = 0;
            }

#ifdef HAVE_PARENTHESIZED_INITIALIZATION_OF_AGGREGATES
            colors.emplace_back(r, g, b, a);
#else
            colors.push_back({r, g, b, a});
#endif
        }
    }

    return colors;
}

class BorderStylePicturesPacker final : public PackerBase<DuneTextures::border_style_type, int> {
public:
    void initialize(const SurfaceLoader* surfaceLoader) {
        auto n = 0;
        for (auto i = 0; i < NUM_DECORATIONFRAMES; ++i) {
            const auto frame = surfaceLoader->getBorderStyle(static_cast<DecorationFrame>(i));

            surfaces_.add(n++, frame.leftUpperCorner);
            surfaces_.add(n++, frame.rightUpperCorner);
            surfaces_.add(n++, frame.leftLowerCorner);
            surfaces_.add(n++, frame.rightLowerCorner);

            // Always use the original surface for color extraction
            // get_colors_vertical/horizontal now properly handle both indexed and 32-bit surfaces
            hborder[i] = get_colors_vertical(frame.hborder);
            vborder[i] = get_colors_horizontal(frame.vborder);
        }
    }

    void update(AtlasFactory23& factory23, int key, SDL_Texture* texture) {
        factory23.update<identifier_type>(
            key, texture, [&](const auto& identifier) -> DuneTexture& { return lookup(identifier); });
    }

    void update_duplicates() {
        surfaces_.update_duplicates([&](const auto& identifier) -> DuneTexture& { return lookup(identifier); });

        for (auto i = 0; i < NUM_DECORATIONFRAMES; ++i) {
            textures_[i].hborder = std::move(hborder[i]);
            textures_[i].vborder = std::move(vborder[i]);
        }
    }

private:
    DuneTexture& lookup(identifier_type n) {
        const auto idx = n / 4;
        const auto id  = n % 4;

        auto& texture = textures_[idx];

        switch (id) {
            case 0: return texture.leftUpperCorner;
            case 1: return texture.rightUpperCorner;
            case 2: return texture.leftLowerCorner;
            case 3: return texture.rightLowerCorner;
            default: THROW(std::out_of_range, "Invalid texture identifier!");
        }
    }

    std::array<std::vector<SDL_Color>, NUM_DECORATIONFRAMES> hborder{};
    std::array<std::vector<SDL_Color>, NUM_DECORATIONFRAMES> vborder{};
};

} // namespace

DuneTextures DuneTextures::create(SDL_Renderer* renderer, SurfaceLoader* surfaceLoader) {
    const auto start = std::chrono::steady_clock::now();

    // SDL3: SDL_RendererInfo removed, use properties
    SDL_PropertiesID props = SDL_GetRendererProperties(renderer);

    SDL_PixelFormat format = SCREEN_FORMAT;
    // Note: SDL3 doesn't expose texture_formats array in properties directly
    // We'll stick to SCREEN_FORMAT or check max texture size only

    const auto max_side = [&] {
        int max_texture_width =
            static_cast<int>(SDL_GetNumberProperty(props, SDL_PROP_RENDERER_MAX_TEXTURE_SIZE_NUMBER, 0));
        int max_texture_height = max_texture_width; // SDL3 property is single dimension usually

        const auto longest_side = std::min(max_texture_width, max_texture_height);
        if (0 == longest_side)
            return 8192;
        return longest_side;
    }();

    ObjectPicturePacker object_picture_packer;
    UiGraphicPacker ui_graphic_packer;
    MapChoicePacker map_choice_packer;
    TinyPicturePacker tiny_picture_packer;
    SmallDetailPicsPacker small_detail_pics_packer;
    GeneratedPicturesPacker generated_pictures_packer;
    DecorationBorderPicturesPacker decoration_border_packer;
    BorderStylePicturesPacker border_style_pictures_packer;

    // Parallelize packer initialization in bounded async batches.
    {
        using init_task_type = std::pair<const char*, std::function<void()>>;
        std::array<init_task_type, 8> init_tasks{{
            {"ObjectPicturePacker", [&] { object_picture_packer.initialize(surfaceLoader); }},
            {"UiGraphicPacker", [&] { ui_graphic_packer.initialize(surfaceLoader); }},
            {"MapChoicePacker", [&] { map_choice_packer.initialize(surfaceLoader); }},
            {"TinyPicturePacker", [&] { tiny_picture_packer.initialize(surfaceLoader); }},
            {"SmallDetailPicsPacker", [&] { small_detail_pics_packer.initialize(surfaceLoader); }},
            {"GeneratedPicturesPacker", [&] { generated_pictures_packer.initialize(surfaceLoader); }},
            {"DecorationBorderPicturesPacker", [&] { decoration_border_packer.initialize(surfaceLoader); }},
            {"BorderStylePicturesPacker", [&] { border_style_pictures_packer.initialize(surfaceLoader); }},
        }};

        const auto hw_threads  = std::max(1u, std::thread::hardware_concurrency());
        const auto max_workers = std::min(init_tasks.size(), static_cast<std::size_t>(hw_threads));

        for (std::size_t begin = 0; begin < init_tasks.size(); begin += max_workers) {
            const auto end = std::min(begin + max_workers, init_tasks.size());

            std::vector<std::future<void>> futures;
            futures.reserve(end - begin);

            for (std::size_t i = begin; i < end; ++i)
                futures.emplace_back(std::async(std::launch::async, [&, i] { init_tasks[i].second(); }));

            for (auto& f : futures)
                f.get();
        }
    }

    std::vector<sdl2::texture_ptr> textures;

    { // Scope
        AtlasFactory23 factory23;

        for (auto zoom = 0; zoom < NUM_ZOOMLEVEL; ++zoom) {
            const auto opp_key = object_picture_packer.add(
                factory23, [&](const auto& identifier, [[maybe_unused]] SDL_Surface* surface) {
                    const auto& [id, h, z, frame] = identifier;
                    (void)id;
                    (void)h;
                    (void)frame;

                    return zoom == z;
                });

            auto texture = factory23.pack(renderer, format, max_side);

            if (!texture)
                THROW(std::runtime_error, "Unable to create object pictures texture");

            object_picture_packer.update(factory23, opp_key, texture.get());

            textures.emplace_back(std::move(texture));

            factory23.clear();
        }

        assert(factory23.empty());

        static const std::set<uint32_t> force_combine_ui_graphic = {UI_RadarAnimation,
                                                                    UI_DuneLegacy,
                                                                    UI_GameMenu,
                                                                    UI_MapChoiceMap,
                                                                    UI_MapChoiceMapOnly,
                                                                    UI_MapChoicePlanet,
                                                                    UI_MapChoiceClickMap,
                                                                    UI_MenuButtonBorder,
                                                                    UI_SelectYourHouseLarge,
                                                                    UI_NewMapWindow};

        auto combined_ui_graphic = [&](const auto& identifier, SDL_Surface* surface) {
            const auto& [id, h] = identifier;

            if (force_combine_ui_graphic.contains(id))
                return true;

            return surface->w < 350 && surface->h < 350;
        };

        // Split the large UI Graphic textures by house pairs
        { // Scope
            auto is_second = false;

            std::vector<int> keys;

            auto flush_factory = [&] {
                is_second = false;

                auto texture = factory23.pack(renderer, format, max_side);

                if (!texture)
                    THROW(std::runtime_error, "Unable to create UI graphics texture");

                for (const auto key : keys)
                    ui_graphic_packer.update(factory23, key, texture.get());
                keys.clear();

                textures.emplace_back(std::move(texture));

                factory23.clear();
            };

            for_each_housetype([&](const auto house) {
                const auto ugp_key =
                    ui_graphic_packer.add(factory23, [&](const auto& identifier, SDL_Surface* surface) {
                        const auto& [id, h] = identifier;

                        return house == h && !combined_ui_graphic(identifier, surface);
                    });

                keys.push_back(ugp_key);

                if (is_second) {
                    flush_factory();
                } else {
                    is_second = true;
                }
            });

            if (!factory23.empty()) {
                flush_factory();
            }
        }

        assert(factory23.empty());

        // Combine everything else
        { // Scope
            const auto ugp_key = ui_graphic_packer.add(factory23, combined_ui_graphic);

            const auto mcp_key = map_choice_packer.add(factory23);
            const auto tpp_key = tiny_picture_packer.add(factory23);
            const auto sdp_key = small_detail_pics_packer.add(factory23);
            const auto gpp_key = generated_pictures_packer.add(factory23);
            const auto dbp_key = decoration_border_packer.add(factory23);
            const auto bsp_key = border_style_pictures_packer.add(factory23);

            auto texture = factory23.pack(renderer, format, max_side);

            if (!texture)
                THROW(std::runtime_error, "Unable to create combined texture");

            ui_graphic_packer.update(factory23, ugp_key, texture.get());
            map_choice_packer.update(factory23, mcp_key, texture.get());
            tiny_picture_packer.update(factory23, tpp_key, texture.get());
            small_detail_pics_packer.update(factory23, sdp_key, texture.get());
            generated_pictures_packer.update(factory23, gpp_key, texture.get());
            decoration_border_packer.update(factory23, dbp_key, texture.get());
            border_style_pictures_packer.update(factory23, bsp_key, texture.get());

            textures.emplace_back(std::move(texture));

            factory23.clear();
        }

        // Now, fill in duplicates
        object_picture_packer.update_duplicates();
        ui_graphic_packer.update_duplicates();
        map_choice_packer.update_duplicates();
        tiny_picture_packer.update_duplicates();
        small_detail_pics_packer.update_duplicates();
        generated_pictures_packer.update_duplicates();
        decoration_border_packer.update_duplicates();
        border_style_pictures_packer.update_duplicates();
    }

    save_texture_atlases(renderer, textures);

    const auto elapsed = std::chrono::steady_clock::now() - start;
    sdl2::log_info("DuneTextures create time: {}", std::chrono::duration<double>(elapsed).count());

    return DuneTextures{std::move(textures),
                        object_picture_packer.object_pictures2(),
                        small_detail_pics_packer.dune_textures(),
                        tiny_picture_packer.dune_textures(),
                        ui_graphic_packer.dune_textures(),
                        map_choice_packer.dune_textures(),
                        generated_pictures_packer.dune_textures(),
                        decoration_border_packer.dune_textures(),
                        border_style_pictures_packer.dune_textures()};
}
