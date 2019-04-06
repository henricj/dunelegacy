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

#include <GUI/TextView.h>

#include <misc/string_util.h>

#include <algorithm>

TextView::TextView() : Widget() {
    enableResizing(true,true);

    resize(TextView::getMinimumSize().x, TextView::getMinimumSize().y);
}

TextView::~TextView() = default;

void TextView::handleMouseMovement(Sint32 x, Sint32 y, bool insideOverlay) {
    scrollbar.handleMouseMovement(x - getSize().x + scrollbar.getSize().x,y,insideOverlay);
}

bool TextView::handleMouseLeft(Sint32 x, Sint32 y, bool pressed) {
    return scrollbar.handleMouseLeft(x - getSize().x + scrollbar.getSize().x,y,pressed);
}

bool TextView::handleMouseWheel(Sint32 x, Sint32 y, bool up)  {
    // forward mouse wheel event to scrollbar
    return scrollbar.handleMouseWheel(0,0,up);
}

bool TextView::handleKeyPress(SDL_KeyboardEvent& key) {
    Widget::handleKeyPress(key);

    scrollbar.handleKeyPress(key);
    return true;
}

void TextView::draw(Point position) {
    if(isVisible() == false) {
        return;
    }

    updateTextures();

    if(pBackground != nullptr) {
        SDL_Rect dest = calcDrawingRect(pBackground.get(), position.x, position.y);
        SDL_RenderCopy(renderer, pBackground.get(), nullptr, &dest);
    }

    if(pForeground != nullptr) {
        int lineHeight = GUIStyle::getInstance().getTextHeight(fontSize) + 2;

        SDL_Rect src = {    0,
                            scrollbar.getCurrentValue() * lineHeight,
                            getWidth(pForeground.get()),
                            std::min(getHeight(pForeground.get()), getSize().y - 2) };

        SDL_Rect dest = {   position.x + 2,
                            position.y + 1,
                            getWidth(pForeground.get()),
                            std::min(getHeight(pForeground.get()), getSize().y - 2) };
        SDL_RenderCopy(renderer, pForeground.get(), &src, &dest);
    }

    Point scrollBarPos = position;
    scrollBarPos.x += getSize().x - scrollbar.getSize().x;

    if(!bAutohideScrollbar || (scrollbar.getRangeMin() != scrollbar.getRangeMax())) {
        scrollbar.draw(scrollBarPos);
    }
}

void TextView::resize(Uint32 width, Uint32 height) {
    invalidateTextures();

    scrollbar.resize(scrollbar.getMinimumSize().x,height);

    int fontSize = this->fontSize;
    std::vector<std::string> textLines = greedyWordWrap(text,
                                                        getSize().x - scrollbar.getSize().x - 4,
                                                        [fontSize](const std::string& tmp) {
                                                            return GUIStyle::getInstance().getMinimumLabelSize(tmp, fontSize).x - 4;
                                                        });

    int lineHeight = GUIStyle::getInstance().getTextHeight(fontSize) + 2;

    int numVisibleLines = height/lineHeight;
    scrollbar.setRange(0,std::max(0, ((int) textLines.size()) - numVisibleLines));
    scrollbar.setBigStepSize(std::max(1, numVisibleLines-1));

    Widget::resize(width,height);
}

void TextView::updateTextures() {
    if(pBackground == nullptr) {
        pBackground = convertSurfaceToTexture(GUIStyle::getInstance().createWidgetBackground(getSize().x, getSize().y));
    }

    if(pForeground == nullptr) {
        int fontSize = this->fontSize;
        std::vector<std::string> textLines = greedyWordWrap(text,
                                                            getSize().x - scrollbar.getSize().x - 4,
                                                            [fontSize](const std::string& tmp) {
                                                                return GUIStyle::getInstance().getMinimumLabelSize(tmp, fontSize).x - 4;
                                                            });

        int lineHeight = GUIStyle::getInstance().getTextHeight(fontSize) + 2;
        int labelHeight = lineHeight * textLines.size() + 2;
        pForeground = convertSurfaceToTexture(GUIStyle::getInstance().createLabelSurface(getSize().x-4,labelHeight,textLines,fontSize,alignment,textcolor,textshadowcolor,backgroundcolor));
    }
}
