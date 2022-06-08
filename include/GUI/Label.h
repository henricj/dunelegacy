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

#ifndef LABEL_H
#define LABEL_H

#include "GUIStyle.h"
#include "Widget.h"

#include "Renderer/DuneTexture.h"
#include <misc/SDL2pp.h>

#include <string>
#include <string_view>

/// a class for a text label
class Label final : public Widget {
    using parent = Widget;

public:
    /// default constructor
    Label();

    /// destructor
    ~Label() override;

    /**
        Sets a font size for this label. Default font size of a label is 14
        \param  fontSize      the font size of the new font
    */
    virtual void setTextFontSize(int fontSize) {
        this->fontSize_ = fontSize;
        resizeAll();
    }

    /**
        Gets the font size of this label. Default font size of a label is 14
        \return the font size of this label
    */
    [[nodiscard]] virtual int getTextFontSize() const { return fontSize_; }

    /**
        Sets the text color for this label.
        \param  textcolor       the color of the text (COLOR_DEFAULT = default color)
        \param  textshadowcolor the color of the shadow of the text (COLOR_DEFAULT = default color)
        \param  backgroundcolor the color of the label background (COLOR_TRANSPARENT = transparent)
    */
    virtual void setTextColor(uint32_t textcolor, Uint32 textshadowcolor = COLOR_DEFAULT,
                              Uint32 backgroundcolor = COLOR_TRANSPARENT) {
        this->text_color_        = textcolor;
        this->text_shadow_color_ = textshadowcolor;
        this->background_color_  = backgroundcolor;
        invalidateTextures();
    }

    /**
        Sets the alignment of the text in this label.
        \param alignment Combination of (Alignment_HCenter, Alignment_Left or Alignment_Right) and (Alignment_VCenter,
       Alignment_Top or Alignment_Bottom)
    */
    virtual void setAlignment(Alignment_Enum alignment) {
        this->alignment_ = alignment;
        invalidateTextures();
    }

    /**
        Returns the alignment of the text in this label.
        \return Combination of (Alignment_HCenter, Alignment_Left or Alignment_Right) and (Alignment_VCenter,
       Alignment_Top or Alignment_Bottom)
    */
    [[nodiscard]] virtual Alignment_Enum getAlignment() const { return alignment_; }

    /**
        This method sets a new text for this label and resizes this label
        to fit this text.
        \param  text The new text for this button
    */
    virtual void setText(std::string_view text) {
        if (text != this->text_) {
            this->text_ = text;
            resizeAll();
        }
    }

    /**
        This method sets a new text for this label and resizes this label
        to fit this text.
        \param  text The new text for this button
    */
    virtual void setText(std::string text) {
        if (text != this->text_) {
            this->text_ = std::move(text);
            resizeAll();
        }
    }

    /**
        Get the text of this label.
        \return the text of this button
    */
    [[nodiscard]] const std::string& getText() const { return text_; }

    /**
        This method resizes the label to width and height. This method should only
        called if the new size is a valid size for this label (See getMinimumSize).
        \param  width   the new width of this label
        \param  height  the new height of this label
    */
    void resize(uint32_t width, uint32_t height) override;

    using parent::resize;

    /**
        Returns the minimum size of this label. The label should not
        resized to a size smaller than this.
        \return the minimum size of this label
    */
    [[nodiscard]] Point getMinimumSize() const override;

    /**
        Draws this label to screen.
        \param  position    Position to draw the label to
    */
    void draw(Point position) override;

    /**
        This static method creates a dynamic label object with Text as the label text.
        The idea behind this method is to simply create a new text label on the fly and
        add it to a container. If the container gets destroyed also this label will be freed.
        \param  text    The label text
       \param   textcolor       the color of the text (COLOR_DEFAULT = default color)
       \param   textshadowcolor the color of the shadow of the text (COLOR_DEFAULT = default color)
       \param  backgroundcolor the color of the label background (COLOR_TRANSPARENT = transparent)
        \return The new created label (will be automatically destroyed when it's parent widget is destroyed)
    */
    static std::unique_ptr<Label>
    create(std::string text, Uint32 textcolor = COLOR_DEFAULT, Uint32 textshadowcolor = COLOR_DEFAULT,
           Uint32 backgroundcolor = COLOR_TRANSPARENT);

    /**
        This static method creates a dynamic label object with Text as the label text.
        The idea behind this method is to simply create a new text label on the fly and
        add it to a container. If the container gets destroyed also this label will be freed.
        \param  text    The label text
       \param   textcolor       the color of the text (COLOR_DEFAULT = default color)
       \param   textshadowcolor the color of the shadow of the text (COLOR_DEFAULT = default color)
       \param  backgroundcolor the color of the label background (COLOR_TRANSPARENT = transparent)
        \return The new created label (will be automatically destroyed when it's parent widget is destroyed)
    */
    static std::unique_ptr<Label>
    create(std::string_view text, Uint32 textcolor = COLOR_DEFAULT, Uint32 textshadowcolor = COLOR_DEFAULT,
           Uint32 backgroundcolor = COLOR_TRANSPARENT) {
        return create(std::string{text}, textcolor, textshadowcolor, backgroundcolor);
    }

protected:
    /**
        This method is called whenever the textures of this widget are needed, e.g. before drawing. This method
        should be overwritten by subclasses if they like to defer texture creation as long as possible.
        This method should first check whether a renewal of the textures is necessary.
    */
    void updateTextures() override;

    /**
        This method frees all textures that are used by this label
    */
    void invalidateTextures() override;

private:
    int fontSize_             = 14;                ///< the size of the font to use
    Uint32 text_color_        = COLOR_DEFAULT;     ///< the text color
    Uint32 text_shadow_color_ = COLOR_DEFAULT;     ///< the color of the shadow of the text
    Uint32 background_color_  = COLOR_TRANSPARENT; ///< the color of the label background
    std::string text_;                             ///< the text of this label
    DuneTextureOwned pTexture_;                    ///< the texture of this label
    Alignment_Enum alignment_ =
        static_cast<Alignment_Enum>(Alignment_Left | Alignment_VCenter); ///< the alignment of this label
};

#endif // LABEL_H
