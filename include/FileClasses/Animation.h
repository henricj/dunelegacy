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

#ifndef ANIMATION_H
#define ANIMATION_H

#include <FileClasses/Palette.h>
#include <misc/SDL2pp.h>

#include <vector>
#include "GUI/ProgressBar.h"

#define INVALID_FRAME ((unsigned int) -1)

class Animation
{
public:
    Animation();
    ~Animation();

    Animation(const Animation &) = delete;
    Animation(Animation &&) = delete;
    Animation& operator=(const Animation &) = delete;
    Animation& operator=(Animation &&) = delete;

    unsigned int getCurrentFrameNumber();

    void setCurrentFrameNumber(unsigned int newCurrentFrame) noexcept { curFrame = newCurrentFrame; }

    [[nodiscard]] unsigned int getNumberOfFrames() const noexcept { return static_cast<unsigned int>(frames.size()); }

    template<typename Visitor>
    void for_each_frame(Visitor&& visitor) {
        for (auto& s : frames)
            visitor(s.get());
    }

    SDL_Surface* getFrame();

    SDL_Texture* getFrameTexture();

    const std::vector<sdl2::surface_ptr>& getFrames() { return frames; }

    void setFrameRate(double FrameRate) noexcept {
        if(FrameRate == 0.0) {
            frameDurationTime = 1;
        } else {
            frameDurationTime = static_cast<int>(1000.0 / FrameRate);
        }
    }

    void setFrameDurationTime(Uint32 frameDurationTime) noexcept {
        this->frameDurationTime = frameDurationTime;
    }

    [[nodiscard]] Uint32 getFrameDurationTime() const noexcept {
        return frameDurationTime;
    }

    void addFrame(sdl2::surface_ptr newFrame, bool bDoublePic = false, bool bSetColorKey = false);

    void setPalette(const Palette& newPalette);

    void setNumLoops(int loops) noexcept {
        curFrame = 0;
        loopsLeft = loops;
    }

    [[nodiscard]] int getLoopsLeft() const noexcept {
        return loopsLeft;
    }

    [[nodiscard]] bool isFinished() const noexcept {
        if(loopsLeft == -1 || loopsLeft > 0) {
            return false;
        } else {
            return (curFrame == (frames.size()-1));
        }
    }

    void setFrameOverride(unsigned int frameOverride) noexcept { curFrameOverride = frameOverride; }

    void resetFrameOverride() noexcept { curFrameOverride = INVALID_FRAME; }

    [[nodiscard]] unsigned int getCurrentFrameOverride() const noexcept { return curFrameOverride; }

private:
    Uint32 curFrameStartTime;
    Uint32 frameDurationTime;
    int loopsLeft;
    unsigned int curFrame;
    unsigned int curFrameOverride;
    std::vector<sdl2::surface_ptr> frames;
    std::vector<sdl2::texture_ptr> frameTextures;
};

#endif // ANIMATION_H
