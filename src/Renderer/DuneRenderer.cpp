#include <Renderer/DuneRenderer.h>

#if _DEBUG

void Dune_RenderDump() {
    using namespace DuneRendererImplementation;

    SDL_Log("present calls: %d, copy calls: %d, texture changes: %d", render_presents, render_copies,
            render_texture_changes);

    auto max_w = 0, max_h = 0;
    auto pixels = 0;

    for(const auto& it : render_textures) {

        int h, w;
        if(SDL_QueryTexture(it.first, nullptr, nullptr, &w, &h)) continue;

        SDL_Log("texture %x of size %dx%d rendered %d times", it.first, w, h, it.second);

        if(w > max_w) max_w = w;
        if(h > max_h) max_h = h;

        pixels += h * w;
    }

    const auto square = static_cast<int>(std::ceil(std::sqrt(pixels)));

    SDL_Log("%ld textures max_w=%d max_h=%d pixels=%d (%dx%d)", render_textures.size(), max_w, max_h, pixels, square);
}

namespace DuneRendererImplementation {
int  render_copies;
int  render_presents;
bool render_dump;

SDL_Texture* render_texture;
int          render_texture_changes;

std::map<SDL_Texture*, int> render_textures;
} // namespace DuneRendererImplementation

#endif // _DEBUG
