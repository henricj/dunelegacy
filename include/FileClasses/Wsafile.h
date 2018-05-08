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
#include <misc/SDL2pp.h>

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
    Wsafile(Wsafile&& wsafile) = delete;
    Wsafile& operator=(const Wsafile& wsafile) = delete;
    Wsafile& operator=(Wsafile&& wsafile) = delete;
    virtual ~Wsafile();

    sdl2::surface_ptr getPicture(Uint32 FrameNumber) const;
    sdl2::surface_ptr getAnimationAsPictureRow(int numFramesX = std::numeric_limits<int>::max()) const;
    std::unique_ptr<Animation> getAnimation(unsigned int startindex, unsigned int endindex, bool bDoublePic=true, bool bSetColorKey=true) const;

    /// Returns the number of frames
    /**
        This method returns the number of frames in this animation
        \return Number of frames.
    */
    int getNumFrames() const noexcept { return static_cast<int>(numFrames); };

    /**
        Get the width of this video
        \return the width in pixels
    */
    Uint16 getWidth() const noexcept { return sizeX; };

    /**
        Get the height of this video
        \return the height in pixels
    */
    Uint16 getHeight() const noexcept { return sizeY; };

    /// Returns whether the animation is looped or not.
    /**
        This method returns whether this animation is looped or not.
        \return true if looped, false if not
    */
    bool isAnimationLooped() const noexcept { return looped; };

private:
    void decodeFrames(const unsigned char* pFiledata, Uint32* index, int numberOfFrames, unsigned char* pDecodedFrames, int x, int y) const;
    std::unique_ptr<unsigned char[]> readfile(SDL_RWops* rwop, int* filesize) const;
    void readdata(int numFiles, ...);
    void readdata(int numFiles, va_list args);
    std::vector<unsigned char> decodedFrames;

    Uint16 numFrames = 0;
    Uint16 sizeX = 0;
    Uint16 sizeY = 0;
    bool looped = false;
};

#endif // WSAFILE_H
