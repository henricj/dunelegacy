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

#include <FileClasses/Palfile.h>
#include <misc/exceptions.h>

#include <stdio.h>

Palette LoadPalette_RW(SDL_RWops* rwop)
{
    if(rwop == nullptr) {
        THROW(std::invalid_argument, "Palfile::Palfile(): rwop == nullptr!");
    }

    Sint64 endOffset = SDL_RWsize(rwop);
    if(endOffset < 0) {
        THROW(std::runtime_error, "Palfile::Palfile(): Cannot determine size of this *.pal-File!");
    }

    size_t filesize = static_cast<size_t>(endOffset);

    if(filesize % 3 != 0) {
        THROW(std::runtime_error, "Palfile::Palfile(): Filesize must be multiple of 3!");
    }

    Palette palette(filesize / 3);

    unsigned char buf;

    for (int i=0; i < palette.getNumColors(); i++)
    {
        if(SDL_RWread(rwop,&buf,1,1) != 1) {
            THROW(std::runtime_error, "Palfile::Palfile(): SDL_RWread failed!");
        }
        palette[i].r = (char) (((double) buf)*255.0/63.0);

        if(SDL_RWread(rwop,&buf,1,1) != 1) {
            THROW(std::runtime_error, "Palfile::Palfile(): SDL_RWread failed!");
        }
        palette[i].g = (char) (((double) buf)*255.0/63.0);

        if(SDL_RWread(rwop,&buf,1,1) != 1) {
            THROW(std::runtime_error, "Palfile::Palfile(): SDL_RWread failed!");
        }
        palette[i].b = (char) (((double) buf)*255.0/63.0);
        palette[i].a = 0xFF;
    }

    return palette;
}
