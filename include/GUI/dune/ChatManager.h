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
#include <misc/SDL2pp.h>

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
        MSGTYPE_PICTURE = 2,
        NUM_MSGTYPE
    };

    /**
        default constructor
    */
    ChatManager();

    /// destructor
    ~ChatManager();

    void addChatMessage(const std::string& username, const std::string& message);

    void addInfoMessage(const std::string& message);

    void addHintMessage(const std::string& message, SDL_Texture* pTexture);

    /**
        Draws this widget to screen. This method is called before drawOverlay().
        \param  position    Position to draw the widget to
    */
    void draw(Point position) override;

private:

    struct ChatMessage {

        ChatMessage(sdl2::texture_unique_or_nonowning_ptr _pMessageTexture, Uint32 _messageTime, MessageType _messageType)
         : pMessageTexture(std::move(_pMessageTexture)), messageTime(_messageTime), messageType(_messageType) {
        }

        ChatMessage(sdl2::texture_unique_or_nonowning_ptr _pTimeTexture, sdl2::texture_unique_or_nonowning_ptr _pUsernameTexture,
                    sdl2::texture_unique_or_nonowning_ptr _pMessageTexture, Uint32 _messageTime, MessageType _messageType)
         : pTimeTexture(std::move(_pTimeTexture)), pUsernameOrPictureTexture(std::move(_pUsernameTexture)), pMessageTexture(std::move(_pMessageTexture)), messageTime(_messageTime), messageType(_messageType) {
        }

        ChatMessage(sdl2::texture_unique_or_nonowning_ptr _pMessageTexture, sdl2::texture_unique_or_nonowning_ptr _pPictureTexture, Uint32 _messageTime, MessageType _messageType)
         : pUsernameOrPictureTexture(std::move(_pPictureTexture)), pMessageTexture(std::move(_pMessageTexture)), messageTime(_messageTime), messageType(_messageType) {
        }

        sdl2::texture_unique_or_nonowning_ptr    pTimeTexture;
        sdl2::texture_unique_or_nonowning_ptr    pUsernameOrPictureTexture;
        sdl2::texture_unique_or_nonowning_ptr    pMessageTexture;

        Uint32      messageTime;
        MessageType messageType;
    };

    std::list<ChatMessage> chatMessages;
};

#endif // CHATMANAGER_H
