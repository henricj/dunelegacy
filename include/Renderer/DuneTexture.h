#ifndef DUNETEXTURE_H
#define DUNETEXTURE_H

struct DuneTextureRect final {
    short x {};
    short y {};
    short w {};
    short h {};

    DuneTextureRect() = default;

    explicit DuneTextureRect(const SDL_Rect& rect) {
        assert(rect.x >= 0 && rect.y >= 0 && rect.w > 0 && rect.h > 0);
        assert(rect.x <= std::numeric_limits<short>::max() && rect.y <= std::numeric_limits<short>::max() &&
               rect.w <= std::numeric_limits<short>::max() && rect.h <= std::numeric_limits<short>::max());

        x = static_cast<short>(rect.x);
        y = static_cast<short>(rect.y);
        w = static_cast<short>(rect.w);
        h = static_cast<short>(rect.h);
    }

    [[nodiscard]] SDL_Rect as_sdl() const noexcept { return SDL_Rect {x, y, w, h}; }
    [[nodiscard]] SDL_FRect as_sdlf() const noexcept {
        return SDL_FRect {static_cast<float>(x), static_cast<float>(y), static_cast<float>(w), static_cast<float>(h)};
    }

    static DuneTextureRect create(int x, int y, int w, int h) {
        const SDL_Rect rect {x, y, w, h};
        return DuneTextureRect(rect);
    }

    DuneTextureRect& operator=(const DuneTextureRect&) = default;
    DuneTextureRect& operator                          =(const SDL_Rect& rect) { return operator=(DuneTextureRect {rect}); }
};

struct DuneTexture final {
    SDL_Texture* texture_ {};
    DuneTextureRect source_;

    DuneTexture()                   = default;
    DuneTexture(const DuneTexture&) = default;
    DuneTexture(DuneTexture&&)      = default;

    DuneTexture(SDL_Texture* texture, const SDL_Rect& rect)
        : texture_ {texture}, source_ {rect} { }

    explicit DuneTexture(SDL_Texture* texture)
        : texture_ {texture} {
        if (!texture) {
            return;
        }

        int w, h;
        SDL_QueryTexture(texture_, nullptr, nullptr, &w, &h);
        source_ = DuneTextureRect::create(0, 0, w, h);
    }

    ~DuneTexture() = default;

    DuneTexture& operator=(const DuneTexture&) = default;
    DuneTexture& operator=(DuneTexture&&) = default;

    operator bool() const noexcept { return nullptr != texture_; }

    [[nodiscard]] SDL_Rect source_rect() const noexcept { return source_.as_sdl(); }

    void draw(SDL_Renderer* renderer, int x, int y) const noexcept;
    void draw(SDL_Renderer* renderer, int x, int y, const SDL_Rect& source) const noexcept;
    void draw(SDL_Renderer* renderer, int x, int y, double angle) const noexcept;
};

#endif // DUNETEXTURE_H
