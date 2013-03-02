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

#include <stdexcept>

Palette LoadPalette_RW(SDL_RWops* rwop, int freesrc)
{
    if(rwop == NULL) {
        throw std::invalid_argument("Palfile::Palfile(): rwop == NULL!");
    }

	int filesize;
	if((filesize = SDL_RWseek(rwop,0,SEEK_END)) < 0) {
	    if(freesrc) SDL_RWclose(rwop);
		throw std::runtime_error("Palfile::Palfile(): SDL_RWseek failed!");
	}

	if(filesize % 3 != 0) {
        if(freesrc) SDL_RWclose(rwop);
		throw std::runtime_error("Palfile::Palfile(): Filesize must be multiple of 3!");
	}

	SDL_RWseek(rwop,0,SEEK_SET);

	Palette palette(filesize / 3);

	char buf;

    for (int i=0; i < palette.getNumColors(); i++)
    {
		if(SDL_RWread(rwop,&buf,1,1) != 1) {
            if(freesrc) SDL_RWclose(rwop);

			throw std::runtime_error("Palfile::Palfile(): SDL_RWread failed!");
		}
        palette[i].r = (char) (((double) buf)*255.0/63.0);

		if(SDL_RWread(rwop,&buf,1,1) != 1) {
            if(freesrc) SDL_RWclose(rwop);

			throw std::runtime_error("Palfile::Palfile(): SDL_RWread failed!");
		}
        palette[i].g = (char) (((double) buf)*255.0/63.0);

		if(SDL_RWread(rwop,&buf,1,1) != 1) {
            if(freesrc) SDL_RWclose(rwop);

			throw std::runtime_error("Palfile::Palfile(): SDL_RWread failed!");
		}
        palette[i].b = (char) (((double) buf)*255.0/63.0);
    }

    if(freesrc) SDL_RWclose(rwop);

    return palette;
}
