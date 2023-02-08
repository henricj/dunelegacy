/*
 *  This file is part of Dune Legacy.
 *
 *  Dune Legacy is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  Dune Legacy is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with Dune Legacy.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef PALETTE_H
#define PALETTE_H

#include <misc/SDL2pp.h>

class Palette final {
public:
    Palette();

    explicit Palette(int numPaletteEntries);

    explicit Palette(const SDL_Palette* pSDLPalette);

    Palette(const Palette& palette);

    ~Palette();

    Palette& operator=(const Palette& palette);

    SDL_Color& operator[](const int i);

    SDL_Color operator[](const int i) const;

    [[nodiscard]] SDL_Palette* getSDLPalette() const;

    void setSDLPalette(const SDL_Palette* pSDLPalette);

    [[nodiscard]] int getNumColors() const;

    void applyToSurface(SDL_Surface* pSurface, int firstColor = 0, int endColor = -1) const;

private:
    sdl2::palette_ptr pSDLPalette_;
};

#endif // PALETTE_H
