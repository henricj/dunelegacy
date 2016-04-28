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

#include <SDL.h>
#include <vector>

class Animation
{
public:
    Animation();
    ~Animation();

    unsigned int getCurrentFrameNumber();

    SDL_Surface* getFrame();

    SDL_Texture* getFrameTexture();

    const std::vector<SDL_Surface*>& getFrames() { return frames; };

    void setFrameRate(double FrameRate) {
        if(FrameRate == 0.0) {
            frameDurationTime = 1;
        } else {
            frameDurationTime = (int) (1000.0/FrameRate);
        }
    }

    void setFrameDurationTime(Uint32 frameDurationTime) {
        this->frameDurationTime = frameDurationTime;
    }

    Uint32 getFrameDurationTime() const {
        return frameDurationTime;
    }

    void addFrame(SDL_Surface* newFrame, bool bDoublePic = false, bool bSetColorKey = false);

    void setPalette(const Palette& newPalette);

    void setNumLoops(int loops) {
        curFrame = 0;
        loopsLeft = loops;
    };

    int getLoopsLeft() const {
        return loopsLeft;
    };

    bool isFinished() const {
        if(loopsLeft == -1 || loopsLeft > 0) {
            return false;
        } else {
            return (curFrame == (frames.size()-1));
        }
    }

private:
    Uint32 curFrameStartTime;
    Uint32 frameDurationTime;
    int loopsLeft;
    unsigned int curFrame;
    std::vector<SDL_Surface*> frames;
    std::vector<SDL_Texture*> frameTextures;
};

#endif // ANIMATION_H
