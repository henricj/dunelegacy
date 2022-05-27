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

#include "GUI/Spacer.h"

Spacer::Spacer() {
    Spacer::enableResizing(true, true);
}

Spacer::~Spacer() = default;

Point Spacer::getMinimumSize() const {
    return {0, 0};
}

HSpacer::HSpacer() {
    HSpacer::resize(0, 0);
    HSpacer::enableResizing(false, false);
}

HSpacer::HSpacer(int32_t width) : width_{width} {
    HSpacer::resize(width, 0);
    HSpacer::enableResizing(false, false);
}

HSpacer::~HSpacer() = default;

Point HSpacer::getMinimumSize() const {
    return {width_, 0};
}

VSpacer::VSpacer() {
    VSpacer::resize(0, 0);
    VSpacer::enableResizing(false, false);
}

VSpacer::VSpacer(int32_t height) : height_{height} {
    VSpacer::resize(0, height);
    VSpacer::enableResizing(false, false);
}

VSpacer::~VSpacer() = default;

Point VSpacer::getMinimumSize() const {
    return {0, height_};
}
