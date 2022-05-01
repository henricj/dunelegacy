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

#ifndef DUNESTYLE_H
#define DUNESTYLE_H

#include "Renderer/DuneSurface.h"
#include <GUI/GUIStyle.h>

class FontManager;

class DuneStyle final : public GUIStyle {
public:
    /// default constructor
    explicit DuneStyle(FontManager* fontManager);

    /// destructor
    ~DuneStyle() override;

    /**
        Returns the minimum size of a label with this text
        \param  text    The text for the label
        \param  fontSize  The size of the font to use
        \return the minimum size of this label
    */
    Point getMinimumLabelSize(std::string_view text, int fontSize) const override;

    /**
        Creates the surface for a label with TextLines as content.
        \param  width           the width of the label
        \param  height          the height of the label
        \param  textLines       a vector of text lines for this label
        \param  fontSize        the size of the font to use
        \param  alignment       the alignment for this label
        \param  textcolor       the color of the text (COLOR_DEFAULT = default color for this style)
        \param  textshadowcolor the color of the shadow under the text (COLOR_DEFAULT = default color for this style)
        \param  backgroundcolor the background color (default is transparent)
        \return the new surface
    */
    sdl2::surface_ptr createLabelSurface(uint32_t width, uint32_t height, const std::vector<std::string>& textLines,
                                         int fontSize, Alignment_Enum alignment = Alignment_HCenter,
                                         Uint32 textcolor = COLOR_DEFAULT, Uint32 textshadowcolor = COLOR_DEFAULT,
                                         Uint32 backgroundcolor = COLOR_TRANSPARENT) const override;

    /**
        Creates the surface for a label with TextLines as content.
        \param  width           the width of the label
        \param  height          the height of the label
        \param  textLines       a vector of text lines for this label
        \param  fontSize        the size of the font to use
        \param  alignment       the alignment for this label
        \param  textcolor       the color of the text (COLOR_DEFAULT = default color for this style)
        \param  textshadowcolor the color of the shadow under the text (COLOR_DEFAULT = default color for this style)
        \param  backgroundcolor the background color (default is transparent)
        \return the new surface
    */
    [[nodiscard]] DuneTextureOwned
    createLabel(SDL_Renderer* renderer, uint32_t width, uint32_t height, const std::vector<std::string>& textLines,
                int fontSize, Alignment_Enum alignment = Alignment_HCenter, Uint32 textcolor = COLOR_DEFAULT,
                Uint32 textshadowcolor = COLOR_DEFAULT, Uint32 backgroundcolor = COLOR_TRANSPARENT) const override;

    /**
        Returns the minimum size of a checkbox with this text
        \param  text    The text for the checkbox
        \return the minimum size of this checkbox
    */
    Point getMinimumCheckboxSize(std::string_view text) const override;

    /**
        Creates the surface for a checkbox with text as content.
        \param  width           the width of the checkbox
        \param  height          the height of the checkbox
        \param  text            the text for this checkbox
        \param  checked         true, if the checkbox is checked, false otherwise
        \param  activated       true if the checkbox is activated (e.g. mouse hover)
        \param  textcolor       the color of the text (COLOR_DEFAULT = default color for this style)
        \param  textshadowcolor the color of the shadow under the text (COLOR_DEFAULT = default color for this style)
        \param  backgroundcolor the background color (default is transparent)
        \return the new surface
    */
    DuneSurfaceOwned
    createCheckboxSurface(uint32_t width, uint32_t height, std::string_view text, bool checked, bool activated,
                          Uint32 textcolor = COLOR_DEFAULT, Uint32 textshadowcolor = COLOR_DEFAULT,
                          Uint32 backgroundcolor = COLOR_TRANSPARENT) const override;

    /**
        Returns the minimum size of a radio button with this text
        \param  text    The text for the radio button
        \return the minimum size of this radio button
    */
    Point getMinimumRadioButtonSize(std::string_view text) const override;

    /**
        Creates the surface for a radio button with text as content.
        \param  width           the width of the radio button
        \param  height          the height of the radio button
        \param  text            the text for this radio button
        \param  checked         true, if the radio button is checked, false otherwise
        \param  activated       true if the radio button is activated (e.g. mouse hover)
        \param  textcolor       the color of the text (COLOR_DEFAULT = default color for this style)
        \param  textshadowcolor the color of the shadow under the text (COLOR_DEFAULT = default color for this style)
        \param  backgroundcolor the background color (default is transparent)
        \return the new surface
    */
    DuneSurfaceOwned
    createRadioButtonSurface(uint32_t width, uint32_t height, std::string_view text, bool checked, bool activated,
                             Uint32 textcolor = COLOR_DEFAULT, Uint32 textshadowcolor = COLOR_DEFAULT,
                             Uint32 backgroundcolor = COLOR_TRANSPARENT) const override;

    /**
        Creates the surface for a drop down box
        \param  size        the width and height of the drop down button
        \param  pressed     true if the button should be pressed
        \param  activated   true if the button is activated (e.g. mouse hover)
        \param  color       the color of the text (COLOR_DEFAULT = default color for this style)
        \return the new surface
    */
    DuneSurfaceOwned
    createDropDownBoxButton(int size, bool pressed, bool activated, Uint32 color = COLOR_DEFAULT) const override;

    /**
        Returns the minimum size of a button with this text
        \param  text    The text for the button
        \return the minimum size of this button
    */
    Point getMinimumButtonSize(std::string_view text) const override;

    /**
        Creates the surface for a button with text as content.
        \param  width           the width of the button
        \param  height          the height of the button
        \param  text            the text for this button
        \param  pressed         true if the button should be pressed
        \param  activated       true if the button is activated (e.g. mouse hover)
        \param  textcolor       the color of the text (COLOR_DEFAULT = default color for this style)
        \param  textshadowcolor the color of the shadow under the text (COLOR_DEFAULT = default color for this style)
        \return the new surface
    */
    sdl2::surface_ptr
    createButtonSurface(uint32_t width, uint32_t height, std::string_view text, bool pressed, bool activated,
                        Uint32 textcolor = COLOR_DEFAULT, Uint32 textshadowcolor = COLOR_DEFAULT) const override;

    DuneSurfaceOwned
    createButtonText(uint32_t width, uint32_t height, std::string_view text, bool activated,
                     Uint32 textcolor = COLOR_DEFAULT, Uint32 textshadowcolor = COLOR_DEFAULT) const override;

    void RenderButton(SDL_Renderer* renderer, const SDL_FRect& dest, const DuneTexture* content,
                      bool pressed) const override;

    /**
        Returns the minimum size of a text box
        \param  fontSize  The size of the font to use
        \return the minimum size of a text box
    */
    Point getMinimumTextBoxSize(int fontSize) const override;

    /**
        Creates the surface for a text box with text as content.
        \param  width           the width of the text box
        \param  height          the height of the text box
        \param  text            the text for this text box
        \param  caret           true if a caret should be shown
        \param  fontSize        the size of the font to use
        \param  alignment       the alignment for this text box
        \param  textcolor       the color of the text (COLOR_DEFAULT = default color for this style)
        \param  textshadowcolor the color of the shadow under the text (COLOR_DEFAULT = default color for this style)
        \return the new surface
    */
    DuneSurfaceOwned
    createTextBoxSurface(uint32_t width, uint32_t height, std::string_view text, bool caret, int fontSize,
                         Alignment_Enum alignment = Alignment_Left, Uint32 textcolor = COLOR_DEFAULT,
                         Uint32 textshadowcolor = COLOR_DEFAULT) const override;

    /**
        Returns the minimum size of a scroll bar arrow button.
        \return the minimum size of a scroll bar arrow
    */
    Point getMinimumScrollBarArrowButtonSize() const override;

    /**
        Creates the surface for a scroll bar arrow button.
        \param  down        true = downward arrow, false = upward arrow
        \param  pressed     true if the button should be pressed
        \param  activated   true if the button is activated (e.g. mouse hover)
        \param  color       the color of the text (COLOR_DEFAULT = default color for this style)
        \return the new surface
    */
    sdl2::surface_ptr
    createScrollBarArrowButton(bool down, bool pressed, bool activated, Uint32 color = COLOR_DEFAULT) const override;

    /**
        Returns the minimum height of a list box entry.
        \return the minimum height of a list box entry
    */
    int getListBoxEntryHeight() const override;

    /**
        Creates the surface for a list box entry with text as content.
        \param  width       the width of the entry
        \param  text        the text for this entry
        \param  selected    true if a entry should be highlighted
        \param  color       the color of the text (COLOR_DEFAULT = default color for this style)
        \return the new surface
    */
    DuneSurfaceOwned
    createListBoxEntry(int width, std::string_view text, bool selected, Uint32 color = COLOR_DEFAULT) const override;

    /**
        Creates a tool tip surface.
        \param  renderer    the renderer for the texture
        \param  text        the tool tip text
        \return the new surface
    */
    DuneTextureOwned createToolTip(SDL_Renderer* renderer, std::string_view text) const override;

    /**
        Draw a frame
        \param  renderer    the renderer to use
        \param  rect        the area to draw
        \return the new surface
    */
    void drawFrame(SDL_Renderer* renderer, DecorationFrame decorationType, const SDL_FRect& rect) override;

    /**
        Draw a simple background for e.g. a window
        \param  renderer    the renderer to use
        \param  rect        the area to draw
        \return the new surface
    */
    void drawBackground(SDL_Renderer* renderer, const SDL_FRect& rect) override;

    /**
        Draw the main background
        \param  renderer    the renderer to use
        \param  rect        the area to draw
        \return the new surface
    */
    void drawMainBackground(SDL_Renderer* renderer, const SDL_FRect& rect) override;

    /**
        Creates a simple background for widgets
        \param  width       the width of the surface
        \param  height      the height of the surface
        \return the new surface
    */
    DuneSurfaceOwned createWidgetBackground(int width, int height) const override;

    /**
        Get the height of the font specified by fontnum
        \param  FontNum     the font
        \return the height of the font
    */
    float getTextHeight(unsigned int FontNum) const override;

    /**
        Get the width of the text with the font specified by fontnum
        \param  text        the text to get the width from
        \param  FontNum     the font
        \return the width of the text
    */
    float getTextWidth(std::string_view text, unsigned int FontNum) const override;

    [[nodiscard]] DuneTextureOwned
    createText(SDL_Renderer* renderer, std::string_view text, uint32_t color, unsigned fontSize) const override;

    [[nodiscard]] DuneTextureOwned createMultilineText(SDL_Renderer* renderer, std::string_view text, uint32_t color,
                                                       unsigned fontSize, bool bCentered) const override;

public:
    static constexpr Uint32 defaultForegroundColor = COLOR_RGB(125, 0, 0);
    static constexpr Uint32 defaultShadowColor     = COLOR_LIGHTYELLOW;

    static constexpr Uint32 buttonBackgroundColor        = COLOR_RGB(202, 141, 16);
    static constexpr Uint32 pressedButtonBackgroundColor = COLOR_RGB(182, 125, 12);
    static constexpr Uint32 buttonBorderColor            = COLOR_RGB(60, 36, 0);
    static constexpr Uint32 buttonEdgeBottomRightColor   = COLOR_RGB(153, 105, 0);
    static constexpr Uint32 buttonEdgeTopLeftColor       = COLOR_RGB(255, 190, 76);

private:
    /**
        Creates a surface with text on it
        \param  text        text to draw
        \param  color       the color of the text
        \param  fontsize    the size of the text
        \return the new created surface (the caller of this method is responsible of freeing it)
    */
    DuneSurfaceOwned createSurfaceWithText(std::string_view text, uint32_t color, unsigned int fontsize) const;

    /**
        Creates a surface with multi-line text on it
        \param  text        text to draw
        \param  color       the color of the text
        \param  fontsize    the size of the text
        \return the new created surface (the caller of this method is responsible of freeing it)
    */
    DuneSurfaceOwned
    createSurfaceWithMultilineText(std::string_view text, uint32_t color, unsigned int fontsize, bool bCentered) const;

    constexpr uint32_t brightenUp(uint32_t color) const {
        Uint32 r       = (color & RMASK) >> RSHIFT;
        Uint32 g       = (color & GMASK) >> GSHIFT;
        Uint32 b       = (color & BMASK) >> BSHIFT;
        const Uint32 a = (color & AMASK) >> ASHIFT;

        r = std::min(255U, (r * 3) / 2);
        g = std::min(255U, (g * 3) / 2);
        b = std::min(255U, (b * 3) / 2);

        return COLOR_RGBA(r, g, b, a);
    }

    uint32_t scaledFontSize(uint32_t font_size) const;
    int getPhysicalTextHeight(unsigned FontNum) const;

    FontManager* fontManager_{};
};

#endif // DUNESTYLEBASE_H
