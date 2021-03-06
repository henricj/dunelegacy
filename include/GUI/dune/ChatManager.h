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
#include <deque>
#include <unordered_map>

/**
    This class manages all the chat messages that are shown on the screen.
*/
class ChatManager : public Widget
{
public:

    enum class MessageType {
        MSGTYPE_NORMAL = 0,
        MSGTYPE_INFO = 1,
        MSGTYPE_PICTURE = 2,
        NUM_MSGTYPE
    };

    /**
        default constructor
    */
    ChatManager();

    ChatManager(const ChatManager &) = delete;
    ChatManager(ChatManager &&) = delete;
    ChatManager& operator=(const ChatManager&) = delete;
    ChatManager& operator=(ChatManager &&) = delete;

    /// destructor
    ~ChatManager();

    void addChatMessage(const std::string& username, const std::string& message);

    void addInfoMessage(const std::string& message);

    void addHintMessage(const std::string& message, const DuneTexture* pTexture);

    /**
        Draws this widget to screen. This method is called before drawOverlay().
        \param  position    Position to draw the widget to
    */
    void draw(Point position) override;

private:
    SDL_Texture * getUserTexture(std::string username);
    void prune_messages();

    struct ChatMessage final {

        ChatMessage(sdl2::texture_ptr _pMessageTexture, Uint32 _messageTime, MessageType _messageType)
            : pMessageTexture(std::move(_pMessageTexture)), messageTime(_messageTime), messageType(_messageType) { }

        ChatMessage(sdl2::texture_ptr _pTimeTexture, sdl2::texture_ptr _pUsernameTexture,
                    sdl2::texture_ptr _pMessageTexture, Uint32 _messageTime, MessageType _messageType)
            : pTimeTexture(std::move(_pTimeTexture)), pUsernameTexture(std::move(_pUsernameTexture)),
              pMessageTexture(std::move(_pMessageTexture)), messageTime(_messageTime), messageType(_messageType) { }

        ChatMessage(sdl2::texture_ptr _pMessageTexture, const DuneTexture* _pPictureTexture, Uint32 _messageTime,
                    MessageType _messageType)
            : pPictureTexture(_pPictureTexture), pMessageTexture(std::move(_pMessageTexture)),
              messageTime(_messageTime), messageType(_messageType) { }

        ChatMessage(const ChatMessage&) = delete;
        ChatMessage(ChatMessage&&) = default;
        ChatMessage& operator=(const ChatMessage&) = delete;
        ChatMessage& operator=(ChatMessage&&) = default;

        auto getUsernamePictureWidth() const {
            return pUsernameTexture ? getWidth(pUsernameTexture.get()) : getWidth(pPictureTexture);
        }

        auto getUsernamePictureHeight() const {
            return pUsernameTexture ? getHeight(pUsernameTexture.get()) : getHeight(pPictureTexture);
        }

        sdl2::texture_ptr  pTimeTexture;
        sdl2::texture_ptr  pUsernameTexture;
        const DuneTexture* pPictureTexture{};
        sdl2::texture_ptr  pMessageTexture;

        Uint32      messageTime;
        MessageType messageType;
    };

    std::deque<ChatMessage> chatMessages;
    std::unordered_map<std::string, sdl2::texture_ptr> user_textures;
};

#endif // CHATMANAGER_H
