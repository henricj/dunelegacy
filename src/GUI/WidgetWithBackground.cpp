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

#include <GUI/WidgetWithBackground.h>

WidgetWithBackground::~WidgetWithBackground() = default;


void WidgetWithBackground::setTransparentBackground(bool bTransparent) { bTransparentBackground = bTransparent; }


void WidgetWithBackground::setBackground(const DuneTexture* pBackground) {
    setBackground(static_cast<SDL_Surface*>(nullptr));

    if(!pBackground || !*pBackground) {
        bSelfGeneratedBackground = true;
        this->pBackground        = nullptr;
    } else {
        bSelfGeneratedBackground = false;
        this->pBackground        = pBackground;
    }
}

void WidgetWithBackground::resize(Uint32 width, Uint32 height)
{
    Widget::resize(width, height);

    if(bSelfGeneratedBackground) {
        localDuneTexture_ = DuneTexture{};
        localTexture_.reset();
        pBackground = nullptr;
    }
}

void WidgetWithBackground::draw(Point position)
{
    const auto* background = getBackground();
    if(!background) return;

    SDL_Rect dst{position.x, position.y, background->source_.w, background->source_.h};

    const auto size = getSize();

    if(dst.w != size.x) dst.x += (size.x - dst.w) / 2;
    if(dst.h != size.y) dst.y += (size.y - dst.h) / 2;

    background->draw(renderer, dst.x, dst.y);
}

void WidgetWithBackground::invalidateTextures()
{
    pBackground = nullptr;

    localDuneTexture_ = DuneTexture{};
    localTexture_.reset();

    Widget::invalidateTextures();
}

sdl2::surface_ptr WidgetWithBackground::createBackground() {
    return GUIStyle::getInstance().createBackground(getSize().x, getSize().y);
}

const DuneTexture* WidgetWithBackground::getBackground() {
    if(bTransparentBackground) return nullptr;

    if(bSelfGeneratedBackground && (!pBackground || !*pBackground || pBackground->source_.w != getSize().x || pBackground->source_.h != getSize().y)) {
        const auto surface = createBackground();

        setBackground(surface.get());
    }

    return pBackground && *pBackground ? pBackground : nullptr;
}

void WidgetWithBackground::setBackground(SDL_Surface* surface) {
    if(surface) {
        localTexture_     = convertSurfaceToTexture(surface);
        localDuneTexture_ = DuneTexture{localTexture_.get()};
        pBackground       = &localDuneTexture_;
    } else {
        localDuneTexture_ = DuneTexture{};
        localTexture_.reset();
        pBackground = nullptr;
    }
}

