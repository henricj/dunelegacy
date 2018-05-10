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

#include <FileClasses/Shpfile.h>
#include <FileClasses/Decode.h>
#include <FileClasses/Palette.h>
#include <misc/exceptions.h>

#include <Definitions.h>

#include <misc/SDL2pp.h>
#include <cstdlib>
#include <cstdio>

extern Palette palette;

/// Constructor
/**
    The constructor reads from the rwop all data and saves them internally. The SDL_RWops can be readonly but must support
    seeking.
    \param  rwop    SDL_RWops to the shp-File. (can be readonly)
*/
Shpfile::Shpfile(SDL_RWops* rwop)
{
    if(rwop == nullptr) {
        THROW(std::invalid_argument, "Shpfile::Shpfile(): rwop == nullptr!");
    }

    const Sint64 endOffset = SDL_RWsize(rwop);
    if(endOffset <= 0) {
        THROW(std::runtime_error, "Shpfile::Shpfile(): Cannot determine size of this *.shp-File!");
    }

    shpFilesize = static_cast<size_t>(endOffset);
    pFiledata = std::make_unique<unsigned char[]>(shpFilesize);

    if(SDL_RWread(rwop, pFiledata.get(), shpFilesize, 1) != 1) {
        THROW(std::runtime_error, "Shpfile::Shpfile(): Reading this *.shp-File failed!");
    }

    readIndex();
}

/// Destructor
/**
    Frees all memory.
*/
Shpfile::~Shpfile() = default;

/// Returns one picture in this shp-File
/**
    This method returns a SDL_Surface containing the nth picture in this shp-File.
    \param  indexOfFile specifies which picture to return (zero based)
    \return nth picture in this shp-File
*/
sdl2::surface_ptr Shpfile::getPicture(Uint32 indexOfFile)
{
    if(indexOfFile >= shpfileEntries.size()) {
        THROW(std::invalid_argument, "Shpfile::getPicture(): Requested index %ud is invalid for a shp file with %ud entries!", indexOfFile, shpfileEntries.size());
    }

    const unsigned char * Fileheader = pFiledata.get() + shpfileEntries[indexOfFile].startOffset;

    const unsigned char type = Fileheader[0];

    const unsigned char sizeY = Fileheader[2];
    const unsigned char sizeX = Fileheader[3];

    std::vector<unsigned char> DecodeDestination;

    /* size and also checksum */
    Uint16 size = SDL_SwapLE16(*reinterpret_cast<const Uint16 *>(Fileheader + 8));

    auto ImageOut = std::make_unique<unsigned char[]>(sizeX * sizeY);

    switch(type) {

        case 0:
        {
            DecodeDestination.clear();
            DecodeDestination.resize(size);

            if(decode80(Fileheader + 10, &DecodeDestination[0], size) == -1) {
                SDL_Log("Warning: Checksum-Error in Shp-File!");
            }

            shpCorrectLF(&DecodeDestination[0], ImageOut.get(), size);
        } break;

        case 1:
        {
            DecodeDestination.clear();
            DecodeDestination.resize(size);

            if(decode80(Fileheader + 10 + 16, &DecodeDestination[0], size) == -1) {
                SDL_Log("Warning: Checksum-Error in Shp-File!");
            }

            shpCorrectLF(&DecodeDestination[0], ImageOut.get(), size);

            applyPalOffsets(Fileheader + 10, ImageOut.get(), sizeX*sizeY);
        } break;

        case 2:
        {
            shpCorrectLF(Fileheader+10, ImageOut.get(), size);
        } break;

        case 3:
        {

            shpCorrectLF(Fileheader + 10 + 16, ImageOut.get(), size);

            applyPalOffsets(Fileheader + 10,ImageOut.get(), sizeX*sizeY);
        } break;

        default:
        {
            THROW(std::runtime_error, "Shpfile::getPicture(): Type %d in SHP-Files not supported!", type);
        }
    }

    // create new picture surface
    sdl2::surface_ptr pic{ SDL_CreateRGBSurface(0, sizeX, sizeY, 8, 0, 0, 0, 0) };
    if(pic == nullptr) {
        THROW(std::runtime_error, "Shpfile::getPicture(): Cannot create surface!");
    }

    palette.applyToSurface(pic.get());
    sdl2::surface_lock lock{ pic.get() };

    //Now we can copy line by line
    for(unsigned int y = 0u; y < sizeY; ++y) {
        memcpy( static_cast<char*>(pic->pixels) + y * pic->pitch , ImageOut.get() + y * sizeX, sizeX);
    }

    return pic;
}

/// Returns multiple pictures in this shp-File
/**
    This method returns a SDL_Surface containing an array of pictures from this shp-File.
    All pictures must be of the same size. tilesX/tilesY specifies how many pictures are in this row/column.
    Afterwards there must be tilesX*tilesY many parameters. Every parameter specifies which picture
    of this shp-File should be used. This indices must be ORed with a parameter specifing hwo they should
    be in the result surface. There are 4 modes and you must OR exactly one:
     - TILE_NORMAL  Normal
     - TILE_FLIPH   mirrored horizontally
     - TILE_FLIPV   mirrored vertically
     - TILE_ROTATE  Rotated by 180 degress

    Example:
    \code
    pPicture = myShpfile->getPictureArray(4,1, TILE_NORMAL | 20, TILE_FLIPH | 23, TILE_ROTATE | 67, TILE_NORMAL | 68);
    \endcode
    This example would create a surface with four pictures in it. From the left to the right there are
    picture 20,23,67 and 68. picture 23 is mirrored horizontally, 67 is rotated.<br><br>
    \param  tilesX  how many pictures in one row
    \param  tilesY  how many pictures in one column
    \param ...
    \return picture in this shp-File containing all specified pictures
*/
sdl2::surface_ptr Shpfile::getPictureArray(unsigned int tilesX, unsigned int tilesY, ...) {
    std::vector<Uint32> tiles;

    if((tilesX == 0) || (tilesY == 0)) {
        THROW(std::invalid_argument, "Shpfile::getPictureArray(): Number of requested image rows or columns must not be 0!");
    }

    tiles.resize(tilesX * tilesY);

    va_list arg_ptr;
    va_start(arg_ptr, tilesY);

    for(unsigned int i = 0u; i < tilesX*tilesY; i++) {
        tiles[i] = va_arg( arg_ptr, int );
        if(TILE_GETINDEX(tiles[i]) >= shpfileEntries.size()) {
            va_end(arg_ptr);
            THROW(std::invalid_argument, "Shpfile::getPictureArray(): Cannot read image %ud as there are only %ud images in this *.shp!", TILE_GETINDEX(tiles[i]), shpfileEntries.size());
        }
    }

    va_end(arg_ptr);

    unsigned char* pData = pFiledata.get();

    unsigned char sizeY = (pData + shpfileEntries[TILE_GETINDEX(tiles[0])].startOffset)[2];
    unsigned char sizeX = (pData + shpfileEntries[TILE_GETINDEX(tiles[0])].startOffset)[3];

    for(unsigned int i = 1u; i < tilesX*tilesY; i++) {
        if(((pData + shpfileEntries[TILE_GETINDEX(tiles[i])].startOffset)[2] != sizeY)
         || ((pData + shpfileEntries[TILE_GETINDEX(tiles[i])].startOffset)[3] != sizeX)) {
            THROW(std::runtime_error, "Shpfile::getPictureArray(): Not all pictures have the same size!");
         }
    }

    std::vector<unsigned char> DecodeDestination;
    auto ImageOut = std::make_unique<unsigned char[]>(sizeX * sizeY);

    // create new picture surface
    sdl2::surface_ptr pic{ SDL_CreateRGBSurface(0, sizeX*tilesX, sizeY*tilesY, 8, 0, 0, 0, 0) };
    if(pic == nullptr) {
        THROW(std::runtime_error, "Shpfile::getPictureArray(): Cannot create Surface!");
    }

    palette.applyToSurface(pic.get());
    sdl2::surface_lock lock{ pic.get() };

    for(unsigned int j = 0u; j < tilesY; j++) {
        for(unsigned int i = 0u; i < tilesX; i++) {

            const unsigned char * Fileheader = pData + shpfileEntries[TILE_GETINDEX(tiles[j*tilesX+i])].startOffset;
            unsigned char type = Fileheader[0];

            /* size and also checksum */
            Uint16 size = SDL_SwapLE16(*(reinterpret_cast<const Uint16 *>(Fileheader + 8)));

            memset(ImageOut.get(), 0, sizeX * sizeY);

            switch(type) {

                case 0:
                {
                    DecodeDestination.clear();
                    DecodeDestination.resize(size);

                    if(decode80(Fileheader + 10,&DecodeDestination[0],size) == -1) {
                        SDL_Log("Warning: Checksum-Error in Shp-File!");
                    }

                    shpCorrectLF(&DecodeDestination[0], ImageOut.get(), size);
                } break;

                case 1:
                {
                    DecodeDestination.clear();
                    DecodeDestination.resize(size);

                    if(decode80(Fileheader + 10 + 16, &DecodeDestination[0], size) == -1) {
                        SDL_Log("Warning: Checksum-Error in Shp-File!");
                    }

                    shpCorrectLF(&DecodeDestination[0], ImageOut.get(), size);

                    applyPalOffsets(Fileheader + 10, ImageOut.get(), sizeX*sizeY);
                } break;

                case 2:
                {
                    shpCorrectLF(Fileheader+10, ImageOut.get(),size);
                } break;

                case 3:
                {
                    shpCorrectLF(Fileheader + 10 + 16, ImageOut.get(), size);
                    applyPalOffsets(Fileheader + 10,ImageOut.get(), sizeX*sizeY);
                } break;

                default:
                {
                    THROW(std::runtime_error, "Shpfile::getPictureArray(): Type %d in SHP-Files not supported!", type);
                }
            }

            //Now we can copy line by line
            switch(TILE_GETTYPE(tiles[i])) {
                case TILE_NORMAL:
                {
                    for(auto y = 0; y < sizeY; y++) {
                        memcpy( static_cast<char*>(pic->pixels) + i*sizeX + (y+j*sizeY) * pic->pitch , ImageOut.get() + y * sizeX, sizeX);
                    }
                } break;

                case TILE_FLIPH:
                {
                    for(auto y = 0; y < sizeY; y++) {
                        memcpy( static_cast<char*>(pic->pixels) + i*sizeX + (y+j*sizeY) * pic->pitch , ImageOut.get() + (sizeY-1-y) * sizeX, sizeX);
                    }
                } break;

                case TILE_FLIPV:
                {
                    for(auto y = 0; y < sizeY; y++) {
                        unsigned char * const RESTRICT out = static_cast<unsigned char*>(pic->pixels) + i * sizeX + (y + j * sizeY) * pic->pitch;
                        const unsigned char* const RESTRICT in = ImageOut.get() + y * sizeX;
                        for(auto x = 0; x < sizeX; x++) {
                            out[x] = in[sizeX-1-x];
                        }
                    }
                } break;

                case TILE_ROTATE:
                {
                    for(auto y = 0; y < sizeY; y++) {
                        unsigned char * const RESTRICT out = static_cast<unsigned char*>(pic->pixels) + i * sizeX + (y + j * sizeY) * pic->pitch;
                        const unsigned char * const RESTRICT in = ImageOut.get() + (sizeY - 1 - y) * sizeX;
                        for(auto x = 0; x < sizeX; x++) {
                            out[x] = in[sizeX-1-x];
                        }
                    }
                } break;

                default:
                {
                    THROW(std::invalid_argument, "Shpfile::getPictureArray(): Invalid tile type %ud; must be one of TILE_NORMAL, TILE_FLIPH, TILE_FLIPV or TILE_ROTATE!", TILE_GETTYPE(tiles[i]));
                } break;
            }
        }
    }

    return pic;
}

/// Returns an animation
/**
    This method returns a new animation object with all pictures from startindex to endindex
    in it.
    \param  startindex  index of the first picture
    \param  endindex    index of the last picture
    \param  bDoublePic  if true, the picture is scaled up by a factor of 2
    \param  bSetColorKey    if true, black is set as transparency
    \param  bLoopRewindBackwards if true, the animation is played forward and then backwards, if false, it is only played forward
    \return a new animation object
*/
std::unique_ptr<Animation> Shpfile::getAnimation(unsigned int startindex,unsigned int endindex, bool bDoublePic, bool bSetColorKey, bool bLoopRewindBackwards)
{
    auto animation = std::make_unique<Animation>();

    for(unsigned int i = startindex; i <= endindex; ++i) {
        animation->addFrame(getPicture(i),bDoublePic,bSetColorKey);
    }

    if(bLoopRewindBackwards) {
        for(unsigned int i = endindex - 1; i >= startindex; --i) {
            animation->addFrame(getPicture(i),bDoublePic,bSetColorKey);
        }
    }

    return animation;
}

/// Helper method for reading the index
/**
    This helper method reads the index of this shp-File.
*/
void Shpfile::readIndex()
{
    // First get number of files in shp-file
    Uint16 NumFiles = SDL_SwapLE16(reinterpret_cast<const Uint16 *>(pFiledata.get())[0]);

    if(NumFiles == 0) {
        THROW(std::runtime_error, "Shpfile::readIndex(): There is no file in this shp-File!");
    }

    if(NumFiles == 1) {
        /* files with only one image might be different */

        ShpfileEntry newShpfileEntry;
        if ((reinterpret_cast<const Uint16*>( pFiledata.get()))[2] != 0) {
            /* File has special header with only 2 byte offset */
            newShpfileEntry.startOffset = static_cast<Uint32>(reinterpret_cast<const Uint16 *>(pFiledata.get())[1]);
            newShpfileEntry.endOffset = static_cast<Uint32>(reinterpret_cast<const Uint16 *>(pFiledata.get())[2]) - 1;
        } else {
            /* File has normal 4 byte offsets */
            newShpfileEntry.startOffset = static_cast<Uint32>(*reinterpret_cast<const Uint32 *>(pFiledata.get()+2)) + 2;
            newShpfileEntry.endOffset = static_cast<Uint32>(reinterpret_cast<const Uint16 *>(pFiledata.get())[3]) - 1 + 2;
        }

        shpfileEntries.push_back(newShpfileEntry);

    } else {
        /* File contains more than one image */

        if (reinterpret_cast<const Uint16 *>(pFiledata.get())[2] != 0) {
            /* File has special header with only 2 byte offset */

            if( shpFilesize < static_cast<Uint32>((NumFiles * 2) + 2 + 2)) {
                THROW(std::runtime_error, "Shpfile::readIndex(): Shp-File-Header is not complete! Header too small!");
            }

            // now fill Index with start and end-offsets
            for(int i = 0; i < NumFiles; i++) {
                ShpfileEntry newShpfileEntry;
                newShpfileEntry.startOffset = SDL_SwapLE16(reinterpret_cast<const Uint16 *>(pFiledata.get() + 2)[i]);

                if(shpfileEntries.empty() == false) {
                    shpfileEntries.back().endOffset = newShpfileEntry.startOffset - 1;

                    if(newShpfileEntry.startOffset >= shpFilesize) {
                        THROW(std::runtime_error, "Shpfile::readIndex(): Entry in this SHP-File is beyond the end of this file!");
                    }
                }

                shpfileEntries.push_back(newShpfileEntry);
            }

            // Add the endOffset for the last file
            shpfileEntries.back().endOffset = static_cast<Uint32>(*reinterpret_cast<const Uint16 *>(pFiledata.get()+ 2 +(NumFiles * 2))) - 1 + 2;
        } else {
            /* File has normal 4 byte offsets */

            if( shpFilesize < static_cast<Uint32>((NumFiles * 4) + 2 + 2)) {
                THROW(std::runtime_error, "Shpfile::readIndex(): Shp-File-Header is not complete! Header too small!");
            }

            // now fill Index with start and end-offsets
            for(auto i = 0; i < NumFiles; i++) {
                ShpfileEntry newShpfileEntry;
                newShpfileEntry.startOffset = SDL_SwapLE32( (reinterpret_cast<const Uint32*>(pFiledata.get() + 2))[i]) + 2;

                if (shpfileEntries.empty() == false) {
                    shpfileEntries.back().endOffset = newShpfileEntry.startOffset - 1;

                    if(newShpfileEntry.startOffset >= shpFilesize) {
                        THROW(std::runtime_error, "Shpfile::readIndex(): Entry in this SHP-File is beyond the end of this file!");
                    }
                }

                shpfileEntries.push_back(newShpfileEntry);
            }

            // Add the endOffset for the last file
            shpfileEntries.back().endOffset = static_cast<Uint32>(*reinterpret_cast<const Uint16 *>(pFiledata.get()+ 2 +(NumFiles * 4))) - 1 + 2;
        }
    }
}

/// Helper method for correcting the decoded picture.
/**
    This helper method corrects the decoded picture.
    \param  in  input picture
    \param  out output picture
    \param  size    size of the input picture
*/
void Shpfile::shpCorrectLF(const unsigned char *in, unsigned char *out, int size)
{
    const unsigned char * end = in + size;
    while(in < end) {
        unsigned char val = *in;
        in++;

        if(val != 0) {
            *out = val;
            out++;
        } else {
            unsigned char count = *in;
            in++;
            if (count == 0) {
                return;
            }
            memset(out, 0, count);

            out += count;
        }
    }
}

/// Helper method for correcting the palette of the decoded picture.
/**
    This helper method corrects the palette of the decoded picture.
    \param  offsets A lookup-table for correcting the palette
    \param  data    the picture to be corrected
    \param  length  size of the picture
*/
void Shpfile::applyPalOffsets(const unsigned char *offsets, unsigned char *data, unsigned int length)
{
    for(unsigned int i = 0; i < length; i ++) {
        data[i] = offsets[data[i]];
    }
}
