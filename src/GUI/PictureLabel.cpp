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

    size_ = Point(duneTexture_.source_rect().w, duneTexture_.source_rect().h);

    resize(size_);
}

void PictureLabel::setTexture(const DuneTexture* pTexture) {
    assert(pTexture != &duneTexture_);

    duneTexture_ = *pTexture;

    localTexture_.reset();

    if (duneTexture_.texture_) {
        size_ = Point(duneTexture_.source_rect().w, duneTexture_.source_rect().h);
    } else {
        size_ = Point{};
    }

    resize(size_);
}

Point PictureLabel::getMinimumSize() const {
    return size_;
}

void PictureLabel::draw(Point position) {
    if (isVisible() == false)
        return;

    if (!duneTexture_.texture_)
        return;

    const SDL_Rect dest{position.x, position.y, size_.x, size_.y};

    Dune_RenderCopy(renderer, &duneTexture_, nullptr, &dest);
}
