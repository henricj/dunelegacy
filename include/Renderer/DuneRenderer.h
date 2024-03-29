#ifndef DUNERENDERER_H
#define DUNERENDERER_H

#include "Colors.h"
#include "DuneTexture.h"

#include <SDL2/SDL.h>

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

inline int
Dune_RenderCopyEx(SDL_Renderer* renderer, SDL_Texture* texture, const SDL_Rect* srcrect, const SDL_Rect* dstrect,
                  const double angle, const SDL_Point* center, const SDL_RendererFlip flip) {
    DuneRendererImplementation::countRenderCopy(texture);

    return SDL_RenderCopyEx(renderer, texture, srcrect, dstrect, angle, center, flip);
}

inline int
Dune_RenderCopyExF(SDL_Renderer* renderer, SDL_Texture* texture, const SDL_Rect* srcrect, const SDL_FRect* dstrect,
                   const double angle, const SDL_FPoint* center, const SDL_RendererFlip flip) {
    DuneRendererImplementation::countRenderCopy(texture);

    return SDL_RenderCopyExF(renderer, texture, srcrect, dstrect, angle, center, flip);
}

int Dune_RenderCopyEx(SDL_Renderer* renderer, const DuneTexture* texture, const SDL_Rect* srcrect,
                      const SDL_Rect* dstrect, double angle, const SDL_Point* center, const SDL_RendererFlip flip);

int Dune_RenderCopyExF(SDL_Renderer* renderer, const DuneTexture* texture, const SDL_Rect* srcrect,
                       const SDL_FRect* dstrect, const double angle, const SDL_FPoint* center,
                       const SDL_RendererFlip flip);

void Dune_RenderCopy(SDL_Renderer* renderer, const DuneTexture* texture, const SDL_Rect* srcrect,
                     const SDL_Rect* dstrect);

void Dune_RenderCopyF(SDL_Renderer* renderer, const DuneTexture* texture, const SDL_Rect* srcrect,
                      const SDL_FRect* dstrect);

void Dune_RenderCopy(SDL_Renderer* renderer, SDL_Texture* texture, int x, int y);

inline void
Dune_RenderCopy(SDL_Renderer* renderer, SDL_Texture* texture, const SDL_Rect* srcrect, const SDL_Rect* dstrect) {
    DuneRendererImplementation::countRenderCopy(texture);

    SDL_RenderCopy(renderer, texture, srcrect, dstrect);
}

inline void
Dune_RenderCopyF(SDL_Renderer* renderer, SDL_Texture* texture, const SDL_Rect* srcrect, const SDL_FRect* dstrect) {
    DuneRendererImplementation::countRenderCopy(texture);

    SDL_RenderCopyF(renderer, texture, srcrect, dstrect);
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

inline int DuneDrawLines(SDL_Renderer* renderer, std::span<SDL_FPoint> points) {
    return SDL_RenderDrawLinesF(renderer, points.data(), points.size());
}

inline int DuneDrawLines(SDL_Renderer* renderer, std::initializer_list<const SDL_FPoint> points) {
    return SDL_RenderDrawLinesF(renderer, std::data(points), points.size());
}

inline int DuneDrawRects(SDL_Renderer* renderer, std::span<SDL_FRect> rects) {
    return SDL_RenderDrawRectsF(renderer, rects.data(), rects.size());
}

inline int DuneDrawRects(SDL_Renderer* renderer, std::initializer_list<const SDL_FRect> rects) {
    return SDL_RenderDrawRectsF(renderer, std::data(rects), rects.size());
}

inline int DuneFillRects(SDL_Renderer* renderer, std::span<SDL_FRect> rects) {
    return SDL_RenderFillRectsF(renderer, rects.data(), rects.size());
}

inline int DuneFillRects(SDL_Renderer* renderer, std::initializer_list<const SDL_FRect> rects) {
    return SDL_RenderFillRectsF(renderer, std::data(rects), rects.size());
}

namespace dune {

class RenderClip final {
public:
    RenderClip(SDL_Renderer* renderer, const SDL_Rect& clip);

    ~RenderClip();

private:
    SDL_Rect old_clip{};
    SDL_bool was_clipping_;
    SDL_Renderer* renderer_;
};

} // namespace dune

#endif // DUNERENDERER_H
