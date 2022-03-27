#ifndef DUNETILETEXTURE_H
#define DUNETILETEXTURE_H

#include <gsl/gsl>

class DuneTileTexture final {
public:
    DuneTileTexture()                       = default;
    DuneTileTexture(const DuneTileTexture&) = default;
    DuneTileTexture(DuneTileTexture&&)      = default;

    DuneTileTexture(SDL_Texture* texture, int rows, int columns, gsl::span<const SDL_Rect> tiles);

    ~DuneTileTexture() = default;

    DuneTileTexture& operator=(const DuneTileTexture&) = default;
    DuneTileTexture& operator=(DuneTileTexture&&) = default;

    operator bool() const noexcept { return nullptr != texture_; }

    void draw(SDL_Renderer* renderer, int x, int y, int column, int row = 0) const noexcept;
    void draw(SDL_Renderer* renderer, int x, int y, double angle, int column, int row = 0) const noexcept;

private:
    [[nodiscard]] SDL_Rect source_rect(int column, int row = 0) const {
#if _DEBUG
        if (column < 0 || column >= columns_)
            THROW(std::invalid_argument, "Column out of range (%d)!", column);
        if (row < 0 || row * columns_ > source_.size())
            THROW(std::invalid_argument, "Row out of range (%d)!", column);
#endif
        return source_.at(column + row * columns_).as_sdl();
    }

    SDL_Texture* texture_{};
    int columns_;
    std::vector<DuneTextureRect> source_;
};

#endif // DUNETILETEXTURE_H
