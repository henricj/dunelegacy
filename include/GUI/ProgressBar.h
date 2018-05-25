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
#include <misc/draw_util.h>
#include <misc/SDL2pp.h>

#include <string>

#include <GUI/GUIStyle.h>

#include <cmath>


/// A class for a progress bar widget
class ProgressBar : public Widget {
public:

    /// default constructor
    ProgressBar() {
        percent = 0.0;
        color = COLOR_DEFAULT;
        bDrawShadow = false;
        pBackground = nullptr;
        pForeground = nullptr;
        enableResizing(true,true);
    }

    /// destructor
    virtual ~ProgressBar() {
        invalidateTextures();
    }

    ProgressBar(const ProgressBar&) = delete;
    ProgressBar(ProgressBar &&) = delete;
    ProgressBar& operator=(const ProgressBar &) = delete;
    ProgressBar& operator=(ProgressBar &&) = delete;

    /**
        Sets the progress of this progress bar.
        \param newPercent  Should be between 0.0 and 100.0
    */
    void setProgress(double newPercent) {
        if(percent != newPercent) {
            percent = newPercent;
            if(percent < 0.0) {
                percent = 0.0;
            } else if(percent > 100.0) {
                percent = 100.0;
            }

            invalidateTextures();
        }
    }

    /**
        Return the current progress.
        \return the current progress in percent
    */
    double getProgress() const noexcept {
        return percent;
    }

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
    void setDrawShadow(bool bDrawShadow) {
        this->bDrawShadow = bDrawShadow;
    }

    /**
        This method resized the progress bar. This method should only
        called if the new size is a valid size for this progress bar (See getMinumumSize).
        \param  newSize the new size of this progress bar
    */
    void resize(Point newSize) override
    {
        resize(newSize.x,newSize.y);
    }

    /**
        This method resizes the progress bar to width and height. This method should only
        called if the new size is a valid size for this progress bar (See getMinumumSize).
        \param  width   the new width of this progress bar
        \param  height  the new height of this progress bar
    */
    void resize(Uint32 width, Uint32 height) override
    {
        Widget::resize(width,height);

        invalidateTextures();
    }

    /**
        Draws this progress bar to screen
        \param  position    Position to draw the progress bar to
    */
    void draw(Point position) override
    {
        if(isVisible() == false) {
            return;
        }

        updateTextures();

        if(pBackground) {
            auto dest = calcDrawingRect(pBackground.get(), position.x, position.y);
            SDL_RenderCopy(renderer, pBackground.get(), nullptr, &dest);
        }

        if(pForeground) {
            auto dest = calcDrawingRect(pForeground.get(), position.x, position.y);
            if(bDrawShadow) {
                SDL_Rect dest2 = { position.x + 2, position.y + 2, (int) lround(percent*(dest.w/100.0)), dest.h };
                renderFillRect(renderer, &dest2, COLOR_BLACK);
            }

            SDL_RenderCopy(renderer, pForeground.get(), nullptr, &dest);
        }
    }

protected:
    /**
        This method is called whenever the textures of this widget are needed, e.g. before drawing. This method
        should be overwritten by subclasses if they like to defer texture creation as long as possible.
        This method should first check whether a renewal of the textures is necessary.
    */
    void updateTextures() override
    {
        if(!pForeground) {
            pForeground = convertSurfaceToTexture(GUIStyle::getInstance().createProgressBarOverlay(getSize().x, getSize().y, percent, color));
        }
    }

    /**
        This method frees all textures that are used by this progress bar
    */
    void invalidateTextures() override
    {
        pForeground.reset();
    }

    sdl2::texture_unique_or_nonowning_ptr   pBackground;
    sdl2::texture_ptr                       pForeground;

    double percent;             ///< Percent from 0.0 to 100.0
    Uint32 color;               ///< The color of the progress overlay
    bool bDrawShadow;           ///< Draw shadow under the foreground surface
};

class TextProgressBar : public ProgressBar {
public:
    TextProgressBar() = default;

    virtual ~TextProgressBar() = default;

    TextProgressBar(const TextProgressBar &) = delete;
    TextProgressBar(TextProgressBar &&) = delete;
    TextProgressBar& operator=(const TextProgressBar &) = delete;
    TextProgressBar& operator=(TextProgressBar &&) = delete;
    /**
        This method sets a new text for this progress bar and resizes it
        to fit this text.
        \param  text The new text for this progress bar
    */
    virtual void setText(const std::string& text) {
        if(this->text != text) {
            this->text = text;
            resizeAll();
        }
    }

    /**
        Get the text of this progress bar.
        \return the text of this button
    */
    const std::string& getText() const noexcept { return text; };

    /**
        Sets the text color for this progress bar.
        \param  textcolor       the color of the text (COLOR_DEFAULT = default color)
        \param  textshadowcolor the color of the shadow of the text (COLOR_DEFAULT = default color)
    */
    virtual void setTextColor(Uint32 textcolor, Uint32 textshadowcolor = COLOR_DEFAULT) {
        this->textcolor = textcolor;
        this->textshadowcolor = textshadowcolor;
        invalidateTextures();
    }

    /**
        Returns the minimum size of this progress bar. The progress bar should not
        resized to a size smaller than this.
        \return the minimum size of this progress bar
    */
    Point getMinimumSize() const override
    {
        if(text.empty()) {
            return Point(4,4);
        } else {
            return GUIStyle::getInstance().getMinimumButtonSize(text);
        }
    }

protected:
    /**
        This method is called whenever the textures of this widget are needed, e.g. before drawing. This method
        should be overwritten by subclasses if they like to defer texture creation as long as possible.
        This method should first check whether a renewal of the textures is necessary.
    */
    void updateTextures() override
    {
        ProgressBar::updateTextures();

        if(!pBackground) {
            pBackground = convertSurfaceToTexture(GUIStyle::getInstance().createButtonSurface(getSize().x, getSize().y, text, true, false, textcolor, textshadowcolor));
        }
    }

    /**
        This method frees all textures that are used by this progress bar
    */
    void invalidateTextures() override
    {
        ProgressBar::invalidateTextures();

        pBackground.reset();
    }

    std::string text = "";           ///< Text of this progress bar

    Uint32 textcolor = COLOR_DEFAULT;
    Uint32 textshadowcolor = COLOR_DEFAULT;
};

class PictureProgressBar: public ProgressBar {
public:
    PictureProgressBar() {
        Widget::enableResizing(false,false);
    }

    virtual ~PictureProgressBar() = default;

    PictureProgressBar(const PictureProgressBar &) = delete;
    PictureProgressBar(PictureProgressBar &&) = delete;
    PictureProgressBar& operator=(const PictureProgressBar &) = delete;
    PictureProgressBar& operator=(PictureProgressBar &&) = delete;

    void setSurface(sdl2::surface_unique_or_nonowning_ptr pBackground) {
        setTexture(convertSurfaceToTexture(pBackground.get()));
    }

    void setTexture(sdl2::texture_unique_or_nonowning_ptr pBackground) {
        this->pBackground = std::move(pBackground);

        if(this->pBackground) {
            resize(getTextureSize(this->pBackground.get()));
        } else {
            resize(4,4);
        }

        resizeAll();
    }

    /**
        This method resized the progress bar. This method should only
        called if the new size is a valid size for this progress bar (See getMinumumSize).
        \param  newSize the new size of this progress bar
    */
    void resize(Point newSize) override
    {
        resize(newSize.x,newSize.y);
    }

    /**
        This method resized the progress bar to width and height. This method should only
        called if the new size is a valid size for this progress bar (See getMinumumSize).
        \param  width   the new width of this progress bar
        \param  height  the new height of this progress bar
    */
    void resize(Uint32 width, Uint32 height) override
    {
        Widget::resize(width,height);
    }

    /**
        Returns the minimum size of this progress bar. The progress bar should not
        resized to a size smaller than this.
        \return the minimum size of this progress bar
    */
    Point getMinimumSize() const override
    {
        if(!pBackground) {
            return Point(4,4);
        } else {
            return getTextureSize(pBackground.get());
        }
    }
};

#endif // PROGRESSBAR_H
