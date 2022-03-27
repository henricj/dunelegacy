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

ListBox::ListBox() : color(COLOR_DEFAULT) {
    ListBox::enableResizing(true, true);

    scrollbar.setOnChange([this] { onScrollbarChange(); });
    resize(ListBox::getMinimumSize().x, ListBox::getMinimumSize().y);
}

ListBox::~ListBox() {
    ListBox::invalidateTextures();
}

void ListBox::handleMouseMovement(int32_t x, int32_t y, bool insideOverlay) {
    scrollbar.handleMouseMovement(x - getSize().x + scrollbar.getSize().x, y, insideOverlay);
}

bool ListBox::handleMouseLeft(int32_t x, int32_t y, bool pressed) {
    using namespace std::chrono_literals;

    const int scrollbarWidth = isScrollbarVisible() ? scrollbar.getSize().x : 0;

    if ((x >= 0) && (x < getSize().x - scrollbarWidth) && (y >= 0) && (y < getSize().y)) {

        if (pressed) {
            const int index = ((y - 1) / GUIStyle::getInstance().getListBoxEntryHeight()) + firstVisibleElement;
            if ((index >= 0) && (index < getNumEntries())) {
                selectedElement = index;

                if (dune::dune_clock::now() - lastClickTime < 200ms) {
                    if (pOnDoubleClick) {
                        pOnDoubleClick();
                    }
                } else {
                    lastClickTime = dune::dune_clock::now();
                    updateList();
                    if (pOnSingleClick) {
                        pOnSingleClick();
                    }
                    if (pOnSelectionChange) {
                        pOnSelectionChange(true);
                    }
                    setActive();
                }
            }
        }

        scrollbar.handleMouseLeft(x - getSize().x + scrollbar.getSize().x, y, pressed);
        return true;
    }
    return scrollbar.handleMouseLeft(x - getSize().x + scrollbar.getSize().x, y, pressed);
}

bool ListBox::handleMouseWheel(int32_t x, int32_t y, bool up) {
    // forward mouse wheel event to scrollbar
    return scrollbar.handleMouseWheel(0, 0, up);
}

bool ListBox::handleKeyPress(SDL_KeyboardEvent& key) {
    Widget::handleKeyPress(key);
    if (isActive()) {
        switch (key.keysym.sym) {
            case SDLK_UP: {
                if (selectedElement > 0) {
                    setSelectedItem(selectedElement - 1, true);
                }
            } break;

            case SDLK_DOWN: {
                if (selectedElement < getNumEntries() - 1) {
                    setSelectedItem(selectedElement + 1, true);
                }
            } break;

            default: {
            } break;
        }

        scrollbar.handleKeyPress(key);
    }
    scrollbar.handleKeyPress(key);
    return true;
}

void ListBox::draw(Point position) {
    if (!isVisible()) {
        return;
    }

    updateTextures();

    if (pBackground != nullptr) {
        const SDL_Rect dest = calcDrawingRect(pBackground.get(), position.x, position.y);
        Dune_RenderCopy(renderer, pBackground.get(), nullptr, &dest);
    }

    const SDL_Rect dest = calcDrawingRect(pForeground.get(), position.x + 2, position.y + 1);
    Dune_RenderCopy(renderer, pForeground.get(), nullptr, &dest);

    Point ScrollBarPos = position;
    ScrollBarPos.x += getSize().x - scrollbar.getSize().x;

    if (isScrollbarVisible()) {
        scrollbar.draw(ScrollBarPos);
    }
}

void ListBox::resize(uint32_t width, uint32_t height) {
    Widget::resize(width, height);

    scrollbar.resize(scrollbar.getMinimumSize().x, height);

    updateList();
}

void ListBox::setActive() {
    Widget::setActive();

    if ((selectedElement == -1) && (getNumEntries() > 0)) {
        selectedElement = 0;
        updateList();
        if (pOnSelectionChange) {
            pOnSelectionChange(false);
        }
    }
}

void ListBox::setSelectedItem(int index, bool bInteractive) {
    const bool bChanged = (index != selectedElement);

    if (index <= -1) {
        selectedElement = -1;
        updateList();
    } else if ((index >= 0) && (index < getNumEntries())) {
        selectedElement = index;

        const int numVisibleElements = ((getSize().y - 2) / GUIStyle::getInstance().getListBoxEntryHeight()) + 1;

        if (selectedElement >= firstVisibleElement + numVisibleElements - 1) {
            firstVisibleElement = selectedElement - (numVisibleElements - 1) + 1;
        } else if (selectedElement < firstVisibleElement) {
            firstVisibleElement = selectedElement;
        }

        if (firstVisibleElement > getNumEntries() - numVisibleElements) {
            firstVisibleElement = std::max(0, getNumEntries() - numVisibleElements + 1);
        }

        scrollbar.setCurrentValue(firstVisibleElement);

        updateList();
    }

    if (bChanged && pOnSelectionChange) {
        pOnSelectionChange(bInteractive);
    }
}

void ListBox::updateTextures() {
    Widget::updateTextures();

    if (!pBackground) {
        pBackground = convertSurfaceToTexture(GUIStyle::getInstance().createWidgetBackground(getSize().x, getSize().y));
    }

    if (!pForeground) {
        // create surfaces
        int surfaceHeight = getSize().y - 2;
        if (surfaceHeight < 0) {
            surfaceHeight = 0;
        }

        sdl2::surface_ptr pForegroundSurface =
            sdl2::surface_ptr {GUIStyle::getInstance().createEmptySurface(getSize().x - 4, surfaceHeight, true)};

        const int numVisibleElements =
            static_cast<int>(surfaceHeight / GUIStyle::getInstance().getListBoxEntryHeight());
        for (int i = firstVisibleElement; i < firstVisibleElement + numVisibleElements; ++i) {
            if (i >= getNumEntries())
                break;

            sdl2::surface_ptr pSurface = sdl2::surface_ptr {GUIStyle::getInstance().createListBoxEntry(
                getSize().x - 4, getEntry(i), bHighlightSelectedElement && (i == selectedElement), color)};

            SDL_Rect dest = calcDrawingRect(pSurface.get(), 0,
                                            (i - firstVisibleElement)
                                                * static_cast<int>(GUIStyle::getInstance().getListBoxEntryHeight()));
            SDL_BlitSurface(pSurface.get(), nullptr, pForegroundSurface.get(), &dest);
        }
        pForeground = convertSurfaceToTexture(std::move(pForegroundSurface));
    }
}

void ListBox::updateList() {
    invalidateTextures();

    // create surfaces
    int surfaceHeight = getSize().y - 2;
    if (surfaceHeight < 0) {
        surfaceHeight = 0;
    }

    const int numVisibleElements = surfaceHeight / GUIStyle::getInstance().getListBoxEntryHeight();

    scrollbar.setRange(0, std::max(0, getNumEntries() - numVisibleElements));
    scrollbar.setBigStepSize(std::max(1, numVisibleElements - 1));
}
