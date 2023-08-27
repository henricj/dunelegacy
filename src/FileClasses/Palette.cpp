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

#include <FileClasses/Palette.h>

#include <misc/exceptions.h>

#include <exception>
#include <utility>

Palette::Palette() = default;

Palette::Palette(int numPaletteEntries) {
    pSDLPalette_ = sdl2::palette_ptr{SDL_AllocPalette(numPaletteEntries)};
    if (!pSDLPalette_) {
        throw;
    }
}

Palette::Palette(const SDL_Palette* pSDLPalette) {
    setSDLPalette(pSDLPalette);
}

Palette::Palette(sdl2::palette_ptr&& palette) {
    pSDLPalette_ = std::move(palette);
}

Palette::Palette(const Palette& palette) {
    *this = palette;
}

Palette::Palette(Palette&& palette) noexcept = default;

Palette::~Palette() = default;

Palette& Palette::operator=(const Palette& palette) {
    if (this == &palette) {
        return *this;
    }

    this->setSDLPalette(palette.pSDLPalette_.get());

    return *this;
}

Palette& Palette::operator=(Palette&& palette) noexcept = default;

SDL_Color& Palette::operator[](const int i) {
    if (pSDLPalette_ == nullptr || i < 0 || i >= pSDLPalette_->ncolors) {
        THROW(std::runtime_error, "Palette::operator[]: Invalid index!");
    }

    return pSDLPalette_->colors[i];
}

SDL_Color Palette::operator[](const int i) const {
    if (pSDLPalette_ == nullptr || i < 0 || i >= pSDLPalette_->ncolors) {
        THROW(std::runtime_error, "Palette::operator[]: Invalid index!");
    }

    return pSDLPalette_->colors[i];
}

SDL_Palette* Palette::getSDLPalette() const {
    return pSDLPalette_.get();
}

void Palette::setSDLPalette(const SDL_Palette* pSDLPalette) {
    if (!pSDLPalette) {
        this->pSDLPalette_.reset();
        return;
    }
    auto pNewSDLPalette = sdl2::palette_ptr{SDL_AllocPalette(pSDLPalette->ncolors)};
    memcpy(pNewSDLPalette->colors, pSDLPalette->colors, pSDLPalette->ncolors * sizeof(SDL_Color));

    this->pSDLPalette_ = std::move(pNewSDLPalette);
}

int Palette::getNumColors() const {
    if (pSDLPalette_ == nullptr) {
        return 0;
    }

    return pSDLPalette_->ncolors;
}

void Palette::applyToSurface(SDL_Surface* pSurface, int firstColor, int endColor) const {
    const auto hasColorKey = SDL_HasColorKey(pSurface);

    if (pSDLPalette_ == nullptr) {
        THROW(std::runtime_error, "Palette::applyToSurface(): Palette not initialized yet!");
    }

    if (pSurface == nullptr) {
        THROW(std::runtime_error, "Palette::applyToSurface(): pSurface == nullptr!");
    }

    if (pSurface->format->palette == nullptr) {
        THROW(std::runtime_error, "Palette::applyToSurface(): Cannot apply palette to surface without a palette!");
    }

    const auto nColors = (endColor != -1) ? (endColor - firstColor + 1) : (pSDLPalette_->ncolors - firstColor);
    SDL_SetPaletteColors(pSurface->format->palette, pSDLPalette_->colors + firstColor, firstColor, nColors);

    if (hasColorKey) {
        Uint32 colorKey{};
        if (SDL_GetColorKey(pSurface, &colorKey)) {
            THROW(std::runtime_error, "Palette::applyToSurface(): Unable to get color key!");
        }

        if (SDL_SetColorKey(pSurface, SDL_TRUE, colorKey)) {
            THROW(std::runtime_error, "Palette::applyToSurface(): Unable to set color key!");
        }
    }
}
