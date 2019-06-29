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

#ifndef QSTBOX_H
#define QSTBOX_H

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


#define QSTBOX_BUTTON_INVALID   (-1)
#define QSTBOX_BUTTON1          (1)
#define QSTBOX_BUTTON2          (2)


/// A simple class for a question box
class QstBox : public Window {
public:

    /**
        This method sets a new text for this question box.
        \param  text The new text for this question box
    */
    virtual inline void setText(const std::string& text) {
        textLabel.setText(text);
        resize(std::max(vbox.getMinimumSize().x,120),vbox.getMinimumSize().y);
    }

    /**
        Get the text of this question box.
        \return the text of this question box
    */
    inline const std::string& getText() { return textLabel.getText(); };

    /**
        Sets the text color for this question box.
        \param  textcolor       the color of the text (COLOR_DEFAULT = default color)
        \param  textshadowcolor the color of the shadow of the text (COLOR_DEFAULT = default color)
    */
    virtual inline void setTextColor(Uint32 textcolor, Uint32 textshadowcolor = COLOR_DEFAULT) {
        textLabel.setTextColor(textcolor, textshadowcolor);
        button1.setTextColor(textcolor, textshadowcolor);
        button2.setTextColor(textcolor, textshadowcolor);
    }

    /**
        This method resizes the question box. This method should only
        called if the new size is a valid size for this question box (See getMinumumSize).
        \param  newSize the new size of this progress bar
    */
    void resize(Point newSize) override
    {
        resize(newSize.x,newSize.y);
    }

    /**
        This method resizes the question box to width and height. This method should only be
        called if the new size is a valid size for this question box (See resizingXAllowed,
        resizingYAllowed, getMinumumSize).
        \param  width   the new width of this question box
        \param  height  the new height of this question box
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
        // QstBox should get bigger if content changes
        if(pWindowWidget != nullptr) {
            Point newSize = pWindowWidget->getMinimumSize();
            newSize.x = std::max(newSize.x,120);
            newSize.y = std::max(newSize.y,30);
            resize(newSize.x,newSize.y);
        }
    };

    /**
        The number of the pressed button.
        \return the pressed button (either QSTBOX_BUTTON_INVALID, QSTBOX_BUTTON1 or QSTBOX_BUTTON2)
    */
    int getPressedButtonID() const {
        return pressedButtonID;
    }

    /**
        This static method creates a dynamic question box object with Text as the text in the question box.
        The idea behind this method is to simply create a new question box on the fly and
        add it as a child window of some other window. If the window gets closed it will be freed.
        \param  text            The question box text
        \param  button1Text     Text of button 1
        \param  button2Text     Text of button 2
        \param  defaultFocus    Button which gets the focus on showing the question box
        \return The new question box (will be automatically destroyed when it's closed)
    */
    static QstBox* create(const std::string& text, const std::string& button1Text = "Yes", const std::string& button2Text = "No", int defaultFocus = QSTBOX_BUTTON_INVALID) {
        QstBox* qstbox = new QstBox(text, button1Text, button2Text, defaultFocus);
        qstbox->pAllocated = true;
        return qstbox;
    }

protected:
    /** protected constructor (See Create)
        \param  text            Text of this question box
        \param  button1Text     Text of button 1
        \param  button2Text     Text of button 2
        \param  defaultFocus    Button which gets the focus on showing the question box
    */
    QstBox(const std::string& text, const std::string& button1Text, const std::string& button2Text, int defaultFocus)
     : Window(50,50,50,50), pressedButtonID(QSTBOX_BUTTON_INVALID) {
        init(text, button1Text, button2Text, defaultFocus);
    }

    /// destructor
    virtual ~QstBox() {
    }

private:
    /**
        Initialization helper method.
        \param  text            Text of this question box
        \param  button1Text     Text of button 1
        \param  button2Text     Text of button 2
        \param  defaultFocus    Button which gets the focus on showing the question box
    */
    void init(const std::string& text, const std::string& button1Text, const std::string& button2Text, int defaultFocus) {
        setWindowWidget(&vbox);
        vbox.addWidget(VSpacer::create(6));
        vbox.addWidget(&textLabel);
        vbox.addWidget(VSpacer::create(3));
        vbox.addWidget(&hbox);
        vbox.addWidget(VSpacer::create(6));
        hbox.addWidget(Spacer::create(), 0.2);
        hbox.addWidget(&vbox2, 0.6);
        vbox2.addWidget(VSpacer::create(4));
        button1.setText(button1Text);
        button1.setOnClick(std::bind(&QstBox::onButton, this, QSTBOX_BUTTON1));
        hbox2.addWidget(&button1);
        hbox2.addWidget(HSpacer::create(6));
        button2.setText(button2Text);
        button2.setOnClick(std::bind(&QstBox::onButton, this, QSTBOX_BUTTON2));
        hbox2.addWidget(&button2);
        vbox2.addWidget(&hbox2);
        vbox2.addWidget(VSpacer::create(4));
        hbox.addWidget(Spacer::create(), 0.2);
        setText(text);
        textLabel.setAlignment(Alignment_HCenter);

        if(defaultFocus == QSTBOX_BUTTON1) {
            button1.setActive();
        } else if(defaultFocus == QSTBOX_BUTTON2) {
            button2.setActive();
        }
    }

    /**
        This method is called when one of the buttons is pressed.
    */
    virtual void onButton(int btnID) {
        pressedButtonID = btnID;

        Window* pParentWindow = dynamic_cast<Window*>(getParent());
        if(pParentWindow != nullptr) {
            pParentWindow->closeChildWindow();
        }
    }

    VBox vbox;                  ///< vertical box
    HBox hbox;                  ///< horizontal box
    VBox vbox2;                 ///< inner vertical box;
    HBox hbox2;                 ///< inner horizontal box;
    Label textLabel;            ///< label that contains the text
    TextButton button1;         ///< button 1
    TextButton button2;         ///< button 2
    int pressedButtonID;        ///< the pressed button
};

#endif // QSTBOX_H
