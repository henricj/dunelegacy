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

ChatManager::ChatManager() = default;

ChatManager::~ChatManager() = default;

void ChatManager::draw(Point position)
{
    // delete all old messages
    Uint32 currentTime = SDL_GetTicks();
    while(!chatMessages.empty() && chatMessages.front().messageTime + MAX_MESSAGESHOWTIME < currentTime) {
        chatMessages.pop_front();
    }

    // determine maximum vertical size of username and time
    int maxUsernameSizeY = 0;
    int maxTimeSizeY = 0;
    for(const auto& chatMessage : chatMessages) {
        if(chatMessage.messageType == MessageType::MSGTYPE_NORMAL) {
            maxUsernameSizeY =
                std::max(maxUsernameSizeY, chatMessage.getUsernamePictureHeight());
            maxTimeSizeY = std::max(maxTimeSizeY, getWidth(chatMessage.pTimeTexture.get()));
        }
    }

    SDL_Rect timeDest = { position.x, position.y, 0, 0};
    SDL_Rect usernameOrPictureDest = { position.x + LEFT_BORDER_WIDTH, position.y, 0, 0};
    SDL_Rect messageDest = { position.x + LEFT_BORDER_WIDTH + maxUsernameSizeY, position.y, 0, 0};
    for(const auto& chatMessage : chatMessages) {

        if(chatMessage.messageType == MessageType::MSGTYPE_NORMAL) {
            timeDest.w = getWidth(chatMessage.pTimeTexture.get());
            timeDest.h = getHeight(chatMessage.pTimeTexture.get());
            Dune_RenderCopy(renderer, chatMessage.pTimeTexture.get(), timeDest.x, timeDest.y);

            if(chatMessage.pUsernameTexture)
            {
                usernameOrPictureDest.w = getWidth(chatMessage.pUsernameTexture.get());
                usernameOrPictureDest.h = getWidth(chatMessage.pUsernameTexture.get());
                usernameOrPictureDest.x =
                    position.x + LEFT_BORDER_WIDTH + maxUsernameSizeY - getWidth(chatMessage.pUsernameTexture.get());
                Dune_RenderCopy(renderer, chatMessage.pUsernameTexture.get(), usernameOrPictureDest.x, usernameOrPictureDest.y);
            } else
            {
                usernameOrPictureDest.w = getWidth(chatMessage.pPictureTexture);
                usernameOrPictureDest.h = getHeight(chatMessage.pPictureTexture);
                usernameOrPictureDest.x =
                    position.x + LEFT_BORDER_WIDTH + maxUsernameSizeY - getWidth(chatMessage.pPictureTexture);
                Dune_RenderCopy(renderer, chatMessage.pPictureTexture, nullptr, &usernameOrPictureDest);
            }

            messageDest.w = getWidth(chatMessage.pMessageTexture.get());
            messageDest.h = getHeight(chatMessage.pMessageTexture.get());
            Dune_RenderCopy(renderer, chatMessage.pMessageTexture.get(), messageDest.x, messageDest.y);


        } else if(chatMessage.messageType == MessageType::MSGTYPE_INFO) {
            const auto infoDest = calcDrawingRectF(chatMessage.pMessageTexture.get(), position.x + LEFT_BORDER_WIDTH - 20, messageDest.y);
            Dune_RenderCopyF(renderer, chatMessage.pMessageTexture.get(), nullptr, &infoDest);

            messageDest.h = getHeight(chatMessage.pMessageTexture.get());
        } else {

            auto infoDest =
                calcDrawingRect(chatMessage.pMessageTexture.get(), position.x + LEFT_BORDER_WIDTH - 20, messageDest.y);

            int maxHeight;

            // MSGTYPE_PICTURE
            if(chatMessage.pUsernameTexture) {
                auto pictureDest = calcDrawingRect(chatMessage.pUsernameTexture.get(),
                                                   position.x + LEFT_BORDER_WIDTH - 30, messageDest.y, HAlign::Right);
                maxHeight        = std::max(pictureDest.h, infoDest.h);

                pictureDest.y = pictureDest.y + (maxHeight - pictureDest.h) / 2;

                Dune_RenderCopy(renderer, chatMessage.pUsernameTexture.get(), pictureDest.x, pictureDest.y);
            } else {
                auto pictureDest = calcDrawingRect(chatMessage.pPictureTexture, position.x + LEFT_BORDER_WIDTH - 30,
                                                   messageDest.y, HAlign::Right);

                maxHeight = std::max(pictureDest.h, infoDest.h);

                pictureDest.y = pictureDest.y + (maxHeight - pictureDest.h) / 2;

                chatMessage.pPictureTexture->draw(renderer, pictureDest.x, pictureDest.y);
            }

            infoDest.y = infoDest.y + (maxHeight - infoDest.h) / 2;
            Dune_RenderCopy(renderer, chatMessage.pMessageTexture.get(), infoDest.x, infoDest.y);

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
    auto unixtime = time(nullptr);

    auto *const timeinfo = localtime( &unixtime );
    strftime(timestring, 80, "(%H:%M:%S)", timeinfo);

    auto pTimeTexture = pFontManager->createTextureWithText( timestring, COLOR_WHITE, 12);
    auto pUsernameTexture = pFontManager->createTextureWithText( username + ": ", COLOR_WHITE, 12);
    auto pMessageTexture =  pFontManager->createTextureWithText( message, COLOR_WHITE, 12);

    chatMessages.emplace_back(std::move(pTimeTexture), std::move(pUsernameTexture), std::move(pMessageTexture),
                              SDL_GetTicks(), MessageType::MSGTYPE_NORMAL);

    prune_messages();
}

void ChatManager::addInfoMessage(const std::string& message)
{
    sdl2::texture_ptr pMessageTexture = pFontManager->createTextureWithText( "*  " + message, COLOR_GREEN, 12);

    chatMessages.emplace_back(std::move(pMessageTexture), SDL_GetTicks(), MessageType::MSGTYPE_INFO);

    prune_messages();
}

void ChatManager::addHintMessage(const std::string& message, const DuneTexture* pTexture)
{
    const auto width = settings.video.width - SIDEBARWIDTH - LEFT_BORDER_WIDTH - 20;

    const auto lines = greedyWordWrap(message,
                                width,
                                [](const std::string& tmp) {
                                    return DuneStyle::getInstance().getTextWidth(tmp, 12);
                                });

    const int height = lines.size() * DuneStyle::getInstance().getTextHeight(12) + 4;

    auto pMessageTexture = convertSurfaceToTexture(DuneStyle::getInstance().createLabelSurface( width, height, lines, 12, Alignment_Left, COLOR_WHITE, COLOR_TRANSPARENT));

    chatMessages.emplace_back(std::move(pMessageTexture), pTexture, SDL_GetTicks(), MessageType::MSGTYPE_PICTURE);

    prune_messages();
}

void ChatManager::prune_messages()
{
    // delete old messages if there are too many messages on the screen
    while (chatMessages.size() >= MAX_NUMBEROFMESSAGES) {
        chatMessages.pop_front();
    }
}
