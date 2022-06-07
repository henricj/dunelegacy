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

#include "GUIStyle.h"
#include "HBox.h"
#include "Label.h"
#include "VBox.h"
#include "Widget.h"
#include "Window.h"

#include <misc/SDL2pp.h>

/// A simple class for a message box
class MsgBox final : public Window {
    using parent = Window;

public:
    /**
        This method sets a new text for this message box.
        \param  text The new text for this message box
    */
    virtual void setText(std::string text);

    /**
        Get the text of this message box.
        \return the text of this message box
    */
    const std::string& getText() { return textLabel.getText(); }

    /**
        Sets the text color for this message box.
        \param  textcolor       the color of the text (COLOR_DEFAULT = default color)
        \param  textshadowcolor the color of the shadow of the text (COLOR_DEFAULT = default color)
    */
    virtual void setTextColor(uint32_t textcolor, Uint32 textshadowcolor = COLOR_DEFAULT);

    /**
        This method resizes the message box to width and height. This method should only be
        called if the new size is a valid size for this message box (See resizingXAllowed,
        resizingYAllowed, getMinimumSize).
        \param  width   the new width of this message box
        \param  height  the new height of this message box
    */
    void resize(uint32_t width, uint32_t height) override;

    using parent::resize;

    /**
        This method is called by the window widget if it requests a resizing of
        this window.
    */
    void resizeAll() override;

    /**
        This static method creates a dynamic message box object with Text as the text in the message box.
        The idea behind this method is to simply create a new message box on the fly and
        add it as a child window of some other window. If the window gets closed it will be freed.
        \param  text    The message box text
        \return The new message box (will be automatically destroyed when it's closed)
    */
    static MsgBox* create(std::string text);

    /**
        This static method creates a dynamic message box object with Text as the text in the message box.
        The idea behind this method is to simply create a new message box on the fly and
        add it as a child window of some other window. If the window gets closed it will be freed.
        \param  text    The message box text
        \return The new message box (will be automatically destroyed when it's closed)
    */
    static MsgBox* create(std::string_view text) { return create(std::string{text}); }

protected:
    /** protected constructor (See create)
        \param  text    Text of this message box
    */
    explicit MsgBox(std::string text);

    /// destructor
    ~MsgBox() override;

private:
    /**
        This method is called when the OK button is pressed.
    */
    virtual void onOK();

    VBox vbox;           ///< vertical box
    HBox hbox;           ///< horizontal box
    VBox vbox2;          ///< inner vertical box;
    Label textLabel;     ///< label that contains the text
    TextButton okbutton; ///< the ok button
};

#endif // MSGBOX_H
