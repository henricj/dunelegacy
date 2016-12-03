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

#ifndef WAITINGFOROTHERPLAYERS_H
#define WAITINGFOROTHERPLAYERS_H

#include <GUI/Window.h>
#include <GUI/HBox.h>
#include <GUI/VBox.h>
#include <GUI/TextButton.h>
#include <GUI/Label.h>
#include <GUI/Spacer.h>

#include <algorithm>

class WaitingForOtherPlayers : public Window
{
public:
    WaitingForOtherPlayers();
    virtual ~WaitingForOtherPlayers();

    void onRemove();

    void update();



private:
    /**
        This method sets a new text for this window.
        \param  Text The new text for this window
    */
    inline void setText(const std::string& text) {
        textLabel.setText(text);
        resize(std::max(vbox.getMinimumSize().x,120),vbox.getMinimumSize().y);
    }

    VBox vbox;                  ///< vertical box
    HBox hbox;                  ///< horizontal box
    VBox vbox2;                 ///< inner vertical box;
    Label textLabel;            ///< label that contains the text
    TextButton removeButton;    ///< the remove button
};


#endif // WAITINGFOROTHERPLAYERS_H
