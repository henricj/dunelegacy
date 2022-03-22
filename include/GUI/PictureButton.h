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
#include <misc/SDL2pp.h>

/// A class for a picture button
class PictureButton : public Button {
public:
    /// Default constructor
    PictureButton() {
        enableResizing(false, false);
    }

    /// destructor
    virtual ~PictureButton() = default;

    /**
        This method is used for setting the different surfaces for this button.
        \param  pUnpressedSurface       This surface is normally shown
        \param  pPressedSurface         This surface is shown when the button is pressed
        \param  pActiveSurface          This surface is shown when the button is activated by keyboard or by mouse hover
    */
    virtual void setSurfaces(sdl2::surface_ptr pUnpressedSurface,
                             sdl2::surface_ptr pPressedSurface = nullptr,
                             sdl2::surface_ptr pActiveSurface  = nullptr) override {

        Button::setSurfaces(std::move(pUnpressedSurface), std::move(pPressedSurface), std::move(pActiveSurface));

        if (this->pUnpressedTexture) {
            resize(getTextureSize(pUnpressedTexture));
        } else {
            resize(0, 0);
        }
    }

    /**
        This method is used for setting the different textures for this button.
        \param  pUnpressedTexture       This texture is normally shown
        \param  pPressedTexture         This texture is shown when the button is pressed
        \param  pActiveTexture          This texture is shown when the button is activated by keyboard or by mouse hover
    */
    void setTextures(const DuneTexture* pUnpressedTexture, const DuneTexture* pPressedTexture = nullptr,
                     const DuneTexture* pActiveTexture = nullptr) override {
        Button::setTextures(pUnpressedTexture, pPressedTexture, pActiveTexture);

        if (pUnpressedTexture) {
            resize(getTextureSize(pUnpressedTexture));
        } else {
            resize(0, 0);
        }
    }

    /**
        Returns the minimum size of this button. The button should not
        be resized to a size smaller than this.
        \return the minimum size of this button
    */
    [[nodiscard]] Point getMinimumSize() const override {
        if (pUnpressedTexture) {
            return getTextureSize(pUnpressedTexture);
        }

        return Point(0, 0);
    }
};

#endif // PICTUREBUTTON_H
