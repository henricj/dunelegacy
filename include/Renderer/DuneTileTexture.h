#ifndef DUNETILETEXTURE_H
#define DUNETILETEXTURE_H

#include "DuneTexture.h"
#include "misc/exceptions.h"

#include <SDL2/SDL_rect.h>
#include <SDL2/SDL_render.h>

#include <cstddef>
#include <span>
#include <vector>

class DuneTileTexture final {
public:
    DuneTileTexture()                       = default;
    DuneTileTexture(const DuneTileTexture&) = default;
    DuneTileTexture(DuneTileTexture&&)      = default;

    DuneTileTexture(SDL_Texture* texture, float tile_width, float tile_height, int rows, int columns,
                    std::span<const SDL_Rect> tiles);

    ~DuneTileTexture() = default;

    DuneTileTexture& operator=(const DuneTileTexture&) = default;
    DuneTileTexture& operator=(DuneTileTexture&&)      = default;

    operator bool() const noexcept { return nullptr != texture_; }

    void draw(SDL_Renderer* renderer, float x, float y, int column, int row = 0) const noexcept;
    void draw(SDL_Renderer* renderer, float x, float y, double angle, int column, int row = 0) const noexcept;

private:
    [[nodiscard]] SDL_Rect source_rect(int column, int row = 0) const {
#if _DEBUG
        if (column < 0 || column >= columns_)
            THROW(std::invalid_argument, "Column out of range ({})!", column);
        if (row < 0 || static_cast<size_t>(row) * columns_ > source_.size())
            THROW(std::invalid_argument, "Row out of range ({})!", column);
#endif
        return source_.at(column + static_cast<size_t>(row) * columns_).as_sdl();
    }

    SDL_Texture* texture_{};
    int columns_{};
    float tile_width_;
    float tile_height_;
    std::vector<DuneTextureRect> source_;
};

#endif // DUNETILETEXTURE_H
