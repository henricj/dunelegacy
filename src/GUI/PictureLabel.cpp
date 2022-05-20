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

PictureLabel::PictureLabel()  = default;
PictureLabel::~PictureLabel() = default;

void PictureLabel::setSurface(sdl2::surface_unique_or_nonowning_ptr pSurface) {
    localTexture_.reset(); // Free the old one before we try to create another.
    localTexture_ = convertSurfaceToTexture(pSurface.get());

    duneTexture_ = DuneTexture{localTexture_.get()};

    resize(getMinimumSize());
}

void PictureLabel::setTexture(const DuneTexture* pTexture) {
    assert(pTexture != &duneTexture_);

    localTexture_.reset();

    duneTexture_ = *pTexture;

    resize(getMinimumSize());
}

Point PictureLabel::getMinimumSize() const {
    return {static_cast<int>(std::ceil(duneTexture_.width_)), static_cast<int>(std::ceil(duneTexture_.height_))};
}

void PictureLabel::draw(Point position) {
    if (isVisible() == false)
        return;

    if (!duneTexture_.texture_)
        return;

    duneTexture_.draw(dune::globals::renderer.get(), static_cast<float>(position.x), static_cast<float>(position.y));
}
