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

#include "GUI/Label.h"

#include <misc/draw_util.h>
#include <misc/string_util.h>

#include <algorithm>

Label::Label() {
    Label::enableResizing(true, true);
}
Label::~Label() = default;

void Label::resize(uint32_t width, uint32_t height) {
    invalidateTextures();
    parent::resize(width, height);
}

Point Label::getMinimumSize() const {
    Point p(0, 0);

    // split text into single lines at every '\n'
    size_t startpos = 0;
    size_t nextpos  = 0;
    std::vector<std::string> hardLines;
    do {
        nextpos = text.find('\n', startpos);
        if (nextpos == std::string::npos) {
            hardLines.emplace_back(text, startpos, text.length() - startpos);
        } else {
            hardLines.emplace_back(text, startpos, nextpos - startpos);
            startpos = nextpos + 1;
        }
    } while (nextpos != std::string::npos);

    for (const std::string& hardLine : hardLines) {
        Point minLabelSize = GUIStyle::getInstance().getMinimumLabelSize(hardLine, fontSize);
        p.x                = std::max(p.x, minLabelSize.x);
        p.y += minLabelSize.y;
    }
    return p;
}

void Label::draw(Point position) {
    if (!isEnabled() || !isVisible())
        return;

    updateTextures();

    if (!pTexture)
        return;

    const auto size = getSize();

    const auto x = position.x + (size.x - pTexture.width_) / 2;
    const auto y = position.y + (size.y - pTexture.height_) / 2;

    pTexture.draw(renderer, x, y);
}

std::unique_ptr<Label>
Label::create(const std::string& text, Uint32 textcolor, Uint32 textshadowcolor, Uint32 backgroundcolor) {
    auto label = std::make_unique<Label>();
    label->setText(text);
    label->setTextColor(textcolor, textshadowcolor, backgroundcolor);
    label->pAllocated = true;
    return label;
}

void Label::updateTextures() {
    parent::updateTextures();

    if (pTexture)
        return;

    auto& gui = GUIStyle::getInstance();

    const auto textLines = greedyWordWrap(text, getSize().x, [&gui, font = fontSize](std::string_view tmp) {
        return gui.getMinimumLabelSize(tmp, font).x - 4.f;
    });

    pTexture = gui.createLabel(renderer, getSize().x, getSize().y, textLines, fontSize, alignment, textcolor,
                               textshadowcolor, backgroundcolor);
}

void Label::invalidateTextures() {
    pTexture = DuneTextureOwned();

    parent::invalidateTextures();
}
