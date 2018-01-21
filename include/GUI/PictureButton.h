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

#ifndef PICTUREBUTTON_H
#define PICTUREBUTTON_H

#include "Button.h"

#include <SDL2/SDL.h>

/// A class for a picture button
class PictureButton : public Button {
public:
    /// Default contructor
    PictureButton() : Button() {
        enableResizing(false,false);
    }

    /// destructor
    virtual ~PictureButton() { ; };

    /**
        This method is used for setting the different surfaces for this button.
        \param  pUnpressedSurface       This surface is normally shown
        \param  bFreeUnpressedSurface   Should pUnpressedSurface be freed if this button is destroyed?
        \param  pPressedSurface         This surface is shown when the button is pressed
        \param  bFreePressedSurface     Should pPressedSurface be freed if this button is destroyed?
        \param  pActiveSurface          This surface is shown when the button is activated by keyboard or by mouse hover
        \param  bFreeActiveSurface      Should pActiveSurface be freed if this button is destroyed?
    */
    void setSurfaces(   SDL_Surface* pUnpressedSurface,bool bFreeUnpressedSurface,
                                SDL_Surface* pPressedSurface = nullptr,bool bFreePressedSurface = false,
                                SDL_Surface* pActiveSurface = nullptr,bool bFreeActiveSurface = false) override
    {
        Button::setSurfaces(pUnpressedSurface,bFreeUnpressedSurface,
                            pPressedSurface,bFreePressedSurface,
                            pActiveSurface,bFreeActiveSurface);

        if(pUnpressedSurface != nullptr) {
            resize(getTextureSize(pUnpressedTexture));
        } else {
            resize(0,0);
        }
    }

    /**
        This method is used for setting the different textures for this button.
        \param  pUnpressedTexture       This texture is normally shown
        \param  bFreeUnpressedTexture   Should pUnpressedTexture be freed if this button is destroyed?
        \param  pPressedTexture         This texture is shown when the button is pressed
        \param  bFreePressedTexture     Should pPressedTexture be freed if this button is destroyed?
        \param  pActiveTexture          This texture is shown when the button is activated by keyboard or by mouse hover
        \param  bFreeActiveTexture      Should pActiveTexture be freed if this button is destroyed?
    */
    void setTextures(   SDL_Texture* pUnpressedTexture,bool bFreeUnpressedTexture,
                                SDL_Texture* pPressedTexture = nullptr,bool bFreePressedTexture = false,
                                SDL_Texture* pActiveTexture = nullptr,bool bFreeActiveTexture = false) override
    {
        Button::setTextures(pUnpressedTexture,bFreeUnpressedTexture,
                            pPressedTexture,bFreePressedTexture,
                            pActiveTexture,bFreeActiveTexture);

        if(pUnpressedTexture != nullptr) {
            resize(getTextureSize(pUnpressedTexture));
        } else {
            resize(0,0);
        }
    }

    /**
        Returns the minimum size of this button. The button should not
        be resized to a size smaller than this.
        \return the minimum size of this button
    */
    Point getMinimumSize() const override
    {
        if(pUnpressedTexture != nullptr) {
            return getTextureSize(pUnpressedTexture);
        } else {
            return Point(0,0);
        }
    }
};

#endif //PICTUREBUTTON_H
