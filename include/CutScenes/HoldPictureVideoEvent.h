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

#ifndef HOLDPICTUREVIDEOEVENT_H
#define HOLDPICTUREVIDEOEVENT_H

#include <CutScenes/VideoEvent.h>
#include <misc/SDL2pp.h>

/**
    This VideoEvent statical shows a picture for a number of frames.
*/
class HoldPictureVideoEvent : public VideoEvent {
public:

    /**
        Constructor
        \param  pSurface            The picture to show
        \param  numFrames2Hold      The number of frames the picture should be shown
        \param  bCenterVertical     true = center the surface vertically on the screen, false = blit the surface at the top of the screen (default is true)
    */
    HoldPictureVideoEvent(SDL_Surface* pSurface, int numFrames2Hold, bool bCenterVertical = true);

    /// destructor
    virtual ~HoldPictureVideoEvent();

    /**
        This method draws the video effect.
        \return the milliseconds until the next frame shall be drawn.
    */
    int draw() override;

    /**
        This method checks if this VideoEvent is already finished
        \return true, if there are no more frames to draw with this VideoEvent
    */
    bool isFinished() override;

private:
    int currentFrame;       ///< the current frame number relative to the start of this HoldPictureVideoEvent
    int numFrames2Hold;     ///< the number of frames the picture should be shown
    sdl2::texture_ptr pTexture;  ///< the picture to show
    bool bCenterVertical;   ///< true = center the surface vertically on the screen, false = blit the surface at the top of the screen
};

#endif // HOLDPICTUREVIDEOEVENT_H
