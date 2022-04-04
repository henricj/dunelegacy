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

#include "GUI/TextBox.h"

TextBox::TextBox() {
    parent::enableResizing(true, false);
    resize(getMinimumSize().x, getMinimumSize().y);
}

TextBox::~TextBox() {
    invalidateTextures();
}

void TextBox::updateTextures() {
    parent::updateTextures();

    if (pTextureWithoutCaret == nullptr || pTextureWithCaret == nullptr) {
        invalidateTextures();

        pTextureWithoutCaret = convertSurfaceToTexture(GUIStyle::getInstance().createTextBoxSurface(
            getSize().x, getSize().y, text, false, fontSize, Alignment_Left, textcolor, textshadowcolor));
        pTextureWithCaret    = convertSurfaceToTexture(GUIStyle::getInstance().createTextBoxSurface(
               getSize().x, getSize().y, text, true, fontSize, Alignment_Left, textcolor, textshadowcolor));
    }
}

void TextBox::draw(Point position) {
    if (!isVisible()) {
        return;
    }

    updateTextures();

    if ((pTextureWithoutCaret == nullptr) || (pTextureWithCaret == nullptr)) {
        return;
    }

    const SDL_Rect dest = calcDrawingRect(pTextureWithoutCaret.get(), position.x, position.y);

    if (isActive()) {
        using namespace std::chrono_literals;

        if ((dune::dune_clock::now() - lastCaretTime) < 500ms) {
            Dune_RenderCopy(renderer, pTextureWithCaret.get(), nullptr, &dest);
        } else {
            Dune_RenderCopy(renderer, pTextureWithoutCaret.get(), nullptr, &dest);
        }

        if (dune::dune_clock::now() - lastCaretTime >= 1000ms) {
            lastCaretTime = dune::dune_clock::now();
        }
    } else {
        Dune_RenderCopy(renderer, pTextureWithoutCaret.get(), nullptr, &dest);
    }
}

bool TextBox::handleMouseLeft(int32_t x, int32_t y, bool pressed) {
    if ((x < 0) || (x >= getSize().x) || (y < 0) || (y >= getSize().y)) {
        return false;
    }

    if ((!isEnabled()) || (!isVisible())) {
        return true;
    }

    if (pressed) {
        setActive();
        lastCaretTime = dune::dune_clock::now();
    }
    return true;
}

bool TextBox::handleKeyPress(SDL_KeyboardEvent& key) {
    if ((!isVisible()) || (!isEnabled()) || (!isActive())) {
        return true;
    }

    if (key.keysym.sym == SDLK_TAB) {
        setInactive();
        return true;
    }

    if (key.keysym.sym == SDLK_BACKSPACE) {
        if (!text.empty()) {
            text = utf8Substr(text, 0, utf8Length(text) - 1);
            if (pOnTextChange) {
                pOnTextChange(true);
            }
        }
    } else if (key.keysym.sym == SDLK_RETURN) {
        if (pOnReturn) {
            pOnReturn();
        }
    }

    invalidateTextures();

    return true;
}

bool TextBox::handleTextInput(SDL_TextInputEvent& textInput) {
    if ((!isVisible()) || (!isEnabled()) || (!isActive())) {
        return true;
    }

    const std::string newText = textInput.text;

    bool bChanged = false;
    for (const char c : newText) {
        if (((maxTextLength < 0) || (static_cast<int>(utf8Length(text)) < maxTextLength))
            && (allowedChars.empty() || allowedChars.find(c) != std::string::npos)
            && (forbiddenChars.find(c) == std::string::npos)) {
            text += c;
            bChanged = true;
        }
    }

    if (bChanged && pOnTextChange) {
        pOnTextChange(true);
    }

    return true;
}