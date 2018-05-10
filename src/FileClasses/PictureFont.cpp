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

#include <FileClasses/PictureFont.h>
#include <misc/exceptions.h>

/// Constructor
/**
    The constructor reads from the surface all data and saves them internally. Immediately after the PictureFont-Object is
    constructed pic can be freed. All data is saved in the class.
    \param  pic The picture which contains the font
*/
PictureFont::PictureFont(SDL_Surface* pic)
{
    if(pic == nullptr) {
        THROW(std::invalid_argument, "PictureFont::PictureFont(): pic == nullptr!");
    }

    sdl2::surface_lock lock{ pic };

    characterHeight = pic->h - 2;

    int curXPos = 1;
    int oldXPos = curXPos;
    char* secondLine = ((char*) pic->pixels) + pic->pitch;
    for(int i = 0; i < 256; i++) {
        while((curXPos < pic->w) && (*(secondLine + curXPos) != 14)) {
            curXPos++;
        }

        if(curXPos >= pic->w) {
            THROW(std::runtime_error, "PictureFont::PictureFont(): No valid surface for loading font!");
        }

        character[i].width = curXPos - oldXPos;
        character[i].data.resize(character[i].width * characterHeight);

        int mempos = 0;
        for(int y = 1; y < pic->h - 1; y++) {
            for(int x = oldXPos; x < curXPos; x++) {
                unsigned char col = *(((unsigned char*) pic->pixels) + y*pic->pitch + x);
                if(col != 0) {
                    col = 1;
                }

                character[i].data[mempos] = col;
                mempos++;
            }
        }
        curXPos++;
        oldXPos = curXPos;
    }
}

/// Destructor
/**
    Frees all memory.
*/
PictureFont::~PictureFont() = default;


void PictureFont::drawTextOnSurface(SDL_Surface* pSurface, const std::string& text, Uint32 baseColor) {
    SDL_LockSurface(pSurface);

    int bpp = pSurface->format->BytesPerPixel;

    int curXPos = 0;
    const unsigned char* pText = (unsigned char*) text.c_str();
    while(*pText != '\0') {
        int index = *pText;

        //Now we can copy pixel by pixel
        for(int y = 0; y < characterHeight; y++) {
            for(int x = 0; x < character[index].width; x++) {
                char color = character[index].data[y*character[index].width+x];
                if(color != 0) {
                    Uint8 *pixel = (Uint8 *)pSurface->pixels + y * pSurface->pitch + (x+curXPos) * bpp;

                    switch(bpp) {
                        case 1:
                            *pixel = baseColor;
                            break;

                        case 2:
                            *(Uint16 *)pixel = baseColor;
                            break;

                        case 3:
                            if(SDL_BYTEORDER == SDL_BIG_ENDIAN) {
                                pixel[0] = (baseColor>> 16) & 0xff;
                                pixel[1] = (baseColor>> 8) & 0xff;
                                pixel[2] = baseColor& 0xff;
                            } else {
                                pixel[0] = baseColor& 0xff;
                                pixel[1] = (baseColor>> 8) & 0xff;
                                pixel[2] = (baseColor>> 16) & 0xff;
                            }
                            break;

                        case 4:
                            *(Uint32 *)pixel = baseColor;
                            break;
                    }
                }

            }
        }

        curXPos += character[index].width;
        pText++;
    }


    SDL_UnlockSurface(pSurface);
}

/// Returns the number of pixels a text needs
/**
        This methods returns the number of pixels this text would need if printed.
        \param  text    The text to be checked for it's length in pixel
        \return Number of pixels needed
*/
int PictureFont::getTextWidth(const std::string& text) const {
    int width = 0;
    const unsigned char* pText = (unsigned char*) text.c_str();
    while(*pText != '\0') {
        width += character[*pText].width;
        pText++;
    }

    return width;
}

