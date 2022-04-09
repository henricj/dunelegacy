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
class PictureLabel final : public Widget {
public:
    /// default constructor
    PictureLabel();

    /// destructor
    ~PictureLabel() override;

    /**
        This method sets the surface for this picture label.
        \param  pSurface    This surface is shown
    */
    virtual void setSurface(sdl2::surface_unique_or_nonowning_ptr pSurface);

    /**
        This method sets the texture for this picture label.
        \param  pTexture        This texture is shown
    */
    virtual void setTexture(const DuneTexture* pTexture);

    /**
        This method sets the texture for this picture label.
        \param  texture        This texture is shown
    */
    virtual void setOwningTexture(DuneTexture texture) {
        localTexture_.reset(texture.texture_);
        privateDuneTexture_ = std::move(texture);

        setTexture(&privateDuneTexture_);
    }

    /**
        Returns the minimum size of this picture label. The picture label should not
        be resized to a size smaller than this.
        \return the minimum size of this picture label
    */
    [[nodiscard]] Point getMinimumSize() const override;

    /**
        Draws this button to screen. This method is called before drawOverlay().
        \param  position    Position to draw the button to
    */
    void draw(Point position) override;

private:
    const DuneTexture* pTexture{}; ///< The texture that is shown
    sdl2::texture_ptr localTexture_;
    DuneTexture privateDuneTexture_;
};

#endif // PICTURELABEL_H
