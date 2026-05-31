#ifndef DUNERENDERER_H
#define DUNERENDERER_H

#include "Colors.h"
#include "DuneTexture.h"

#include <misc/dune_sdl.h>

#include <initializer_list>
#include <span>

struct DuneTexture;

#if _DEBUG
#    include <map>

namespace DuneRendererImplementation {
extern SDL_Texture* render_texture;
extern int render_texture_changes;
extern int render_copies;
extern int render_presents;
extern bool render_dump;
extern std::map<SDL_Texture*, int> render_textures;

void countRenderCopy(SDL_Texture* texture);
} // namespace DuneRendererImplementation

void Dune_RenderDump();
#else // DEBUG
namespace DuneRendererImplementation {
inline void countRenderCopy(SDL_Texture* texture) { }
} // namespace DuneRendererImplementation

#endif // _DEBUG

// SDL3: SDL_RenderCopyEx -> SDL_RenderTextureRotated, SDL_RendererFlip -> SDL_FlipMode
inline bool
Dune_RenderCopyEx(SDL_Renderer* renderer, SDL_Texture* texture, const SDL_Rect* srcrect, const SDL_Rect* dstrect,
                  const double angle, const SDL_Point* center, const SDL_FlipMode flip) {
    DuneRendererImplementation::countRenderCopy(texture);

    const SDL_FRect* dstf = nullptr;
    SDL_FRect dst_frect;
    if (dstrect) {
        dst_frect = {static_cast<float>(dstrect->x),
                     static_cast<float>(dstrect->y),
                     static_cast<float>(dstrect->w),
                     static_cast<float>(dstrect->h)};
        dstf      = &dst_frect;
    }
    const SDL_FRect* srcf = nullptr;
    SDL_FRect src_frect;
    if (srcrect) {
        src_frect = {static_cast<float>(srcrect->x),
                     static_cast<float>(srcrect->y),
                     static_cast<float>(srcrect->w),
                     static_cast<float>(srcrect->h)};
        srcf      = &src_frect;
    }
    const SDL_FPoint* centerf = nullptr;
    SDL_FPoint center_fpoint;
    if (center) {
        center_fpoint = {static_cast<float>(center->x), static_cast<float>(center->y)};
        centerf       = &center_fpoint;
    }
    return SDL_RenderTextureRotated(renderer, texture, srcf, dstf, angle, centerf, flip);
}

inline bool
Dune_RenderCopyExF(SDL_Renderer* renderer, SDL_Texture* texture, const SDL_Rect* srcrect, const SDL_FRect* dstrect,
                   const double angle, const SDL_FPoint* center, const SDL_FlipMode flip) {
    DuneRendererImplementation::countRenderCopy(texture);

    const SDL_FRect* srcf = nullptr;
    SDL_FRect src_frect;
    if (srcrect) {
        src_frect = {static_cast<float>(srcrect->x),
                     static_cast<float>(srcrect->y),
                     static_cast<float>(srcrect->w),
                     static_cast<float>(srcrect->h)};
        srcf      = &src_frect;
    }
    return SDL_RenderTextureRotated(renderer, texture, srcf, dstrect, angle, center, flip);
}

int Dune_RenderCopyEx(SDL_Renderer* renderer, const DuneTexture* texture, const SDL_Rect* srcrect,
                      const SDL_Rect* dstrect, double angle, const SDL_Point* center, const SDL_FlipMode flip);

int Dune_RenderCopyExF(SDL_Renderer* renderer, const DuneTexture* texture, const SDL_Rect* srcrect,
                       const SDL_FRect* dstrect, const double angle, const SDL_FPoint* center, const SDL_FlipMode flip);

void Dune_RenderCopy(SDL_Renderer* renderer, const DuneTexture* texture, const SDL_Rect* srcrect,
                     const SDL_Rect* dstrect);

void Dune_RenderCopyF(SDL_Renderer* renderer, const DuneTexture* texture, const SDL_Rect* srcrect,
                      const SDL_FRect* dstrect);

void Dune_RenderCopy(SDL_Renderer* renderer, SDL_Texture* texture, int x, int y);

// SDL3: SDL_RenderCopy -> SDL_RenderTexture
inline void
Dune_RenderCopy(SDL_Renderer* renderer, SDL_Texture* texture, const SDL_Rect* srcrect, const SDL_Rect* dstrect) {
    DuneRendererImplementation::countRenderCopy(texture);

    const SDL_FRect* dstf = nullptr;
    SDL_FRect dst_frect;
    if (dstrect) {
        dst_frect = {static_cast<float>(dstrect->x),
                     static_cast<float>(dstrect->y),
                     static_cast<float>(dstrect->w),
                     static_cast<float>(dstrect->h)};
        dstf      = &dst_frect;
    }
    const SDL_FRect* srcf = nullptr;
    SDL_FRect src_frect;
    if (srcrect) {
        src_frect = {static_cast<float>(srcrect->x),
                     static_cast<float>(srcrect->y),
                     static_cast<float>(srcrect->w),
                     static_cast<float>(srcrect->h)};
        srcf      = &src_frect;
    }
    SDL_RenderTexture(renderer, texture, srcf, dstf);
}

inline void
Dune_RenderCopyF(SDL_Renderer* renderer, SDL_Texture* texture, const SDL_Rect* srcrect, const SDL_FRect* dstrect) {
    DuneRendererImplementation::countRenderCopy(texture);

    const SDL_FRect* srcf = nullptr;
    SDL_FRect src_frect;
    if (srcrect) {
        src_frect = {static_cast<float>(srcrect->x),
                     static_cast<float>(srcrect->y),
                     static_cast<float>(srcrect->w),
                     static_cast<float>(srcrect->h)};
        srcf      = &src_frect;
    }
    SDL_RenderTexture(renderer, texture, srcf, dstrect);
}

inline void Dune_RenderPresent(SDL_Renderer* renderer) {
#if _DEBUG
    using namespace DuneRendererImplementation;

    if (render_dump)
        Dune_RenderDump();

    render_textures.clear();
    render_copies = 0;
    ++render_presents;
    render_texture_changes = 0;

    render_texture = nullptr;
#endif // _DEBUG

    SDL_RenderPresent(renderer);

    dune::destroy_textures();
}

void DuneDrawSelectionBox(SDL_Renderer* renderer, float x, float y, float w, float h, Uint32 color = COLOR_WHITE);

inline void DuneDrawSelectionBox(SDL_Renderer* renderer, const SDL_Rect& rect, Uint32 color = COLOR_WHITE) {
    DuneDrawSelectionBox(renderer, rect.x, rect.y, rect.w, rect.h, color);
}

inline void DuneDrawSelectionBox(SDL_Renderer* renderer, const SDL_FRect& rect, Uint32 color = COLOR_WHITE) {
    DuneDrawSelectionBox(renderer, rect.x, rect.y, rect.w, rect.h, color);
}

// SDL3: SDL_RenderDrawLinesF -> SDL_RenderLines
inline bool DuneDrawLines(SDL_Renderer* renderer, std::span<SDL_FPoint> points) {
    return SDL_RenderLines(renderer, points.data(), static_cast<int>(points.size()));
}

inline bool DuneDrawLines(SDL_Renderer* renderer, std::initializer_list<const SDL_FPoint> points) {
    return SDL_RenderLines(renderer, std::data(points), static_cast<int>(points.size()));
}

// SDL3: SDL_RenderDrawRectsF -> SDL_RenderRects
inline bool DuneDrawRects(SDL_Renderer* renderer, std::span<SDL_FRect> rects) {
    return SDL_RenderRects(renderer, rects.data(), static_cast<int>(rects.size()));
}

inline bool DuneDrawRects(SDL_Renderer* renderer, std::initializer_list<const SDL_FRect> rects) {
    return SDL_RenderRects(renderer, std::data(rects), static_cast<int>(rects.size()));
}

inline bool DuneFillRects(SDL_Renderer* renderer, std::span<SDL_FRect> rects) {
    return SDL_RenderFillRects(renderer, rects.data(), static_cast<int>(rects.size()));
}

inline bool DuneFillRects(SDL_Renderer* renderer, std::initializer_list<const SDL_FRect> rects) {
    return SDL_RenderFillRects(renderer, std::data(rects), static_cast<int>(rects.size()));
}

namespace dune {

class RenderClip final {
public:
    RenderClip(SDL_Renderer* renderer, const SDL_Rect& clip);

    ~RenderClip();

private:
    SDL_Rect old_clip{};
    bool was_clipping_;
    SDL_Renderer* renderer_;
};

} // namespace dune

#endif // DUNERENDERER_H
