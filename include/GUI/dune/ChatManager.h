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
		\param	position	Position to draw the widget to
	*/
	virtual void draw(Point position);

private:

    struct ChatMessage {

        ChatMessage(std::shared_ptr<SDL_Texture> _pMessageTexture, Uint32 _messageTime, MessageType _messageType)
         : pMessageTexture(_pMessageTexture), messageTime(_messageTime), messageType(_messageType) {
        }

        ChatMessage(std::shared_ptr<SDL_Texture> _pTimeTexture, std::shared_ptr<SDL_Texture> _pUsernameTexture,
                    std::shared_ptr<SDL_Texture> _pMessageTexture, Uint32 _messageTime, MessageType _messageType)
         : pTimeTexture(_pTimeTexture), pUsernameTexture(_pUsernameTexture), pMessageTexture(_pMessageTexture), messageTime(_messageTime), messageType(_messageType) {
        }

        std::shared_ptr<SDL_Texture>    pTimeTexture;
        std::shared_ptr<SDL_Texture>    pUsernameTexture;
        std::shared_ptr<SDL_Texture>    pMessageTexture;

        Uint32      messageTime;
        MessageType messageType;
    };

    std::list<ChatMessage> chatMessages;
};

#endif // CHATMANAGER_H
