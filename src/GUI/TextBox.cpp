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

#include "GUI/GUIStyle.h"
#include "globals.h"
#include "misc/dune_clock.h"
#include "misc/string_util.h"

#include <chrono>

TextBox::TextBox() {
    parent::enableResizing(true, false);
    resize(getMinimumSize().x, getMinimumSize().y);
}

TextBox::~TextBox() {
    invalidateTextures();
}

void TextBox::setTextColor(uint32_t textcolor, Uint32 textshadowcolor) {
    this->text_color_        = textcolor;
    this->text_shadow_color_ = textshadowcolor;
    invalidateTextures();
}

void TextBox::updateTextures() {
    parent::updateTextures();

    if (!pTextureWithoutCaret_ || !pTextureWithCaret_) {
        invalidateTextures();

        const auto& gui      = GUIStyle::getInstance();
        auto* const renderer = dune::globals::renderer.get();

        const auto size = getSize();

        pTextureWithoutCaret_ = gui.createTextBoxSurface(size.x, size.y, text_, false, fontSize_, Alignment_Left,
                                                         text_color_, text_shadow_color_)
                                    .createTexture(renderer);
        pTextureWithCaret_ = gui.createTextBoxSurface(size.x, size.y, text_, true, fontSize_, Alignment_Left,
                                                      text_color_, text_shadow_color_)
                                 .createTexture(renderer);
    }
}

void TextBox::draw(Point position) {
    if (!isVisible())
        return;

    updateTextures();

    if (!pTextureWithoutCaret_ || !pTextureWithCaret_)
        return;

    auto* const renderer = dune::globals::renderer.get();

    if (isActive()) {
        using namespace std::chrono_literals;

        if (dune::dune_clock::now() - lastCaretTime_ < 500ms)
            pTextureWithCaret_.draw(renderer, position.x, position.y);
        else
            pTextureWithoutCaret_.draw(renderer, position.x, position.y);

        if (dune::dune_clock::now() - lastCaretTime_ >= 1000ms)
            lastCaretTime_ = dune::dune_clock::now();
    } else
        pTextureWithoutCaret_.draw(renderer, position.x, position.y);
}

bool TextBox::handleMouseLeft(int32_t x, int32_t y, bool pressed) {
    if (x < 0 || x >= getSize().x || y < 0 || y >= getSize().y)
        return false;

    if (!isEnabled() || !isVisible())
        return true;

    if (pressed) {
        setActive();
        lastCaretTime_ = dune::dune_clock::now();
    }

    return true;
}

bool TextBox::handleKeyPress(const SDL_KeyboardEvent& key) {
    if (!isVisible() || !isEnabled() || !isActive())
        return true;

    if (key.keysym.sym == SDLK_TAB) {
        setInactive();
        return true;
    }

    if (key.keysym.sym == SDLK_BACKSPACE) {
        if (!text_.empty()) {
            text_ = utf8Substr(text_, 0, utf8Length(text_) - 1);
            if (pOnTextChange_) {
                pOnTextChange_(true);
            }
        }
    } else if (key.keysym.sym == SDLK_RETURN) {
        if (pOnReturn_) {
            pOnReturn_();
        }
    }

    invalidateTextures();

    return true;
}

bool TextBox::handleTextInput(const SDL_TextInputEvent& textInput) {
    if (!isVisible() || !isEnabled() || !isActive())
        return true;

    const std::string newText = textInput.text;

    bool bChanged = false;
    for (const char c : newText) {
        if ((maxTextLength_ < 0 || static_cast<int>(utf8Length(text_)) < maxTextLength_)
            && (allowedChars_.empty() || allowedChars_.find(c) != std::string::npos)
            && forbiddenChars_.find(c) == std::string::npos) {
            text_ += c;
            bChanged = true;
        }
    }

    if (bChanged && pOnTextChange_)
        pOnTextChange_(true);

    return true;
}

void TextBox::setText(std::string_view text, bool bInteractive) {
    const bool bChanged = (text != this->text_);
    this->text_         = text;
    invalidateTextures();
    if (bChanged && pOnTextChange_) {
        pOnTextChange_(bInteractive);
    }
}

void TextBox::invalidateTextures() {
    pTextureWithoutCaret_.reset();
    pTextureWithCaret_.reset();

    parent::invalidateTextures();
}
