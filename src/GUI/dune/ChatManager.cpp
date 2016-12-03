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
#include <GUI/dune/DuneStyle.h>
#include <misc/draw_util.h>
#include <misc/string_util.h>
#include <globals.h>

#include <ctime>
#include <algorithm>

#define MAX_MESSAGESHOWTIME     11000
#define MAX_NUMBEROFMESSAGES    25
#define LEFT_BORDER_WIDTH       70

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
            maxUsernameSizeY = std::max(maxUsernameSizeY, getWidth(chatMessage.pUsernameOrPictureTexture.get()));
            maxTimeSizeY = std::max(maxTimeSizeY, getWidth(chatMessage.pTimeTexture.get()));
        }
    }

    SDL_Rect timeDest = { position.x, position.y, 0, 0};
    SDL_Rect usernameOrPictureDest = { position.x + LEFT_BORDER_WIDTH, position.y, 0, 0};
    SDL_Rect messageDest = { position.x + LEFT_BORDER_WIDTH + maxUsernameSizeY, position.y, 0, 0};
    for(const ChatMessage& chatMessage : chatMessages) {

        if(chatMessage.messageType == MSGTYPE_NORMAL) {
            timeDest.w = getWidth(chatMessage.pTimeTexture.get());
            timeDest.h = getHeight(chatMessage.pTimeTexture.get());
            SDL_Rect tmpDest1 = timeDest;
            SDL_RenderCopy(renderer, chatMessage.pTimeTexture.get(), nullptr, &tmpDest1);

            usernameOrPictureDest.w = getWidth(chatMessage.pUsernameOrPictureTexture.get());
            usernameOrPictureDest.h = getHeight(chatMessage.pUsernameOrPictureTexture.get());
            usernameOrPictureDest.x = position.x + LEFT_BORDER_WIDTH + maxUsernameSizeY - getWidth(chatMessage.pUsernameOrPictureTexture.get());
            SDL_Rect tmpDest2 = usernameOrPictureDest;
            SDL_RenderCopy(renderer, chatMessage.pUsernameOrPictureTexture.get(), nullptr, &tmpDest2);

            messageDest.w = getWidth(chatMessage.pMessageTexture.get());
            messageDest.h = getHeight(chatMessage.pMessageTexture.get());
            SDL_Rect tmpDest3 = messageDest;
            SDL_RenderCopy(renderer, chatMessage.pMessageTexture.get(), nullptr, &tmpDest3);


        } else if(chatMessage.messageType == MSGTYPE_INFO) {
            SDL_Rect infoDest = calcDrawingRect(chatMessage.pMessageTexture.get(), position.x + LEFT_BORDER_WIDTH - 20, messageDest.y);
            SDL_RenderCopy(renderer, chatMessage.pMessageTexture.get(), nullptr, &infoDest);

            messageDest.h = getHeight(chatMessage.pMessageTexture.get());
        } else {
            // MSGTYPE_PICTURE
            SDL_Rect pictureDest = calcDrawingRect(chatMessage.pUsernameOrPictureTexture.get(), position.x + LEFT_BORDER_WIDTH - 30, messageDest.y, HAlign::Right);
            SDL_Rect infoDest = calcDrawingRect(chatMessage.pMessageTexture.get(), position.x + LEFT_BORDER_WIDTH - 20, messageDest.y);
            int maxHeight = std::max(pictureDest.h, infoDest.h);

            pictureDest.y = pictureDest.y + (maxHeight - pictureDest.h)/2;
            infoDest.y = infoDest.y + (maxHeight - infoDest.h)/2;

            SDL_RenderCopy(renderer, chatMessage.pUsernameOrPictureTexture.get(), nullptr, &pictureDest);
            SDL_RenderCopy(renderer, chatMessage.pMessageTexture.get(), nullptr, &infoDest);

            messageDest.h = maxHeight + 10;
        }

        timeDest.y += messageDest.h;
        usernameOrPictureDest.y += messageDest.h;
        messageDest.y += messageDest.h;
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

void ChatManager::addHintMessage(const std::string& message, SDL_Texture* pTexture)
{
    int width = settings.video.width - SIDEBARWIDTH - LEFT_BORDER_WIDTH - 20;

    std::vector<std::string> lines = greedyWordWrap(message,
                                                    width,
                                                    [](const std::string& tmp) {
                                                        return DuneStyle::getInstance().getTextWidth(tmp, FONT_STD10);
                                                    });

    int height = lines.size() * DuneStyle::getInstance().getTextHeight(FONT_STD10) + 4;


    SDL_Texture* pTextTexture = convertSurfaceToTexture(DuneStyle::getInstance().createLabelSurface( width, height, lines, FONT_STD10, Alignment_Left, COLOR_WHITE, COLOR_TRANSPARENT), true);
    std::shared_ptr<SDL_Texture> pMessageTexture = std::shared_ptr<SDL_Texture>(pTextTexture, SDL_DestroyTexture);
    std::shared_ptr<SDL_Texture> pPictureTexture = std::shared_ptr<SDL_Texture>(std::shared_ptr<SDL_Texture>(), pTexture);

    chatMessages.push_back( ChatMessage(pMessageTexture, pPictureTexture, SDL_GetTicks(), MSGTYPE_PICTURE) );

    // delete old messages if there are too many messages on the screen
    while(chatMessages.size() > MAX_NUMBEROFMESSAGES) {
        chatMessages.pop_front();
    }
}
