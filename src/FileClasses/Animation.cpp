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

#include <stdio.h>
#include <stdlib.h>

Animation::Animation() {
	curFrameStartTime = SDL_GetTicks();
	frameDurationTime = 1;
	curFrame = 0;
	loopsLeft = -1;
}

Animation::~Animation() {
    while(frames.empty() == false) {
        SDL_FreeSurface(frames.back());
        frames.pop_back();
    }
}

SDL_Surface* Animation::getFrame() {
    if(frames.empty()) {
		return NULL;
	}

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
	return frames[curFrame];
}

void Animation::addFrame(SDL_Surface* newFrame, bool bDoublePic, bool bSetColorKey) {
	if(bDoublePic == true) {
	    newFrame = Scaler::defaultDoubleSurface(newFrame,true);
	}

	if(bSetColorKey == true) {
		SDL_SetColorKey(newFrame, SDL_TRUE, 0);
	}

	frames.push_back(newFrame);
}

void Animation::setPalette(const Palette& newPalette) {
    std::vector<SDL_Surface*>::const_iterator iter;
    for(iter = frames.begin(); iter != frames.end(); ++iter) {
        newPalette.applyToSurface(*iter);
    }
}
