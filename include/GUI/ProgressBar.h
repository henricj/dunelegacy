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

#ifndef PROGRESSBAR_H
#define PROGRESSBAR_H

#include "Widget.h"

#include "GUIStyle.h"

#include <string>
#include <variant>

/// A class for a progress bar widget
class ProgressBar : public Widget {
    using parent = Widget;

public:
    /// default constructor
    ProgressBar();

    /// destructor
    ~ProgressBar() override;

    ProgressBar(const ProgressBar&)            = delete;
    ProgressBar(ProgressBar&&)                 = delete;
    ProgressBar& operator=(const ProgressBar&) = delete;
    ProgressBar& operator=(ProgressBar&&)      = delete;

    /**
        Sets the progress of this progress bar.
        \param newPercent  Should be between 0.0 and 100.0
    */
    void setProgress(float newPercent);

    /**
        Return the current progress.
        \return the current progress in percent
    */
    [[nodiscard]] double getProgress() const noexcept { return percent; }

    /**
        Sets the color of the progress bar overlay
        \param  color   the new color (COLOR_DEFAULT = default)
    */
    void setColor(Uint32 color = COLOR_DEFAULT) {
        this->color = color;
        invalidateTextures();
    }

    /**
        Specifies if a shadow is drawn or not.
        \param  bDrawShadow if true, a shadow is drawn
    */
    void setDrawShadow(bool bDrawShadow) { this->bDrawShadow = bDrawShadow; }

    /**
        This method resizes the progress bar to width and height. This method should only
        called if the new size is a valid size for this progress bar (See getMinimumSize).
        \param  width   the new width of this progress bar
        \param  height  the new height of this progress bar
    */
    void resize(uint32_t width, uint32_t height) override {
        parent::resize(width, height);

        invalidateTextures();
    }

    using parent::resize;

    /**
        Draws this progress bar to screen
        \param  position    Position to draw the progress bar to
    */
    void draw(Point position) override;

protected:
    float percent{};                ///< Percent from 0.0 to 100.0
    uint32_t color = COLOR_DEFAULT; ///< The color of the progress overlay
    bool bDrawShadow{};             ///< Draw shadow under the foreground surface
    std::variant<std::monostate, DuneTextureOwned, const DuneTexture*> pContent;
};

class TextProgressBar final : public ProgressBar {
    using parent = ProgressBar;

public:
    TextProgressBar();

    ~TextProgressBar() override;

    TextProgressBar(const TextProgressBar&)            = delete;
    TextProgressBar(TextProgressBar&&)                 = delete;
    TextProgressBar& operator=(const TextProgressBar&) = delete;
    TextProgressBar& operator=(TextProgressBar&&)      = delete;
    /**
        This method sets a new text for this progress bar and resizes it
        to fit this text.
        \param  text The new text for this progress bar
    */
    virtual void setText(std::string text);

    /**
        Get the text of this progress bar.
        \return the text of this button
    */
    [[nodiscard]] const std::string& getText() const noexcept { return text; }

    /**
        Sets the text color for this progress bar.
        \param  textcolor       the color of the text (COLOR_DEFAULT = default color)
        \param  textshadowcolor the color of the shadow of the text (COLOR_DEFAULT = default color)
    */
    virtual void setTextColor(uint32_t textcolor, Uint32 textshadowcolor = COLOR_DEFAULT) {
        this->textcolor       = textcolor;
        this->textshadowcolor = textshadowcolor;
        invalidateTextures();
    }

    /**
        Returns the minimum size of this progress bar. The progress bar should not
        resized to a size smaller than this.
        \return the minimum size of this progress bar
    */
    [[nodiscard]] Point getMinimumSize() const override;

protected:
    /**
        This method is called whenever the textures of this widget are needed, e.g. before drawing. This method
        should be overwritten by subclasses if they like to defer texture creation as long as possible.
        This method should first check whether a renewal of the textures is necessary.
    */
    void updateTextures() override;

    /**
        This method frees all textures that are used by this widget
    */
    void invalidateTextures() override;

    std::string text; ///< Text of this progress bar

    Uint32 textcolor       = COLOR_DEFAULT;
    Uint32 textshadowcolor = COLOR_DEFAULT;
};

class PictureProgressBar final : public ProgressBar {
    using parent = ProgressBar;

public:
    PictureProgressBar();

    ~PictureProgressBar() override;

    PictureProgressBar(const PictureProgressBar&)            = delete;
    PictureProgressBar(PictureProgressBar&&)                 = delete;
    PictureProgressBar& operator=(const PictureProgressBar&) = delete;
    PictureProgressBar& operator=(PictureProgressBar&&)      = delete;

    void setTexture(const DuneTexture* content);

    /**
        Returns the minimum size of this progress bar. The progress bar should not
        resized to a size smaller than this.
        \return the minimum size of this progress bar
    */
    [[nodiscard]] Point getMinimumSize() const override;
};

#endif // PROGRESSBAR_H
