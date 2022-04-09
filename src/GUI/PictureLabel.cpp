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

#include <GUI/PictureLabel.h>

PictureLabel::PictureLabel() { }
PictureLabel::~PictureLabel() = default;

void PictureLabel::setSurface(sdl2::surface_unique_or_nonowning_ptr pSurface) {
    localTexture_.reset(); // Free the old one before we try to create another.
    localTexture_ = convertSurfaceToTexture(pSurface.get());

    privateDuneTexture_ = DuneTexture{localTexture_.get()};

    setTexture(&privateDuneTexture_);
}

void PictureLabel::setTexture(const DuneTexture* pTexture) {
    this->pTexture = pTexture;

    if (this->pTexture) {
        resize(getTextureSize(this->pTexture));
    } else {
        resize(0, 0);
    }

    if (this->pTexture->texture_ != localTexture_.get())
        localTexture_.reset();
}

Point PictureLabel::getMinimumSize() const {
    if (pTexture) {
        return getTextureSize(pTexture);
    }

    return {0, 0};
}

void PictureLabel::draw(Point position) {
    if (isVisible() == false) {
        return;
    }

    if (!pTexture) {
        return;
    }

    pTexture->draw(renderer, position.x, position.y);
}
