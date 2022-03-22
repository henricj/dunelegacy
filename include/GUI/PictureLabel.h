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

#ifndef PICTURELABEL_H
#define PICTURELABEL_H

#include "Widget.h"
#include <misc/SDL2pp.h>
#include <misc/draw_util.h>

/// A class for showing a static picture
class PictureLabel : public Widget {
public:
    /// default constructor
    PictureLabel();

    /// destructor
    virtual ~PictureLabel() = default;

    /**
        This method sets the surface for this picture label.
        \param  pSurface    This surface is shown
    */
    virtual void setSurface(sdl2::surface_unique_or_nonowning_ptr pSurface) {
        localTexture_.reset(); // Free the old one before we try to create another.
        localTexture_ = convertSurfaceToTexture(pSurface.get());

        privateDuneTexture_ = DuneTexture {localTexture_.get()};

        setTexture(&privateDuneTexture_);
    }

    /**
        This method sets the texture for this picture label.
        \param  pTexture        This texture is shown
    */
    virtual void setTexture(const DuneTexture* pTexture) {
        this->pTexture = pTexture;

        if (this->pTexture) {
            resize(getTextureSize(this->pTexture));
        } else {
            resize(0, 0);
        }

        if (this->pTexture->texture_ != localTexture_.get())
            localTexture_.reset();
    }

    /**
        Returns the minimum size of this picture label. The picture label should not
        be resized to a size smaller than this.
        \return the minimum size of this picture label
    */
    [[nodiscard]] Point getMinimumSize() const override {
        if (pTexture) {
            return getTextureSize(pTexture);
        }

        return Point(0, 0);
    }

    /**
        Draws this button to screen. This method is called before drawOverlay().
        \param  position    Position to draw the button to
    */
    void draw(Point position) override {
        if (isVisible() == false) {
            return;
        }

        if (!pTexture) {
            return;
        }

        pTexture->draw(renderer, position.x, position.y);
    }

private:
    const DuneTexture* pTexture {}; ///< The texture that is shown
    sdl2::texture_ptr localTexture_;
    DuneTexture privateDuneTexture_;
};

#endif // PICTURELABEL_H
