#include <Renderer/DuneTileTexture.h>

#include <SDL2/SDL_render.h>

#include <span>
#include <stdexcept>
#include <utility>

DuneTileTexture::DuneTileTexture(SDL_Texture* texture, float tile_width, float tile_height, int rows, int columns,
                                 std::span<const SDL_Rect> tiles)
    : texture_{texture}, columns_{columns}, tile_width_(tile_width), tile_height_(tile_height) {
    if (rows < 1)
        THROW(std::invalid_argument, "The rows argument is out of range (%d)", rows);

    if (columns < 1)
        THROW(std::invalid_argument, "The columns argument is out of range (%d)", columns);

    if (std::cmp_greater(tiles.size(), std::numeric_limits<int>::max())
        || static_cast<int>(tiles.size()) != rows * columns)
        THROW(std::invalid_argument, "The size of the tiles does not match the rows and columns (%d != %dx%d)",
              tiles.size(), columns, rows);

#if _DEBUG
    int w = 0, h = 0;
    if (SDL_QueryTexture(texture, nullptr, nullptr, &w, &h))
        THROW(std::invalid_argument, "Unable to query texture: %s", SDL_GetError());

    for (const auto& tile : tiles) {
        if (tile.x + tile.w > w || tile.y + tile.h > h)
            THROW(std::invalid_argument, "The tile (%dx%d at %dx%d) must be inside the texture (%dx%d)", tile.w, tile.h,
                  tile.x, tile.x, w, h);
    }
#endif

    source_.reserve(tiles.size());
    std::ranges::transform(tiles, std::back_inserter(source_), [](const auto& tile) { return DuneTextureRect{tile}; });
}

void DuneTileTexture::draw(SDL_Renderer* renderer, float x, float y, int column, int row) const noexcept {
    const auto src = source_rect(column, row);
    const SDL_FRect dst{x, y, tile_width_, tile_height_};

    if (SDL_RenderCopyF(renderer, texture_, &src, &dst))
        sdl2::log_error("DuneTileTexture::draw() SDL_RenderCopyF failed: %s", SDL_GetError());
}

void DuneTileTexture::draw(SDL_Renderer* renderer, float x, float y, double angle, int column, int row) const noexcept {
    const auto src = source_rect(column, row);
    const SDL_FRect dst{x, y, tile_width_, tile_height_};

    if (SDL_RenderCopyExF(renderer, texture_, &src, &dst, angle, nullptr, SDL_RendererFlip::SDL_FLIP_NONE))
        sdl2::log_error("DuneTileTexture::draw() SDL_RenderCopyExF failed: %s", SDL_GetError());
}
