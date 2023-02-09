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

#include <cstdint>
#include <initializer_list>
#include <memory>
#include <tuple>

/// A class for loading a *.WSA-File.
/**
    This class can read the animation in a *.WSA-File and return it as SDL_Surfaces.
*/
class Wsafile final {
public:
    /**
        The constructor reads from the RWops all data and saves them internally. The SDL_RWops can be readonly but must
       support seeking. Immediately after the Wsafile-Object is constructed the RWops can be closed. All data is saved
       in the class. All animations are concatenated.
       \param  rwops     SDL_RWops for each wsa-File. (can be readonly)
    */
    explicit Wsafile(std::convertible_to<SDL_RWops*> auto... rwops) {
        readdata(std::initializer_list<SDL_RWops*>{rwops...});
    }

    Wsafile(const Wsafile& wsafile)            = delete;
    Wsafile(Wsafile&& wsafile)                 = delete;
    Wsafile& operator=(const Wsafile& wsafile) = delete;
    Wsafile& operator=(Wsafile&& wsafile)      = delete;
    ~Wsafile();

    [[nodiscard]] sdl2::surface_ptr getPicture(uint32_t FrameNumber) const;
    [[nodiscard]] sdl2::surface_ptr getAnimationAsPictureRow(int numFramesX = std::numeric_limits<int>::max()) const;
    [[nodiscard]] std::unique_ptr<Animation> getAnimation(unsigned int startindex, unsigned int endindex,
                                                          bool bDoublePic = true, bool bSetColorKey = true) const;

    /// Returns the number of frames
    /**
        This method returns the number of frames in this animation
        \return Number of frames.
    */
    [[nodiscard]] int getNumFrames() const noexcept { return static_cast<int>(numFrames); }

    /**
        Get the width of this video
        \return the width in pixels
    */
    [[nodiscard]] uint16_t getWidth() const noexcept { return sizeX; }

    /**
        Get the height of this video
        \return the height in pixels
    */
    [[nodiscard]] uint16_t getHeight() const noexcept { return sizeY; }

    /// Returns whether the animation is looped or not.
    /**
        This method returns whether this animation is looped or not.
        \return true if looped, false if not
    */
    [[nodiscard]] bool isAnimationLooped() const noexcept { return looped; }

private:
    void decodeFrames(const unsigned char* pFiledata, uint32_t* index, int numberOfFrames,
                      unsigned char* pDecodedFrames, int x, int y) const;
    std::tuple<std::unique_ptr<unsigned char[]>, size_t> readfile(SDL_RWops* rwop) const;
    void readdata(const std::initializer_list<SDL_RWops*>& rwops);

    std::vector<unsigned char> decodedFrames;

    uint16_t numFrames = 0;
    uint16_t sizeX     = 0;
    uint16_t sizeY     = 0;
    bool looped        = false;
};

#endif // WSAFILE_H
