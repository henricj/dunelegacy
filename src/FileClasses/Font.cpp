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

#include "FileClasses/Font.h"

#include "Definitions.h"

#include <vector>

Font::~Font() = default;

sdl2::surface_ptr Font::createMultilineTextSurface(std::string_view text, uint32_t color, bool bCentered) const {
    std::vector<std::string_view> textLines;

    auto pos = std::string_view::size_type{};

    for (;;) {
        const auto next = text.find('\n', pos);

        if (next == std::string::npos) {
            textLines.emplace_back(text.substr(pos, text.length() - pos));
            break;
        }

        textLines.emplace_back(text.substr(pos, next - pos));
        pos = next + 1;
    }

    auto width = 0;
    for (const auto& textLine : textLines) {
        const auto w = getTextWidth(textLine);

        if (w > width)
            width = w;
    }

    const auto lineHeight = getTextHeight();
    const auto width0     = getTextWidth(text);
    const auto lines      = static_cast<int>(textLines.size());
    const int height      = lineHeight * lines + lineHeight * (lines - 1) / 2;

    // create new picture surface
    auto pic = sdl2::surface_ptr{SDL_CreateRGBSurfaceWithFormat(0, width, height, SCREEN_BPP, SCREEN_FORMAT)};
    if (pic == nullptr) {
        return nullptr;
    }

    SDL_SetSurfaceBlendMode(pic.get(), SDL_BLENDMODE_BLEND);
    SDL_FillRect(pic.get(), nullptr, SDL_MapRGBA(pic->format, 0, 0, 0, 0));

    auto currentLineNum = 0;
    for (const auto& textLine : textLines) {
        auto text_surface = createTextSurface(textLine, color);

        const auto x = bCentered ? (width - text_surface->w) / 2 : 0;
        const auto y = currentLineNum * lineHeight;

        SDL_Rect dest{x, y, text_surface->w, text_surface->h};

        SDL_BlitSurface(text_surface.get(), nullptr, pic.get(), &dest);

        currentLineNum++;
    }

    return pic;
}
