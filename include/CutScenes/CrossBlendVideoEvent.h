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

#ifndef CROSSBLENDVIDEOEVENT_H
#define CROSSBLENDVIDEOEVENT_H

#include <CutScenes/VideoEvent.h>
#include <misc/BlendBlitter.h>
#include <misc/SDL2pp.h>

/**
    This VideoEvent blends between two pictures. The blending is done in 30 steps revealing more and more pixels
    of the destination picture in every frame.
*/
class CrossBlendVideoEvent : public VideoEvent {
public:

    /**
        Constructor
        \param  pStartSurface      The picture to blend from
        \param  pEndSurface        The picture to blend to
        \param  bCenterVertical    true = center the surfaces vertically on the screen, false = blit the surfaces at the top of the screen (default is true)
    */
    CrossBlendVideoEvent(SDL_Surface* pStartSurface, SDL_Surface* pEndSurface, bool bCenterVertical = true);

    /// destructor
    virtual ~CrossBlendVideoEvent();

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
    int currentFrame;               ///< the current frame number relative to the start of this CrossBlendVideoEvent
    std::unique_ptr<BlendBlitter> pBlendBlitter;    ///< the used blend blitter
    sdl2::surface_ptr pBlendBlitterTargetSurface;   ///< the picture holding the current blending frame
    sdl2::texture_ptr pStreamingTexture; ///< the texture used for rendering from
    bool bCenterVertical;           ///< true = center the surfaces vertically on the screen, false = blit the surfaces at the top of the screen
};

#endif // CROSSBLENDVIDEOEVENT_H
