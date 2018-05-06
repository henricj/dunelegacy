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

#ifndef WSAVIDEOEVENT_H
#define WSAVIDEOEVENT_H

#include <CutScenes/VideoEvent.h>
#include <FileClasses/Wsafile.h>

/**
    This VideoEvent is used for playing a wsa video.
*/
class WSAVideoEvent : public VideoEvent {
public:

    /**
        Constructor
        \param  pWsafile            The video to play
        \param  bCenterVertical     true = center the video vertically on the screen, false = blit the video frames at the top of the screen (default is true)
    */
    WSAVideoEvent(Wsafile* pWsafile, bool bCenterVertical = true);

    /// destructor
    virtual ~WSAVideoEvent();

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
    int currentFrame;               ///< the current frame number relative to the start of this WSAVideoEvent
    Wsafile* pWsafile;              ///< the video to play
    sdl2::texture_ptr pStreamingTexture; ///< the texture used for rendering from
    bool bCenterVertical;           ///< true = center the video vertically on the screen, false = blit the video frames at the top of the screen
};

#endif // WSAVIDEOEVENT_H
