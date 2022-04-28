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

#include "misc/DrawingRectHelper.h"

#include <algorithm>

DropDownBox::DropDownBox() {
    DropDownBox::enableResizing(true, false);

    updateButtonSurface();

    openListBoxButton.setOnClick([this] { onOpenListBoxButton(); });

    listBox.setOnSelectionChange([this](auto flag) { onSelectionChange(flag); });

    resize(DropDownBox::getMinimumSize().x, DropDownBox::getMinimumSize().y);
}

DropDownBox::~DropDownBox() = default;

void DropDownBox::handleMouseMovement(int32_t x, int32_t y, bool insideOverlay) {
    if ((x < 0) || (x >= getSize().x - openListBoxButton.getSize().x - 1) || (y < 0) || (y >= getSize().y)) {
        bHover = false;
    } else if ((isEnabled() || (bOnClickEnabled && pOnClick)) && !insideOverlay) {
        bHover = true;
    } else {
        bHover = false;
    }

    openListBoxButton.handleMouseMovement(x - (getSize().x - openListBoxButton.getSize().x - 1), y - 1, insideOverlay);

    if (bShowListBox) {
        listBox.handleMouseMovement(x, bListBoxAbove ? (y + listBox.getSize().y) : (y - getSize().y), insideOverlay);
    }
}

bool DropDownBox::handleMouseMovementOverlay(int32_t x, int32_t y) {
    const int newY = bListBoxAbove ? (y + listBox.getSize().y) : (y - getSize().y);
    return bShowListBox && x >= 0 && x < listBox.getSize().x && newY >= 0 && newY < listBox.getSize().y;
}

bool DropDownBox::handleMouseLeft(int32_t x, int32_t y, bool pressed) {
    if ((!isEnabled()) || (!isVisible())) {
        // onClick works even when widget is disabled
        if (bOnClickEnabled && isVisible() && pOnClick) {
            if ((x >= 0) && (x < getSize().x - openListBoxButton.getSize().x - 1) && (y >= 0) && (y < getSize().y)
                && (pressed)) {
                pOnClick();
            }
        }

        return true;
    }

    if (openListBoxButton.handleMouseLeft(x - (getSize().x - openListBoxButton.getSize().x - 1), y - 1, pressed)) {
        setActive();
        return true;
    }
    if ((x >= 0) && (x < getSize().x - openListBoxButton.getSize().x - 1)

        && (y >= 0) && (y < getSize().y) && (pressed)) {

        if (bOnClickEnabled && pOnClick) {

            pOnClick();

        } else {

            setActive();

            onOpenListBoxButton();
        }

        return true;
    }
    return false;
}

bool DropDownBox::handleMouseLeftOverlay(int32_t x, int32_t y, bool pressed) {
    if ((!isEnabled()) || (!isVisible())) {
        return false;
    }

    if (bShowListBox) {
        if (!listBox.handleMouseLeft(x, bListBoxAbove ? (y + listBox.getSize().y) : (y - getSize().y), pressed)) {
            if (x < 0 || x >= getSize().x || y < 0 || y >= getSize().y) {
                // if not on drop down box => click is handled by closing drop down box
                bShowListBox = false;
                return true;
            } // on drop down box we don't handle overlay click

            return false;
        }
        return true;
    }
    return false;
}

bool DropDownBox::handleMouseWheel(int32_t x, int32_t y, bool up) {
    if ((!isEnabled()) || (!isVisible())) {
        return false;
    }

    // forward mouse wheel event to list box
    if (x >= 0 && x < getSize().x && y >= 0 && y < getSize().y) {
        if (up) {
            if (listBox.getSelectedIndex() < 0) {
                listBox.setSelectedItem(0, true);
            } else if (listBox.getSelectedIndex() > 0) {
                listBox.setSelectedItem(listBox.getSelectedIndex() - 1, true);
            }
        } else {
            if (listBox.getSelectedIndex() < 0) {
                listBox.setSelectedItem(0, true);
            } else if (listBox.getSelectedIndex() < listBox.getNumEntries() - 1) {
                listBox.setSelectedItem(listBox.getSelectedIndex() + 1, true);
            }
        }
        return true;
    }
    return false;
}

bool DropDownBox::handleMouseWheelOverlay(int32_t x, int32_t y, bool up) {
    if ((!isEnabled()) || (!isVisible())) {
        return false;
    }

    // forward mouse wheel event to list box
    if (bShowListBox) {
        const auto newY = bListBoxAbove ? (y + listBox.getSize().y) : (y - getSize().y);
        listBox.handleMouseWheel(x, newY, up);
        return x >= 0 && x < listBox.getSize().x && newY >= 0 && newY < listBox.getSize().y;
    }
    return false;
}

bool DropDownBox::handleKeyPress(SDL_KeyboardEvent& key) {
    if ((!isEnabled()) || (!isVisible())) {
        return false;
    }

    Widget::handleKeyPress(key);
    if (isActive()) {
        // disable autoclosing of the list box
        const bool bSavedAutoclose         = bAutocloseListBoxOnSelectionChange;
        bAutocloseListBoxOnSelectionChange = false;

        switch (key.keysym.sym) {
            case SDLK_UP: {
                if (listBox.getSelectedIndex() < 0) {
                    listBox.setSelectedItem(0, true);
                } else if (listBox.getSelectedIndex() > 0) {
                    listBox.setSelectedItem(listBox.getSelectedIndex() - 1, true);
                }
            } break;

            case SDLK_DOWN: {
                if (listBox.getSelectedIndex() < 0) {
                    listBox.setSelectedItem(0, true);
                } else if (listBox.getSelectedIndex() < listBox.getNumEntries() - 1) {
                    listBox.setSelectedItem(listBox.getSelectedIndex() + 1, true);
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
    if (!isVisible())
        return;

    updateBackground();

    auto* const renderer = dune::globals::renderer.get();

    if (pBackground)
        pBackground.draw(renderer, position.x, position.y);

    updateForeground();

    if (pForeground && pActiveForeground) {
        if ((bHover && pOnClick) || isActive()) {
            pActiveForeground.draw(renderer, position.x + 2, position.y + 2);
        } else {
            pForeground.draw(renderer, position.x + 2, position.y + 2);
        }
    }

    openListBoxButton.draw(position + Point(getSize().x - openListBoxButton.getSize().x - 1, 1));
}

void DropDownBox::drawOverlay(Point position) {
    if (bShowListBox) {
        bListBoxAbove = (position.y + listBox.getSize().y > getRendererHeight());
        listBox.draw(position + Point(0, bListBoxAbove ? -listBox.getSize().y : getSize().y));
    }
}

void DropDownBox::resize(uint32_t width, uint32_t height) {
    Widget::resize(width, height);

    invalidateForeground();
    invalidateBackground();

    resizeListBox();
}

void DropDownBox::resizeListBox() {
    const int listBoxHeight = std::max(1, std::min(numVisibleEntries, getNumEntries()))
                                * static_cast<int>(GUIStyle::getInstance().getListBoxEntryHeight())
                            + 2;
    listBox.resize(getSize().x - 1, listBoxHeight);
}

void DropDownBox::setActive(bool bActive) {
    if (!bActive) {
        bShowListBox = false;
        openListBoxButton.setInactive();
    } else {
        openListBoxButton.setActive();
    }
    Widget::setActive(bActive);
}

void DropDownBox::onSelectionChange(bool bInteractive) {
    invalidateForeground();

    if (bAutocloseListBoxOnSelectionChange) {
        bShowListBox = false;
    }

    if (pOnSelectionChange) {
        pOnSelectionChange(bInteractive);
    }
}

void DropDownBox::updateButtonSurface() {
    const auto& gui = GUIStyle::getInstance();

    auto* const renderer = dune::globals::renderer.get();

    openListBoxButton.setTextures(gui.createDropDownBoxButton(17, false, false, color).createTexture(renderer),
                                  gui.createDropDownBoxButton(17, true, true, color).createTexture(renderer),
                                  gui.createDropDownBoxButton(17, false, true, color).createTexture(renderer));
}

void DropDownBox::invalidateForeground() {
    pForeground.reset();
    pActiveForeground.reset();
}

void DropDownBox::updateForeground() {
    if (pForeground || pActiveForeground)
        return;

    if (listBox.getSelectedIndex() < 0)
        return;

    const auto& gui = GUIStyle::getInstance();

    const auto width = getSize().x - 17;
    const auto text  = listBox.getEntry(listBox.getSelectedIndex());

    auto* const renderer = dune::globals::renderer.get();

    pForeground       = gui.createListBoxEntry(width, text, false, color).createTexture(renderer);
    pActiveForeground = gui.createListBoxEntry(width, text, true, color).createTexture(renderer);
}

void DropDownBox::invalidateBackground() {
    pBackground.reset();
}

void DropDownBox::updateBackground() {
    if (pBackground)
        return;

    pBackground = GUIStyle::getInstance()
                      .createWidgetBackground(getSize().x, getSize().y)
                      .createTexture(dune::globals::renderer.get());
}
