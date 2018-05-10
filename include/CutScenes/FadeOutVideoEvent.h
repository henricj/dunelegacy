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

#ifndef FADEOUTVIDEOEVENT_H
#define FADEOUTVIDEOEVENT_H

#include <CutScenes/VideoEvent.h>
#include <misc/SDL2pp.h>

/**
    This VideoEvent is used for fading out a picture
*/
class FadeOutVideoEvent : public VideoEvent {
public:

    /**
        Constructor
        \param  pSurface            The picture to fade out
        \param  numFrames2FadeOut   The number of frames the fading should take
        \param  bCenterVertical     true = center the surface vertically on the screen, false = blit the surface at the top of the screen (default is true)
        \param  bFadeWhite          true = fade to white, false = fade to black (default is false)
    */
    FadeOutVideoEvent(SDL_Surface* pSurface, int numFrames2FadeOut, bool bCenterVertical = true, bool bFadeWhite = false);

    /// destructor
    virtual ~FadeOutVideoEvent();

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
    int currentFrame;           ///< the current frame number relative to the start of this FadeOutVideoEvent
    int numFrames2FadeOut;      ///< the number of frames the fading should take
    sdl2::texture_ptr pTexture; ///< the picture to fade out
    bool bCenterVertical;       ///< true = center the surface vertically on the screen, false = blit the surface at the top of the screen
    bool bFadeWhite;            ///< true = fade to white, false = fade to black
};

#endif // FADEOUTVIDEOEVENT_H
