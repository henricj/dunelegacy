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

#include <SDL_endian.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

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
Wsafile::~Wsafile()
{
    free(decodedFrames);
}

/// Returns a picture in this wsa-File
/**
    This method returns a SDL_Surface containing the nth frame of this animation.
    The returned SDL_Surface should be freed with SDL_FreeSurface() if no longer needed.
    \param  frameNumber specifies which frame to return (zero based)
    \return nth frame in this animation
*/
SDL_Surface * Wsafile::getPicture(Uint32 frameNumber)
{
    if(frameNumber >= numFrames) {
        return nullptr;
    }

    SDL_Surface * pic;
    unsigned char * pImage = decodedFrames + (frameNumber * sizeX * sizeY);

    // create new picture surface
    if((pic = SDL_CreateRGBSurface(0,sizeX,sizeY,8,0,0,0,0))== nullptr) {
        return nullptr;
    }

    palette.applyToSurface(pic);
    SDL_LockSurface(pic);

    //Now we can copy line by line
    for(int y = 0; y < sizeY;y++) {
        memcpy( ((char*) (pic->pixels)) + y * pic->pitch , pImage + y * sizeX, sizeX);
    }

    SDL_UnlockSurface(pic);
    return pic;
}

/// Returns a picture-row
/**
    This method returns a SDL_Surface containing the complete animation.
    The returned SDL_Surface should be freed with SDL_FreeSurface() if no longer needed.
    \return the complete animation
*/
SDL_Surface * Wsafile::getAnimationAsPictureRow() {
    SDL_Surface * pic;

    // create new picture surface
    if((pic = SDL_CreateRGBSurface(0,sizeX*numFrames,sizeY,8,0,0,0,0))== nullptr) {
        return nullptr;
    }

    palette.applyToSurface(pic);
    SDL_LockSurface(pic);

    for(int i = 0; i < numFrames; i++) {
        unsigned char * Image = decodedFrames + (i * sizeX * sizeY);

        //Now we can copy this frame line by line
        for(int y = 0; y < sizeY;y++) {
            memcpy( ((char*) (pic->pixels)) + y * pic->pitch + i*sizeX, Image + y * sizeX, sizeX);
        }
    }

    SDL_UnlockSurface(pic);
    return pic;
}

/// Returns an animation
/**
    This method returns a new animation object with all pictures from startindex to endindex
    in it. The returned pointer should be freed with delete if no longer needed. If an error
    occured, nullptr is returned.
    \param  startindex  index of the first picture
    \param  endindex    index of the last picture
    \param  bDoublePic  if true, the picture is scaled up by a factor of 2
    \param  bSetColorKey    if true, black is set as transparency
    \return a new animation object or nullptr on error
*/
Animation* Wsafile::getAnimation(unsigned int startindex, unsigned int endindex, bool bDoublePic, bool bSetColorKey)
{


    Animation* tmpAnimation = new Animation();

    for(unsigned int i = startindex; i <= endindex; i++) {
        SDL_Surface* tmp = getPicture(i);
        if(tmp == nullptr) {
            delete tmpAnimation;
            return nullptr;
        }
        tmpAnimation->addFrame(tmp,bDoublePic,bSetColorKey);
    }
    return tmpAnimation;
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
void Wsafile::decodeFrames(unsigned char* pFiledata, Uint32* index, int numberOfFrames, unsigned char* pDecodedFrames, int x, int y)
{
    for(int i=0;i<numberOfFrames;i++) {
        unsigned char *dec80 = (unsigned char*) malloc(x*y*2);
        if(dec80 == nullptr) {
            SDL_Log("Error: Unable to allocate memory for decoded WSA-Frames!");
            exit(EXIT_FAILURE);
        }

        decode80(pFiledata + SDL_SwapLE32(index[i]), dec80, 0);

        decode40(dec80, pDecodedFrames + i * x*y);

        free(dec80);

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
unsigned char* Wsafile::readfile(SDL_RWops* rwop, int* filesize) {
    unsigned char* pFiledata;

    if(filesize == nullptr) {
        SDL_Log("Wsafile: filesize == nullptr!");
        exit(EXIT_FAILURE);
    }

    if(rwop == nullptr) {
        SDL_Log("Wsafile: rwop == nullptr!");
        exit(EXIT_FAILURE);
    }

    int wsaFilesize = SDL_RWseek(rwop,0,SEEK_END);
    if(wsaFilesize <= 0) {
        SDL_Log("Wsafile: Cannot determine size of this *.wsa-File!");
        exit(EXIT_FAILURE);
    }

    if(wsaFilesize < 10) {
        SDL_Log("Wsafile: No valid WSA-File: File too small!");
        exit(EXIT_FAILURE);
    }

    if(SDL_RWseek(rwop,0,SEEK_SET) != 0) {
        SDL_Log("Wsafile: Seeking in this *.wsa-File failed!");
        exit(EXIT_FAILURE);
    }

    if( (pFiledata = (unsigned char*) malloc(wsaFilesize)) == nullptr) {
        SDL_Log("Wsafile: Allocating memory failed!");
        exit(EXIT_FAILURE);
    }

    if(SDL_RWread(rwop, pFiledata, wsaFilesize, 1) != 1) {
        SDL_Log("Wsafile: Reading this *.wsa-File failed!");
        exit(EXIT_FAILURE);
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
    unsigned char** pFiledata;
    Uint32** index;
    Uint16* numberOfFrames;
    bool* extended;

    if((pFiledata = (unsigned char**) malloc(sizeof(unsigned char*) * numFiles)) == nullptr) {
        SDL_Log("Wsafile::readdata(): Unable to allocate memory!");
        exit(EXIT_FAILURE);
    }

    if((index = (Uint32**) malloc(sizeof(Uint32*) * numFiles)) == nullptr) {
        SDL_Log("Wsafile::readdata(): Unable to allocate memory!");
        exit(EXIT_FAILURE);
    }

    if((numberOfFrames = (Uint16*) malloc(sizeof(Uint16) * numFiles)) == nullptr) {
        SDL_Log("Wsafile::readdata(): Unable to allocate memory!");
        exit(EXIT_FAILURE);
    }

    if((extended = (bool*) malloc(sizeof(bool) * numFiles)) == nullptr) {
        SDL_Log("Wsafile::readdata(): Unable to allocate memory!");
        exit(EXIT_FAILURE);
    }

    numFrames = 0;
    looped = false;

    for(int i = 0; i < numFiles; i++) {
        SDL_RWops* rwop;
        int wsaFilesize;
        rwop = va_arg(args,SDL_RWops*);
        pFiledata[i] = readfile(rwop,&wsaFilesize);
        numberOfFrames[i] = SDL_SwapLE16(*((Uint16*) pFiledata[i]) );

        if(i == 0) {
            sizeX = SDL_SwapLE16(*((Uint16*) (pFiledata[0] + 2)) );
            sizeY = SDL_SwapLE16(*((Uint16*) (pFiledata[0] + 4)) );
        } else {
            if( (sizeX != (SDL_SwapLE16(*((Uint16*) (pFiledata[i] + 2)) )))
                || (sizeY != (SDL_SwapLE16(*((Uint16*) (pFiledata[i] + 4)) )))) {
                SDL_Log("Wsafile: The wsa-files have different picture dimensions. Cannot concatenate them!");
                exit(EXIT_FAILURE);
            }
        }

        if( ((unsigned short *) pFiledata[i])[6] == 0) {
            index[i] = (Uint32 *) (pFiledata[i] + 10);
        } else {
            index[i] = (Uint32 *) (pFiledata[i] + 8);
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

        if(pFiledata[i] + wsaFilesize < (((unsigned char *) index[i]) + sizeof(Uint32) * numberOfFrames[i])) {
            SDL_Log("Wsafile: No valid WSA-File: File too small!");
            exit(EXIT_FAILURE);
        }

        numFrames += numberOfFrames[i];
    }


    if( (decodedFrames = (unsigned char*) calloc(1,sizeX*sizeY*numFrames)) == nullptr) {
        SDL_Log("Wsafile: Unable to allocate memory for decoded WSA-Frames!");
        exit(EXIT_FAILURE);
    }

    decodeFrames(pFiledata[0],index[0],numberOfFrames[0],decodedFrames,sizeX,sizeY);
    unsigned char* nextFreeFrame = decodedFrames + (numberOfFrames[0] * sizeX * sizeY);
    free(pFiledata[0]);

    for(int i = 1 ; i < numFiles; i++) {
        if(extended[i]) {
            // copy last frame
            memcpy(nextFreeFrame,nextFreeFrame - (sizeX*sizeY),sizeX*sizeY);
        }
        decodeFrames(pFiledata[i],index[i],numberOfFrames[i],nextFreeFrame,sizeX,sizeY);
        nextFreeFrame += numberOfFrames[i] * sizeX * sizeY;
        free(pFiledata[i]);
    }

    free(pFiledata);
    free(numberOfFrames);
    free(index);
    free(extended);
}

SDL_Surface * LoadWSA_RW(SDL_RWops* RWop, Uint32 FrameNumber, int freesrc) {
    Wsafile* wsafile = new Wsafile(RWop);

    SDL_Surface* pPic = wsafile->getPicture(FrameNumber);

    delete wsafile;

    if(freesrc) {
        SDL_RWclose(RWop);
    }

    return pPic;
}
