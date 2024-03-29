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

#include "misc/dune_clock.h"
#include <FileClasses/Palette.h>
#include <misc/SDL2pp.h>

#include "GUI/ProgressBar.h"

#include <vector>

inline constexpr auto INVALID_FRAME = ~0u;

class Animation {
public:
    Animation();
    ~Animation();

    Animation(const Animation&)            = delete;
    Animation(Animation&&)                 = delete;
    Animation& operator=(const Animation&) = delete;
    Animation& operator=(Animation&&)      = delete;

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
        if (FrameRate == 0.0) {
            frameDurationTime = dune::as_dune_clock_duration(1);
        } else {
            frameDurationTime = dune::as_dune_clock_duration(1000.0 / FrameRate);
        }
    }

    void setFrameDurationTime(uint32_t duration) noexcept {
        frameDurationTime = dune::as_dune_clock_duration(duration);
    }
    void setFrameDurationTime(dune::dune_clock::duration duration) noexcept { frameDurationTime = duration; }

    [[nodiscard]] auto getFrameDurationTime() const noexcept { return frameDurationTime; }

    void addFrame(sdl2::surface_ptr newFrame, bool bDoublePic = false, bool bSetColorKey = false);

    void setPalette(const Palette& newPalette);

    void setNumLoops(int loops) noexcept {
        curFrame  = 0;
        loopsLeft = loops;
    }

    [[nodiscard]] int getLoopsLeft() const noexcept { return loopsLeft; }

    [[nodiscard]] bool isFinished() const noexcept {
        if (loopsLeft == -1 || loopsLeft > 0) {
            return false;
        }
        return (curFrame == (frames.size() - 1));
    }

    void setFrameOverride(unsigned int frameOverride) noexcept { curFrameOverride = frameOverride; }

    void resetFrameOverride() noexcept { curFrameOverride = INVALID_FRAME; }

    [[nodiscard]] unsigned int getCurrentFrameOverride() const noexcept { return curFrameOverride; }

private:
    dune::dune_clock::time_point curFrameStartTime{dune::dune_clock::now()};
    dune::dune_clock::duration frameDurationTime{dune::as_dune_clock_duration(1)};
    int loopsLeft         = -1;
    unsigned int curFrame = 0;
    unsigned int curFrameOverride;
    std::vector<sdl2::surface_ptr> frames;
    std::vector<sdl2::texture_ptr> frameTextures;
};

#endif // ANIMATION_H
