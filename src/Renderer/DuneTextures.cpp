#include <Renderer/DuneTextures.h>

#include <FileClasses/SurfaceLoader.h>
#include <GUI/ObjectInterfaces/PalaceInterface.h>
#include <rectpack2D/finders_interface.h>

DuneTextures::DuneTextures() = default;

DuneTextures::DuneTextures(std::vector<sdl2::texture_ptr>&& textures, object_pictures_type&& object_pictures,
                           small_details_type&& small_details, tiny_pictures_type&& tiny_pictures,
                           ui_graphics_type&& ui_graphics, generated_type&& generated_pictures)
    : object_pictures_{std::move(object_pictures)}, small_details_{std::move(small_details)},
      tiny_pictures_{std::move(tiny_pictures)}, ui_graphics_{std::move(ui_graphics)},
      generated_pictures_{std::move(generated_pictures)}, textures_{std::move(textures)} { }


DuneTextures::~DuneTextures() = default;

namespace {
constexpr bool allow_flip            = false;
constexpr auto runtime_flipping_mode = rectpack2D::flipping_option::DISABLED;

using spaces_type = rectpack2D::empty_spaces<allow_flip, rectpack2D::default_empty_spaces>;
using rect_type   = rectpack2D::output_rect_t<spaces_type>;

std::tuple<bool, rectpack2D::rect_wh> packRectangles(const int max_side, std::vector<rect_type>& rectangles) {
    const auto discard_step = 1;

    auto failed = false;

    const auto total_pixels = [&]() {
        auto sum = 0;
        for(auto& r : rectangles)
            sum += r.w * r.h;
        return sum;
    }();

    const auto result_size = rectpack2D::find_best_packing<spaces_type>(
        rectangles,
        make_finder_input(
            max_side, discard_step, [](const auto& rect) { return rectpack2D::callback_result::CONTINUE_PACKING; },
            [&failed](const auto& rect) {
                failed = true;
                return rectpack2D::callback_result::ABORT_PACKING;
            },
            runtime_flipping_mode));

    if(failed) {
        SDL_Log("Packing failed ");
        return {false, rectpack2D::rect_wh{}};
    }

    SDL_Log(fmt::format("Packed in {}x{}", result_size.w, result_size.h).c_str());

    const auto side = static_cast<int>(ceil(sqrt(total_pixels)));

    SDL_Log(fmt::format("Pixels {0} ({1}x{1}) for efficiency {2:.1f}", total_pixels, side,
                        100 * static_cast<double>(total_pixels) / (result_size.w * result_size.h))
                .c_str());

    // for(const auto& r : rectangles) {
    //    SDL_Log(fmt::format("   {}x{} at {}x{}", r.w, r.h, r.x, r.y).c_str());
    //}

#if _DEBUG
    for(auto i = 0; i < rectangles.size() - 1; ++i) {
        const auto& a = rectangles[i];
        SDL_Rect    sa{a.x, a.y, a.w, a.h};

        for(auto j = i + 1; j < rectangles.size(); ++j) {
            const auto& b = rectangles[j];
            SDL_Rect    sb{b.x, b.y, b.w, b.h};

            if(SDL_HasIntersection(&sa, &sb)) { SDL_Log("Failed packing"); }
        }
    }
#endif // _DEBUG

    return {true, result_size};
}

bool compare_surfaces(SDL_Surface* a, SDL_Surface* b) {
    if(a->w != b->w || a->h != b->h) return false;

    if (a == b) return true;

    const sdl2::surface_lock lock_a{a};
    const sdl2::surface_lock lock_b{b};

    const auto* pa = static_cast<const char *>(lock_a.pixels());
    const auto* pb = static_cast<const char *>(lock_b.pixels());

    if(lock_a.pitch() != lock_b.pitch()) return false;

    const auto pitch = lock_a.pitch();

    for(auto i = 0; i < a->h; ++i, pa += pitch, pb += pitch) {
        if(0 != memcmp(pa, pb, lock_a.pitch())) return false;
    }

    return true;
}

// There is only one kind of these items, stored in the Harkonnen slot.
const std::set<Uint32> harkonnenOnly = {
    ObjPic_ExplosionSmall,
    ObjPic_ExplosionMedium1,
    ObjPic_ExplosionMedium2,
    ObjPic_ExplosionLarge1,
    ObjPic_ExplosionLarge2,
    ObjPic_ExplosionSmallUnit,
    ObjPic_ExplosionFlames,
    ObjPic_ExplosionSpiceBloom,
    ObjPic_SandwormShimmerMask,
    ObjPic_Terrain,
    ObjPic_DestroyedStructure,
    ObjPic_RockDamage,
    ObjPic_SandDamage,
    ObjPic_Terrain_Hidden,
    ObjPic_Terrain_HiddenFog,
    ObjPic_Terrain_Tracks,
    ObjPic_Star,
};

void add_object_pictures(SurfaceLoader* surfaceLoader, std::vector<rect_type>& rectangles,
                 std::vector<std::tuple<int, Uint32, HOUSETYPE, SDL_Surface*>>& object_pictures, int zoom) {
    if(rectangles.size() + NUM_OBJPICS < rectangles.capacity()) rectangles.reserve(rectangles.size() + NUM_OBJPICS);

    for(auto id = 0u; id < NUM_OBJPICS; ++id) {
        if(id == ObjPic_Bullet_SonicTemp || id == ObjPic_SandwormShimmerTemp) continue;

        const auto harkonnen_only = harkonnenOnly.end() != harkonnenOnly.find(id);

        SDL_Surface* harkonnen       = nullptr;
        auto         harkonnen_index = 0;
        for(auto h = 0; h < static_cast<int>(HOUSETYPE::NUM_HOUSES); ++h) {
            if(h > 0 && harkonnen_only) continue;

            auto* surface = surfaceLoader->getZoomedObjSurface(id, static_cast<HOUSETYPE>(h), zoom);

            if(!surface) continue;

            if(0 == h) {
                harkonnen       = surface;
                harkonnen_index = rectangles.size();
            } else if(compare_surfaces(harkonnen, surface)) {
                // We are identical to the Harkonnen image, so let it find the Harkonnen version.
                object_pictures.emplace_back(harkonnen_index, id, static_cast<HOUSETYPE>(h), nullptr);
                continue;
            }

            object_pictures.emplace_back(rectangles.size(), id, static_cast<HOUSETYPE>(h), surface);
            rectangles.emplace_back(0, 0, surface->w, surface->h);
        }
    }
}

template<typename Predicate>
void add_ui_graphics(SurfaceLoader* surfaceLoader, HOUSETYPE house, std::vector<rect_type>& rectangles,
                     std::vector<std::tuple<int, Uint32, HOUSETYPE, SDL_Surface*>>& ui_graphics, Predicate&& predicate) {
    static_assert(std::is_invocable_r<bool, Predicate, Uint32, HOUSETYPE, SDL_Surface*>::value);

    if(rectangles.size() + NUM_UIGRAPHICS < rectangles.capacity())
        rectangles.reserve(rectangles.size() + NUM_UIGRAPHICS);

    for(auto id = 0u; id < NUM_UIGRAPHICS; ++id) {
        auto* surface = surfaceLoader->getUIGraphicSurface(id, house);

        if(!surface || !predicate(id, house, surface)) continue;

        ui_graphics.emplace_back(rectangles.size(), id, house, surface);
        rectangles.emplace_back(0, 0, surface->w, surface->h);
    }
}

void add_small_details(SurfaceLoader* surfaceLoader, std::vector<rect_type>& rectangles,
                       std::vector<std::tuple<int, int, SDL_Surface*>>& small_detail) {
    for(auto id = 0; id < NUM_SMALLDETAILPICS; ++id) {
        auto* pic = surfaceLoader->getSmallDetailSurface(id);

        if(!pic) continue;

        small_detail.emplace_back(rectangles.size(), id, pic);
        rectangles.emplace_back(0, 0, pic->w, pic->h);
    }
}

void add_tiny_pictures(SurfaceLoader* surfaceLoader, std::vector<rect_type>& rectangles,
                       std::vector<std::tuple<int, int, SDL_Surface*>>& tiny_pictures) {
    for(auto id = 0; id < NUM_TINYPICTURE; ++id) {
        auto* pic = surfaceLoader->getTinyPictureSurface(id);

        if(!pic) continue;

        tiny_pictures.emplace_back(rectangles.size(), id, pic);
        rectangles.emplace_back(0, 0, pic->w, pic->h);
    }
}

bool draw_object_pictures(Uint32 format, const std::vector<rect_type>& rectangles,
                          const std::vector<std::tuple<int, Uint32, HOUSETYPE, SDL_Surface*>>& object_pictures,
                          SDL_Surface*                                                         atlas_surface) {
    for(const auto& [idx, objectID, house, surface] : object_pictures) {
        if(!surface) continue; // We use the same graphics as another object_picture.

        const auto& r = rectangles[idx];

        SDL_Rect dest{r.x, r.y, r.w, r.h};

        if(SDL_BlitSurface(surface, nullptr, atlas_surface, &dest)) {
            // Retry after converting from palette to 32-bit surface...
            sdl2::surface_ptr copy{SDL_ConvertSurfaceFormat(surface, format, 0)};

            if(!copy) {
                SDL_Log("Unable to copy surface for object %u for house %d: %s", objectID, static_cast<int>(house),
                        SDL_GetError());
                return false;
            }

            if(SDL_BlitSurface(copy.get(), nullptr, atlas_surface, &dest)) {
                SDL_Log("Unable to blit object %u for house %d: %s", objectID, static_cast<int>(house), SDL_GetError());
                return false;
            }
        }
    }

    return true;
}

bool draw_ui_graphics(SDL_Surface* atlas_surface, Uint32 format,
                      const std::vector<rect_type>& rectangles,
                      const std::vector<std::tuple<int, Uint32, HOUSETYPE, SDL_Surface*>>& ui_graphics) {
    for(const auto& [idx, objectID, house, surface] : ui_graphics) {
        if(!surface) continue; // We use the same graphics as another object_picture.

        const auto& r = rectangles[idx];

        SDL_Rect dest{r.x, r.y, r.w, r.h};

        if(SDL_BlitSurface(surface, nullptr, atlas_surface, &dest)) {
            // Retry after converting from palette to 32-bit surface...
            sdl2::surface_ptr copy{SDL_ConvertSurfaceFormat(surface, format, 0)};

            if(!copy) {
                SDL_Log("Unable to copy surface for object %u for house %d: %s", objectID, static_cast<int>(house),
                        SDL_GetError());
                return false;
            }

            if(SDL_BlitSurface(copy.get(), nullptr, atlas_surface, &dest)) {
                SDL_Log("Unable to blit object %u for house %d: %s", objectID, static_cast<int>(house), SDL_GetError());
                return false;
            }
        }
    }

    return true;
}

bool draw_small_details(SDL_Surface* atlas_surface, const std::vector<rect_type>& rectangles,
                        const std::vector<std::tuple<int, int, SDL_Surface*>>& small_detail) {
    for(const auto& [idx, id, surface] : small_detail) {
        const auto& r = rectangles[idx];

        SDL_Rect dest{r.x, r.y, r.w, r.h};

        if(SDL_BlitSurface(surface, nullptr, atlas_surface, &dest)) {
            SDL_Log("Unable to blit object %d", id);
            return false;
        }
    }

    return true;
}

bool draw_tiny_pictures(SDL_Surface*                                           atlas_surface,
                        const std::vector<rect_type>&                          rectangles,
                        const std::vector<std::tuple<int, int, SDL_Surface*>>& tiny_picture) {
    for(const auto& [idx, id, surface] : tiny_picture) {
        const auto& r = rectangles[idx];

        SDL_Rect dest{r.x, r.y, r.w, r.h};

        if(SDL_BlitSurface(surface, nullptr, atlas_surface, &dest)) {
            SDL_Log("Unable to blit tiny picture %u", id);
            return false;
        }
    }

    return true;
}

class AtlasFactory final {
    std::vector<rect_type> rectangles;
    int                    count{};

public:
    template<typename Add, typename Draw, typename Update>
    sdl2::texture_ptr create_surface(SDL_Renderer* renderer, Uint32 format, int max_side, Add&& add, Draw&& draw,
                                     Update&& update) {
        static_assert(std::is_invocable_r<bool, Add, std::vector<rect_type>&>::value);
        static_assert(std::is_invocable_r<bool, Draw, SDL_Surface*, const std::vector<rect_type>&>::value);
        static_assert(std::is_invocable_r<bool, Update, SDL_Texture*, const std::vector<rect_type>&>::value);

        rectangles.clear();

        if(!add(rectangles)) return nullptr;

        const auto& [ok, size] = packRectangles(max_side, rectangles);

        if(!ok) return nullptr;

        const sdl2::surface_ptr atlas_surface{
            SDL_CreateRGBSurfaceWithFormat(0, size.w, size.h, SDL_BITSPERPIXEL(format), format)};

        SDL_SetSurfaceBlendMode(atlas_surface.get(), SDL_BlendMode::SDL_BLENDMODE_BLEND);

        if(!draw(atlas_surface.get(), rectangles)) return nullptr;

        auto path = std::filesystem::path{"c:/temp/"} / fmt::format("surface_{}.bmp", count++);
        path      = path.lexically_normal().make_preferred();

        SDL_SaveBMP(atlas_surface.get(), path.u8string().c_str());

        auto texture = sdl2::texture_ptr{SDL_CreateTextureFromSurface(renderer, atlas_surface.get())};

        if(!texture) return nullptr;

        if(!update(texture.get(), rectangles)) return nullptr;

        return texture;
    }
};

class Packer final {
    std::vector<rect_type> rectangles_;

    int w_{};
    int h_{};

public:
    void clear() { rectangles_.clear(); }

    int add(int w, int h) {
        const auto ret = static_cast<int>(rectangles_.size());

        rectangles_.emplace_back(0, 0, w, h);

        return ret;
    }

    bool pack(int max_side) {
        const auto& [ok, size] = packRectangles(max_side, rectangles_);

        if(ok) {
            w_ = size.w;
            h_ = size.h;
        } else {
            w_ = 0;
            h_ = 0;
        }

        return ok;
    }

    const rect_type& operator[](int index) const { return rectangles_.at(index); }

    int width() const noexcept { return w_; }
    int height() const noexcept { return h_; }
};

class PackableSet {
public:
    using packer_set_type = std::vector<std::tuple<int, int, SDL_Surface*>>;

    PackableSet(packer_set_type&& set) : set_{set} { }

    template<typename F>
    void for_each(const Packer& packer, F&& f) {
        for(const auto& s : set_) {
            const auto& [rect_idx, s_idx, surface] = s;

            const auto& r = packer[rect_idx];

            f(r, s_idx, surface);
        }
    }

    template<typename F>
    void for_each(const Packer& packer, F&& f) const {
        for(const auto& s : set_) {
            const auto& [rect_idx, s_idx, surface] = s;

            const auto& r = packer[rect_idx];

            f(r, s_idx, surface);
        }
    }

private:
    packer_set_type set_;
};

template<typename Identifier>
class PackableSurfaces {
    struct record {
        Identifier   identifier;
        SDL_Surface* surface;
    };

    std::vector<record> surfaces_;
    std::vector<std::tuple<int, Identifier>> duplicates_;

public:
    using identifier_type = Identifier;

    int add(Identifier identifier, SDL_Surface* surface) {
        const auto idx = static_cast<int>(surfaces_.size());

        surfaces_.push_back({identifier, surface});

        return idx;
    }

    template<typename Predicate>
    PackableSet create_packer_set(Packer& packer, Predicate&& predicate) {
        static_assert(std::is_invocable_r<bool, Predicate, Identifier, SDL_Surface*>::value);

        PackableSet::packer_set_type output;
        for(auto i = 0u; i < surfaces_.size(); ++i) {
            const auto& record = surfaces_[i];

            if(!predicate(record.identifier, record.surface)) continue;

            const auto idx = packer.add(record.surface->w, record.surface->h);

            output.emplace_back(static_cast<int>(idx), i, record.surface);
        }

        return PackableSet{std::move(output)};
    }

    void add_duplicate(int key, Identifier identifier) {
        assert(key >= 0 && key < surfaces_.size());

        duplicates_.emplace_back(key, identifier);
    }

    template<typename F>
    void for_each_duplicate(F&& f) const {
        for(const auto& duplicate : duplicates_) {
            const auto& [s_idx, identifier] = duplicate;

            f(surfaces_[s_idx].identifier, identifier);
        }
    }

    template<typename Lookup>
    void update_duplicates(Lookup&& lookup)
    {
        static_assert(std::is_invocable_r<DuneTexture&, Lookup, const Identifier&>::value);

        for(const auto& duplicate : duplicates_) {
            const auto& [s_idx, identifier] = duplicate;

            lookup(identifier) = lookup(surfaces_.at(s_idx).identifier);
        }
    }


    Identifier operator[](int key) const { return surfaces_.at(key).identifier; }
};


class AtlasFactory23 final {
public:
    template<typename Identifier, typename Predicate>
    int add(PackableSurfaces<Identifier>& packable, Predicate&& predicate) {
        auto set = packable.create_packer_set(packer_, predicate);

        const auto ret = static_cast<int>(surface_sets_.size());
        surface_sets_.emplace_back(std::move(set));

        return ret;
    }

    sdl2::texture_ptr pack(SDL_Renderer* render, Uint32 format, int max_side) {
        if(!packer_.pack(max_side)) return nullptr;

        const sdl2::surface_ptr atlas_surface{
            SDL_CreateRGBSurfaceWithFormat(0, packer_.width(), packer_.height(), SDL_BITSPERPIXEL(format), format)};

        SDL_SetSurfaceBlendMode(atlas_surface.get(), SDL_BlendMode::SDL_BLENDMODE_BLEND);

        const auto draw = [&](const auto r, int s_idx, SDL_Surface* surface) {
            SDL_Rect dst{r.x, r.y, r.w, r.h};

            if(SDL_BlitSurface(surface, nullptr, atlas_surface.get(), &dst)) {
                // Retry after converting from palette to 32-bit surface...
                const sdl2::surface_ptr copy{SDL_ConvertSurfaceFormat(surface, format, 0)};

                if(!copy) {
                    SDL_Log("Unable to copy surface: %s", SDL_GetError());
                    return false;
                }

                if(SDL_BlitSurface(copy.get(), nullptr, atlas_surface.get(), &dst)) {
                    SDL_Log("Unable to blit object %u for house %d: %s", SDL_GetError());
                    return false;
                }
            }

            return true;
        };

        for(const auto& set : surface_sets_) {
            set.for_each(packer_, draw);
        }

        auto path = std::filesystem::path{"c:/temp/"} / fmt::format("surface_f23_{}.bmp", count_++);
        path      = path.lexically_normal().make_preferred();

        SDL_SaveBMP(atlas_surface.get(), path.u8string().c_str());

        auto texture = sdl2::texture_ptr{SDL_CreateTextureFromSurface(renderer, atlas_surface.get())};

        return texture;
    }

    template<typename Identifier, typename Lookup>
    void update(int key, SDL_Texture* texture, Lookup&& lookup) {
        static_assert(std::is_invocable_r<const DuneTexture&, Lookup, int>::value);

        const auto& set = surface_sets_.at(key);

        set.for_each(packer_, [&](const auto& r, auto s_idx, auto* surface) {
            const SDL_Rect rect{r.x, r.y, r.w, r.h};

            lookup(s_idx) = DuneTexture{texture, rect};
        });
    }

    void clear() {
        packer_.clear();
        surface_sets_.clear();
    }

private:
    Packer packer_;

    std::vector<PackableSet> surface_sets_;

    int count_{};
};


class ObjectPicturePacker {
    using object_picture_identifier_type = std::tuple<Uint32, HOUSETYPE, int>;
    PackableSurfaces<object_picture_identifier_type> object_picture_surfaces_;

    DuneTextures::object_pictures_type object_pictures_;

    [[nodiscard]] DuneTexture& lookup_dune_texture(int n) {
        const auto identifier = object_picture_surfaces_[n];

        const auto& [id, house, zoom] = identifier;

        return object_pictures_.at(zoom).at(id).at(static_cast<int>(house));
    };

public:
    void initialize(SurfaceLoader* surfaceLoader) {

        for(auto id = 0u; id < NUM_OBJPICS; ++id) {
            if(id == ObjPic_Bullet_SonicTemp || id == ObjPic_SandwormShimmerTemp) continue;

            const auto harkonnen_only = harkonnenOnly.end() != harkonnenOnly.find(id);

            for(auto zoom = 0; zoom < NUM_ZOOMLEVEL; ++zoom) {
                SDL_Surface* harkonnen     = nullptr;
                auto         harkonnen_key = 0;

                for_each_housetype([&](auto house) {
                    const auto is_harkonnen = house == HOUSETYPE::HOUSE_HARKONNEN;

                    if(harkonnen_only && !is_harkonnen) return;

                    auto* const surface = surfaceLoader->getZoomedObjSurface(id, house, zoom);

                    if(!surface) return;

                    if(is_harkonnen) {
                        harkonnen     = surface;
                        harkonnen_key = object_picture_surfaces_.add({id, house, zoom}, surface);
                    } else if(harkonnen && compare_surfaces(harkonnen, surface)) {
                        // We are identical to the Harkonnen image, so let it find the Harkonnen version.
                        object_picture_surfaces_.add_duplicate(harkonnen_key, {id, house, zoom});
                    } else {
                        object_picture_surfaces_.add({id, house, zoom}, surface);
                    }
                });
            }
        }
    }

    template<typename Predicate>
    int add(AtlasFactory23& factory23, Predicate&& predicate) {
        return factory23.add(object_picture_surfaces_, predicate);
    }

    void update(AtlasFactory23& factory23, int key, SDL_Texture* texture) {
        factory23.update<object_picture_identifier_type>(key, texture, [&](auto n) -> DuneTexture& { return lookup_dune_texture(n); });
    }

    void update_duplicates() {
        object_picture_surfaces_.update_duplicates([&](const auto& identifier) -> DuneTexture& {
            const auto& [id, house, zoom] = identifier;

            return object_pictures_.at(zoom).at(id).at(static_cast<int>(house));
        });
    }

    [[nodiscard]] DuneTextures::object_pictures_type object_pictures2() const { return object_pictures_; }
};


} // namespace

DuneTextures DuneTextures::create(SDL_Renderer* renderer, SurfaceLoader* surfaceLoader) {
    SDL_RendererInfo info;
    SDL_GetRendererInfo(renderer, &info);

    Uint32 format = SCREEN_FORMAT;
    for(auto i = 0u; i < info.num_texture_formats; ++i) {
        const auto f = info.texture_formats[i];

        if(SDL_ISPIXELFORMAT_FOURCC(f)) continue;

        if(SDL_ISPIXELFORMAT_ALPHA(f)) {
            format = f;
            break;
        }
    }

    const auto max_side = [&] {
        const auto longest_side = std::min(info.max_texture_width, info.max_texture_height);
        if(0 == longest_side) return 8192;
        return longest_side;
    }();

    //object_pictures_type object_pictures;
    ui_graphics_type     ui_graphics;
    small_details_type   small_details;
    tiny_pictures_type   tiny_pictures;
    generated_type       generated_pictures;

    std::vector<sdl2::texture_ptr> textures;

        ObjectPicturePacker opp;

    { // Scope
        AtlasFactory23 factory23;

        opp.initialize(surfaceLoader);

        for(auto zoom = 0; zoom < NUM_ZOOMLEVEL; ++zoom) {

            const auto opp_key = opp.add(factory23, [&](const auto& identifier, SDL_Surface* surface) {
                const auto& [id, h, z] = identifier;

                return zoom == z;
            });

            auto texture = factory23.pack(renderer, format, max_side);

            opp.update(factory23, opp_key, texture.get());

            textures.emplace_back(std::move(texture));

            factory23.clear();
        }

        // Now, fill in duplicates
        opp.update_duplicates();

    }

    AtlasFactory factory;
    { // Scope
        //std::vector<std::tuple<int, Uint32, HOUSETYPE, SDL_Surface*>> object_pictures_index;
        //object_pictures_index.reserve(NUM_OBJPICS * static_cast<int>(HOUSETYPE::NUM_HOUSES));

        //for(auto zoom = 0; zoom < NUM_ZOOMLEVEL; ++zoom) {
        //    object_pictures_index.clear();

        //    auto atlas_texture = factory.create_surface(
        //        renderer, format, max_side,
        //        [&](std::vector<rect_type>& rectangles) {
        //            add_object_pictures(surfaceLoader, rectangles, object_pictures_index, zoom);

        //            return true;
        //        },
        //        [&](SDL_Surface* atlas_surface, const std::vector<rect_type>& rectangles) {
        //            return draw_object_pictures(format, rectangles, object_pictures_index, atlas_surface);
        //        },
        //        [&](SDL_Texture* texture, const std::vector<rect_type>& rectangles) {
        //            for(const auto& [rectangle_idx, id, house, surface] : object_pictures_index) {
        //                const auto& r = rectangles[rectangle_idx];

        //                object_pictures.at(zoom).at(id).at(static_cast<int>(house)) =
        //                    DuneTexture{texture, SDL_Rect{r.x, r.y, r.w, r.h}};
        //            }
        //            return true;
        //        });

        //    if(!atlas_texture) return DuneTextures{};

        //    textures.emplace_back(std::move(atlas_texture));
        //}
    }

    // Create Large UI Graphics textures
    { // Scope
        std::vector<std::tuple<int, Uint32, HOUSETYPE, SDL_Surface*>> ui_graphics_index;
        ui_graphics_index.reserve(NUM_UIGRAPHICS);

        for(auto house = 0; house < static_cast<int>(HOUSETYPE::NUM_HOUSES); ++house) {
            ui_graphics_index.clear();

            auto atlas_texture = factory.create_surface(
                renderer, format, max_side,
                [&](std::vector<rect_type>& rectangles) {
                    add_ui_graphics(surfaceLoader, static_cast<HOUSETYPE>(house), rectangles, ui_graphics_index,
                                    [](Uint32 id, HOUSETYPE house, SDL_Surface* surface) { return surface->h >= 100 || surface->w >= 100; });

                    return true;
                },
                [&](SDL_Surface* atlas_surface, const std::vector<rect_type>& rectangles) {
                    if(!draw_ui_graphics(atlas_surface, format, rectangles, ui_graphics_index)) return false;

                    return true;
                },
                [&](SDL_Texture* texture, const std::vector<rect_type>& rectangles) {
                    for(const auto& [rectangle_idx, id, idx_house, surface] : ui_graphics_index) {
                        const auto& r = rectangles[rectangle_idx];

                        assert(idx_house == static_cast<HOUSETYPE>(house));

                        ui_graphics.at(house).at(id) =
                            DuneTexture{texture, SDL_Rect{r.x, r.y, r.w, r.h}};
                    }

                    return true;
                });

            if(!atlas_texture) return DuneTextures{};

            textures.emplace_back(std::move(atlas_texture));
        }
    }

    { // Scope
        std::vector<std::tuple<int, Uint32, HOUSETYPE, SDL_Surface*>> ui_graphics_index;
        ui_graphics_index.reserve(NUM_UIGRAPHICS);

        std::vector<std::tuple<int, int, SDL_Surface*>> small_detail_index;
        small_detail_index.reserve(NUM_SMALLDETAILPICS);

        std::vector<std::tuple<int, int, SDL_Surface*>> tiny_picture_index;
        tiny_picture_index.reserve(NUM_TINYPICTURE);

        std::vector<std::tuple<int, GeneratedPicture, SDL_Surface*>> generated_pictures_index;
        generated_pictures_index.reserve(NUM_GENERATEDPICTURES);

        std::vector<std::tuple<GeneratedPicture, sdl2::surface_ptr>> generated;

        auto palaceReady = PalaceInterface::createSurface(surfaceLoader, GeneratedPicture::PalaceReadyText);
        if(palaceReady) generated.emplace_back(GeneratedPicture::PalaceReadyText, std::move(palaceReady));

        auto atlas_texture = factory.create_surface(
            renderer, format, max_side,
            [&](std::vector<rect_type>& rectangles) {
                for(auto house = 0; house < static_cast<int>(HOUSETYPE::NUM_HOUSES); ++house) {
                    add_ui_graphics(surfaceLoader, static_cast<HOUSETYPE>(house), rectangles, ui_graphics_index,
                                    [](Uint32, HOUSETYPE, SDL_Surface* surface) {
                                        return surface->h < 100 && surface->w < 100;
                                    });
                }

                add_small_details(surfaceLoader, rectangles, small_detail_index);

                add_tiny_pictures(surfaceLoader, rectangles, tiny_picture_index);

                for(const auto& pic : generated) {
                    const auto& [id, surface] = pic;
                    generated_pictures_index.emplace_back(rectangles.size(), id, surface.get());
                    rectangles.emplace_back(0, 0, surface->w, surface->h);
                }

                return true;
            },
            [&](SDL_Surface* atlas_surface, const std::vector<rect_type>& rectangles) {
                    if(!draw_ui_graphics(atlas_surface, format, rectangles, ui_graphics_index))
                        return false;

                if(!draw_small_details(atlas_surface, rectangles, small_detail_index)) return false;

                if(!draw_tiny_pictures(atlas_surface, rectangles, tiny_picture_index)) return false;

                for(const auto& [idx, id, surface] : generated_pictures_index) {
                    const auto& r = rectangles[idx];

                    SDL_Rect dest{r.x, r.y, r.w, r.h};

                    if(SDL_BlitSurface(surface, nullptr, atlas_surface, &dest)) {
                        SDL_Log("Unable to generated picture %u", id);
                        return false;
                    }
                }
                return true;
            },
            [&](SDL_Texture* texture, const std::vector<rect_type>& rectangles) {
                for(const auto& [rectangle_idx, id, house, surface] : ui_graphics_index) {
                    const auto& r = rectangles[rectangle_idx];

                    ui_graphics.at(static_cast<int>(house)).at(id) = DuneTexture{texture, SDL_Rect{r.x, r.y, r.w, r.h}};
                }

                for(const auto& [idx, id, surface] : small_detail_index) {
                    const auto& r = rectangles[idx];

                    small_details.at(id) = DuneTexture{texture, SDL_Rect{r.x, r.y, r.w, r.h}};
                }

                for(const auto& [idx, id, surface] : tiny_picture_index) {
                    const auto& r = rectangles[idx];

                    tiny_pictures.at(id) = DuneTexture{texture, SDL_Rect{r.x, r.y, r.w, r.h}};
                }

                for(const auto& [idx, id, surface] : generated_pictures_index) {
                    const auto& r = rectangles[idx];

                    generated_pictures.at(static_cast<int>(id)) = DuneTexture{texture, SDL_Rect{r.x, r.y, r.w, r.h}};
                }

                return true;
            });

        if(!atlas_texture) return DuneTextures{};

        textures.emplace_back(std::move(atlas_texture));
    }

    //auto count = 0;
    //for(const auto& texture : textures) {
    //    auto path = std::filesystem::path{"c:/temp/"} / fmt::format("texture_{}.bmp", count++);
    //    path      = path.lexically_normal().make_preferred();

    //    SaveTextureAsBmp(renderer, texture.get(), path.u8string().c_str());
    //}

    return DuneTextures{std::move(textures), opp.object_pictures2(), std::move(small_details),
                        std::move(tiny_pictures), std::move(ui_graphics), std::move(generated_pictures)};
}
