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
#include "GUI/PictureButton.h"

PictureButton::PictureButton() {
    PictureButton::enableResizing(false, false);
}
PictureButton::~PictureButton() = default;

void PictureButton::setSurfaces(sdl2::surface_ptr pUnpressedSurface, sdl2::surface_ptr pPressedSurface,
                                sdl2::surface_ptr pActiveSurface) {

    parent::setSurfaces(std::move(pUnpressedSurface), std::move(pPressedSurface), std::move(pActiveSurface));

    if (this->pUnpressedTexture_) {
        resize(getTextureSize(pUnpressedTexture_));
    } else {
        resize(0, 0);
    }
}

void PictureButton::setTextures(const DuneTexture* pUnpressedTexture, const DuneTexture* pPressedTexture,
                                const DuneTexture* pActiveTexture) {
    parent::setTextures(pUnpressedTexture, pPressedTexture, pActiveTexture);

    if (pUnpressedTexture) {
        resize(getTextureSize(pUnpressedTexture));
    } else {
        resize(0, 0);
    }
}

void PictureButton::setTextures(DuneTextureOwned pUnpressedTexture, DuneTextureOwned pPressedTexture,
                                DuneTextureOwned pActiveTexture) {

    const auto width  = pUnpressedTexture ? static_cast<int>(std::ceil(pUnpressedTexture.width_)) : 0;
    const auto height = pUnpressedTexture ? static_cast<int>(std::ceil(pUnpressedTexture.height_)) : 0;

    parent::setTextures(std::move(pUnpressedTexture), std::move(pPressedTexture), std::move(pActiveTexture));

    resize(width, height);
}

Point PictureButton::getMinimumSize() const {
    if (pUnpressedTexture_) {
        return getTextureSize(pUnpressedTexture_);
    }

    return {0, 0};
}
