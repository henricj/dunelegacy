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

#ifndef GUISTYLE_H
#define GUISTYLE_H

#include <Colors.h>

#include <misc/SDL2pp.h>
#include <misc/exceptions.h>

#include "Widget.h"

#include <memory>
#include <string_view>

#include "Renderer/DuneSurface.h"

enum class DecorationFrame;

inline constexpr auto COLOR_DEFAULT = COLOR_INVALID;

enum Alignment_Enum {
    Alignment_VCenter = 1,
    Alignment_Top     = 2,
    Alignment_Bottom  = 4,
    Alignment_HCenter = 8,
    Alignment_Left    = 16,
    Alignment_Right   = 32
};

class GUIStyle {
public:
    inline static constexpr auto MINIMUM_WIDTH  = 640;
    inline static constexpr auto MINIMUM_HEIGHT = 480;

    /// default constructor
    GUIStyle();

    /// destructor
    virtual ~GUIStyle();

    static void setGUIStyle(std::unique_ptr<GUIStyle> newGUIStyle) { currentGUIStyle = std::move(newGUIStyle); }

    static void destroyGUIStyle() { currentGUIStyle.reset(); }

    static GUIStyle& getInstance() {
        if (currentGUIStyle == nullptr) {
            THROW(std::runtime_error,
                  "GUIStyle::getInstance(): currentGUIStyle == nullptr. Call setGUIStyle before using getInstance()!");
        }
        return *currentGUIStyle;
    }

    /**
        Returns the current UI drawing zoom.
        \return the zoom (nominally 1.0f)
    */
    float getZoom() const noexcept { return zoom_; }

    /**
        Set the current UI drawing zoom.
        \param zoom The drawing zoom (nominally 1.0f)
    */
    virtual void setZoom(float zoom);

    /**
     * Set the display DPI as a fraction of the nominal DPI.
     * Microsoft uses 96dpi and Apple 72dpi, so the scale is
     * represented as a ratio of the actual with respect to the
     * nominal.  For example, a Windows DPI of 144 would set
     * scale to 144 / 96 = 1.5.
     * \param ratio The current display's scale
     */
    virtual void setDisplayDpi(float ratio);

    /**
        Returns the current display DPI ratio.
        \return the display DPI ratio (nominally 1.0f)
    */
    float getDisplayDpi() const noexcept { return dpi_ratio_; }

    /**
        Returns the overall UI scaling factor.
        \return the UI scaling factor (nominally 1.0f)
    */
    float getScale() const noexcept { return dpi_ratio_ * zoom_; }

    /**
       Returns the actual UI scaling factor, taking into account constraints like
       the window size.
        \return the UI scaling factor (nominally 1.0f)
     */
    float getActualScale() const noexcept { return actual_scale_; }

    /**
        Compute the logical size of the output from the physical size.
     */
    virtual void setLogicalSize(SDL_Renderer* renderer, int physical_width, int physical_height);

    /**
        Returns the minimum size of a label with this text
        \param  text    The text for the label
        \param  fontSize  The size of the font to use
        \return the minimum size of this label
    */
    virtual Point getMinimumLabelSize(std::string_view text, int fontSize) const = 0;

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
    virtual sdl2::surface_ptr
    createLabelSurface(uint32_t width, uint32_t height, const std::vector<std::string>& textLines, int fontSize,
                       Alignment_Enum alignment = Alignment_HCenter, Uint32 textcolor = COLOR_DEFAULT,
                       Uint32 textshadowcolor = COLOR_DEFAULT, Uint32 backgroundcolor = COLOR_TRANSPARENT) const = 0;

    /**
        Creates the texture for a label with TextLines as content.
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
    virtual DuneTextureOwned
    createLabel(SDL_Renderer* renderer, uint32_t width, uint32_t height, const std::vector<std::string>& textLines,
                int fontSize, Alignment_Enum alignment = Alignment_HCenter, Uint32 textcolor = COLOR_DEFAULT,
                Uint32 textshadowcolor = COLOR_DEFAULT, Uint32 backgroundcolor = COLOR_TRANSPARENT) const = 0;

    /**
        Returns the minimum size of a checkbox with this text
        \param  text    The text for the checkbox
        \return the minimum size of this checkbox
    */
    virtual Point getMinimumCheckboxSize(std::string_view text) const = 0;

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
    virtual DuneSurfaceOwned
    createCheckboxSurface(uint32_t width, uint32_t height, std::string_view text, bool checked, bool activated,
                          Uint32 textcolor = COLOR_DEFAULT, Uint32 textshadowcolor = COLOR_DEFAULT,
                          Uint32 backgroundcolor = COLOR_TRANSPARENT) const = 0;

    /**
        Returns the minimum size of a radio button with this text
        \param  text    The text for the radio button
        \return the minimum size of this radio button
    */
    virtual Point getMinimumRadioButtonSize(std::string_view text) const = 0;

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
    virtual DuneSurfaceOwned
    createRadioButtonSurface(uint32_t width, uint32_t height, std::string_view text, bool checked, bool activated,
                             Uint32 textcolor = COLOR_DEFAULT, Uint32 textshadowcolor = COLOR_DEFAULT,
                             Uint32 backgroundcolor = COLOR_TRANSPARENT) const = 0;

    /**
        Creates the surface for a drop down box
        \param  size        the width and height of the drop down button
        \param  pressed     true if the button should be pressed
        \param  activated   true if the button is activated (e.g. mouse hover)
        \param  color       the color of the text (COLOR_DEFAULT = default color for this style)
        \return the new surface
    */
    virtual DuneSurfaceOwned
    createDropDownBoxButton(int size, bool pressed, bool activated, Uint32 color = COLOR_DEFAULT) const = 0;

    /**
        Returns the minimum size of a button with this text
        \param  text    The text for the button
        \return the minimum size of this button
    */
    virtual Point getMinimumButtonSize(std::string_view text) const = 0;

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
    virtual sdl2::surface_ptr
    createButtonSurface(uint32_t width, uint32_t height, std::string_view text, bool pressed, bool activated,
                        Uint32 textcolor = COLOR_DEFAULT, Uint32 textshadowcolor = COLOR_DEFAULT) const = 0;

    virtual DuneSurfaceOwned
    createButtonText(uint32_t width, uint32_t height, std::string_view text, bool activated,
                     Uint32 textcolor = COLOR_DEFAULT, Uint32 textshadowcolor = COLOR_DEFAULT) const = 0;

    virtual void
    RenderButton(SDL_Renderer* renderer, const SDL_FRect& dest, const DuneTexture* content, bool pressed) const = 0;

    /**
        Returns the minimum size of a text box
        \param  fontSize  The size of the font to use
        \return the minimum size of a text box
    */
    virtual Point getMinimumTextBoxSize(int fontSize) const = 0;

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
    virtual DuneSurfaceOwned
    createTextBoxSurface(uint32_t width, uint32_t height, std::string_view text, bool caret, int fontSize,
                         Alignment_Enum alignment = Alignment_Left, Uint32 textcolor = COLOR_DEFAULT,
                         Uint32 textshadowcolor = COLOR_DEFAULT) const = 0;

    /**
        Returns the minimum size of a scroll bar arrow button.
        \return the minimum size of a scroll bar arrow
    */
    virtual Point getMinimumScrollBarArrowButtonSize() const = 0;

    /**
        Creates the surface for a scroll bar arrow button.
        \param  down        true = downward arrow, false = upward arrow
        \param  pressed     true if the button should be pressed
        \param  activated   true if the button is activated (e.g. mouse hover)
        \param  color       the color of the text (COLOR_DEFAULT = default color for this style)
        \return the new surface
    */
    virtual sdl2::surface_ptr
    createScrollBarArrowButton(bool down, bool pressed, bool activated, Uint32 color = COLOR_DEFAULT) const = 0;

    /**
        Returns the minimum height of a list box entry.
        \return the minimum height of a list box entry
    */
    virtual int getListBoxEntryHeight() const = 0;

    /**
        Creates the surface for a list box entry with text as content.
        \param  width       the width of the entry
        \param  text        the text for this entry
        \param  selected    true if a entry should be highlighted
        \param  color       the color of the text (COLOR_DEFAULT = default color for this style)
        \return the new surface
    */
    virtual DuneSurfaceOwned
    createListBoxEntry(int width, std::string_view text, bool selected, Uint32 color = COLOR_DEFAULT) const = 0;

    /**
        Creates a tool tip surface.
        \param  renderer    the renderer to use
        \param  text        the tool tip text
        \return the new surface
    */
    virtual DuneTextureOwned createToolTip(SDL_Renderer* renderer, std::string_view text) const = 0;

    /**
        Draw a frame
        \param  renderer        the renderer to use
        \param  decorationType  the type of frame to draw
        \param  rect            the area to draw
        \return the new surface
    */
    virtual void drawFrame(SDL_Renderer* renderer, DecorationFrame decorationType, const SDL_FRect& rect) = 0;

    /**
        Draw a simple background for e.g. a window
        \param  renderer    the renderer to use
        \param  rect        the area to draw
        \return the new surface
    */
    virtual void drawBackground(SDL_Renderer* renderer, const SDL_FRect& rect) = 0;

    /**
        Draw the main background
        \param  renderer    the renderer to use
        \param  rect        the area to draw
        \return the new surface
    */
    virtual void drawMainBackground(SDL_Renderer* renderer, const SDL_FRect& rect) = 0;

    /**
        Creates a simple background for widgets
        \param  width       the width of the surface
        \param  height      the height of the surface
        \return the new surface
    */
    virtual DuneSurfaceOwned createWidgetBackground(int width, int height) const = 0;

    /**
        Creates an empty surface. This surface is transparent or black.
        \param  width       the width of the surface
        \param  height      the height of the surface
        \param  transparent true = transparent surface, false = black
        \return the new surface
    */
    virtual sdl2::surface_ptr createEmptySurface(uint32_t width, uint32_t height, bool transparent) const;

    /**
        Get the height of the font specified by fontnum
        \param  FontNum     the font
        \return the height of the font
    */
    virtual float getTextHeight(unsigned int FontNum) const = 0;

    /**
        Get the width of the text with the font specified by fontnum
        \param  text        the text to get the width from
        \param  FontNum     the font
        \return the width of the text
    */
    virtual float getTextWidth(std::string_view text, unsigned int FontNum) const = 0;

    virtual DuneTextureOwned
    createText(SDL_Renderer* renderer, std::string_view text, uint32_t color, unsigned int fontSize) const = 0;

    virtual DuneTextureOwned createMultilineText(SDL_Renderer* renderer, std::string_view text, uint32_t color,
                                                 unsigned int fontSize, bool bCentered = false) const = 0;

    /**

    */
    std::pair<int, int> getPhysicalSize(int width, int height) const;

protected:
    int scale_to_physical_integer(int logical) const {
        return static_cast<int>(std::round(logical * getActualScale()));
    }

private:
    static std::unique_ptr<GUIStyle> currentGUIStyle;
    float zoom_         = 1.f;
    float dpi_ratio_    = 1.f;
    float actual_scale_ = 1.f;
};

#endif // GUISTYLE_H
