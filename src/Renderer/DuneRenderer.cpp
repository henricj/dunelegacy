#include <Renderer/DuneRenderer.h>


void DuneDrawSelectionBox(SDL_Renderer* renderer, int x, int y, int w, int h, Uint32 color) {
    setRenderDrawColor(renderer, color);

    std::array<SDL_FPoint, 3> points;

    const auto to_pt = [](int u, int v) { return SDL_FPoint{static_cast<float>(u), static_cast<float>(v)}; };

    // now draw the box with parts at all corners
    for(auto i = 0; i <= currentZoomlevel; i++) {
        const auto offset = (currentZoomlevel + 1) * 3;

        // top left bit
        points[0] = to_pt(x + i, y + offset);
        points[1] = to_pt(x + i, y + i);
        points[2] = to_pt(x + offset, y + i);

        SDL_RenderDrawLinesF(renderer, points.data(), points.size());

        // top right bit
        points[0] = to_pt(x + w - 1 - i, y + offset);
        points[1] = to_pt(x + w - 1 - i, y + i);
        points[2] = to_pt(x + w - 1 - offset, y + i);

        SDL_RenderDrawLinesF(renderer, points.data(), points.size());

        // bottom left bit
        points[0] = to_pt(x + i, y + h - 1 - offset);
        points[1] = to_pt(x + i, y + h - i);
        points[2] = to_pt(x + offset, y + h - i);

        SDL_RenderDrawLinesF(renderer, points.data(), points.size());

        // bottom right bit
        points[0] = to_pt(x + w - 1 - offset, y + h - 1 - i);
        points[1] = to_pt(x + w - 1 - i, y + h - 1 - i);
        points[2] = to_pt(x + w - 1 - i, y + h - 1 - offset);

        SDL_RenderDrawLinesF(renderer, points.data(), points.size());
    }
}

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
