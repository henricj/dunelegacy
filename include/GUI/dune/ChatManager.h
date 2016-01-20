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

#ifndef CHATMANAGER_H
#define CHATMANAGER_H

#include <GUI/Widget.h>

#include <SDL.h>

#include <memory>
#include <string>
#include <list>

/**
    This class manages all the chat messages that are shown on the screen.
*/
class ChatManager : public Widget
{
public:

    enum MessageType {
        MSGTYPE_NORMAL = 0,
        MSGTYPE_INFO = 1,
        NUM_MSGTYPE
    };

    /**
        default constructor
    */
    ChatManager();

    /// destructor
    ~ChatManager();

    void addChatMessage(std::string username, std::string message);

    void addInfoMessage(std::string message);

	/**
		Draws this widget to screen. This method is called before drawOverlay().
		\param	screen	Surface to draw on
		\param	position	Position to draw the widget to
	*/
	virtual void draw(SDL_Surface* screen, Point position);

private:

    struct ChatMessage {

        ChatMessage(std::shared_ptr<SDL_Surface> _pMessageSurface, Uint32 _messageTime, MessageType _messageType)
         : pMessageSurface(_pMessageSurface), messageTime(_messageTime), messageType(_messageType) {
        }

        ChatMessage(std::shared_ptr<SDL_Surface> _pTimeSurface, std::shared_ptr<SDL_Surface> _pUsernameSurface,
                    std::shared_ptr<SDL_Surface> _pMessageSurface, Uint32 _messageTime, MessageType _messageType)
         : pTimeSurface(_pTimeSurface), pUsernameSurface(_pUsernameSurface), pMessageSurface(_pMessageSurface), messageTime(_messageTime), messageType(_messageType) {
        }

        std::shared_ptr<SDL_Surface>    pTimeSurface;
        std::shared_ptr<SDL_Surface>    pUsernameSurface;
        std::shared_ptr<SDL_Surface>    pMessageSurface;

        Uint32      messageTime;
        MessageType messageType;
    };

    std::list<ChatMessage> chatMessages;
};

#endif // CHATMANAGER_H
