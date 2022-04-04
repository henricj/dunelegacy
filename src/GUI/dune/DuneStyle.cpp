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

#include <GUI/Widget.h>
#include <GUI/dune/DuneStyle.h>
#include <misc/draw_util.h>

#include <FileClasses/FontManager.h>
#include <FileClasses/GFXManager.h>

#include <globals.h>

#include <algorithm>
#include <string_view>

sdl2::surface_ptr DuneStyle::createSurfaceWithText(std::string_view text, uint32_t color, unsigned int fontsize) {
    if (pFontManager) {
        return pFontManager->createSurfaceWithText(text, color, fontsize);
    }

    // create dummy surface
    auto surface = sdl2::surface_ptr{
        SDL_CreateRGBSurface(0, static_cast<int>(text.length() * 10), 12, SCREEN_BPP, RMASK, GMASK, BMASK, AMASK)};
    if (surface == nullptr) {
        return nullptr;
    }

    SDL_FillRect(surface.get(), nullptr, COLOR_TRANSPARENT);

    return surface;
}

unsigned int DuneStyle::getTextHeight(unsigned int FontNum) const {
    if (pFontManager) {
        return pFontManager->getTextHeight(FontNum);
    }

    return 12;
}

unsigned int DuneStyle::getTextWidth(std::string_view text, unsigned int FontNum) const {
    if (pFontManager) {
        return pFontManager->getTextWidth(text, FontNum);
    }

    return static_cast<int>(text.length() * 10);
}

Point DuneStyle::getMinimumLabelSize(std::string_view text, int fontSize) {
    return {static_cast<int>(getTextWidth(text, fontSize)) + 12, static_cast<int>(getTextHeight(fontSize)) + 4};
}

sdl2::surface_ptr DuneStyle::createLabelSurface(uint32_t width, uint32_t height,
                                                const std::vector<std::string>& textLines, int fontSize,
                                                Alignment_Enum alignment, uint32_t textcolor, uint32_t textshadowcolor,
                                                uint32_t backgroundcolor) {

    // create surfaces
    sdl2::surface_ptr surface =
        sdl2::surface_ptr{SDL_CreateRGBSurface(0, width, height, SCREEN_BPP, RMASK, GMASK, BMASK, AMASK)};
    if (surface == nullptr) {
        return nullptr;
    }

    SDL_FillRect(surface.get(), nullptr, backgroundcolor);

    if (textcolor == COLOR_DEFAULT)
        textcolor = defaultForegroundColor;
    if (textshadowcolor == COLOR_DEFAULT)
        textshadowcolor = defaultShadowColor;

    const int fontheight = getTextHeight(fontSize);
    const int spacing    = 2;

    int textpos_y = 0;

    if (alignment & Alignment_VCenter) {
        const int textheight = fontheight * textLines.size() + spacing * (textLines.size() - 1);
        textpos_y            = (static_cast<int>(height) - textheight) / 2;
    } else if (alignment & Alignment_Bottom) {
        const int textheight = fontheight * textLines.size() + spacing * (textLines.size() - 1);
        textpos_y            = static_cast<int>(height) - textheight - spacing;
    } else {
        // Alignment_Top
        textpos_y = spacing;
    }

    for (const std::string& textLine : textLines) {
        auto textSurface1 = createSurfaceWithText(textLine, textshadowcolor, fontSize);
        auto textSurface2 = createSurfaceWithText(textLine, textcolor, fontSize);

        auto textRect1 = calcDrawingRect(textSurface1.get(), 0, textpos_y + 4);
        auto textRect2 = calcDrawingRect(textSurface2.get(), 0, textpos_y + 3);
        if (alignment & Alignment_Left) {
            textRect1.x = 4;
            textRect2.x = 3;
        } else if (alignment & Alignment_Right) {
            textRect1.x = width - textSurface1->w - 4;
            textRect2.x = width - textSurface2->w - 3;
        } else {
            // Alignment_HCenter
            textRect1.x = ((surface->w - textSurface1->w) / 2) + 3;
            textRect2.x = ((surface->w - textSurface2->w) / 2) + 2;
        }

        SDL_BlitSurface(textSurface1.get(), nullptr, surface.get(), &textRect1);

        SDL_BlitSurface(textSurface2.get(), nullptr, surface.get(), &textRect2);

        textpos_y += fontheight + spacing;
    }

    return surface;
}

Point DuneStyle::getMinimumCheckboxSize(std::string_view text) {
    return {static_cast<int>(getTextWidth(text, 14)) + 20 + 17, static_cast<int>(getTextHeight(14)) + 8};
}

sdl2::surface_ptr DuneStyle::createCheckboxSurface(uint32_t width, uint32_t height, std::string_view text, bool checked,
                                                   bool activated, uint32_t textcolor, uint32_t textshadowcolor,
                                                   uint32_t backgroundcolor) {
    sdl2::surface_ptr surface =
        sdl2::surface_ptr{SDL_CreateRGBSurface(0, width, height, SCREEN_BPP, RMASK, GMASK, BMASK, AMASK)};
    if (!surface) {
        return nullptr;
    }

    SDL_FillRect(surface.get(), nullptr, backgroundcolor);

    if (textcolor == COLOR_DEFAULT)
        textcolor = defaultForegroundColor;
    if (textshadowcolor == COLOR_DEFAULT)
        textshadowcolor = defaultShadowColor;

    if (activated) {
        textcolor = brightenUp(textcolor);
    }

    drawRect(surface.get(), 4, 5, 4 + 17, 5 + 17, textcolor);
    drawRect(surface.get(), 4 + 1, 5 + 1, 4 + 17 - 1, 5 + 17 - 1, textcolor);

    if (checked) {
        int x1 = 4 + 2;
        int y1 = 5 + 2;
        int x2 = 4 + 17 - 2;

        for (int i = 0; i < 15; i++) {
            // North-West to South-East
            putPixel(surface.get(), x1, y1, textcolor);
            putPixel(surface.get(), x1 + 1, y1, textcolor);
            putPixel(surface.get(), x1 - 1, y1, textcolor);

            // North-East to South-West
            putPixel(surface.get(), x2, y1, textcolor);
            putPixel(surface.get(), x2 + 1, y1, textcolor);
            putPixel(surface.get(), x2 - 1, y1, textcolor);

            x1++;
            y1++;
            x2--;
        }
    }

    const sdl2::surface_ptr textSurface1 = createSurfaceWithText(text, textshadowcolor, 14);
    SDL_Rect textRect1 =
        calcDrawingRect(textSurface1.get(), 10 + 2 + 17, surface->h / 2 + 5, HAlign::Left, VAlign::Center);
    SDL_BlitSurface(textSurface1.get(), nullptr, surface.get(), &textRect1);

    const sdl2::surface_ptr textSurface2 = createSurfaceWithText(text, textcolor, 14);
    SDL_Rect textRect2 =
        calcDrawingRect(textSurface2.get(), 10 + 1 + 17, surface->h / 2 + 4, HAlign::Left, VAlign::Center);
    SDL_BlitSurface(textSurface2.get(), nullptr, surface.get(), &textRect2);

    return surface;
}

Point DuneStyle::getMinimumRadioButtonSize(std::string_view text) {
    return {static_cast<int>(getTextWidth(text, 14)) + 16 + 15, static_cast<int>(getTextHeight(14)) + 8};
}

sdl2::surface_ptr DuneStyle::createRadioButtonSurface(uint32_t width, uint32_t height, std::string_view text,
                                                      bool checked, bool activated, uint32_t textcolor,
                                                      uint32_t textshadowcolor, uint32_t backgroundcolor) {
    sdl2::surface_ptr surface =
        sdl2::surface_ptr{SDL_CreateRGBSurface(0, width, height, SCREEN_BPP, RMASK, GMASK, BMASK, AMASK)};
    if (!surface) {
        return nullptr;
    }

    SDL_FillRect(surface.get(), nullptr, backgroundcolor);

    if (textcolor == COLOR_DEFAULT)
        textcolor = defaultForegroundColor;
    if (textshadowcolor == COLOR_DEFAULT)
        textshadowcolor = defaultShadowColor;

    if (activated) {
        textcolor = brightenUp(textcolor);
    }

    drawHLineNoLock(surface.get(), 8, 7, 13, textcolor);
    drawHLineNoLock(surface.get(), 7, 8, 14, textcolor);
    drawHLineNoLock(surface.get(), 6, 9, 8, textcolor);
    drawHLineNoLock(surface.get(), 13, 9, 15, textcolor);
    drawHLineNoLock(surface.get(), 5, 10, 6, textcolor);
    drawHLineNoLock(surface.get(), 15, 10, 16, textcolor);
    drawHLineNoLock(surface.get(), 4, 11, 6, textcolor);
    drawHLineNoLock(surface.get(), 15, 11, 17, textcolor);

    drawVLineNoLock(surface.get(), 4, 12, 15, textcolor);
    drawVLineNoLock(surface.get(), 5, 12, 15, textcolor);
    drawVLineNoLock(surface.get(), 16, 12, 15, textcolor);
    drawVLineNoLock(surface.get(), 17, 12, 15, textcolor);

    drawHLineNoLock(surface.get(), 4, 16, 6, textcolor);
    drawHLineNoLock(surface.get(), 15, 16, 17, textcolor);
    drawHLineNoLock(surface.get(), 5, 17, 6, textcolor);
    drawHLineNoLock(surface.get(), 15, 17, 16, textcolor);
    drawHLineNoLock(surface.get(), 6, 18, 8, textcolor);
    drawHLineNoLock(surface.get(), 13, 18, 15, textcolor);
    drawHLineNoLock(surface.get(), 7, 19, 14, textcolor);
    drawHLineNoLock(surface.get(), 8, 20, 13, textcolor);

    if (checked) {
        drawHLineNoLock(surface.get(), 9, 11, 12, textcolor);
        drawHLineNoLock(surface.get(), 8, 12, 13, textcolor);
        drawHLineNoLock(surface.get(), 8, 13, 13, textcolor);
        drawHLineNoLock(surface.get(), 8, 14, 13, textcolor);
        drawHLineNoLock(surface.get(), 8, 15, 13, textcolor);
        drawHLineNoLock(surface.get(), 9, 16, 12, textcolor);
    }

    const sdl2::surface_ptr textSurface1 = createSurfaceWithText(text, textshadowcolor, 14);
    SDL_Rect textRect1 =
        calcDrawingRect(textSurface1.get(), 8 + 2 + 15, surface->h / 2 + 5, HAlign::Left, VAlign::Center);
    SDL_BlitSurface(textSurface1.get(), nullptr, surface.get(), &textRect1);

    const sdl2::surface_ptr textSurface2 = createSurfaceWithText(text, textcolor, 14);
    SDL_Rect textRect2 =
        calcDrawingRect(textSurface2.get(), 8 + 1 + 15, surface->h / 2 + 4, HAlign::Left, VAlign::Center);
    SDL_BlitSurface(textSurface2.get(), nullptr, surface.get(), &textRect2);

    return surface;
}

sdl2::surface_ptr DuneStyle::createDropDownBoxButton(uint32_t size, bool pressed, bool activated, uint32_t color) {
    if (color == COLOR_DEFAULT) {
        color = defaultForegroundColor;
    }

    // create surfaces
    sdl2::surface_ptr surface =
        sdl2::surface_ptr{SDL_CreateRGBSurface(0, size, size, SCREEN_BPP, RMASK, GMASK, BMASK, AMASK)};
    if (!surface) {
        return nullptr;
    }

    // create button background
    if (!pressed) {
        // normal mode
        SDL_FillRect(surface.get(), nullptr, buttonBackgroundColor);
        drawRect(surface.get(), 0, 0, surface->w - 1, surface->h - 1, buttonBorderColor);
        drawHLine(surface.get(), 1, 1, surface->w - 2, buttonEdgeTopLeftColor);
        drawVLine(surface.get(), 1, 1, surface->h - 2, buttonEdgeTopLeftColor);
        drawHLine(surface.get(), 1, surface->h - 2, surface->w - 2, buttonEdgeBottomRightColor);
        drawVLine(surface.get(), surface->w - 2, 1, surface->h - 2, buttonEdgeBottomRightColor);
    } else {
        // pressed button mode
        SDL_FillRect(surface.get(), nullptr, pressedButtonBackgroundColor);
        drawRect(surface.get(), 0, 0, surface->w - 1, surface->h - 1, buttonBorderColor);
        drawRect(surface.get(), 1, 1, surface->w - 2, surface->h - 2, buttonEdgeBottomRightColor);
    }

    const int col = (pressed | activated) ? brightenUp(color) : color;

    int x1 = 3;
    int x2 = size - 3 - 1;
    int y  = size / 3 - 1;

    // draw separated hline
    drawHLine(surface.get(), x1, y, x2, col);
    y += 2;

    // down arrow
    for (; x1 <= x2; ++x1, --x2, ++y) {
        drawHLine(surface.get(), x1, y, x2, col);
    }

    return surface;
}

Point DuneStyle::getMinimumButtonSize(std::string_view text) {
    return {static_cast<int>(getTextWidth(text, 12)) + 12, static_cast<int>(getTextHeight(12))};
}

sdl2::surface_ptr DuneStyle::createButtonSurface(uint32_t width, uint32_t height, std::string_view text, bool pressed,
                                                 bool activated, uint32_t textcolor, uint32_t textshadowcolor) {
    sdl2::surface_ptr surface =
        sdl2::surface_ptr{SDL_CreateRGBSurface(0, width, height, SCREEN_BPP, RMASK, GMASK, BMASK, AMASK)};
    if (!surface) {
        return nullptr;
    }

    // create button background
    if (!pressed) {
        // normal mode
        SDL_FillRect(surface.get(), nullptr, buttonBackgroundColor);
        drawRect(surface.get(), 0, 0, surface->w - 1, surface->h - 1, buttonBorderColor);
        drawHLine(surface.get(), 1, 1, surface->w - 2, buttonEdgeTopLeftColor);
        drawVLine(surface.get(), 1, 1, surface->h - 2, buttonEdgeTopLeftColor);
        drawHLine(surface.get(), 1, surface->h - 2, surface->w - 2, buttonEdgeBottomRightColor);
        drawVLine(surface.get(), surface->w - 2, 1, surface->h - 2, buttonEdgeBottomRightColor);
    } else {
        // pressed button mode
        SDL_FillRect(surface.get(), nullptr, pressedButtonBackgroundColor);
        drawRect(surface.get(), 0, 0, surface->w - 1, surface->h - 1, buttonBorderColor);
        drawRect(surface.get(), 1, 1, surface->w - 2, surface->h - 2, buttonEdgeBottomRightColor);
    }

    // create text on this button
    int fontsize = 0;
    if ((width < getTextWidth(text, 14) + 12) || (height < getTextHeight(14) + 2)) {
        fontsize = 12;
    } else {
        fontsize = 14;
    }

    if (textcolor == COLOR_DEFAULT)
        textcolor = defaultForegroundColor;
    if (textshadowcolor == COLOR_DEFAULT)
        textshadowcolor = defaultShadowColor;

    const sdl2::surface_ptr textSurface1 = createSurfaceWithText(text, textshadowcolor, fontsize);
    SDL_Rect textRect1                   = calcDrawingRect(textSurface1.get(), surface->w / 2 + 2 + (pressed ? 1 : 0),
                                                           surface->h / 2 + 3 + (pressed ? 1 : 0), HAlign::Center, VAlign::Center);
    SDL_BlitSurface(textSurface1.get(), nullptr, surface.get(), &textRect1);

    const sdl2::surface_ptr textSurface2 =
        createSurfaceWithText(text, (activated) ? brightenUp(textcolor) : textcolor, fontsize);
    SDL_Rect textRect2 = calcDrawingRect(textSurface2.get(), surface->w / 2 + 1 + (pressed ? 1 : 0),
                                         surface->h / 2 + 2 + (pressed ? 1 : 0), HAlign::Center, VAlign::Center);
    SDL_BlitSurface(textSurface2.get(), nullptr, surface.get(), &textRect2);

    return surface;
}

Point DuneStyle::getMinimumTextBoxSize(int fontSize) {
    return {10, static_cast<int>(getTextHeight(fontSize)) + 6};
}

sdl2::surface_ptr DuneStyle::createTextBoxSurface(uint32_t width, uint32_t height, std::string_view text, bool carret,
                                                  int fontSize, Alignment_Enum alignment, uint32_t textcolor,
                                                  uint32_t textshadowcolor) {
    sdl2::surface_ptr surface =
        sdl2::surface_ptr{SDL_CreateRGBSurface(0, width, height, SCREEN_BPP, RMASK, GMASK, BMASK, AMASK)};
    if (!surface) {
        return nullptr;
    }

    SDL_FillRect(surface.get(), nullptr, buttonBackgroundColor);
    drawRect(surface.get(), 0, 0, surface->w - 1, surface->h - 1, buttonBorderColor);

    drawHLine(surface.get(), 1, 1, surface->w - 2, buttonEdgeBottomRightColor);
    drawHLine(surface.get(), 1, 2, surface->w - 2, buttonEdgeBottomRightColor);
    drawVLine(surface.get(), 1, 1, surface->h - 2, buttonEdgeBottomRightColor);
    drawVLine(surface.get(), 2, 1, surface->h - 2, buttonEdgeBottomRightColor);
    drawHLine(surface.get(), 1, surface->h - 2, surface->w - 2, buttonEdgeTopLeftColor);
    drawVLine(surface.get(), surface->w - 2, 1, surface->h - 2, buttonEdgeTopLeftColor);

    if (textcolor == COLOR_DEFAULT)
        textcolor = defaultForegroundColor;
    if (textshadowcolor == COLOR_DEFAULT)
        textshadowcolor = defaultShadowColor;

    SDL_Rect cursorPos;

    // create text in this text box
    if (text.size() != 0) {
        const auto textSurface1 = createSurfaceWithText(text, textshadowcolor, fontSize);
        const auto textSurface2 = createSurfaceWithText(text, textcolor, fontSize);
        SDL_Rect textRect1 = calcDrawingRect(textSurface1.get(), 0, surface->h / 2 + 4, HAlign::Left, VAlign::Center);
        SDL_Rect textRect2 = calcDrawingRect(textSurface2.get(), 0, surface->h / 2 + 3, HAlign::Left, VAlign::Center);

        if (alignment & Alignment_Left) {
            textRect1.x = 6;
            textRect2.x = 5;
        } else if (alignment & Alignment_Right) {
            textRect1.x = width - textSurface1->w - 5;
            textRect2.x = width - textSurface2->w - 4;
        } else {
            // Alignment_HCenter
            textRect1.x = ((surface->w - textSurface1->w) / 2) + 3;
            textRect2.x = ((surface->w - textSurface2->w) / 2) + 2;
        }

        if (textRect1.w > surface->w - 10) {
            textRect1.x -= (textSurface1->w - (surface->w - 10));
            textRect2.x -= (textSurface2->w - (surface->w - 10));
        }

        cursorPos.x = textRect2.x + textSurface2->w + 2;

        SDL_BlitSurface(textSurface1.get(), nullptr, surface.get(), &textRect1);

        SDL_BlitSurface(textSurface2.get(), nullptr, surface.get(), &textRect2);

        cursorPos.w = 1;
    } else {
        if (alignment & Alignment_Left) {
            cursorPos.x = 6;
        } else if (alignment & Alignment_Right) {
            cursorPos.x = width - 5;
        } else {
            // Alignment_HCenter
            cursorPos.x = surface->w / 2;
        }
        cursorPos.w = 1;
    }

    cursorPos.y = surface->h / 2 - 8;
    cursorPos.h = 16;

    if (carret) {
        drawVLine(surface.get(), cursorPos.x - 2, cursorPos.y, cursorPos.y + cursorPos.h, textcolor);
        drawVLine(surface.get(), cursorPos.x - 1, cursorPos.y, cursorPos.y + cursorPos.h, textcolor);
    }

    return surface;
}

Point DuneStyle::getMinimumScrollBarArrowButtonSize() {
    return {17, 17};
}

sdl2::surface_ptr DuneStyle::createScrollBarArrowButton(bool down, bool pressed, bool activated, uint32_t color) {
    if (color == COLOR_DEFAULT) {
        color = defaultForegroundColor;
    }

    // create surfaces
    sdl2::surface_ptr surface =
        sdl2::surface_ptr{SDL_CreateRGBSurface(0, 17, 17, SCREEN_BPP, RMASK, GMASK, BMASK, AMASK)};
    if (!surface) {
        return nullptr;
    }

    // create button background
    if (!pressed) {
        // normal mode
        SDL_FillRect(surface.get(), nullptr, buttonBackgroundColor);
        drawRect(surface.get(), 0, 0, surface->w - 1, surface->h - 1, buttonBorderColor);
        drawHLine(surface.get(), 1, 1, surface->w - 2, buttonEdgeTopLeftColor);
        drawVLine(surface.get(), 1, 1, surface->h - 2, buttonEdgeTopLeftColor);
        drawHLine(surface.get(), 1, surface->h - 2, surface->w - 2, buttonEdgeBottomRightColor);
        drawVLine(surface.get(), surface->w - 2, 1, surface->h - 2, buttonEdgeBottomRightColor);
    } else {
        // pressed button mode
        SDL_FillRect(surface.get(), nullptr, pressedButtonBackgroundColor);
        drawRect(surface.get(), 0, 0, surface->w - 1, surface->h - 1, buttonBorderColor);
        drawRect(surface.get(), 1, 1, surface->w - 2, surface->h - 2, buttonEdgeBottomRightColor);
    }

    const int col = (pressed | activated) ? brightenUp(color) : color;

    // draw arrow
    if (down) {
        // down arrow
        drawHLine(surface.get(), 3, 4, 13, col);
        drawHLine(surface.get(), 4, 5, 12, col);
        drawHLine(surface.get(), 5, 6, 11, col);
        drawHLine(surface.get(), 6, 7, 10, col);
        drawHLine(surface.get(), 7, 8, 9, col);
        drawHLine(surface.get(), 8, 9, 8, col);
    } else {
        // up arrow
        drawHLine(surface.get(), 8, 5, 8, col);
        drawHLine(surface.get(), 7, 6, 9, col);
        drawHLine(surface.get(), 6, 7, 10, col);
        drawHLine(surface.get(), 5, 8, 11, col);
        drawHLine(surface.get(), 4, 9, 12, col);
        drawHLine(surface.get(), 3, 10, 13, col);
    }

    return surface;
}

uint32_t DuneStyle::getListBoxEntryHeight() {
    return 16;
}

sdl2::surface_ptr DuneStyle::createListBoxEntry(uint32_t width, std::string_view text, bool selected, uint32_t color) {
    if (color == COLOR_DEFAULT) {
        color = defaultForegroundColor;
    }

    // create surfaces
    sdl2::surface_ptr surface = sdl2::surface_ptr{
        SDL_CreateRGBSurface(0, width, getListBoxEntryHeight(), SCREEN_BPP, RMASK, GMASK, BMASK, AMASK)};
    if (!surface) {
        return nullptr;
    }

    if (selected) {
        SDL_FillRect(surface.get(), nullptr, buttonBackgroundColor);
    } else {
        SDL_FillRect(surface.get(), nullptr, COLOR_TRANSPARENT);
    }

    const sdl2::surface_ptr textSurface = createSurfaceWithText(text, color, 12);
    SDL_Rect textRect = calcDrawingRect(textSurface.get(), 3, surface->h / 2 + 2, HAlign::Left, VAlign::Center);
    SDL_BlitSurface(textSurface.get(), nullptr, surface.get(), &textRect);

    return surface;
}

sdl2::surface_ptr DuneStyle::createProgressBarOverlay(uint32_t width, uint32_t height, double percent, uint32_t color) {
    sdl2::surface_ptr pSurface =
        sdl2::surface_ptr{SDL_CreateRGBSurface(0, width, height, SCREEN_BPP, RMASK, GMASK, BMASK, AMASK)};
    if (!pSurface) {
        return nullptr;
    }

    SDL_FillRect(pSurface.get(), nullptr, COLOR_TRANSPARENT);

    if (color == COLOR_DEFAULT) {
        // default color

        const int max_i = std::max(static_cast<int>(lround(percent * ((static_cast<int>(width) - 4) / 100.0))), 0);

        sdl2::surface_lock lock(pSurface.get());

        const SDL_Rect dest = {2, 2, max_i, static_cast<int>(height) - 4};
        SDL_FillRect(pSurface.get(), &dest, COLOR_HALF_TRANSPARENT);
    } else {
        const int max_i = lround(percent * (width / 100.0));

        const SDL_Rect dest = {0, 0, max_i, static_cast<int>(height)};
        SDL_FillRect(pSurface.get(), &dest, color);
    }

    return pSurface;
}

sdl2::surface_ptr DuneStyle::createToolTip(std::string_view text) {
    const sdl2::surface_ptr helpTextSurface = createSurfaceWithText(text, COLOR_YELLOW, 12);
    if (helpTextSurface == nullptr) {
        return nullptr;
    }

    // create surfaces
    sdl2::surface_ptr surface = sdl2::surface_ptr{SDL_CreateRGBSurface(
        0, helpTextSurface->w + 5, helpTextSurface->h + 2, SCREEN_BPP, RMASK, GMASK, BMASK, AMASK)};
    if (surface == nullptr) {
        return nullptr;
    }

    SDL_FillRect(surface.get(), nullptr, COLOR_HALF_TRANSPARENT);

    drawRect(surface.get(), 0, 0, helpTextSurface->w + 5 - 1, helpTextSurface->h + 2 - 1, COLOR_YELLOW);

    SDL_Rect textRect = calcDrawingRect(helpTextSurface.get(), 3, 3);
    SDL_BlitSurface(helpTextSurface.get(), nullptr, surface.get(), &textRect);

    return surface;
}

sdl2::surface_ptr DuneStyle::createBackground(uint32_t width, uint32_t height) {
    sdl2::surface_ptr pSurface;
    if (pGFXManager) {
        pSurface = getSubPicture(pGFXManager->getBackgroundSurface(), 0, 0, width, height);
        if (!pSurface) {
            return nullptr;
        }
    } else {
        // data manager not yet loaded
        pSurface = sdl2::surface_ptr{SDL_CreateRGBSurface(0, width, height, SCREEN_BPP, RMASK, GMASK, BMASK, AMASK)};
        if (pSurface == nullptr) {
            return nullptr;
        }
        SDL_FillRect(pSurface.get(), nullptr, buttonBackgroundColor);
    }

    drawRect(pSurface.get(), 0, 0, pSurface->w - 1, pSurface->h - 1, buttonBorderColor);
    drawHLine(pSurface.get(), 1, 1, pSurface->w - 2, buttonEdgeTopLeftColor);
    drawHLine(pSurface.get(), 2, 2, pSurface->w - 3, buttonEdgeTopLeftColor);
    drawVLine(pSurface.get(), 1, 1, pSurface->h - 2, buttonEdgeTopLeftColor);
    drawVLine(pSurface.get(), 2, 2, pSurface->h - 3, buttonEdgeTopLeftColor);
    drawHLine(pSurface.get(), 1, pSurface->h - 2, pSurface->w - 2, buttonEdgeBottomRightColor);
    drawHLine(pSurface.get(), 2, pSurface->h - 3, pSurface->w - 3, buttonEdgeBottomRightColor);
    drawVLine(pSurface.get(), pSurface->w - 2, 1, pSurface->h - 2, buttonEdgeBottomRightColor);
    drawVLine(pSurface.get(), pSurface->w - 3, 2, pSurface->h - 3, buttonEdgeBottomRightColor);

    return pSurface;
}

sdl2::surface_ptr DuneStyle::createWidgetBackground(uint32_t width, uint32_t height) {
    sdl2::surface_ptr surface =
        sdl2::surface_ptr{SDL_CreateRGBSurface(0, width, height, SCREEN_BPP, RMASK, GMASK, BMASK, AMASK)};

    if (!surface) {
        return nullptr;
    }

    SDL_FillRect(surface.get(), nullptr, pressedButtonBackgroundColor);
    drawRect(surface.get(), 0, 0, surface->w - 1, surface->h - 1, buttonBorderColor);
    drawRect(surface.get(), 1, 1, surface->w - 2, surface->h - 2, buttonEdgeBottomRightColor);

    return surface;
}
