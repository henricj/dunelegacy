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

#include <gsl/gsl>

#include <algorithm>

DropDownBox::DropDownBox() {
    DropDownBox::enableResizing(true, false);

    updateButtonSurface();

    openListBoxButton_.setOnClick([this] { onOpenListBoxButton(); });

    listBox_.setOnSelectionChange([this](auto flag) { onSelectionChange(flag); });

    resize(DropDownBox::getMinimumSize().x, DropDownBox::getMinimumSize().y);
}

DropDownBox::~DropDownBox() = default;

void DropDownBox::handleMouseMovement(int32_t x, int32_t y, bool insideOverlay) {
    if ((x < 0) || (x >= getSize().x - openListBoxButton_.getSize().x - 1) || (y < 0) || (y >= getSize().y)) {
        bHover_ = false;
    } else if ((isEnabled() || (bOnClickEnabled_ && pOnClick_)) && !insideOverlay) {
        bHover_ = true;
    } else {
        bHover_ = false;
    }

    openListBoxButton_.handleMouseMovement(x - (getSize().x - openListBoxButton_.getSize().x - 1), y - 1,
                                           insideOverlay);

    if (bShowListBox_) {
        listBox_.handleMouseMovement(x, bListBoxAbove_ ? (y + listBox_.getSize().y) : (y - getSize().y), insideOverlay);
    }
}

bool DropDownBox::handleMouseMovementOverlay(int32_t x, int32_t y) {
    const int newY = bListBoxAbove_ ? (y + listBox_.getSize().y) : (y - getSize().y);
    return bShowListBox_ && x >= 0 && x < listBox_.getSize().x && newY >= 0 && newY < listBox_.getSize().y;
}

bool DropDownBox::handleMouseLeft(int32_t x, int32_t y, bool pressed) {
    if ((!isEnabled()) || (!isVisible())) {
        // onClick works even when widget is disabled
        if (bOnClickEnabled_ && isVisible() && pOnClick_) {
            if ((x >= 0) && (x < getSize().x - openListBoxButton_.getSize().x - 1) && (y >= 0) && (y < getSize().y)
                && (pressed)) {
                pOnClick_();
            }
        }

        return true;
    }

    if (openListBoxButton_.handleMouseLeft(x - (getSize().x - openListBoxButton_.getSize().x - 1), y - 1, pressed)) {
        setActive();
        return true;
    }
    if ((x >= 0) && (x < getSize().x - openListBoxButton_.getSize().x - 1)

        && (y >= 0) && (y < getSize().y) && (pressed)) {

        if (bOnClickEnabled_ && pOnClick_) {

            pOnClick_();

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

    if (bShowListBox_) {
        if (!listBox_.handleMouseLeft(x, bListBoxAbove_ ? (y + listBox_.getSize().y) : (y - getSize().y), pressed)) {
            if (x < 0 || x >= getSize().x || y < 0 || y >= getSize().y) {
                // if not on drop down box => click is handled by closing drop down box
                bShowListBox_ = false;
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
            if (listBox_.getSelectedIndex() < 0) {
                listBox_.setSelectedItem(0, true);
            } else if (listBox_.getSelectedIndex() > 0) {
                listBox_.setSelectedItem(listBox_.getSelectedIndex() - 1, true);
            }
        } else {
            if (listBox_.getSelectedIndex() < 0) {
                listBox_.setSelectedItem(0, true);
            } else if (listBox_.getSelectedIndex() < listBox_.getNumEntries() - 1) {
                listBox_.setSelectedItem(listBox_.getSelectedIndex() + 1, true);
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
    if (bShowListBox_) {
        const auto newY = bListBoxAbove_ ? (y + listBox_.getSize().y) : (y - getSize().y);
        listBox_.handleMouseWheel(x, newY, up);
        return x >= 0 && x < listBox_.getSize().x && newY >= 0 && newY < listBox_.getSize().y;
    }
    return false;
}

bool DropDownBox::handleKeyPress(const SDL_KeyboardEvent& key) {
    if ((!isEnabled()) || (!isVisible())) {
        return false;
    }

    Widget::handleKeyPress(key);
    if (isActive()) {
        // disable autoclosing of the list box
        const bool bSavedAutoclose          = bAutocloseListBoxOnSelectionChange_;
        bAutocloseListBoxOnSelectionChange_ = false;

        switch (key.keysym.sym) {
            case SDLK_UP: {
                if (listBox_.getSelectedIndex() < 0) {
                    listBox_.setSelectedItem(0, true);
                } else if (listBox_.getSelectedIndex() > 0) {
                    listBox_.setSelectedItem(listBox_.getSelectedIndex() - 1, true);
                }
            } break;

            case SDLK_DOWN: {
                if (listBox_.getSelectedIndex() < 0) {
                    listBox_.setSelectedItem(0, true);
                } else if (listBox_.getSelectedIndex() < listBox_.getNumEntries() - 1) {
                    listBox_.setSelectedItem(listBox_.getSelectedIndex() + 1, true);
                }
            } break;

            case SDLK_SPACE: {
                bShowListBox_ = !bShowListBox_;
            } break;

            case SDLK_TAB: {
                setInactive();
            } break;

            default: {
            } break;
        }

        bAutocloseListBoxOnSelectionChange_ = bSavedAutoclose;
        return true;
    }

    return false;
}

void DropDownBox::draw(Point position) {
    if (!isVisible())
        return;

    updateBackground();

    auto* const renderer = dune::globals::renderer.get();

    if (pBackground_)
        pBackground_.draw(renderer, position.x, position.y);

    updateForeground();

    if (pForeground_ && pActiveForeground_) {
        if ((bHover_ && pOnClick_) || isActive()) {
            pActiveForeground_.draw(renderer, position.x + 2, position.y + 2);
        } else {
            pForeground_.draw(renderer, position.x + 2, position.y + 2);
        }
    }

    openListBoxButton_.draw(position + Point(getSize().x - openListBoxButton_.getSize().x - 1, 1));
}

void DropDownBox::drawOverlay(Point position) {
    if (bShowListBox_) {
        bListBoxAbove_ = (position.y + listBox_.getSize().y > getRendererHeight());
        listBox_.draw(position + Point(0, bListBoxAbove_ ? -listBox_.getSize().y : getSize().y));
    }
}

void DropDownBox::resize(uint32_t width, uint32_t height) {
    Widget::resize(width, height);

    invalidateForeground();
    invalidateBackground();

    resizeListBox();
}

void DropDownBox::addEntry(std::string text, int data) {
    listBox_.addEntry(std::move(text), data);
    resizeListBox();
}

void DropDownBox::addEntry(std::string text, void* data) {
    listBox_.addEntry(std::move(text), data);
    resizeListBox();
}

void DropDownBox::insertEntry(int index, std::string text, int data) {
    listBox_.insertEntry(index, std::move(text), data);
    resizeListBox();
}

void DropDownBox::insertEntry(int index, std::string text, void* data) {
    listBox_.insertEntry(index, std::move(text), data);
    resizeListBox();
}

void DropDownBox::setEntry(unsigned index, const std::string& text) {
    listBox_.setEntry(index, text);
    invalidateForeground();
    resizeListBox();
}

void DropDownBox::removeEntry(int index) {
    listBox_.removeEntry(index);
    if (listBox_.getSelectedIndex() < 0) {
        invalidateForeground();
    }
    resizeListBox();
}

void DropDownBox::clearAllEntries() {
    listBox_.clearAllEntries();
    invalidateForeground();
    resizeListBox();
}

void DropDownBox::setActive() {
    openListBoxButton_.setActive();
    parent::setActive();
}

void DropDownBox::setColor(uint32_t color) {
    this->color_ = color;
    updateButtonSurface();
    invalidateForeground();
    listBox_.setColor(color);
}

void DropDownBox::setEnabled(bool bEnabled) {
    openListBoxButton_.setEnabled(bEnabled);

    parent::setEnabled(bEnabled);
}

void DropDownBox::resizeListBox() {
    const int listBoxHeight = std::max(1, std::min(numVisibleEntries_, gsl::narrow<int>(getNumEntries())))
                                * GUIStyle::getInstance().getListBoxEntryHeight()
                            + 2;
    listBox_.resize(getSize().x - 1, listBoxHeight);
}

std::unique_ptr<DropDownBox> DropDownBox::create() {
    auto dropDownBox         = std::make_unique<DropDownBox>();
    dropDownBox->pAllocated_ = true;
    return dropDownBox;
}

void DropDownBox::setActive(bool bActive) {
    if (!bActive) {
        bShowListBox_ = false;
        openListBoxButton_.setInactive();
    } else {
        openListBoxButton_.setActive();
    }
    Widget::setActive(bActive);
}

void DropDownBox::onSelectionChange(bool bInteractive) {
    invalidateForeground();

    if (bAutocloseListBoxOnSelectionChange_) {
        bShowListBox_ = false;
    }

    if (pOnSelectionChange_) {
        pOnSelectionChange_(bInteractive);
    }
}

void DropDownBox::updateButtonSurface() {
    const auto& gui = GUIStyle::getInstance();

    auto* const renderer = dune::globals::renderer.get();

    openListBoxButton_.setTextures(gui.createDropDownBoxButton(17, false, false, color_).createTexture(renderer),
                                   gui.createDropDownBoxButton(17, true, true, color_).createTexture(renderer),
                                   gui.createDropDownBoxButton(17, false, true, color_).createTexture(renderer));
}

void DropDownBox::invalidateForeground() {
    pForeground_.reset();
    pActiveForeground_.reset();
}

void DropDownBox::updateForeground() {
    if (pForeground_ || pActiveForeground_)
        return;

    if (listBox_.getSelectedIndex() < 0)
        return;

    const auto& gui = GUIStyle::getInstance();

    const auto width = getSize().x - 17;
    const auto text  = listBox_.getEntry(listBox_.getSelectedIndex());

    auto* const renderer = dune::globals::renderer.get();

    pForeground_       = gui.createListBoxEntry(width, text, false, color_).createTexture(renderer);
    pActiveForeground_ = gui.createListBoxEntry(width, text, true, color_).createTexture(renderer);
}

void DropDownBox::invalidateBackground() {
    pBackground_.reset();
}

void DropDownBox::updateBackground() {
    if (pBackground_)
        return;

    pBackground_ = GUIStyle::getInstance()
                       .createWidgetBackground(getSize().x, getSize().y)
                       .createTexture(dune::globals::renderer.get());
}
