#include <Renderer/DuneRenderer.h>

void DuneDrawSelectionBox(SDL_Renderer* renderer, float x, float y, float w, float h, uint32_t color) {
    setRenderDrawColor(renderer, color);

    // now draw the box with parts at all corners
    for (auto i = 0; i <= currentZoomlevel; i++) {
        const auto offset = static_cast<float>(currentZoomlevel + 1) * 3.f;
        const auto fi     = static_cast<float>(i);

        // top left bit
        DuneDrawLines(renderer, {{x + fi, y + offset}, {x + fi, y + fi}, {x + offset, y + fi}});

        // top right bit
        DuneDrawLines(renderer, {{x + w - 1 - fi, y + offset}, {x + w - 1 - fi, y + fi}, {x + w - 1 - offset, y + fi}});

        // bottom left bit
        DuneDrawLines(renderer, {{x + fi, y + h - 1 - offset}, {x + fi, y + h - fi}, {x + offset, y + h - fi}});

        // bottom right bit
        DuneDrawLines(renderer, {{x + w - 1 - offset, y + h - 1 - fi},
                                 {x + w - 1 - fi, y + h - 1 - fi},
                                 {x + w - 1 - fi, y + h - 1 - offset}});
    }
}

#if _DEBUG

void Dune_RenderDump() {
    using namespace DuneRendererImplementation;

    sdl2::log_info("present calls: %d, copy calls: %d, texture changes: %d", render_presents, render_copies,
                   render_texture_changes);

    auto max_w = 0, max_h = 0;
    auto pixels = 0;

    for (const auto& it : render_textures) {

        int h, w;
        if (SDL_QueryTexture(it.first, nullptr, nullptr, &w, &h))
            continue;

        sdl2::log_info("texture %x of size %dx%d rendered %d times", reinterpret_cast<intptr_t>(it.first), w, h,
                       it.second);

        if (w > max_w)
            max_w = w;
        if (h > max_h)
            max_h = h;

        pixels += h * w;
    }

    const auto square = static_cast<int>(std::ceil(std::sqrt(pixels)));

    sdl2::log_info("%ld textures max_w=%d max_h=%d pixels=%d (%dx%d)", render_textures.size(), max_w, max_h, pixels,
                   square, square);
}

namespace DuneRendererImplementation {
int render_copies;
int render_presents;
bool render_dump;

SDL_Texture* render_texture;
int render_texture_changes;

std::map<SDL_Texture*, int> render_textures;
} // namespace DuneRendererImplementation

#endif // _DEBUG
