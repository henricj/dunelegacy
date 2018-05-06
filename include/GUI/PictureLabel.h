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
#include <misc/draw_util.h>
#include <misc/SDL2pp.h>

/// A class for showning a static picture
class PictureLabel : public Widget {
public:

    /// default constructor
    PictureLabel() {
        bFreeTexture = false;
    }

    /// destructor
    virtual ~PictureLabel() {
        if (!bFreeTexture)
            pTexture.release();
    }

    /**
        This method sets the surface for this picture label.
        \param  pSurface    This surface is shown
        \param  bFreeSurface    Should pSurface be freed if this picture label is destroyed?
    */
    virtual void setSurface(SDL_Surface* pSurface, bool bFreeSurface) {
        setTexture(convertSurfaceToTexture(pSurface, bFreeSurface));
    }

    /**
        This method sets the texture for this picture label.
        \param  pTexture        This texture is shown
        \param  bFreeTexture    Should pTexture be freed if this picture label is destroyed?
    */
    virtual void setTexture(SDL_Texture* pTexture, bool bFreeTexture) {
        if ((this->bFreeTexture == false)) {
            this->pTexture.release();
        }

        this->bFreeTexture = bFreeTexture;
        this->pTexture = sdl2::texture_ptr{ pTexture };

        if(this->pTexture != nullptr) {
            resize(getTextureSize(this->pTexture.get()));
        } else {
            resize(0,0);
        }
    }

    void setTexture(sdl2::texture_ptr&& texture) {
        setTexture(texture.release(), true);
    }

    /**
        Returns the minimum size of this picture label. The picture label should not
        be resized to a size smaller than this.
        \return the minimum size of this picture label
    */
    Point getMinimumSize() const override
    {
        if(pTexture != nullptr) {
            return getTextureSize(pTexture.get());
        } else {
            return Point(0,0);
        }
    }

    /**
        Draws this button to screen. This method is called before drawOverlay().
        \param  position    Position to draw the button to
    */
    void draw(Point position) override
    {
        if(isVisible() == false) {
            return;
        }

        if(pTexture == nullptr) {
            return;
        }

        SDL_Rect dest = calcDrawingRect(pTexture.get(), position.x, position.y);
        SDL_RenderCopy(renderer, pTexture.get(), nullptr, &dest);
    }


private:
    sdl2::texture_ptr pTexture;  ///< The texture that is shown
    bool bFreeTexture;      ///< Should pTexture be freed if this picture label is destroyed?
};

#endif // PICTURELABEL_H
