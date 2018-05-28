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

#ifndef SYMBOLBUTTON_H
#define SYMBOLBUTTON_H

#include "Button.h"
#include <misc/SDL2pp.h>

/// A class for a symbol button
class SymbolButton : public Button {
public:
    /// Default contructor
    SymbolButton() : Button() {
        enableResizing(true,true);
        pSymbolSurface = nullptr;
        pActiveSymbolSurface = nullptr;
    }

    /// destructor
    virtual ~SymbolButton() = default;

    /**
        This method is used for setting the symbol for this button.
        \param  pSymbolSurface          This is the symbol to show
        \param  pActiveSymbolSurface    This is the symbol to show on mouse over
    */
    virtual void setSymbol(sdl2::surface_unique_or_nonowning_ptr pSymbolSurface, sdl2::surface_unique_or_nonowning_ptr pActiveSymbolSurface = nullptr) {
        if(!pSymbolSurface) {
            return;
        }

        this->pSymbolSurface = std::move(pSymbolSurface);
        this->pActiveSymbolSurface = std::move(pActiveSymbolSurface);

        resizeAll();
    }

    /**
        This method resizes the button. This method should only
        called if the new size is a valid size for this button (See getMinumumSize).
        \param  newSize the new size of this progress bar
    */
    void resize(Point newSize) override
    {
        resize(newSize.x,newSize.y);
    }

    /**
        This method resized the button to width and height. This method should only
        called if the new size is a valid size for this button (See getMinumumSize).
        \param  width   the new width of this button
        \param  height  the new height of this button
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
        if(pSymbolSurface) {
            return Point((Sint32) pSymbolSurface->w + 5, (Sint32) pSymbolSurface->h + 5);
        } else {
            return Point(0,0);
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
        Button::updateTextures();

        if(!pUnpressedTexture) {
            invalidateTextures();

            sdl2::surface_ptr pUnpressed = GUIStyle::getInstance().createButtonSurface(getSize().x, getSize().y, "", false, true);
            sdl2::surface_ptr pPressed = GUIStyle::getInstance().createButtonSurface(getSize().x, getSize().y, "", true, true);
            sdl2::surface_ptr pActive;

            if(pSymbolSurface) {
                SDL_Rect dest = calcAlignedDrawingRect(pSymbolSurface.get(), pUnpressed.get());
                SDL_BlitSurface(pSymbolSurface.get(), nullptr, pUnpressed.get(), &dest);

                dest.x++;
                dest.y++;
                SDL_BlitSurface(pActiveSymbolSurface ? pActiveSymbolSurface.get() : pSymbolSurface.get(), nullptr, pPressed.get(), &dest);
            }

            if(pActiveSymbolSurface) {
                pActive = GUIStyle::getInstance().createButtonSurface(getSize().x, getSize().y, "", false, true);

                SDL_Rect dest = calcAlignedDrawingRect(pActiveSymbolSurface.get(), pActive.get());
                SDL_BlitSurface(pActiveSymbolSurface.get(), nullptr, pActive.get(), &dest);
            }

            Button::setSurfaces(std::move(pUnpressed), std::move(pPressed), std::move(pActive));
        }
    }

private:
    sdl2::surface_unique_or_nonowning_ptr pSymbolSurface;
    sdl2::surface_unique_or_nonowning_ptr pActiveSymbolSurface;

};

#endif //SYMBOLBUTTON_H
