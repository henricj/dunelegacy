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

#include <GUI/ListBox.h>

#include "misc/DrawingRectHelper.h"
#include "misc/dune_clock.h"

#include <chrono>

ListBox::ListBox() : color_(COLOR_DEFAULT) {
    ListBox::enableResizing(true, true);

    scrollbar_.setOnChange([this] { onScrollbarChange(); });
    resize(ListBox::getMinimumSize().x, ListBox::getMinimumSize().y);
}

ListBox::~ListBox() = default;

void ListBox::handleMouseMovement(int32_t x, int32_t y, bool insideOverlay) {
    scrollbar_.handleMouseMovement(x - getSize().x + scrollbar_.getSize().x, y, insideOverlay);
}

bool ListBox::handleMouseLeft(int32_t x, int32_t y, bool pressed) {
    using namespace std::chrono_literals;

    const int scrollbarWidth = isScrollbarVisible() ? scrollbar_.getSize().x : 0;

    if (x >= 0 && x < getSize().x - scrollbarWidth && y >= 0 && y < getSize().y) {

        if (pressed) {
            const int index = (y - 1) / GUIStyle::getInstance().getListBoxEntryHeight() + firstVisibleElement_;
            if (index >= 0 && index < getNumEntries()) {
                selectedElement_ = index;

                if (dune::dune_clock::now() - lastClickTime_ < 200ms) {
                    if (pOnDoubleClick_) {
                        pOnDoubleClick_();
                    }
                } else {
                    lastClickTime_ = dune::dune_clock::now();
                    updateList();
                    if (pOnSingleClick_) {
                        pOnSingleClick_();
                    }
                    if (pOnSelectionChange_) {
                        pOnSelectionChange_(true);
                    }
                    setActive();
                }
            }
        }

        scrollbar_.handleMouseLeft(x - getSize().x + scrollbar_.getSize().x, y, pressed);
        return true;
    }
    return scrollbar_.handleMouseLeft(x - getSize().x + scrollbar_.getSize().x, y, pressed);
}

bool ListBox::handleMouseWheel(int32_t x, int32_t y, bool up) {
    // forward mouse wheel event to scrollbar
    return scrollbar_.handleMouseWheel(0, 0, up);
}

bool ListBox::handleKeyPress(const SDL_KeyboardEvent& key) {
    parent::handleKeyPress(key);
    if (isActive()) {
        switch (key.keysym.sym) {
            case SDLK_UP: {
                if (selectedElement_ > 0) {
                    setSelectedItem(selectedElement_ - 1, true);
                }
            } break;

            case SDLK_DOWN: {
                if (selectedElement_ < getNumEntries() - 1) {
                    setSelectedItem(selectedElement_ + 1, true);
                }
            } break;

            default: {
            } break;
        }

        scrollbar_.handleKeyPress(key);
    }
    scrollbar_.handleKeyPress(key);
    return true;
}

void ListBox::draw(Point position) {
    if (!isVisible()) {
        return;
    }

    updateTextures();

    auto* const renderer = dune::globals::renderer.get();

    if (pBackground_)
        pBackground_.draw(renderer, position.x, position.y);

    if (pForeground_)
        pForeground_.draw(renderer, position.x + 2, position.y + 1);

    Point ScrollBarPos = position;
    ScrollBarPos.x += getSize().x - scrollbar_.getSize().x;

    if (isScrollbarVisible()) {
        scrollbar_.draw(ScrollBarPos);
    }
}

void ListBox::resize(uint32_t width, uint32_t height) {
    parent::resize(width, height);

    scrollbar_.resize(scrollbar_.getMinimumSize().x, height);

    updateList();
}

void ListBox::setActive() {
    parent::setActive();

    if (selectedElement_ == -1 && getNumEntries() > 0) {
        selectedElement_ = 0;
        updateList();
        if (pOnSelectionChange_) {
            pOnSelectionChange_(false);
        }
    }
}

void ListBox::setSelectedItem(int index, bool bInteractive) {
    const bool bChanged = index != selectedElement_;

    if (index <= -1) {
        selectedElement_ = -1;
        updateList();
    } else if (index >= 0 && index < getNumEntries()) {
        selectedElement_ = index;

        const auto& gui = GUIStyle::getInstance();

        const int numVisibleElements = (getSize().y - 2) / gui.getListBoxEntryHeight() + 1;

        if (selectedElement_ >= firstVisibleElement_ + numVisibleElements - 1) {
            firstVisibleElement_ = selectedElement_ - (numVisibleElements - 1) + 1;
        } else if (selectedElement_ < firstVisibleElement_) {
            firstVisibleElement_ = selectedElement_;
        }

        if (firstVisibleElement_ > getNumEntries() - numVisibleElements) {
            firstVisibleElement_ = std::max(0, getNumEntries() - numVisibleElements + 1);
        }

        scrollbar_.setCurrentValue(firstVisibleElement_);

        updateList();
    }

    if (bChanged && pOnSelectionChange_) {
        pOnSelectionChange_(bInteractive);
    }
}

void ListBox::updateTextures() {
    parent::updateTextures();

    const auto& gui = GUIStyle::getInstance();

    if (!pBackground_)
        pBackground_ =
            gui.createWidgetBackground(getSize().x, getSize().y).createTexture(dune::globals::renderer.get());

    if (!pForeground_) {
        const auto scale = gui.getActualScale();

        // create surfaces
        const auto surfaceHeight = getSize().y - 2;

        if (surfaceHeight <= 0)
            return;

        const auto surfaceWidth = getSize().x - 4;

        if (surfaceWidth <= 0)
            return;

        const auto scaled_height = static_cast<int>(std::ceil(static_cast<float>(surfaceHeight) * scale));
        const auto scaled_width  = static_cast<int>(std::ceil(static_cast<float>(surfaceWidth) * scale));

        sdl2::surface_ptr pForegroundSurface{gui.createEmptySurface(scaled_width, scaled_height, true)};

        const auto entry_height        = gui.getListBoxEntryHeight();
        const auto scaled_entry_height = static_cast<int>(std::ceil(static_cast<float>(entry_height) * scale));

        const auto numVisibleElements = surfaceHeight / static_cast<int>(entry_height);
        for (int i = firstVisibleElement_; i < firstVisibleElement_ + numVisibleElements; ++i) {
            if (i >= getNumEntries())
                break;

            auto pSurface = gui.createListBoxEntry(scaled_width, getEntry(i),
                                                   bHighlightSelectedElement_ && i == selectedElement_, color_);

            SDL_Rect dest = calcDrawingRect(pSurface.get(), 0, (i - firstVisibleElement_) * scaled_entry_height);
            SDL_BlitSurface(pSurface.get(), nullptr, pForegroundSurface.get(), &dest);
        }

        auto texture = convertSurfaceToTexture(std::move(pForegroundSurface));

        pForeground_ =
            DuneTextureOwned{std::move(texture), static_cast<float>(surfaceWidth), static_cast<float>(surfaceHeight)};
    }
}

void ListBox::invalidateTextures() {
    pBackground_.reset();
    pForeground_.reset();

    parent::invalidateTextures();
}

void ListBox::updateList() {
    invalidateTextures();

    // create surfaces
    int surfaceHeight = getSize().y - 2;
    if (surfaceHeight < 0) {
        surfaceHeight = 0;
    }

    const auto& gui = GUIStyle::getInstance();

    const auto numVisibleElements = surfaceHeight / gui.getListBoxEntryHeight();

    scrollbar_.setRange(0, std::max(0, getNumEntries() - numVisibleElements));
    scrollbar_.setBigStepSize(std::max(1, numVisibleElements - 1));
}
