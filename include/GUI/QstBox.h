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

#include "Button.h"
#include "GUIStyle.h"
#include "HBox.h"
#include "Label.h"
#include "Spacer.h"
#include "VBox.h"
#include "Widget.h"
#include "Window.h"
#include <misc/SDL2pp.h>

inline constexpr auto QSTBOX_BUTTON_INVALID = -1;
inline constexpr auto QSTBOX_BUTTON1        = 1;
inline constexpr auto QSTBOX_BUTTON2        = 2;

/// A simple class for a question box
class QstBox final : public Window {
    using parent = Window;

public:
    /**
        This method sets a new text for this question box.
        \param  text The new text for this question box
    */
    void setText(std::string text);

    /**
        Get the text of this question box.
        \return the text of this question box
    */
    [[nodiscard]] const std::string& getText() const { return textLabel.getText(); }

    /**
        Sets the text color for this question box.
        \param  textcolor       the color of the text (COLOR_DEFAULT = default color)
        \param  textshadowcolor the color of the shadow of the text (COLOR_DEFAULT = default color)
    */
    void setTextColor(uint32_t textcolor, Uint32 textshadowcolor = COLOR_DEFAULT);

    /**
        This method resizes the question box to width and height. This method should only be
        called if the new size is a valid size for this question box (See resizingXAllowed,
        resizingYAllowed, getMinimumSize).
        \param  width   the new width of this question box
        \param  height  the new height of this question box
    */
    void resize(uint32_t width, uint32_t height) override;

    using parent::resize;

    /**
        This method is called by the window widget if it requests a resizing of
        this window.
    */
    void resizeAll() override;

    /**
        The number of the pressed button.
        \return the pressed button (either QSTBOX_BUTTON_INVALID, QSTBOX_BUTTON1 or QSTBOX_BUTTON2)
    */
    [[nodiscard]] int getPressedButtonID() const;

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
    static QstBox* create(std::string text, std::string button1Text = "Yes", std::string button2Text = "No",
                          int defaultFocus = QSTBOX_BUTTON_INVALID);

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
    static QstBox* create(std::string_view text, std::string_view button1Text = "Yes",
                          std::string_view button2Text = "No", int defaultFocus = QSTBOX_BUTTON_INVALID) {
        return create(std::string{text}, std::string{button1Text}, std::string{button2Text}, defaultFocus);
    }

protected:
    /** protected constructor (See Create)
        \param  text            Text of this question box
        \param  button1Text     Text of button 1
        \param  button2Text     Text of button 2
        \param  defaultFocus    Button which gets the focus on showing the question box
    */
    QstBox(std::string text, std::string button1Text, std::string button2Text, int defaultFocus);

    /// destructor
    ~QstBox() override;

private:
    /**
        This method is called when one of the buttons is pressed.
    */
    void onButton(int btnID);

    VBox vbox;           ///< vertical box
    HBox hbox;           ///< horizontal box
    VBox vbox2;          ///< inner vertical box;
    HBox hbox2;          ///< inner horizontal box;
    Label textLabel;     ///< label that contains the text
    TextButton button1;  ///< button 1
    TextButton button2;  ///< button 2
    int pressedButtonID; ///< the pressed button
};

#endif // QSTBOX_H
