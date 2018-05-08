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

#ifndef PALFILE_H
#define PALFILE_H

#include <FileClasses/Palette.h>
#include <misc/SDL2pp.h>

/// A class for reading palettes out of PAL-Files.
/**
    This method can be used to read PAL-Files. PAL-Files are palette files used by Dune2. The read palette is returned.
    \param  rwop    SDL_RWops to the PAL-File. (can be readonly)
    \return The Palette in this PAL-File
*/
Palette LoadPalette_RW(SDL_RWops* rwop);

#endif // PALFILE_H
