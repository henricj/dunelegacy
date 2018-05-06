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

#ifndef TEXTEVENT_H
#define TEXTEVENT_H

#include <Definitions.h>
#include <Colors.h>
#include <misc/SDL2pp.h>

#include <string>

#define TEXT_FADE_TIME 16

/**
    This class is used for showing text while playing a cutscene. It can show the video in the middle of the screen
    or at the bottom of the screen. The text can be faded in and out.
*/
class TextEvent final {
public:

    /**
        Constructor
        \param  text            the text to show
        \param  color           the color of the text
        \param  startFrame      the first frame relative to the current Scene start where the text should be shown
        \param  lengthInFrames  the number of frames the text shall be shown (if fading in/out is selected, the neccessary time for fading is included here)
        \param  bFadeIn         true = fade in the text (see TEXT_FADE_TIME for the number of frames it takes), false = simply show the text (default is true)
        \param  bFadeOut        true = fade out the text (see TEXT_FADE_TIME for the number of frames it takes), false = text simply disapears (default is true)
        \param  bCenterVertical true = center the text vertically on the screen, false = draw the text near the bottom of the screen (default is false)
    */
    TextEvent(const std::string& text, Uint32 color, int startFrame, int lengthInFrames, bool bFadeIn = true, bool bFadeOut = true, bool bCenterVertical = false);

    /// destructor
    ~TextEvent();

    /**
        This method draws the text.
        \param  currentFrameNumber  the current frame number relative to the current Scene start
    */
    void draw(int currentFrameNumber) const;

private:
    std::string text;       ///< the text to show
    int startFrame;         ///< the first frame relative to the current Scene start where the text should be shown
    int lengthInFrames;     ///< the number of frames the text shall be shown (if fading in/out is selected, the neccessary time for fading is included here)
    bool bFadeIn;           ///< true = fade in the text (see TEXT_FADE_TIME for the number of frames it takes), false = simply show the text
    bool bFadeOut;          ///< true = fade out the text (see TEXT_FADE_TIME for the number of frames it takes), false = text simply disapears
    bool bCenterVertical;   ///< true = center the text vertically on the screen, false = draw the text near the bottom of the screen
    sdl2::texture_ptr pTexture;  ///< a texture containing the rendered text
};

#endif // TEXTEVENT_H
