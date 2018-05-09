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

#ifndef GAMEINTERFACE_H
#define GAMEINTERFACE_H

#include <GUI/Window.h>
#include <GUI/HBox.h>
#include <GUI/StaticContainer.h>
#include <GUI/Spacer.h>
#include <GUI/PictureButton.h>
#include <GUI/PictureLabel.h>
#include <GUI/dune/ChatManager.h>
#include <GUI/dune/NewsTicker.h>

#include <RadarView.h>

class ObjectInterface;

/// This class represents the in-game interface.
class GameInterface : public Window {
public:
    /// default constructor
    GameInterface();

    /// destructor
    virtual ~GameInterface();

    /**
        Draws this window to screen. This method should be called every frame.
        \param  position    Position to draw the window to. The position of the window is added to this.
    */
    void draw(Point position) override;

    /**
        Checks whether the newticker currently shows a message
        \return true if a message is shown, false otherwise
    */
    virtual bool newsTickerHasMessage() {
        return newsticker.hasMessage();
    }

    /**
        This method adds a message to the news ticker
        \param  text    the message to add
    */
    virtual void addToNewsTicker(const std::string& text) {
        newsticker.addMessage(text);
    }

    /**
        This method adds a urgent message to the news ticker
        \param  text    the urgent message to add
    */
    virtual void addUrgentMessageToNewsTicker(const std::string& text) {
        newsticker.addUrgentMessage(text);
    }

    /**
        Returns the radar view
        \return the radar view
    */
    RadarView& getRadarView() { return radarView; };

    /**
        Returns the chat manager
        \return the chat manager
    */
    ChatManager& getChatManager() { return chatManager; };


    /**
        This method updates the object interface
    */
    virtual void updateObjectInterface();

private:
    void removeOldContainer();


    ObjectInterface*    pObjectContainer;       ///< The container holding information about the currently selected unit/structure
    Uint32              objectID;               ///< The id of the currently selected object

    StaticContainer     windowWidget;           ///< The main widget of this interface

    HBox                topBarHBox;             ///< The container for the top bar containing newsticker, options button and mentat button
    NewsTicker          newsticker;             ///< The newsticker showing news on the game (e.g. new starport prices, harvester fill level, etc.)
    PictureButton       optionsButton;          ///< Button for accessing the ingame menu
    PictureButton       mentatButton;           ///< Button for accessing the mentat menu
    PictureLabel        topBar;                 ///< The background of the top bar

    PictureLabel        sideBar;                ///< The background of the side bar

    RadarView           radarView;              ///< This is the minimap/radar in the side bar

    ChatManager         chatManager;            ///< Manages chat manages shown overlayed with the main map
};
#endif // GAMEINTERFACE_H
