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

#include "WidgetWithBackground.h"

#include "misc/DrawingRectHelper.h"
#include <misc/SDL2pp.h>
#include <misc/draw_util.h>

#include <string>

#include <GUI/GUIStyle.h>

#include <cmath>

/// A class for a progress bar widget
class ProgressBar : public WidgetWithBackground {
    using parent = WidgetWithBackground;

public:
    /// default constructor
    ProgressBar();

    /// destructor
    ~ProgressBar() override;

    ProgressBar(const ProgressBar&) = delete;
    ProgressBar(ProgressBar&&)      = delete;
    ProgressBar& operator=(const ProgressBar&) = delete;
    ProgressBar& operator=(ProgressBar&&) = delete;

    /**
        Sets the progress of this progress bar.
        \param newPercent  Should be between 0.0 and 100.0
    */
    void setProgress(double newPercent) {
        if (percent != newPercent) {
            percent = newPercent;
            if (percent < 0.0) {
                percent = 0.0;
            } else if (percent > 100.0) {
                percent = 100.0;
            }

            invalidateTextures();
        }
    }

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
        This method resized the progress bar. This method should only
        called if the new size is a valid size for this progress bar (See getMinimumSize).
        \param  newSize the new size of this progress bar
    */
    void resize(Point newSize) override { resize(newSize.x, newSize.y); }

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

    /**
        Draws this progress bar to screen
        \param  position    Position to draw the progress bar to
    */
    void draw(Point position) override;

protected:
    /**
        This method is called whenever the textures of this widget are needed, e.g. before drawing. This method
        should be overwritten by subclasses if they like to defer texture creation as long as possible.
        This method should first check whether a renewal of the textures is necessary.
    */
    void updateTextures() override;

    /**
        This method frees all textures that are used by this progress bar
    */
    void invalidateTextures() override;

    sdl2::texture_ptr pForeground = nullptr;

    double percent{};               ///< Percent from 0.0 to 100.0
    uint32_t color = COLOR_DEFAULT; ///< The color of the progress overlay
    bool bDrawShadow{};             ///< Draw shadow under the foreground surface
};

class TextProgressBar : public ProgressBar {
    using parent = ProgressBar;

public:
    TextProgressBar();

    ~TextProgressBar() override;

    TextProgressBar(const TextProgressBar&) = delete;
    TextProgressBar(TextProgressBar&&)      = delete;
    TextProgressBar& operator=(const TextProgressBar&) = delete;
    TextProgressBar& operator=(TextProgressBar&&) = delete;
    /**
        This method sets a new text for this progress bar and resizes it
        to fit this text.
        \param  text The new text for this progress bar
    */
    virtual void setText(const std::string& text);

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
    [[nodiscard]] Point getMinimumSize() const override {
        if (text.empty()) {
            return Point(4, 4);
        }
        return GUIStyle::getInstance().getMinimumButtonSize(text);
    }

protected:
    /**
        This method is called whenever the textures of this widget are needed, e.g. before drawing. This method
        should be overwritten by subclasses if they like to defer texture creation as long as possible.
        This method should first check whether a renewal of the textures is necessary.
    */
    void updateTextures() override;

    std::string text; ///< Text of this progress bar

    Uint32 textcolor       = COLOR_DEFAULT;
    Uint32 textshadowcolor = COLOR_DEFAULT;
};

class PictureProgressBar : public ProgressBar {
    using parent = ProgressBar;

public:
    PictureProgressBar();

    ~PictureProgressBar() override;

    PictureProgressBar(const PictureProgressBar&) = delete;
    PictureProgressBar(PictureProgressBar&&)      = delete;
    PictureProgressBar& operator=(const PictureProgressBar&) = delete;
    PictureProgressBar& operator=(PictureProgressBar&&) = delete;

    void setTexture(const DuneTexture* pBackground) {
        setBackground(pBackground);

        if (const auto* const background = getBackground()) {
            resize(getTextureSize(background));
        } else {
            resize(4, 4);
        }

        resizeAll();
    }

    /**
        This method resized the progress bar. This method should only
        called if the new size is a valid size for this progress bar (See getMinimumSize).
        \param  newSize the new size of this progress bar
    */
    void resize(Point newSize) override { resize(newSize.x, newSize.y); }

    /**
        This method resized the progress bar to width and height. This method should only
        called if the new size is a valid size for this progress bar (See getMinimumSize).
        \param  width   the new width of this progress bar
        \param  height  the new height of this progress bar
    */
    void resize(uint32_t width, uint32_t height) override { ProgressBar::resize(width, height); }

    /**
        Returns the minimum size of this progress bar. The progress bar should not
        resized to a size smaller than this.
        \return the minimum size of this progress bar
    */
    [[nodiscard]] Point getMinimumSize() const override;
};

#endif // PROGRESSBAR_H
