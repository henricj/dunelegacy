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

#include <GUI/DropDownBox.h>

#include <algorithm>

DropDownBox::DropDownBox() {
    enableResizing(true,false);

    numVisibleEntries = 7;

    color = COLOR_DEFAULT;
    bHover = false;
    updateButtonSurface();

    openListBoxButton.setOnClick(std::bind(&DropDownBox::onOpenListBoxButton, this));

    listBox.setOnSelectionChange(std::bind(&DropDownBox::onSelectionChange, this, std::placeholders::_1));

    pBackground = nullptr;
    pForeground = nullptr;
    pActiveForeground = nullptr;

    bShowListBox = false;
    bListBoxAbove = false;
    bAutocloseListBoxOnSelectionChange = true;
    bOnClickEnabled = true;

    resize(DropDownBox::getMinimumSize().x, DropDownBox::getMinimumSize().y);
}

DropDownBox::~DropDownBox() = default;

void DropDownBox::handleMouseMovement(Sint32 x, Sint32 y, bool insideOverlay) {
    if((x < 0) || (x >= getSize().x - openListBoxButton.getSize().x - 1) || (y < 0) || (y >= getSize().y)) {
        bHover = false;
    } else if((isEnabled() || (bOnClickEnabled && pOnClick)) && !insideOverlay) {
        bHover = true;
    } else {
        bHover = false;
    }

    openListBoxButton.handleMouseMovement(x - (getSize().x - openListBoxButton.getSize().x - 1), y - 1, insideOverlay);

    if(bShowListBox) {
        listBox.handleMouseMovement(x, bListBoxAbove ? (y + listBox.getSize().y) : (y - getSize().y), insideOverlay);
    }
}

bool DropDownBox::handleMouseMovementOverlay(Sint32 x, Sint32 y) {
    int newY = bListBoxAbove ? (y + listBox.getSize().y) : (y - getSize().y);
    return bShowListBox && x >= 0 && x < listBox.getSize().x && newY >= 0 && newY < listBox.getSize().y;
}

bool DropDownBox::handleMouseLeft(Sint32 x, Sint32 y, bool pressed) {
    if((isEnabled() == false) || (isVisible() == false)) {
        // onClick works even when widget is disabled
        if(bOnClickEnabled && isVisible() && pOnClick) {
            if((x>=0) && (x < getSize().x - openListBoxButton.getSize().x - 1)
                && (y>=0) && (y < getSize().y) && (pressed == true)) {
                pOnClick();
            }
        }

        return true;
    }

    if(openListBoxButton.handleMouseLeft(x - (getSize().x - openListBoxButton.getSize().x - 1), y - 1, pressed)) {
        setActive();
        return true;
    } else {
        if((x>=0) && (x < getSize().x - openListBoxButton.getSize().x - 1)
            && (y>=0) && (y < getSize().y) && (pressed == true)) {

            if(bOnClickEnabled && pOnClick) {
                pOnClick();
            } else {
                setActive();
                onOpenListBoxButton();
            }
            return true;
        } else {
            return false;
        }
    }
}

bool DropDownBox::handleMouseLeftOverlay(Sint32 x, Sint32 y, bool pressed) {
    if((isEnabled() == false) || (isVisible() == false)) {
        return false;
    }

    if(bShowListBox) {
        if(listBox.handleMouseLeft(x, bListBoxAbove ? (y + listBox.getSize().y) : (y - getSize().y), pressed) == false) {
            if(x < 0 || x >= getSize().x || y < 0 || y >= getSize().y) {
                // if not on drop down box => click is handled by closing drop down box
                bShowListBox = false;
                return true;
            } else {
                // on drop down box we don't handle overlay click
                return false;
            }
        } else {
            return true;
        }
    } else {
        return false;
    }
}

bool DropDownBox::handleMouseWheel(Sint32 x, Sint32 y, bool up) {
    if((isEnabled() == false) || (isVisible() == false)) {
        return false;
    }

    // forward mouse wheel event to list box
    if(x >= 0 && x < getSize().x && y >= 0 && y < getSize().y) {
        if(up) {
            if(listBox.getSelectedIndex() < 0) {
                listBox.setSelectedItem(0,true);
            } else if(listBox.getSelectedIndex() > 0) {
                listBox.setSelectedItem(listBox.getSelectedIndex()-1,true);
            }
        } else {
            if(listBox.getSelectedIndex() < 0) {
                listBox.setSelectedItem(0,true);
            } else if(listBox.getSelectedIndex() < listBox.getNumEntries()-1) {
                listBox.setSelectedItem(listBox.getSelectedIndex()+1,true);
            }
        }
        return true;
    } else {
        return false;
    }
}

bool DropDownBox::handleMouseWheelOverlay(Sint32 x, Sint32 y, bool up) {
    if((isEnabled() == false) || (isVisible() == false)) {
        return false;
    }

    // forward mouse wheel event to list box
    if(bShowListBox) {
        int newY = bListBoxAbove ? (y + listBox.getSize().y) : (y - getSize().y);
        listBox.handleMouseWheel(x,newY,up);
        return x >= 0 && x < listBox.getSize().x && newY >= 0 && newY < listBox.getSize().y;
    } else {
        return false;
    }
}

bool DropDownBox::handleKeyPress(SDL_KeyboardEvent& key) {
    if((isEnabled() == false) || (isVisible() == false)) {
        return false;
    }

    Widget::handleKeyPress(key);
    if(isActive()) {
        // disable autoclosing of the list box
        bool bSavedAutoclose = bAutocloseListBoxOnSelectionChange;
        bAutocloseListBoxOnSelectionChange = false;

        switch(key.keysym.sym) {
            case SDLK_UP: {
                if(listBox.getSelectedIndex() < 0) {
                    listBox.setSelectedItem(0,true);
                } else if(listBox.getSelectedIndex() > 0) {
                    listBox.setSelectedItem(listBox.getSelectedIndex()-1,true);
                }
            } break;

            case SDLK_DOWN: {
                if(listBox.getSelectedIndex() < 0) {
                    listBox.setSelectedItem(0,true);
                } else if(listBox.getSelectedIndex() < listBox.getNumEntries()-1) {
                    listBox.setSelectedItem(listBox.getSelectedIndex()+1,true);
                }
            } break;

            case SDLK_SPACE: {
                bShowListBox = !bShowListBox;
            } break;

            case SDLK_TAB: {
                setInactive();
            } break;

            default: {
            } break;
        }

        bAutocloseListBoxOnSelectionChange = bSavedAutoclose;
        return true;
    }

    return false;

}

void DropDownBox::draw(Point position) {
    if(isVisible() == false) {
        return;
    }

    updateBackground();

    if(pBackground != nullptr) {
        SDL_Rect dest = calcDrawingRect(pBackground.get(), position.x, position.y);
        SDL_RenderCopy(renderer, pBackground.get(), nullptr, &dest);
    }

    updateForeground();

    if(pForeground != nullptr && pActiveForeground != nullptr) {
        if(((bHover == true) && pOnClick) || isActive()) {
            SDL_Rect dest = calcDrawingRect(pActiveForeground.get(), position.x + 2, position.y + 2);
            SDL_RenderCopy(renderer, pActiveForeground.get(), nullptr, &dest);
        } else {
            SDL_Rect dest = calcDrawingRect(pForeground.get(), position.x + 2, position.y + 2);
            SDL_RenderCopy(renderer, pForeground.get(), nullptr, &dest);
        }
    }

    openListBoxButton.draw(position + Point(getSize().x - openListBoxButton.getSize().x - 1, 1));
}

void DropDownBox::drawOverlay(Point position) {
    if(bShowListBox) {
        bListBoxAbove = (position.y + listBox.getSize().y > getRendererHeight());
        listBox.draw(position + Point(0,bListBoxAbove ? -listBox.getSize().y : getSize().y));
    }
}

void DropDownBox::resize(Uint32 width, Uint32 height) {
    Widget::resize(width,height);

    invalidateForeground();
    invalidateBackground();

    resizeListBox();
}

void DropDownBox::resizeListBox() {
    int listBoxHeight = std::max(1,std::min(numVisibleEntries,getNumEntries())) * static_cast<int>(GUIStyle::getInstance().getListBoxEntryHeight()) + 2;
    listBox.resize(getSize().x-1, listBoxHeight);
}

void DropDownBox::setActive(bool bActive) {
    if(bActive == false) {
        bShowListBox = false;
        openListBoxButton.setInactive();
    } else {
        openListBoxButton.setActive();
    }
    Widget::setActive(bActive);
}

void DropDownBox::onSelectionChange(bool bInteractive) {
    invalidateForeground();

    if(bAutocloseListBoxOnSelectionChange) {
        bShowListBox = false;
    }

    if(pOnSelectionChange) {
        pOnSelectionChange(bInteractive);
    }
}

void DropDownBox::updateButtonSurface() {
    openListBoxButton.setSurfaces(  GUIStyle::getInstance().createDropDownBoxButton(17,false,false,color),
                                    GUIStyle::getInstance().createDropDownBoxButton(17,true,true,color),
                                    GUIStyle::getInstance().createDropDownBoxButton(17,false,true,color));
}

void DropDownBox::invalidateForeground() {
    pForeground.reset();
    pActiveForeground.reset();
}

void DropDownBox::updateForeground() {
    if(!pForeground && !pActiveForeground) {
        if(listBox.getSelectedIndex() >= 0) {
            pForeground = convertSurfaceToTexture(GUIStyle::getInstance().createListBoxEntry(getSize().x - 17, listBox.getEntry(listBox.getSelectedIndex()), false, color));
            pActiveForeground = convertSurfaceToTexture(GUIStyle::getInstance().createListBoxEntry(getSize().x - 17, listBox.getEntry(listBox.getSelectedIndex()), true, color));
        }
    }
}

void DropDownBox::invalidateBackground() {
    pBackground.reset();
}

void DropDownBox::updateBackground() {
    if(!pBackground) {
        pBackground = convertSurfaceToTexture(GUIStyle::getInstance().createWidgetBackground(getSize().x, getSize().y));
    }
}
