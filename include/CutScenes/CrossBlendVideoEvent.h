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
#include <SDL.h>

/**
    This VideoEvent blends between two pictures. The blending is done in 30 steps revealing more and more pixels
	of the destination picture in every frame.
*/
class CrossBlendVideoEvent : public VideoEvent {
public:

    /**
        Constructor
        \param  pSourceSurface      The picture to blend from
        \param  pDestSurface		The picture to blend to
        \param  bFreeSurfaces       true = SDL_FreeSurface(pSourceSurface) and SDL_FreeSurface(pDestSurface) after blending in is done, false = pSourceSurface and pDestSurface are not freed
        \param  bCenterVertical     true = center the surfaces vertically on the screen, false = blit the surfaces at the top of the screen (default is true)
    */
	CrossBlendVideoEvent(SDL_Surface* pSourceSurface, SDL_Surface* pDestSurface, bool bFreeSurfaces, bool bCenterVertical = true);
	
	/// destructor
	virtual ~CrossBlendVideoEvent();

    /**
        This method draws the video effect. It is called before setupPalette() is called.
        \param  pScreen the surface to draw to
        \return the milliseconds until the next frame shall be drawn.
    */
	virtual int draw(SDL_Surface* pScreen);

    /**
        This method checks if this VideoEvent is already finished
        \return true, if there are no more frames to draw with this VideoEvent
    */
	virtual bool isFinished();
	
private:
    int currentFrame;				///< the current frame number relative to the start of this CrossBlendVideoEvent
    BlendBlitter* pBlendBlitter;	///< the used blend blitter
    SDL_Surface* pSourceSurface;	///< the picture to blend from
    SDL_Surface* pDestSurface;		///< the picture to blend to
    bool bFreeSurfaces;				///< true = SDL_FreeSurface(pSourceSurface) and SDL_FreeSurface(pDestSurface) after blending in is done, false = pSourceSurface and pDestSurface are not freed
	bool bCenterVertical;			///< true = center the surfaces vertically on the screen, false = blit the surfaces at the top of the screen
};

#endif // CROSSBLENDVIDEOEVENT_H
