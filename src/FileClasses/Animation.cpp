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

#include <FileClasses/Animation.h>

#include <misc/Scaler.h>
#include <misc/draw_util.h>

Animation::Animation() {
    curFrameStartTime = SDL_GetTicks();
    frameDurationTime = 1;
    curFrame = 0;
    curFrameOverride = INVALID_FRAME;
    loopsLeft = -1;
}

Animation::~Animation() = default;

unsigned int Animation::getCurrentFrameNumber() {
    if((SDL_GetTicks() - curFrameStartTime) > frameDurationTime) {
        curFrameStartTime = SDL_GetTicks();

        if(loopsLeft == -1) {
            curFrame++;
            if(curFrame >= frames.size()) {
                curFrame = 0;
            }
        } else if(loopsLeft >= 1) {
            curFrame++;
            if(curFrame >= frames.size()) {
                loopsLeft--;
                if(loopsLeft > 0) {
                    curFrame = 0;
                } else {
                    curFrame--;
                }
            }
        }
    }

    return (curFrameOverride != INVALID_FRAME) ? curFrameOverride : curFrame;
}

SDL_Surface* Animation::getFrame() {
    if(frames.empty()) {
        return nullptr;
    }

    return frames[getCurrentFrameNumber()].get();
}

SDL_Texture* Animation::getFrameTexture() {
    if(frames.empty()) {
        return nullptr;
    }

    unsigned int index = getCurrentFrameNumber();

    if(frameTextures.size() <= index) {
        // vector<>.resize() doesn't work with unique_ptr<>
        frameTextures.reserve(frames.size());
        unsigned int needed = index - frameTextures.size() + 1;
        std::fill_n(std::back_inserter(frameTextures), needed, nullptr);
    }

    if(frameTextures[index] == nullptr) {
        frameTextures[index] = convertSurfaceToTexture(frames[index].get());
    }

    return frameTextures[index].get();
}

void Animation::addFrame(sdl2::surface_ptr newFrame, bool bDoublePic, bool bSetColorKey) {
    if(bDoublePic == true) {
        newFrame = Scaler::defaultDoubleSurface(newFrame.get());
    }

    if(bSetColorKey == true) {
        SDL_SetColorKey(newFrame.get(), SDL_TRUE, 0);
    }

    frames.emplace_back(std::move(newFrame));
}

void Animation::setPalette(const Palette& newPalette) {
    for(auto& pSurface : frames) {
        newPalette.applyToSurface(pSurface.get());
    }
}
