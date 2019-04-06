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

#include "Widget.h"
#include "GUIStyle.h"
#include <misc/draw_util.h>
#include <misc/string_util.h>
#include <misc/SDL2pp.h>

#include <string>
#include <list>
#include <algorithm>

/// a class for a text label
class Label : public Widget {
public:
    /// default constructor
    Label() {
        enableResizing(true,true);
    }

    /// destructor
    virtual ~Label() = default;

    /**
        Sets a font size for this label. Default font size of a label is 14
        \param  fontSize      the font size of the new font
    */
    virtual void setTextFontSize(int fontSize) {
        this->fontSize = fontSize;
        resizeAll();
    }

    /**
        Gets the font size of this label. Default font size of a label is 14
        \return the font size of this label
    */
    virtual int getTextFontSize() const {
       return fontSize;
    }

    /**
        Sets the text color for this label.
        \param  textcolor       the color of the text (COLOR_DEFAULT = default color)
        \param  textshadowcolor the color of the shadow of the text (COLOR_DEFAULT = default color)
        \param  backgroundcolor the color of the label background (COLOR_TRANSPARENT = transparent)
    */
    virtual void setTextColor(Uint32 textcolor, Uint32 textshadowcolor = COLOR_DEFAULT, Uint32 backgroundcolor = COLOR_TRANSPARENT) {
        this->textcolor = textcolor;
        this->textshadowcolor = textshadowcolor;
        this->backgroundcolor = backgroundcolor;
        invalidateTextures();
    }

    /**
        Sets the alignment of the text in this label.
        \param alignment Combination of (Alignment_HCenter, Alignment_Left or Alignment_Right) and (Alignment_VCenter, Alignment_Top or Alignment_Bottom)
    */
    virtual inline void setAlignment(Alignment_Enum alignment) {
        this->alignment = alignment;
        invalidateTextures();
    }

    /**
        Returns the alignment of the text in this label.
        \return Combination of (Alignment_HCenter, Alignment_Left or Alignment_Right) and (Alignment_VCenter, Alignment_Top or Alignment_Bottom)
    */
    virtual Alignment_Enum getAlignment() const {
        return alignment;
    }

    /**
        This method sets a new text for this label and resizes this label
        to fit this text.
        \param  Text The new text for this button
    */
    virtual void setText(const std::string& text) {
        if(text != this->text) {
            this->text = text;
            resizeAll();
        }
    }

    /**
        Get the text of this label.
        \return the text of this button
    */
    const std::string& getText() const { return text; };

    /**
        This method resizes the label. This method should only
        called if the new size is a valid size for this label (See getMinumumSize).
        \param  newSize the new size of this progress bar
    */
    void resize(Point newSize) override
    {
        resize(newSize.x,newSize.y);
    }

    /**
        This method resizes the label to width and height. This method should only
        called if the new size is a valid size for this label (See getMinumumSize).
        \param  width   the new width of this label
        \param  height  the new height of this label
    */
    void resize(Uint32 width, Uint32 height) override
    {
        invalidateTextures();
        Widget::resize(width,height);
    }

    /**
        Returns the minimum size of this label. The label should not
        resized to a size smaller than this.
        \return the minimum size of this label
    */
    Point getMinimumSize() const override
    {
        Point p(0,0);

        //split text into single lines at every '\n'
        size_t startpos = 0;
        size_t nextpos;
        std::vector<std::string> hardLines;
        do {
            nextpos = text.find('\n',startpos);
            if(nextpos == std::string::npos) {
                hardLines.emplace_back(text, startpos, text.length()-startpos);
            } else {
                hardLines.emplace_back(text, startpos, nextpos-startpos);
                startpos = nextpos+1;
            }
        } while(nextpos != std::string::npos);

        for(const std::string& hardLine : hardLines) {
            Point minLabelSize = GUIStyle::getInstance().getMinimumLabelSize(hardLine, fontSize);
            p.x = std::max(p.x, minLabelSize.x);
            p.y += minLabelSize.y;
        }
        return p;
    }

    /**
        Draws this label to screen.
        \param  position    Position to draw the label to
    */
    void draw(Point position) override
    {
        if((isEnabled() == false) || (isVisible() == false)) {
            return;
        }

        updateTextures();

        if(pTexture == nullptr) {
            return;
        }

        SDL_Rect dest = calcDrawingRect(pTexture.get(), position.x + getSize().x/2, position.y + getSize().y/2, HAlign::Center, VAlign::Center);
        SDL_RenderCopy(renderer, pTexture.get(), nullptr, &dest);
    };

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
    static Label* create(const std::string& text, Uint32 textcolor = COLOR_DEFAULT, Uint32 textshadowcolor = COLOR_DEFAULT, Uint32 backgroundcolor = COLOR_TRANSPARENT) {
        Label* label = new Label();
        label->setText(text);
        label->setTextColor(textcolor, textshadowcolor, backgroundcolor);
        label->pAllocated = true;
        return label;
    }

protected:
    /**
        This method is called whenever the textures of this widget are needed, e.g. before drawing. This method
        should be overwritten by subclasses if they like to defer texture creation as long as possible.
        This method should first check whether a renewal of the textures is necessary.
    */
    void updateTextures() override
    {
        Widget::updateTextures();

        if(!pTexture) {
            std::vector<std::string> textLines = greedyWordWrap(text,
                                                                getSize().x,
                                                                [font = fontSize](const std::string& tmp) {
                                                                    return GUIStyle::getInstance().getMinimumLabelSize(tmp, font).x - 4;
                                                                });

            pTexture = convertSurfaceToTexture(GUIStyle::getInstance().createLabelSurface(getSize().x,getSize().y,textLines,fontSize,alignment,textcolor,textshadowcolor,backgroundcolor));
        }
    }

    /**
        This method frees all textures that are used by this label
    */
    void invalidateTextures() override
    {
        pTexture.reset();
    }

private:
    int fontSize = 14;                  ///< the size of the font to use
    Uint32 textcolor = COLOR_DEFAULT;           ///< the text color
    Uint32 textshadowcolor = COLOR_DEFAULT;     ///< the color of the shadow of the text
    Uint32 backgroundcolor = COLOR_TRANSPARENT; ///< the color of the label background
    std::string text;                           ///< the text of this label
    sdl2::texture_ptr pTexture = nullptr;       ///< the texture of this label
    Alignment_Enum alignment = static_cast<Alignment_Enum>(Alignment_Left | Alignment_VCenter);   ///< the alignment of this label
};

#endif // LABEL_H
