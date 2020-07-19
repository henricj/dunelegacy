#ifndef DUNERENDERER_H
#define DUNERENDERER_H

#include "DuneTextures.h"

#if _DEBUG
namespace DuneRendererImplementation
{
extern SDL_Texture* render_texture;
extern int          render_texture_changes;
extern int          render_copies;
extern int          render_presents;
extern bool         render_dump;
extern std::map<SDL_Texture*, int> render_textures;

inline void countRenderCopy(SDL_Texture* texture)
{
    if(render_texture != texture) {
        render_texture = texture;
        ++render_texture_changes;
    }
    ++render_copies;
    ++render_textures[texture];
}
} // namespace DuneRendererImplementation

void Dune_RenderDump();
#else // DEBUG
namespace DuneRendererImplementation
{
inline void countRenderCopy(SDL_Texture* texture)
{ }
}

#endif // _DEBUG

inline int Dune_RenderCopyEx(SDL_Renderer* renderer, SDL_Texture* texture, const SDL_Rect* srcrect,
                             const SDL_Rect* dstrect, const double angle, const SDL_Point* center,
                             const SDL_RendererFlip flip) {
    DuneRendererImplementation::countRenderCopy(texture);
 
    return SDL_RenderCopyEx(renderer, texture, srcrect, dstrect, angle, center, flip);
}

inline int Dune_RenderCopyEx(SDL_Renderer* renderer, const DuneTexture* texture, const SDL_Rect* srcrect,
                             const SDL_Rect* dstrect, const double angle, const SDL_Point* center,
                             const SDL_RendererFlip flip) {
    assert(texture && texture->texture_);
    assert(texture->source_.x >= 0 && texture->source_.y >= 0 && texture->source_.w > 0 && texture->source_.h > 0);

    DuneRendererImplementation::countRenderCopy(texture->texture_);

    if(srcrect) {
        assert(srcrect->x >= 0 && srcrect->y >= 0 && srcrect->w > 0 && srcrect->h > 0);
        assert(srcrect->x + srcrect->w <= texture->source_.w);
        assert(srcrect->y + srcrect->h <= texture->source_.h);

        SDL_Rect offset{texture->source_.x + srcrect->x, texture->source_.y + srcrect->y, srcrect->w, srcrect->h};

        return SDL_RenderCopyEx(renderer, texture->texture_, &offset, dstrect, angle, center, flip);
    } else
        return SDL_RenderCopyEx(renderer, texture->texture_, &texture->source_, dstrect, angle, center, flip);
}

inline int Dune_RenderCopy(SDL_Renderer* renderer, const DuneTexture* texture, const SDL_Rect* srcrect,
                           const SDL_Rect* dstrect) {
    assert(texture && texture->texture_);
    assert(texture->source_.x >= 0 && texture->source_.y >= 0 && texture->source_.w > 0 && texture->source_.h > 0);

    DuneRendererImplementation::countRenderCopy(texture->texture_);

    if(srcrect) {
        assert(srcrect->x >= 0 && srcrect->y >= 0 && srcrect->w > 0 && srcrect->h > 0);
        assert(srcrect->x + srcrect->w <= texture->source_.w);
        assert(srcrect->y + srcrect->h <= texture->source_.h);

        SDL_Rect offset{texture->source_.x + srcrect->x, texture->source_.y + srcrect->y, srcrect->w, srcrect->h};

        return SDL_RenderCopy(renderer, texture->texture_, &offset, dstrect);
    } else
        return SDL_RenderCopy(renderer, texture->texture_, &texture->source_, dstrect);
}

inline int Dune_RenderCopy(SDL_Renderer* renderer, SDL_Texture* texture, const SDL_Rect* srcrect,
                           const SDL_Rect* dstrect) {
    DuneRendererImplementation::countRenderCopy(texture);

    return SDL_RenderCopy(renderer, texture, srcrect, dstrect);
}

inline void Dune_RenderPresent(SDL_Renderer* renderer) {
#if _DEBUG
    using namespace DuneRendererImplementation;

    if(render_dump) Dune_RenderDump();

    render_textures.clear();
    render_copies = 0;
    ++render_presents;
    render_texture_changes = 0;

    render_texture = nullptr;
#endif // _DEBUG

    SDL_RenderPresent(renderer);
}

#endif // DUNERENDERER_H
