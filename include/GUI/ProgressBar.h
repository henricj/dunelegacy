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

#include <string>
#include <SDL2/SDL.h>

#include <cmath>


/// A class for a progress bar widget
class ProgressBar : public Widget {
public:

    /// default constructor
    ProgressBar() : Widget() {
        percent = 0.0;
        color = COLOR_DEFAULT;
        bDrawShadow = false;
        pBackground = nullptr;
        pForeground = nullptr;
        bFreeBackground = true;
        enableResizing(true,true);
    }

    /// destructor
    virtual ~ProgressBar() {
        if((bFreeBackground == true) && (pBackground != nullptr)) {
            SDL_DestroyTexture(pBackground);
        }

        invalidateTextures();
    }

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
    double getProgress() {
        return percent;
    }

    /**
        Sets the color of the progress bar overlay
        \param  color   the new color (COLOR_DEFAULT = default)
    */
    inline void setColor(Uint32 color = COLOR_DEFAULT) {
        this->color = color;
        invalidateTextures();
    }

    /**
        Specifies if a shadow is drawn or not.
        \param  bDrawShadow if true, a shadow is drawn
    */
    inline void setDrawShadow(bool bDrawShadow) {
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

        if(pBackground != nullptr) {
            SDL_Rect dest = calcDrawingRect(pBackground, position.x, position.y);
            SDL_RenderCopy(renderer, pBackground, nullptr, &dest);
        }

        if(pForeground != nullptr) {
            SDL_Rect dest = calcDrawingRect(pForeground, position.x, position.y);
            if(bDrawShadow) {
                SDL_Rect dest2 = { position.x + 2, position.y + 2, (int) lround(percent*(dest.w/100.0)), dest.h };
                renderFillRect(renderer, &dest2, COLOR_BLACK);
            }

            SDL_RenderCopy(renderer, pForeground, nullptr, &dest);
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
        if(pForeground == nullptr) {
            pForeground = convertSurfaceToTexture(GUIStyle::getInstance().createProgressBarOverlay(getSize().x, getSize().y, percent, color), true);
        }
    }

    /**
        This method frees all textures that are used by this progress bar
    */
    void invalidateTextures() override
    {
        if(pForeground != nullptr) {
            SDL_DestroyTexture(pForeground);
            pForeground = nullptr;
        }
    }

    SDL_Texture*    pBackground;
    bool            bFreeBackground;
    SDL_Texture*    pForeground;

    double percent;             ///< Percent from 0.0 to 100.0
    Uint32 color;               ///< The color of the progress overlay
    bool bDrawShadow;           ///< Draw shadow under the foreground surface
};

class TextProgressBar : public ProgressBar {
public:
    TextProgressBar() : ProgressBar() {
        bFreeBackground = true;
    }

    virtual ~TextProgressBar() { ; };

    /**
        This method sets a new text for this progress bar and resizes it
        to fit this text.
        \param  text The new text for this progress bar
    */
    virtual inline void setText(const std::string& text) {
        if(this->text != text) {
            this->text = text;
            resizeAll();
        }
    }

    /**
        Get the text of this progress bar.
        \return the text of this button
    */
    inline const std::string& getText() const { return text; };

    /**
        Sets the text color for this progress bar.
        \param  textcolor       the color of the text (COLOR_DEFAULT = default color)
        \param  textshadowcolor the color of the shadow of the text (COLOR_DEFAULT = default color)
    */
    virtual inline void setTextColor(Uint32 textcolor, Uint32 textshadowcolor = COLOR_DEFAULT) {
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
        if(text == "") {
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

        if(pBackground == nullptr) {
            pBackground = convertSurfaceToTexture(GUIStyle::getInstance().createButtonSurface(getSize().x, getSize().y, text, true, false, textcolor, textshadowcolor), true);
        }
    }

    /**
        This method frees all textures that are used by this progress bar
    */
    void invalidateTextures() override
    {
        ProgressBar::invalidateTextures();

        if(pBackground != nullptr) {
            SDL_DestroyTexture(pBackground);
            pBackground = nullptr;
        }
    }

    std::string text = "";           ///< Text of this progress bar

    Uint32 textcolor = COLOR_DEFAULT;
    Uint32 textshadowcolor = COLOR_DEFAULT;
};

class PictureProgressBar: public ProgressBar {
public:
    PictureProgressBar() : ProgressBar() {
        enableResizing(false,false);
    }

    virtual ~PictureProgressBar() { ; }

    void setSurface(SDL_Surface* pBackground, bool bFreeBackground) {
        setTexture(convertSurfaceToTexture(pBackground, bFreeBackground), true);
    }

    void setTexture(SDL_Texture* pBackground, bool bFreeBackground) {
        if((this->bFreeBackground == true) && (this->pBackground != nullptr)) {
            SDL_DestroyTexture(this->pBackground);
        }

        this->pBackground = pBackground;
        this->bFreeBackground = bFreeBackground;

        if(this->pBackground != nullptr) {
            resize(getTextureSize(pBackground));
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
        if(pBackground == nullptr) {
            return Point(4,4);
        } else {
            return getTextureSize(pBackground);
        }
    }
};

#endif // PROGRESSBAR_H
