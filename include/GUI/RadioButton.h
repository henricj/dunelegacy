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

#ifndef RADIOBUTTON_H
#define RADIOBUTTON_H

#include "Button.h"
#include "GUIStyle.h"

#include "RadioButtonManager.h"

#include <string>

/// A class for a radio button implemented as a toggle button
class RadioButton : public Button {
public:
    /// Default constructor
    RadioButton() {
        textcolor = COLOR_DEFAULT;
        textshadowcolor = COLOR_DEFAULT;

        enableResizing(true,false);
        setToggleButton(true);
        pCheckedActiveTexture = nullptr;

        pRadioButtonManager = nullptr;
    }

    /// destructor
    virtual ~RadioButton() {
        invalidateTextures();

        unregisterFromRadioButtonManager();
    }


    void registerRadioButtonManager(RadioButtonManager* pNewRadioButtonManager) {
        if(pNewRadioButtonManager != pRadioButtonManager) {
            unregisterFromRadioButtonManager();
            pRadioButtonManager = pNewRadioButtonManager;
            if(pRadioButtonManager->isRegistered(this) == false) {
                pRadioButtonManager->registerRadioButton(this);
            }
        }
    }

    void unregisterFromRadioButtonManager() {
        if(pRadioButtonManager != nullptr) {
            RadioButtonManager* pOldRadioButtonManager = pRadioButtonManager;
            pRadioButtonManager = nullptr;
            if(pOldRadioButtonManager->isRegistered(this)) {
                pOldRadioButtonManager->unregisterRadioButton(this);
            }
        }
    }

    /**
        This method sets a new text for this radio button and resizes it
        to fit this text.
        \param  text The new text for this radio button
    */
    virtual void setText(const std::string& text) {
        this->text = text;
        resizeAll();
    }

    /**
        Get the text of this radio button.
        \return the text of this radio button
    */
    const std::string& getText() const { return text; };

    /**
        Sets the text color for this radio button.
        \param  textcolor       the color of the text (COLOR_DEFAULT = default color)
        \param  textshadowcolor the color of the shadow of the text (COLOR_DEFAULT = default color)
    */
    virtual void setTextColor(Uint32 textcolor, Uint32 textshadowcolor = COLOR_DEFAULT) {
        this->textcolor = textcolor;
        this->textshadowcolor = textshadowcolor;
        invalidateTextures();
    }


    /**
        This method sets the current toggle state. On radio buttons this is only
        effective for bToggleState == true, so you can only set a radio button to be selected
        but cannot deselect it
        \param bToggleState true = toggled, false = untoggled
    */
    void setToggleState(bool bToggleState) override
    {
        if(bToggleState != true) {
            return;
        }

        if(bToggleState != getToggleState()) {
            if(pRadioButtonManager != nullptr) {
                pRadioButtonManager->setChecked(this);
            }
        }
    }

    /**
        This method sets this radio button to checked or unchecked. It does the same as setToggleState().
        \param bChecked true = checked, false = unchecked
    */
    inline void setChecked(bool bChecked) {
        setToggleState(bChecked);
    }

    /**
        This method returns whether this radio button is checked. It is the same as getToggleState().
        \return true = checked, false = unchecked
    */
    inline bool isChecked() const {
        return getToggleState();
    }

    /**
        Draws this button to screen. This method is called before drawOverlay().
        \param  position    Position to draw the button to
    */
    void draw(Point position) override
    {
        if(isVisible() == false) {
            return;
        }

        updateTextures();

        SDL_Texture* tex;
        if(isChecked()) {
            if((isActive() || bHover) && pCheckedActiveTexture) {
                tex = pCheckedActiveTexture.get();
            } else {
                tex = pPressedTexture.get();
            }
        } else {
            if((isActive() || bHover) && pActiveTexture) {
                tex = pActiveTexture.get();
            } else {
                tex = pUnpressedTexture.get();
            }
        }

        if(tex == nullptr) {
            return;
        }

        SDL_Rect dest = calcDrawingRect(tex, position.x, position.y);
        SDL_RenderCopy(renderer, tex, nullptr, &dest);
    }

    /**
        This method resizes the radio button. This method should only
        called if the new size is a valid size for this radio button (See getMinumumSize).
        \param  newSize the new size of this progress bar
    */
    void resize(Point newSize) override
    {
        resize(newSize.x,newSize.y);
    }

    /**
        This method resizes the radio button to width and height. This method should only
        called if the new size is a valid size for this radio button (See getMinimumSize).
        \param  width   the new width of this radio button
        \param  height  the new height of this radio button
    */
    void resize(Uint32 width, Uint32 height) override
    {
        invalidateTextures();
        Widget::resize(width,height);
    }

    /**
        Returns the minimum size of this button. The button should not
        resized to a size smaller than this.
        \return the minimum size of this button
    */
    Point getMinimumSize() const override
    {
        return GUIStyle::getInstance().getMinimumRadioButtonSize(text);
    }
protected:
    /**
        This method is called whenever the textures of this widget are needed, e.g. before drawing. This method
        should be overwritten by subclasses if they like to defer texture creation as long as possible.
        This method should first check whether a renewal of the textures is necessary.
    */
    void updateTextures() override
    {
        Button::updateTextures();

        if(!pUnpressedTexture) {
            invalidateTextures();

            setSurfaces(GUIStyle::getInstance().createRadioButtonSurface(getSize().x, getSize().y, text, false, false, textcolor, textshadowcolor),
                        GUIStyle::getInstance().createRadioButtonSurface(getSize().x, getSize().y, text, true, false, textcolor, textshadowcolor),
                        GUIStyle::getInstance().createRadioButtonSurface(getSize().x, getSize().y, text, false, true, textcolor, textshadowcolor));

            pCheckedActiveTexture = convertSurfaceToTexture(GUIStyle::getInstance().createRadioButtonSurface(getSize().x, getSize().y, text, true, true, textcolor, textshadowcolor));
        }
    }

    /**
        This method frees all textures that are used by this radio button
    */
    void invalidateTextures() override
    {
        Button::invalidateTextures();

        pCheckedActiveTexture.reset();
    }

private:
    Uint32 textcolor;                           ///< Text color
    Uint32 textshadowcolor;                     ///< Text shadow color
    std::string text;                           ///< Text of this radio button
    sdl2::texture_ptr pCheckedActiveTexture;    ///< Texture that is shown when the radio button is activated by keyboard or by mouse hover

    RadioButtonManager* pRadioButtonManager;///< The Manager for managing the toggle states
};

#endif // RADIOBUTTON_H
