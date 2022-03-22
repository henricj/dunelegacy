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

#include <misc/exceptions.h>
#include <misc/SDL2pp.h>

#include <string.h>

class Palette
{
    public:
        Palette() {

        }

        explicit Palette(int numPaletteEntries) {
            pSDLPalette = sdl2::palette_ptr{ SDL_AllocPalette(numPaletteEntries) };
            if (!pSDLPalette) {
                throw;
            }
        }

        explicit Palette(const SDL_Palette* pSDLPalette) {
            setSDLPalette(pSDLPalette);
        }

        Palette(const Palette& palette) {
            *this = palette;
        }

        virtual ~Palette() = default;

        Palette& operator=(const Palette& palette) {
            if(this == &palette) {
                return *this;
            }

            this->setSDLPalette(palette.pSDLPalette.get());

            return *this;
        }

        SDL_Color& operator[](const int i) {
            if(pSDLPalette == nullptr || i < 0 || i >= pSDLPalette->ncolors) {
                THROW(std::runtime_error, "Palette::operator[]: Invalid index!");
            }

            return pSDLPalette->colors[i];
        }

        const SDL_Color& operator[](const int i) const {
            if(pSDLPalette == nullptr || i < 0 || i >= pSDLPalette->ncolors) {
                THROW(std::runtime_error, "Palette::operator[]: Invalid index!");
            }

            return pSDLPalette->colors[i];
        }

        [[nodiscard]] SDL_Palette* getSDLPalette() const {
            return pSDLPalette.get();
        }

        void setSDLPalette(const SDL_Palette* pSDLPalette) {
            if(!pSDLPalette) {
                this->pSDLPalette.reset();
                return;
            }
            auto pNewSDLPalette = sdl2::palette_ptr{ SDL_AllocPalette(pSDLPalette->ncolors) };
            memcpy(pNewSDLPalette->colors, pSDLPalette->colors, pSDLPalette->ncolors * sizeof(SDL_Color));

            this->pSDLPalette = std::move(pNewSDLPalette);
        }

        [[nodiscard]] int getNumColors() const {
            if(pSDLPalette == nullptr) {
                return 0;
            }

            return pSDLPalette->ncolors;
        }

        void applyToSurface(SDL_Surface* pSurface, int firstColor = 0, int endColor = -1) const {
            const auto hasColorKey = SDL_HasColorKey(pSurface);

            if(pSDLPalette == nullptr) {
                THROW(std::runtime_error, "Palette::applyToSurface(): Palette not initialized yet!");
            }

            if(pSurface == nullptr) {
                THROW(std::runtime_error, "Palette::applyToSurface(): pSurface == nullptr!");
            }

            if(pSurface->format->palette == nullptr) {
                THROW(std::runtime_error, "Palette::applyToSurface(): Cannot apply palette to surface without a palette!");
            }

            const auto nColors = (endColor != -1) ? (endColor - firstColor + 1) : (pSDLPalette->ncolors - firstColor);
            SDL_SetPaletteColors(pSurface->format->palette, pSDLPalette->colors + firstColor, firstColor, nColors);

            if(hasColorKey) {
                Uint32 colorKey = 0;
                if(SDL_GetColorKey(pSurface, &colorKey)) {
                    THROW(std::runtime_error, "Palette::applyToSurface(): Unable to get color key!");
                }

                if (SDL_SetColorKey(pSurface, SDL_TRUE, colorKey)) {
                    THROW(std::runtime_error, "Palette::applyToSurface(): Unable to set color key!");
                }
            }
        }

    private:
        sdl2::palette_ptr pSDLPalette;
};

#endif // PALETTE_H
