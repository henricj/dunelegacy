#include <Renderer/DuneTileTexture.h>

#include <SDL2/SDL_render.h>

#include <span>
#include <stdexcept>

DuneTileTexture::DuneTileTexture(SDL_Texture* texture, int rows, int columns, std::span<const SDL_Rect> tiles)
    : texture_{texture} {
    if (rows < 1)
        THROW(std::invalid_argument, "The rows argument is out of range (%d)", rows);

    if (columns < 1 || columns > std::numeric_limits<decltype(columns_)>::max())
        THROW(std::invalid_argument, "The columns argument is out of range (%d)", columns);

    if (tiles.size() > std::numeric_limits<int>::max() || static_cast<int>(tiles.size()) != rows * columns)
        THROW(std::invalid_argument, "The size of the tiles does not match the rows and columns (%d != %dx%d)",
              tiles.size(), columns, rows);

#if _DEBUG
    int w, h;
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
