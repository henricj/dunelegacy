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

#ifndef MSGBOX_H
#define MSGBOX_H

#include "Window.h"
#include "Button.h"
#include "Label.h"
#include "Spacer.h"
#include "GUIStyle.h"
#include "Widget.h"
#include "VBox.h"
#include "HBox.h"
#include <misc/SDL2pp.h>

#include <iostream>
#include <algorithm>

/// A simple class for a message box
class MsgBox : public Window {
public:

    /**
        This method sets a new text for this message box.
        \param  text The new text for this message box
    */
    virtual inline void setText(const std::string& text) {
        textLabel.setText(text);
        resize(std::max(vbox.getMinimumSize().x,120),vbox.getMinimumSize().y);
    }

    /**
        Get the text of this message box.
        \return the text of this message box
    */
    inline const std::string& getText() { return textLabel.getText(); };

    /**
        Sets the text color for this message box.
        \param  textcolor       the color of the text (COLOR_DEFAULT = default color)
        \param  textshadowcolor the color of the shadow of the text (COLOR_DEFAULT = default color)
    */
    virtual inline void setTextColor(Uint32 textcolor, Uint32 textshadowcolor = COLOR_DEFAULT) {
        textLabel.setTextColor(textcolor, textshadowcolor);
        okbutton.setTextColor(textcolor, textshadowcolor);
    }

    /**
        This method resizes the message box. This method should only
        called if the new size is a valid size for this message box (See getMinumumSize).
        \param  newSize the new size of this progress bar
    */
    void resize(Point newSize) override
    {
        resize(newSize.x,newSize.y);
    }

    /**
        This method resizes the message box to width and height. This method should only be
        called if the new size is a valid size for this message box (See resizingXAllowed,
        resizingYAllowed, getMinumumSize).
        \param  width   the new width of this message box
        \param  height  the new height of this message box
    */
    void resize(Uint32 width, Uint32 height) override
    {
        Window::resize(width,height);
        position.x = (getRendererWidth() - getSize().x)/2;
        position.y = (getRendererHeight() - getSize().y)/2;
    }

    /**
        This method is called by the window widget if it requests a resizing of
        this window.
    */
    void resizeAll() override
    {
        // MsgBox should get bigger if content changes
        if(pWindowWidget != nullptr) {
            Point newSize = pWindowWidget->getMinimumSize();
            newSize.x = std::max(newSize.x,120);
            newSize.y = std::max(newSize.y,30);
            resize(newSize.x,newSize.y);
        }
    };

    /**
        This static method creates a dynamic message box object with Text as the text in the message box.
        The idea behind this method is to simply create a new message box on the fly and
        add it as a child window of some other window. If the window gets closed it will be freed.
        \param  text    The message box text
        \return The new message box (will be automatically destroyed when it's closed)
    */
    static MsgBox* create(const std::string& text) {
        MsgBox* msgbox = new MsgBox(text);
        msgbox->pAllocated = true;
        return msgbox;
    }

protected:
    /** protected constructor (See create)
        \param  text    Text of this message box
    */
    explicit MsgBox(const std::string& text)
     : Window(50,50,50,50) {
        init(text);
    }

    /// destructor
    virtual ~MsgBox() {
    }

private:
    /**
        Initialization helper method.
        \param  Text    Text of this message box
    */
    void init(const std::string& text) {
        setWindowWidget(&vbox);
        vbox.addWidget(VSpacer::create(6));
        vbox.addWidget(&textLabel);
        vbox.addWidget(VSpacer::create(3));
        vbox.addWidget(&hbox);
        vbox.addWidget(VSpacer::create(6));
        hbox.addWidget(Spacer::create());
        hbox.addWidget(&vbox2);
        vbox2.addWidget(VSpacer::create(4));
        okbutton.setText("OK");
        okbutton.setOnClick(std::bind(&MsgBox::onOK, this));
        vbox2.addWidget(&okbutton);
        vbox2.addWidget(VSpacer::create(4));
        hbox.addWidget(Spacer::create());
        setText(text);
        textLabel.setAlignment(Alignment_HCenter);
        okbutton.setActive();
    }

    /**
        This method is called when the OK button is pressed.
    */
    virtual void onOK() {
        Window* pParentWindow = dynamic_cast<Window*>(getParent());
        if(pParentWindow != nullptr) {
            pParentWindow->closeChildWindow();
        }
    }

    VBox vbox;                  ///< vertical box
    HBox hbox;                  ///< horizontal box
    VBox vbox2;                 ///< inner vertical box;
    Label textLabel;            ///< label that contains the text
    TextButton okbutton;        ///< the ok button
};

#endif // MSGBOX_H
