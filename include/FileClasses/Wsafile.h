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

#ifndef WSAFILE_H
#define WSAFILE_H

#include "Animation.h"
#include <SDL2/SDL.h>
#include <SDL2/SDL_rwops.h>
#include <stdarg.h>

/// A class for loading a *.WSA-File.
/**
    This class can read the animation in a *.WSA-File and return it as SDL_Surfaces.
*/
class Wsafile
{
public:
    explicit Wsafile(SDL_RWops* rwop);
    Wsafile(SDL_RWops* rwop0, SDL_RWops* rwop1);
    Wsafile(SDL_RWops* rwop0, SDL_RWops* rwop1, SDL_RWops* rwop2);
    Wsafile(SDL_RWops* rwop0, SDL_RWops* rwop1, SDL_RWops* rwop2, SDL_RWops* rwop3);
    Wsafile(int num,...);
    Wsafile(const Wsafile& wsafile) = delete;
    Wsafile& operator=(const Wsafile& wsafile) = delete;
    virtual ~Wsafile();

    SDL_Surface * getPicture(Uint32 FrameNumber);
    SDL_Surface * getAnimationAsPictureRow(int numFramesX = std::numeric_limits<int>::max());
    Animation* getAnimation(unsigned int startindex, unsigned int endindex, bool bDoublePic=true, bool bSetColorKey=true);

    /// Returns the number of frames
    /**
        This method returns the number of frames in this animation
        \return Number of frames.
    */
    inline int getNumFrames() const { return (int) numFrames; };

    /**
        Get the width of this video
        \return the width in pixels
    */
    inline Uint16 getWidth() const { return sizeX; };

    /**
        Get the height of this video
        \return the height in pixels
    */
    inline Uint16 getHeight() const { return sizeY; };

    /// Returns whether the animation is looped or not.
    /**
        This method returns whether this animation is looped or not.
        \return true if looped, false if not
    */
    inline bool isAnimationLooped() const { return looped; };

private:
    void decodeFrames(unsigned char* pFiledata, Uint32* index, int numberOfFrames, unsigned char* pDecodedFrames, int x, int y);
    unsigned char* readfile(SDL_RWops* rwop, int* filesize);
    void readdata(int numFiles, ...);
    void readdata(int numFiles, va_list args);
    unsigned char *decodedFrames = nullptr;

    Uint16 numFrames = 0;
    Uint16 sizeX = 0;
    Uint16 sizeY = 0;
    bool looped = false;
};

/**
    This function reads a wsa-File from a SDL_RWop and returns the nth frame as a SDL_Surface. The SDL_RWops can be readonly but must support
    seeking. The returned SDL_Surface should be freed with SDL_FreeSurface() if no longer needed.
    \param  RWop    SDL_RWops to the wsa-File. (can be readonly)
    \param  FrameNumber the frame to retrieve (zero-based index)
    \param  freesrc A non-zero value means it will automatically close/free the rwop for you.
    \return Picture in this WSA-File
*/
SDL_Surface * LoadWSA_RW(SDL_RWops* RWop, Uint32 FrameNumber, int freesrc);

#endif // WSAFILE_H
