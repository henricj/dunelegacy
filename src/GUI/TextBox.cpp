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

void TextBox::setTextColor(uint32_t textcolor, Uint32 textshadowcolor) {
    this->textcolor       = textcolor;
    this->textshadowcolor = textshadowcolor;
    invalidateTextures();
}

void TextBox::updateTextures() {
    parent::updateTextures();

    if (!pTextureWithoutCaret || !pTextureWithCaret) {
        invalidateTextures();

        const auto& gui      = GUIStyle::getInstance();
        auto* const renderer = dune::globals::renderer.get();

        const auto size = getSize();

        pTextureWithoutCaret =
            gui.createTextBoxSurface(size.x, size.y, text, false, fontSize, Alignment_Left, textcolor, textshadowcolor)
                .createTexture(renderer);
        pTextureWithCaret =
            gui.createTextBoxSurface(size.x, size.y, text, true, fontSize, Alignment_Left, textcolor, textshadowcolor)
                .createTexture(renderer);
    }
}

void TextBox::draw(Point position) {
    if (!isVisible())
        return;

    updateTextures();

    if (!pTextureWithoutCaret || !pTextureWithCaret)
        return;

    auto* const renderer = dune::globals::renderer.get();

    if (isActive()) {
        using namespace std::chrono_literals;

        if (dune::dune_clock::now() - lastCaretTime < 500ms)
            pTextureWithCaret.draw(renderer, position.x, position.y);
        else
            pTextureWithoutCaret.draw(renderer, position.x, position.y);

        if (dune::dune_clock::now() - lastCaretTime >= 1000ms)
            lastCaretTime = dune::dune_clock::now();
    } else
        pTextureWithoutCaret.draw(renderer, position.x, position.y);
}

bool TextBox::handleMouseLeft(int32_t x, int32_t y, bool pressed) {
    if (x < 0 || x >= getSize().x || y < 0 || y >= getSize().y)
        return false;

    if (!isEnabled() || !isVisible())
        return true;

    if (pressed) {
        setActive();
        lastCaretTime = dune::dune_clock::now();
    }

    return true;
}

bool TextBox::handleKeyPress(SDL_KeyboardEvent& key) {
    if (!isVisible() || !isEnabled() || !isActive())
        return true;

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
    if (!isVisible() || !isEnabled() || !isActive())
        return true;

    const std::string newText = textInput.text;

    bool bChanged = false;
    for (const char c : newText) {
        if ((maxTextLength < 0 || static_cast<int>(utf8Length(text)) < maxTextLength)
            && (allowedChars.empty() || allowedChars.find(c) != std::string::npos)
            && forbiddenChars.find(c) == std::string::npos) {
            text += c;
            bChanged = true;
        }
    }

    if (bChanged && pOnTextChange)
        pOnTextChange(true);

    return true;
}

void TextBox::setText(std::string_view text, bool bInteractive) {
    const bool bChanged = (text != this->text);
    this->text          = text;
    invalidateTextures();
    if (bChanged && pOnTextChange) {
        pOnTextChange(bInteractive);
    }
}

void TextBox::invalidateTextures() {
    pTextureWithoutCaret.reset();
    pTextureWithCaret.reset();

    parent::invalidateTextures();
}
