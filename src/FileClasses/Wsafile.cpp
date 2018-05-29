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

#include <FileClasses/Wsafile.h>
#include <FileClasses/Decode.h>
#include <FileClasses/Palette.h>

#include <Definitions.h>

#include <SDL2/SDL_endian.h>
#include <stdlib.h>
#include <stdio.h>
#include <algorithm>

extern Palette palette;

/// Constructor
/**
    The constructor reads from the RWop all data and saves them internal. The SDL_RWops can be readonly but must support
    seeking. Immediately after the Wsafile-Object is constructed RWop can be closed. All data is saved in the class.
    \param  rwop    SDL_RWops to the wsa-File. (can be readonly)
*/
Wsafile::Wsafile(SDL_RWops* rwop)
{
    readdata(1,rwop);
}

/// Constructor
/**
    The constructor reads from the RWops all data and saves them internal. The SDL_RWops can be readonly but must support
    seeking. Immediately after the Wsafile-Object is constructed both RWops can be closed. All data is saved in the class.
    Both animations are concatinated.
    \param  rwop0   SDL_RWops for the first wsa-File. (can be readonly)
    \param  rwop1   SDL_RWops for the second wsa-File. (can be readonly)
*/
Wsafile::Wsafile(SDL_RWops* rwop0, SDL_RWops* rwop1)
{
    readdata(2,rwop0,rwop1);
}

/// Constructor
/**
    The constructor reads from the RWops all data and saves them internal. The SDL_RWops can be readonly but must support
    seeking. Immediately after the Wsafile-Object is constructed both RWops can be closed. All data is saved in the class.
    All three animations are concatinated.
    \param  rwop0   SDL_RWops for the first wsa-File. (can be readonly)
    \param  rwop1   SDL_RWops for the second wsa-File. (can be readonly)
    \param  rwop2   SDL_RWops for the third wsa-File. (can be readonly)
*/
Wsafile::Wsafile(SDL_RWops* rwop0, SDL_RWops* rwop1, SDL_RWops* rwop2)
{
    readdata(3,rwop0,rwop1,rwop2);
}

/// Constructor
/**
    The constructor reads from the RWops all data and saves them internal. The SDL_RWops can be readonly but must support
    seeking. Immediately after the Wsafile-Object is constructed both RWops can be closed. All data is saved in the class.
    All four animations are concatinated.
    \param  rwop0   SDL_RWops for the first wsa-File. (can be readonly)
    \param  rwop1   SDL_RWops for the second wsa-File. (can be readonly)
    \param  rwop2   SDL_RWops for the third wsa-File. (can be readonly)
    \param  rwop3   SDL_RWops for the forth wsa-File. (can be readonly)
*/
Wsafile::Wsafile(SDL_RWops* rwop0, SDL_RWops* rwop1, SDL_RWops* rwop2, SDL_RWops* rwop3)
{
    readdata(4,rwop0,rwop1,rwop2,rwop3);
}

/// Constructor
/**
    The constructor reads from the RWops all data and saves them internal. The SDL_RWops can be readonly but must support
    seeking. Immediately after the Wsafile-Object is constructed both RWops can be closed. All data is saved in the class.
    All animations are concatinated.
    \param  num     Number of Files
    \param  ...     SDL_RWops for each wsa-File. (can be readonly)
*/
Wsafile::Wsafile(int num,...) {
    va_list args;
    va_start(args,num);

    readdata(num,args);
    va_end(args);
}

/// Destructor
/**
    Frees all memory.
*/
Wsafile::~Wsafile() = default;

/// Returns a picture in this wsa-File
/**
    This method returns a SDL_Surface containing the nth frame of this animation.
    \param  frameNumber specifies which frame to return (zero based)
    \return nth frame in this animation
*/
sdl2::surface_ptr Wsafile::getPicture(Uint32 frameNumber) const {
    if(frameNumber >= numFrames) {
        THROW(std::invalid_argument, "Wsafile::getPicture(): Requested frame number is %ud but the file contains only %ud frames!", frameNumber, numFrames);
    }

    // create new picture surface
    auto pic = sdl2::surface_ptr{ SDL_CreateRGBSurface(0,sizeX,sizeY,8,0,0,0,0) };
    if(pic== nullptr) {
        THROW(std::runtime_error, "Wsafile::getPicture(): Cannot create surface!");
    }

    palette.applyToSurface(pic.get());

    const unsigned char* const RESTRICT pImage = &decodedFrames[frameNumber * sizeX * sizeY];
    unsigned char* const RESTRICT pixels = static_cast<unsigned char*>(pic->pixels);

    sdl2::surface_lock lock{ pic.get() };

    //Now we can copy line by line
    for(int y = 0; y < sizeY; ++y) {
        memcpy( pixels + y * pic->pitch, pImage + y * sizeX, sizeX);
    }

    return pic;
}

/// Returns a picture-row
/**
    This method returns a SDL_Surface containing the complete animation.
    \param  numFramesX  the maximum number of frames in X direction
    \return the complete animation
*/
sdl2::surface_ptr Wsafile::getAnimationAsPictureRow(int numFramesX) const {

    numFramesX = std::min(numFramesX, static_cast<int>(numFrames));
    int numFramesY = (numFrames + numFramesX -1) / numFramesX;

    // create new picture surface
    auto pic = sdl2::surface_ptr{ SDL_CreateRGBSurface(0,sizeX*numFramesX,sizeY*numFramesY,8,0,0,0,0) };
    if(pic == nullptr) {
        THROW(std::runtime_error, "Wsafile::getAnimationAsPictureRow(): Cannot create surface!");
    }

    palette.applyToSurface(pic.get());

    char* const RESTRICT pixels = static_cast<char*>(pic->pixels);

    sdl2::surface_lock lock{ pic.get() };

    for(int y = 0; y < numFramesY; y++) {
        for(int x = 0; x < numFramesX; x++) {
            int i = y*numFramesX + x;
            if(i >= numFrames) {
                return pic;
            }

            const unsigned char * const RESTRICT pImage = &decodedFrames[i * static_cast<size_t>(sizeX) * static_cast<size_t>(sizeY)];

            //Now we can copy this frame line by line
            for(int line = 0; line < sizeY; ++line) {
                memcpy( pixels + (y*sizeY + line) * pic->pitch + x*sizeX, pImage + line * sizeX, sizeX);
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
    \return a new animation object
*/
std::unique_ptr<Animation> Wsafile::getAnimation(unsigned int startindex, unsigned int endindex, bool bDoublePic, bool bSetColorKey) const
{
    auto animation = std::make_unique<Animation>();

    for(unsigned int i = startindex; i <= endindex; i++) {
        animation->addFrame(getPicture(i),bDoublePic,bSetColorKey);
    }

    return animation;
}

/// Helper method to decode one frame
/**
    This helper method decodes one frame.
    \param  pFiledata       Pointer to the data of this wsa-File
    \param  index           Array with startoffsets
    \param  numberOfFrames  Number of frames to decode
    \param  pDecodedFrames  memory to copy decoded frames to (must be x*y*NumberOfFrames bytes long)
    \param  x               x-dimension of one frame
    \param  y               y-dimension of one frame
*/
void Wsafile::decodeFrames(const unsigned char* pFiledata, Uint32* index, int numberOfFrames, unsigned char* pDecodedFrames, int x, int y) const
{
    for(int i = 0; i < numberOfFrames; ++i) {
        auto dec80 = std::make_unique<unsigned char[]>(x * y * 2);

        decode80(pFiledata + SDL_SwapLE32(index[i]), dec80.get(), 0);

        decode40(dec80.get(), pDecodedFrames + i * x*y);

        dec80.reset();

        if (i < numberOfFrames - 1) {
            memcpy(pDecodedFrames + (i+1) * x*y, pDecodedFrames + i * x*y,x*y);
        }
    }
}

/// Helper method for reading the complete wsa-file into memory.
/**
    This method reads the complete file into memory. A pointer to this memory is returned and
    should be freed with free when no longer needed.

*/
std::unique_ptr<unsigned char[]> Wsafile::readfile(SDL_RWops* rwop, int* filesize) const {
    if(filesize == nullptr) {
        THROW(std::runtime_error, "Wsafile::readfile(): filesize == nullptr!");
    }

    if(rwop == nullptr) {
        THROW(std::runtime_error, "Wsafile::readfile(): rwop == nullptr!");
    }

    Sint64 endOffset = SDL_RWsize(rwop);
    if(endOffset < 0) {
        THROW(std::runtime_error, "Wsafile::readfile(): Cannot determine size of this *.wsa-File!");
    }
    size_t wsaFilesize = static_cast<size_t>(endOffset);

    if(wsaFilesize < 10) {
        THROW(std::runtime_error, "Wsafile::readfile(): No valid WSA-File: File too small!");
    }

    auto pFiledata = std::make_unique<unsigned char[]>(wsaFilesize);

    if(SDL_RWread(rwop, pFiledata.get(), wsaFilesize, 1) != 1) {
        THROW(std::runtime_error, "Wsafile::readfile(): Reading this *.wsa-File failed!");
    }

    *filesize = wsaFilesize;
    return pFiledata;
}


/// Helper method for reading and concatinating various WSA-Files.
/**
    This methods reads from the RWops all data and concatinates all the frames to one animation. The SDL_RWops
    can be readonly but must support seeking.
    \param  NumFiles    Number of SDL_RWops
    \param  ...         SDL_RWops for each wsa-File. (can be readonly)
*/
void Wsafile::readdata(int numFiles, ...) {
    va_list args;
    va_start(args,numFiles);
    readdata(numFiles,args);
    va_end(args);
}

/// Helper method for reading and concatinating various WSA-Files.
/**
    This methods reads from the RWops all data and concatinates all the frames to one animation. The SDL_RWops
    can be readonly but must support seeking.
    \param  numFiles    Number of SDL_RWops
    \param  args        SDL_RWops for each wsa-File should be in this va_list. (can be readonly)
*/
void Wsafile::readdata(int numFiles, va_list args) {
    std::vector<std::unique_ptr<unsigned char[]>> pFiledata(numFiles);
    std::vector<Uint32*> index(numFiles);
    std::vector<Uint16> numberOfFrames(numFiles);
    std::vector<bool> extended(numFiles);

    numFrames = 0;
    looped = false;

    for(int i = 0; i < numFiles; i++) {
        int wsaFilesize;
        const auto rwop = va_arg(args,SDL_RWops*);
        pFiledata[i] = readfile(rwop,&wsaFilesize);
        numberOfFrames[i] = SDL_SwapLE16(*(reinterpret_cast<Uint16*>(pFiledata[i].get())) );

        if(i == 0) {
            sizeX = SDL_SwapLE16(*(reinterpret_cast<Uint16*>(pFiledata[0].get()+ 2)) );
            sizeY = SDL_SwapLE16(*(reinterpret_cast<Uint16*>(pFiledata[0].get() + 4)) );
        } else {
            if( (sizeX != (SDL_SwapLE16(*(reinterpret_cast<Uint16*>(pFiledata[i].get()+ 2)) )))
                || (sizeY != (SDL_SwapLE16(*(reinterpret_cast<Uint16*>(pFiledata[i].get()+ 4)) )))) {
                THROW(std::runtime_error, "Wsafile::readdata(): The wsa-files have different image dimensions. Cannot concatenate them!");
            }
        }

        if( reinterpret_cast<unsigned short *>(pFiledata[i].get())[6] == 0) {
            index[i] = reinterpret_cast<Uint32 *>(pFiledata[i].get() + 10);
        } else {
            index[i] = reinterpret_cast<Uint32 *>(pFiledata[i].get() + 8);
        }

        if(index[i][0] == 0) {
            // extended animation
            if(i == 0) {
                SDL_Log("Extended WSA-File!");
            }
            index[i]++;
            numberOfFrames[i]--;
            extended[i] = true;
        } else {
            extended[i] = false;
        }

        if(i == 0) {
            if(index[0][numberOfFrames[0]+1] == 0) {
                // index[numberOfFrames[0]] point to end of file
                // => no loop
                looped = false;
            } else {
                // index[numberOfFrames[0]] point to loop frame
                // => looped animation
                //  SDL_Log("Looped WSA-File!");
                looped = true;
            }
        }

        if(pFiledata[i].get() + wsaFilesize < (reinterpret_cast<unsigned char *>(index[i]) + sizeof(Uint32) * numberOfFrames[i])) {
            THROW(std::runtime_error, "Wsafile::readdata(): No valid WSA-File: File too small!");
        }

        numFrames += numberOfFrames[i];
    }


    decodedFrames.clear();
    decodedFrames.resize(static_cast<size_t>(sizeX) * static_cast<size_t>(sizeY) * numFrames);

    assert(decodedFrames.size() >= sizeX * sizeY);
    decodeFrames(pFiledata[0].get(),index[0],numberOfFrames[0],&decodedFrames[0],sizeX,sizeY);
    pFiledata[0].reset();

    if (numFiles > 1) {
        auto nextFreeFrame = &decodedFrames[(numberOfFrames[0] * static_cast<size_t>(sizeX) * static_cast<size_t>(sizeY))];

        for (int i = 1; i < numFiles; i++) {
            if (extended[i]) {
                // copy last frame
                memcpy(nextFreeFrame, nextFreeFrame - static_cast<size_t>(sizeX) * static_cast<size_t>(sizeY), static_cast<size_t>(sizeX) * static_cast<size_t>(sizeY));
            }
            assert(nextFreeFrame + sizeX * sizeY <= &decodedFrames[decodedFrames.size() - 1]);
            decodeFrames(pFiledata[i].get(), index[i], numberOfFrames[i], nextFreeFrame, sizeX, sizeY);
            nextFreeFrame += numberOfFrames[i] * sizeX * sizeY;
            pFiledata[i].reset();
        }
    }
}
