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

#ifndef DIGITSCOUNTER_H
#define DIGITSCOUNTER_H

#include <GUI/Widget.h>

#include <FileClasses/GFXManager.h>

#include <algorithm>
#include <memory>

extern std::unique_ptr<GFXManager> pGFXManager;


/// A widget for showing digits (like the credits in dune are shown)
class DigitsCounter : public Widget
{
public:

    /// default constructor
    DigitsCounter() {
        enableResizing(false,false);
        count = 0;
    }

    /// destructor
    virtual ~DigitsCounter() { ; };

    /**
        Get the current count of this digits counter
        \return the number that this digits counter currently shows
    */
    inline unsigned int getCount() { return count; }

    /**
        Set the count of this digits counter
        \param  newCount    the new number to show
    */
    inline void setCount(unsigned int newCount) { count = std::min(99u,newCount); }

    /**
        Draws this widget to screen. This method is called before drawOverlay().
        \param  position    Position to draw the widget to
    */
    inline void draw(Point position) override
    {
        SDL_Texture* tex = pGFXManager->getUIGraphic(UI_MissionSelect);

        SDL_Rect dest = calcDrawingRect(tex, position.x, position.y);
        SDL_RenderCopy(renderer, tex, nullptr, &dest);

        SDL_Texture* digitsTex = pGFXManager->getUIGraphic(UI_CreditsDigits);

        char creditsBuffer[3];
        snprintf(creditsBuffer, 3, "%u", count);
        int digits = strlen(creditsBuffer);

        for(int i=digits-1; i>=0; i--) {
            SDL_Rect source = calcSpriteSourceRect(digitsTex, creditsBuffer[i] - '0', 10);
            SDL_Rect dest2 = calcSpriteDrawingRect(digitsTex, position.x + 40 + (6 - digits + i)*10, position.y + 16, 10);
            SDL_RenderCopy(renderer, digitsTex, &source, &dest2);
        }

    };

    /**
        Returns the minimum size of this digits counter. The widget should not
        be resized to a size smaller than this.
        \return the minimum size of this digits counter
    */
    Point getMinimumSize() const override
    {
        SDL_Texture* tex = pGFXManager->getUIGraphic(UI_MissionSelect);
        if(tex != nullptr) {
            return getTextureSize(tex);
        } else {
            return Point(0,0);
        }
    }

private:

    unsigned int count;
};

#endif // DIGITSCOUNTER_H
