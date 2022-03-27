#include <Renderer/DuneTextures.h>

#include <FileClasses/SurfaceLoader.h>
#include <GUI/ObjectInterfaces/PalaceInterface.h>
#include <rectpack2D/finders_interface.h>

DuneTextures::DuneTextures() = default;

// clang-format off

DuneTextures::DuneTextures(std::vector<sdl2::texture_ptr>&& textures, object_pictures_type&& object_pictures,
                           small_details_type&& small_details, tiny_pictures_type&& tiny_pictures,
                           ui_graphics_type&& ui_graphics, map_choice_type&& map_choice,
                           generated_type&& generated_pictures)
    : object_pictures_{object_pictures}, small_details_{small_details}, tiny_pictures_{tiny_pictures},
      ui_graphics_{ui_graphics}, map_choice_{map_choice}, generated_pictures_{generated_pictures},
      textures_{std::move(textures)} { }

// clang-format on

DuneTextures::~DuneTextures() = default;

namespace {
constexpr bool allow_flip            = false;
constexpr auto runtime_flipping_mode = rectpack2D::flipping_option::DISABLED;

using spaces_type = rectpack2D::empty_spaces<allow_flip, rectpack2D::default_empty_spaces>;
using rect_type   = rectpack2D::output_rect_t<spaces_type>;

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
            max_side, discard_step, [](const auto& rect) { return rectpack2D::callback_result::CONTINUE_PACKING; },
            [&failed](const auto& rect) {
                failed = true;
                return rectpack2D::callback_result::ABORT_PACKING;
            },
            runtime_flipping_mode));

    if (failed) {
        sdl2::log_info("Packing failed ");
        return {false, rectpack2D::rect_wh{}};
    }

    sdl2::log_info(fmt::format("Packed in {}x{}", result_size.w, result_size.h).c_str());

    const auto side = static_cast<int>(ceil(sqrt(total_pixels)));

    sdl2::log_info(fmt::format("Pixels {0} ({1}x{1}) for efficiency {2:.1f}", total_pixels, side,
                               100 * static_cast<double>(total_pixels) / (result_size.w * result_size.h))
                       .c_str());

    // for(const auto& r : rectangles) {
    //    sdl2::log_info(fmt::format("   {}x{} at {}x{}", r.w, r.h, r.x, r.y).c_str());
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

    const auto* pa = static_cast<const char*>(lock_a.pixels());
    const auto* pb = static_cast<const char*>(lock_b.pixels());

    if (lock_a.pitch() != lock_b.pitch())
        return false;

    const auto pitch = lock_a.pitch();

    for (auto i = 0; i < a->h; ++i, pa += pitch, pb += pitch) {
        if (0 != memcmp(pa, pb, lock_a.pitch()))
            return false;
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

        rectangles_.emplace_back(0, 0, w, h);

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
    using packer_set_type = std::vector<std::tuple<int, int, SDL_Surface*>>;

    PackableSet(packer_set_type&& set) : set_{set} { }

    template<typename F>
    void for_each(const Packer& packer, F&& f) {
        for (const auto& s : set_) {
            const auto& [rect_idx, s_idx, surface] = s;

            const auto& r = packer[rect_idx];

            f(r, s_idx, surface);
        }
    }

    template<typename F>
    void for_each(const Packer& packer, F&& f) const {
        for (const auto& s : set_) {
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
        Identifier identifier;
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

    PackableSet create_packer_set(Packer& packer) {
        PackableSet::packer_set_type output;
        for (auto i = 0u; i < surfaces_.size(); ++i) {
            const auto& record = surfaces_[i];

            const auto idx = packer.add(record.surface->w, record.surface->h);

            output.emplace_back(static_cast<int>(idx), i, record.surface);
        }

        return PackableSet{std::move(output)};
    }

    template<typename Predicate>
    PackableSet create_packer_set(Packer& packer, Predicate&& predicate) {
        static_assert(std::is_invocable_r<bool, Predicate, Identifier, SDL_Surface*>::value);

        PackableSet::packer_set_type output;
        for (auto i = 0u; i < surfaces_.size(); ++i) {
            const auto& record = surfaces_[i];

            if (!predicate(record.identifier, record.surface))
                continue;

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
        for (const auto& duplicate : duplicates_) {
            const auto& [s_idx, identifier] = duplicate;

            f(surfaces_[s_idx].identifier, identifier);
        }
    }

    template<typename Lookup>
    void update_duplicates(Lookup&& lookup) {
        static_assert(std::is_invocable_r<DuneTexture&, Lookup, const Identifier&>::value);
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

    sdl2::texture_ptr pack(SDL_Renderer* render, uint32_t format, int max_side) {
        if (!packer_.pack(max_side))
            return nullptr;

        const sdl2::surface_ptr atlas_surface{
            SDL_CreateRGBSurfaceWithFormat(0, packer_.width(), packer_.height(), SDL_BITSPERPIXEL(format), format)};

        const auto draw = [&](const auto r, int s_idx, SDL_Surface* surface) {
            SDL_Rect dst{r.x, r.y, r.w, r.h};

            if (!drawSurface(surface, nullptr, atlas_surface.get(), &dst)) {
                // Retry after converting from palette to 32-bit surface...
                const sdl2::surface_ptr copy{SDL_ConvertSurfaceFormat(surface, format, 0)};

                if (!copy) {
                    sdl2::log_warn("Unable to copy surface: %s", SDL_GetError());
                    return false;
                }

                if (!drawSurface(copy.get(), nullptr, atlas_surface.get(), &dst)) {
                    sdl2::log_warn("Unable to draw object %u for house %d");
                    return false;
                }
            }

            return true;
        };

        for (const auto& set : surface_sets_) {
            set.for_each(packer_, draw);
        }

        // auto [ok, cache_path] = fnkdat("cache/", FNKDAT_USER | FNKDAT_CREAT);

        // auto path = cache_path / fmt::format("f23_{}.bmp", texture_identifier);
        // path      = path.lexically_normal().make_preferred();

        // SDL_SaveBMP(atlas_surface.get(), path.u8string().c_str());

        auto texture = sdl2::texture_ptr{SDL_CreateTextureFromSurface(renderer, atlas_surface.get())};

        if (texture && SDL_SetTextureBlendMode(texture.get(), SDL_BlendMode::SDL_BLENDMODE_BLEND)) {
            sdl2::log_warn("Unable to set texture atlas blend mode");
        }

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

    [[nodiscard]] bool empty() const noexcept { return packer_.empty(); }

private:
    Packer packer_;

    std::vector<PackableSet> surface_sets_;
};

class ObjectPicturePacker final {
public:
    using identifier_type = std::tuple<uint32_t, HOUSETYPE, int>;
    using textures_type   = DuneTextures::object_pictures_type;

    void initialize(SurfaceLoader* surfaceLoader) {

        for (auto id = 0u; id < NUM_OBJPICS; ++id) {
            if (id == ObjPic_Bullet_SonicTemp || id == ObjPic_SandwormShimmerTemp)
                continue;

            const auto harkonnen_only = harkonnen_only_.end() != harkonnen_only_.find(id);

            for (auto zoom = 0; zoom < NUM_ZOOMLEVEL; ++zoom) {
                SDL_Surface* harkonnen = nullptr;
                auto harkonnen_key     = 0;

                for_each_housetype([&](auto house) {
                    const auto is_harkonnen = house == HOUSETYPE::HOUSE_HARKONNEN;

                    if (harkonnen_only && !is_harkonnen)
                        return;

                    auto* const surface = surfaceLoader->getZoomedObjSurface(id, house, zoom);

                    if (!surface)
                        return;

                    if (is_harkonnen) {
                        harkonnen     = surface;
                        harkonnen_key = surfaces_.add({id, house, zoom}, surface);
                    } else if (harkonnen && compare_surfaces(harkonnen, surface)) {
                        // We are identical to the Harkonnen image, so let it find the Harkonnen version.
                        surfaces_.add_duplicate(harkonnen_key, {id, house, zoom});
                    } else {
                        surfaces_.add({id, house, zoom}, surface);
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
        factory23.update<identifier_type>(key, texture, [&](auto n) -> DuneTexture& { return lookup_dune_texture(n); });
    }

    void update_duplicates() {
        surfaces_.update_duplicates([&](const auto& identifier) -> DuneTexture& {
            const auto& [id, house, zoom] = identifier;

            return dune_textures_.at(zoom).at(id).at(static_cast<int>(house));
        });
    }

    [[nodiscard]] DuneTextures::object_pictures_type object_pictures2() const { return dune_textures_; }

private:
    [[nodiscard]] DuneTexture& lookup_dune_texture(int n) {
        const auto identifier = surfaces_[n];

        const auto& [id, house, zoom] = identifier;

        return dune_textures_.at(zoom).at(id).at(static_cast<int>(house));
    }

    PackableSurfaces<identifier_type> surfaces_;

    textures_type dune_textures_;

    // There is only one kind of these items, stored in the Harkonnen slot.
    static const std::set<uint32_t> harkonnen_only_;
};

// There is only one kind of these items, stored in the Harkonnen slot.
const std::set<uint32_t> ObjectPicturePacker::harkonnen_only_ = {
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
        factory23.update<identifier_type>(key, texture,
                                          [&](auto n) -> DuneTexture& { return textures_.at(surfaces_[n]); });
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
        for (auto id = 0u; id < textures_.size(); ++id) {
            auto* surface = surfaceLoader->getTinyPictureSurface(id);

            if (!surface) {
                sdl2::log_warn("No surface available for tiny picture %d", id);
                continue;
            }

            surfaces_.add(id, surface);
        }
    }
};

class SmallDetailPicsPacker final : public PackerBase<DuneTextures::small_details_type, uint32_t> {
public:
    void initialize(SurfaceLoader* surfaceLoader) {
        for (auto id = 0u; id < textures_.size(); ++id) {
            auto* surface = surfaceLoader->getSmallDetailSurface(id);

            if (!surface) {
                sdl2::log_warn("No surface available for small detail %d", id);
                continue;
            }

            surfaces_.add(id, surface);
        }
    }
};

class GeneratedPicturesPacker final : public PackerBase<DuneTextures::generated_type, uint32_t> {
public:
    void initialize(SurfaceLoader* surfaceLoader) {
        auto palaceReady = PalaceInterface::createSurface(surfaceLoader, GeneratedPicture::PalaceReadyText);

        if (palaceReady)
            generated_.at(static_cast<int>(GeneratedPicture::PalaceReadyText)) = std::move(palaceReady);

        for (auto id = 0u; id < textures_.size(); ++id) {
            auto* surface = generated_.at(id).get();

            if (!surface) {
                sdl2::log_warn("No surface available for generated picture %d", id);
                continue;
            }

            surfaces_.add(id, surface);
        }
    }

private:
    std::array<sdl2::surface_ptr, std::tuple_size<textures_type>::value> generated_;
};

} // namespace

DuneTextures DuneTextures::create(SDL_Renderer* renderer, SurfaceLoader* surfaceLoader) {
    SDL_RendererInfo info;
    SDL_GetRendererInfo(renderer, &info);

    Uint32 format = SCREEN_FORMAT;
    for (auto i = 0u; i < info.num_texture_formats; ++i) {
        const auto f = info.texture_formats[i];

        if (SDL_ISPIXELFORMAT_FOURCC(f))
            continue;

        if (SDL_ISPIXELFORMAT_ALPHA(f)) {
            format = f;
            break;
        }
    }

    const auto max_side = [&] {
        const auto longest_side = std::min(info.max_texture_width, info.max_texture_height);
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

    object_picture_packer.initialize(surfaceLoader);
    ui_graphic_packer.initialize(surfaceLoader);
    map_choice_packer.initialize(surfaceLoader);
    tiny_picture_packer.initialize(surfaceLoader);
    small_detail_pics_packer.initialize(surfaceLoader);
    generated_pictures_packer.initialize(surfaceLoader);

    std::vector<sdl2::texture_ptr> textures;

    { // Scope
        AtlasFactory23 factory23;

        for (auto zoom = 0; zoom < NUM_ZOOMLEVEL; ++zoom) {

            const auto opp_key =
                object_picture_packer.add(factory23, [&](const auto& identifier, SDL_Surface* surface) {
                    const auto& [id, h, z] = identifier;

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

        static const std::set<uint32_t> force_combine_ui_graphic = {
            UI_RadarAnimation,   UI_DuneLegacy,           UI_MenuBackground,  UI_GameMenu,
            UI_MapChoiceMap,     UI_MapChoiceMapOnly,     UI_MapChoicePlanet, UI_MapChoiceClickMap,
            UI_MenuButtonBorder, UI_SelectYourHouseLarge, UI_NewMapWindow};

        auto combined_ui_graphic = [&](const auto& identifier, SDL_Surface* surface) {
            const auto& [id, h] = identifier;

            if (force_combine_ui_graphic.find(id) != force_combine_ui_graphic.end())
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

            auto texture = factory23.pack(renderer, format, max_side);

            if (!texture)
                THROW(std::runtime_error, "Unable to create combined texture");

            ui_graphic_packer.update(factory23, ugp_key, texture.get());
            map_choice_packer.update(factory23, mcp_key, texture.get());
            tiny_picture_packer.update(factory23, tpp_key, texture.get());
            small_detail_pics_packer.update(factory23, sdp_key, texture.get());
            generated_pictures_packer.update(factory23, gpp_key, texture.get());

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
    }

    // auto count = 0;
    // for(const auto& texture : textures) {
    //    auto path = std::filesystem::path{"c:/temp/"} / fmt::format("texture_{}.bmp", count++);
    //    path      = path.lexically_normal().make_preferred();

    //    SaveTextureAsBmp(renderer, texture.get(), path.u8string().c_str());
    //}

    return DuneTextures{std::move(textures),
                        object_picture_packer.object_pictures2(),
                        small_detail_pics_packer.dune_textures(),
                        tiny_picture_packer.dune_textures(),
                        ui_graphic_packer.dune_textures(),
                        map_choice_packer.dune_textures(),
                        generated_pictures_packer.dune_textures()};
}
