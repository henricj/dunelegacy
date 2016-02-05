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

#include <SDL.h>
#include <string.h>
#include <stdexcept>

class Palette
{
    public:
        Palette() : pSDLPalette(NULL) {

        }

        Palette(int numPaletteEntries) : pSDLPalette(NULL) {
            pSDLPalette = SDL_AllocPalette(numPaletteEntries);
            if (!pSDLPalette) {
                throw;
            }
        }

        Palette(const SDL_Palette* pSDLPalette) : pSDLPalette(NULL) {
            setSDLPalette(pSDLPalette);
        }

        Palette(const Palette& palette) : pSDLPalette(NULL) {
            *this = palette;
        }

        virtual ~Palette() {
            deleteSDLPalette();
        }

        Palette& operator=(const Palette& palette) {
            if(this == &palette) {
                return *this;
            }

            this->setSDLPalette(palette.pSDLPalette);

            return *this;
        }

        inline SDL_Color& operator[](const int i) {
            if(pSDLPalette == NULL || i < 0 || i >= pSDLPalette->ncolors) {
                throw std::runtime_error("Palette::operator[]: Invalid index!");
            }

            return pSDLPalette->colors[i];
        }

        inline const SDL_Color& operator[](const int i) const {
            if(pSDLPalette == NULL || i < 0 || i >= pSDLPalette->ncolors) {
                throw std::runtime_error("Palette::operator[]: Invalid index!");
            }

            return pSDLPalette->colors[i];
        }

        inline SDL_Palette* getSDLPalette() const {
            return pSDLPalette;
        }

        void setSDLPalette(const SDL_Palette* pSDLPalette) {
            if(!pSDLPalette) {
                this->pSDLPalette = NULL;
                return;
            }
            SDL_Palette* pNewSDLPalette = SDL_AllocPalette(pSDLPalette->ncolors);
            if(!pNewSDLPalette) {
                throw;
            }
            memcpy(pNewSDLPalette->colors, pSDLPalette->colors, pSDLPalette->ncolors * sizeof(SDL_Color));

            deleteSDLPalette();
            this->pSDLPalette = pNewSDLPalette;
        }

        inline int getNumColors() const {
            if(pSDLPalette == NULL) {
                return 0;
            }

            return pSDLPalette->ncolors;
        }

        void applyToSurface(SDL_Surface* pSurface, int firstColor = 0, int endColor = -1) const {
            if(pSDLPalette == NULL) {
                throw std::runtime_error("Palette::applyToSurface(): Palette not initialized yet!");
            }

            if(pSurface == NULL) {
                throw std::runtime_error("Palette::applyToSurface(): pSurface == NULL!");
            }

            if(pSurface->format->palette == NULL) {
                throw std::runtime_error("Palette::applyToSurface(): Cannot apply palette to surface without a palette!");
            }

            int nColors = (endColor != -1) ? (endColor - firstColor + 1) : (pSDLPalette->ncolors - firstColor);
            SDL_SetPaletteColors(pSurface->format->palette, pSDLPalette->colors + firstColor, firstColor, nColors);
        }

    private:

        void deleteSDLPalette() {
            if (pSDLPalette != NULL) {
                SDL_FreePalette(pSDLPalette);
            }
            pSDLPalette = NULL;
        }

        SDL_Palette* pSDLPalette;
};

#endif // PALETTE_H
