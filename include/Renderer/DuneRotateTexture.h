#ifndef DUNEROTATETEXTURE_H
#define DUNEROTATETEXTURE_H

#include <fixmath/FixPoint.h>

#include "DuneTileTexture.h"

class DuneRotateTexture final {
public:
    explicit DuneRotateTexture(const DuneTileTexture& tileTexture) : tile_texture_{tileTexture} { }
    explicit DuneRotateTexture(DuneTileTexture&& tileTexture) : tile_texture_{std::move(tileTexture)} { }

    void draw(SDL_Renderer* renderer, int x, int y, FixPoint angle) const noexcept;

private:
    DuneTileTexture tile_texture_;
};

#endif // DUNEROTATETEXTURE_H
