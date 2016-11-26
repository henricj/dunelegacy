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

#include <GUI/dune/ChatManager.h>

#include <FileClasses/FontManager.h>
#include <misc/draw_util.h>
#include <globals.h>

#include <ctime>
#include <algorithm>

#define MAX_MESSAGESHOWTIME     15000
#define MAX_NUMBEROFMESSAGES    25

ChatManager::ChatManager() : Widget()
{

}

ChatManager::~ChatManager()
{

}

void ChatManager::draw(Point position)
{
    // delete all old messages
    Uint32 currentTime = SDL_GetTicks();
    while(chatMessages.empty() == false && chatMessages.front().messageTime + MAX_MESSAGESHOWTIME < currentTime) {
        chatMessages.pop_front();
    }

    // determine maximum vertical size of username and time
    int maxUsernameSizeY = 0;
    int maxTimeSizeY = 0;
    for(const ChatMessage& chatMessage : chatMessages) {
        if(chatMessage.messageType == MSGTYPE_NORMAL) {
            maxUsernameSizeY = std::max(maxUsernameSizeY, getWidth(chatMessage.pUsernameTexture.get()));
            maxTimeSizeY = std::max(maxTimeSizeY, getWidth(chatMessage.pTimeTexture.get()));
        }
    }

    SDL_Rect timedest = { position.x, position.y, 0, 0};
    SDL_Rect usernamedest = { position.x + 70, position.y, 0, 0};
    SDL_Rect messagedest = { position.x + 70 + maxUsernameSizeY, position.y, 0, 0};
    for(const ChatMessage& chatMessage : chatMessages) {

        if(chatMessage.messageType == MSGTYPE_NORMAL) {
            timedest.w = getWidth(chatMessage.pTimeTexture.get());
            timedest.h = getHeight(chatMessage.pTimeTexture.get());
            SDL_Rect tmpDest1 = timedest;
            SDL_RenderCopy(renderer, chatMessage.pTimeTexture.get(), nullptr, &tmpDest1);

            usernamedest.w = getWidth(chatMessage.pUsernameTexture.get());
            usernamedest.h = getHeight(chatMessage.pUsernameTexture.get());
            usernamedest.x = position.x + 70 + maxUsernameSizeY - getWidth(chatMessage.pUsernameTexture.get());
            SDL_Rect tmpDest2 = usernamedest;
            SDL_RenderCopy(renderer, chatMessage.pUsernameTexture.get(), nullptr, &tmpDest2);

            messagedest.w = getWidth(chatMessage.pMessageTexture.get());
            messagedest.h = getHeight(chatMessage.pMessageTexture.get());
            SDL_Rect tmpDest3 = messagedest;
            SDL_RenderCopy(renderer, chatMessage.pMessageTexture.get(), nullptr, &tmpDest3);


        } else {
            // MSGTYPE_INFO
            SDL_Rect infodest = calcDrawingRect(chatMessage.pMessageTexture.get(), position.x + 70 - 20, messagedest.y);
            SDL_RenderCopy(renderer, chatMessage.pMessageTexture.get(), nullptr, &infodest);

            messagedest.h = getHeight(chatMessage.pMessageTexture.get());
        }

        timedest.y += messagedest.h;
        usernamedest.y += messagedest.h;
        messagedest.y += messagedest.h;
    }
}

void ChatManager::addChatMessage(const std::string& username, const std::string& message)
{
    char timestring[80];
    time_t unixtime = time(nullptr);
    struct tm* timeinfo;

    timeinfo = localtime( &unixtime );
    strftime(timestring, 80, "(%H:%M:%S)", timeinfo);

    std::shared_ptr<SDL_Texture> pTimeTexture = std::shared_ptr<SDL_Texture>( pFontManager->createTextureWithText( timestring, COLOR_WHITE, FONT_STD10), SDL_DestroyTexture);
    std::shared_ptr<SDL_Texture> pUsernameTexture = std::shared_ptr<SDL_Texture>( pFontManager->createTextureWithText( username + ": ", COLOR_WHITE, FONT_STD10), SDL_DestroyTexture);
    std::shared_ptr<SDL_Texture> pMessageTexture = std::shared_ptr<SDL_Texture>( pFontManager->createTextureWithText( message, COLOR_WHITE, FONT_STD10), SDL_DestroyTexture);

    chatMessages.push_back( ChatMessage(pTimeTexture, pUsernameTexture, pMessageTexture, SDL_GetTicks(), MSGTYPE_NORMAL) );

    // delete old messages if there are too many messages on the screen
    while(chatMessages.size() > MAX_NUMBEROFMESSAGES) {
        chatMessages.pop_front();
    }
}

void ChatManager::addInfoMessage(const std::string& message)
{
    std::shared_ptr<SDL_Texture> pMessageTexture = std::shared_ptr<SDL_Texture>( pFontManager->createTextureWithText( "*  " + message, COLOR_GREEN, FONT_STD10), SDL_DestroyTexture);

    chatMessages.push_back( ChatMessage(pMessageTexture, SDL_GetTicks(), MSGTYPE_INFO) );

    // delete old messages if there are too many messages on the screen
    while(chatMessages.size() > MAX_NUMBEROFMESSAGES) {
        chatMessages.pop_front();
    }
}
