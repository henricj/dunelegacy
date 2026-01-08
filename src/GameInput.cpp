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

#include <Game.h>

#include <globals.h>

#include "FileClasses/DuneConfig.h"
#include "FileClasses/music/MusicPlayer.h"
#include "misc/Fullscreen.h"
#include <FileClasses/GFXManager.h>
#include <FileClasses/TextManager.h>
#include <GUI/GUIStyle.h>
#include <GUI/dune/InGameMenu.h>
#include <GUI/dune/WaitingForOtherPlayers.h>
#include <GameInterface.h>
#include <House.h>
#include <Map.h>
#include <Menu/MentatHelp.h>
#include <Network/NetworkManager.h>
#include <ScreenBorder.h>
#include <SoundPlayer.h>
#include <misc/dune_sdl.h>
#include <misc/md5.h>
#include <players/HumanPlayer.h>
#include <sand.h>
#include <structures/BuilderBase.h>
#include <structures/ConstructionYard.h>
#include <structures/Palace.h>
#include <structures/StructureBase.h>
#include <units/GroundUnit.h>
#include <units/Harvester.h>
#include <units/InfantryBase.h>
#include <units/UnitBase.h>

#include <fmt/format.h>

namespace {
// Cheats are available in singleplayer modes:
// "Let me cheat"
constexpr auto CHEAT_MODE_ENABLE = std::to_array<unsigned char>(
    {0xB8, 0x76, 0x6C, 0x8E, 0xC7, 0xA6, 0x10, 0x36, 0xB6, 0x98, 0x93, 0xFC, 0x17, 0xAA, 0xF2, 0x1E});
// "Let me win"
constexpr auto CHEAT_WIN_GAME = std::to_array<unsigned char>(
    {0x57, 0x58, 0x32, 0x91, 0xCB, 0x37, 0xF8, 0x16, 0x7E, 0xDB, 0x06, 0x11, 0xD8, 0xD1, 0x9E, 0x58});
// "Start debugging"
constexpr auto CHEAT_START_DEBUGGING = std::to_array<unsigned char>(
    {0x1A, 0x12, 0xBE, 0x3D, 0xBE, 0x54, 0xC5, 0xA5, 0x04, 0xCA, 0xA6, 0xEE, 0x97, 0x82, 0xC1, 0xC8});
// "Stop debugging"
constexpr auto CHEAT_STOP_DEBUGGING = std::to_array<unsigned char>(
    {0x54, 0xF6, 0x81, 0x55, 0xFC, 0x64, 0xA5, 0xBC, 0x66, 0xDC, 0xD5, 0x0C, 0x1E, 0x92, 0x5C, 0x0B});
// "Give me some credits"
constexpr auto CHEAT_GIVE_CREDITS = std::to_array<unsigned char>(
    {0xCE, 0xF1, 0xD2, 0x6C, 0xE4, 0xB1, 0x45, 0xDE, 0x98, 0x55, 0x03, 0xCA, 0x35, 0x23, 0x2E, 0xD8});
} // namespace

void Game::doInput(const GameContext& context, SDL_Event& event) {
    // check for a key press

    // first of all update mouse
    if (event.type == SDL_EVENT_MOUSE_MOTION) {
        const SDL_MouseMotionEvent* mouse = &event.motion;

        const auto& video = dune::globals::settings.video;

        dune::globals::drawnMouseX = std::max(0, std::min(static_cast<int>(mouse->x), video.width - 1));
        dune::globals::drawnMouseY = std::max(0, std::min(static_cast<int>(mouse->y), video.height - 1));
    }

    if (pInGameMenu_ != nullptr) {
        pInGameMenu_->handleInput(event);

        if (!bMenu_) {
            pInGameMenu_.reset();
        }

    } else if (pInGameMentat_ != nullptr) {
        pInGameMentat_->doInput(event);

        if (!bMenu_) {
            pInGameMentat_.reset();
        }

    } else if (pWaitingForOtherPlayers_ != nullptr) {
        pWaitingForOtherPlayers_->handleInput(event);

        if (!bMenu_) {
            pWaitingForOtherPlayers_.reset();
        }
    } else {
        /* Look for a keypress */
        switch (event.type) {

            case SDL_EVENT_KEY_DOWN: {
                if (chatMode_) {
                    handleChatInput(context, event.key);
                } else {
                    handleKeyInput(context, event.key);
                }
            } break;

            case SDL_EVENT_TEXT_INPUT: {
                if (chatMode_) {
                    const auto* const newText = event.text.text;
                    if (utf8Length(typingChatMessage_) + utf8Length(newText) <= 60) {
                        typingChatMessage_ += newText;
                    }
                }
            } break;

            case SDL_EVENT_MOUSE_WHEEL: {
                if (event.wheel.y != 0) {
                    pInterface_->handleMouseWheel(
                        dune::globals::drawnMouseX, dune::globals::drawnMouseY, (event.wheel.y > 0));
                }
            } break;

            case SDL_EVENT_MOUSE_BUTTON_DOWN: {
                const auto* const mouse = &event.button;

                switch (mouse->button) {
                    case SDL_BUTTON_LEFT: {
                        pInterface_->handleMouseLeft(static_cast<int>(mouse->x), static_cast<int>(mouse->y), true);
                    } break;

                    case SDL_BUTTON_RIGHT: {
                        pInterface_->handleMouseRight(static_cast<int>(mouse->x), static_cast<int>(mouse->y), true);
                    } break;

                    default: break;
                }

                const auto* const screenborder = dune::globals::screenborder.get();

                const auto mouseX = mouse->x;
                const auto mouseY = mouse->y;

                switch (mouse->button) {

                    case SDL_BUTTON_LEFT: {

                        switch (currentCursorMode) {

                            case CursorMode_Placing: {
                                if (screenborder->isScreenCoordInsideMap(mouseX, mouseY)) {
                                    handlePlacementClick(
                                        context, screenborder->screen2MapX(mouseX), screenborder->screen2MapY(mouseY));
                                }
                            } break;

                            case CursorMode_Attack: {

                                if (screenborder->isScreenCoordInsideMap(mouseX, mouseY)) {
                                    handleSelectedObjectsAttackClick(
                                        context, screenborder->screen2MapX(mouseX), screenborder->screen2MapY(mouseY));
                                }

                            } break;

                            case CursorMode_Move: {

                                if (screenborder->isScreenCoordInsideMap(mouseX, mouseY)) {
                                    handleSelectedObjectsMoveClick(
                                        context, screenborder->screen2MapX(mouseX), screenborder->screen2MapY(mouseY));
                                }

                            } break;

                            case CursorMode_CarryallDrop: {

                                if (screenborder->isScreenCoordInsideMap(mouseX, mouseY)) {
                                    handleSelectedObjectsRequestCarryallDropClick(
                                        context, screenborder->screen2MapX(mouseX), screenborder->screen2MapY(mouseY));
                                }

                            } break;

                            case CursorMode_Capture: {

                                if (screenborder->isScreenCoordInsideMap(mouseX, mouseY)) {
                                    handleSelectedObjectsCaptureClick(
                                        context, screenborder->screen2MapX(mouseX), screenborder->screen2MapY(mouseY));
                                }

                            } break;

                            case CursorMode_Normal:
                            default: {

                                if (mouseX < sideBarPos_.x && mouseY >= topBarPos_.h) {
                                    // it isn't on the gamebar

                                    if (!selectionMode_) {
                                        // if we have started the selection rectangle
                                        // the starting point of the selection rectangle
                                        selectionRect_.x = screenborder->screen2worldX(mouseX);
                                        selectionRect_.y = screenborder->screen2worldY(mouseY);
                                    }
                                    selectionMode_ = true;
                                }
                            } break;
                        }
                    } break; // end of SDL_BUTTON_LEFT

                    case SDL_BUTTON_RIGHT: {
                        // if the right mouse button is pressed

                        if (currentCursorMode != CursorMode_Normal) {
                            // cancel special cursor mode
                            currentCursorMode = CursorMode_Normal;

                            break;
                        }

                        if (selectedList_.empty())
                            break;

                        const auto object_id = *selectedList_.begin();

                        if (const auto* const object = objectManager_.getObject(object_id)) {
                            if (object->getOwner() == dune::globals::pLocalHouse && object->isRespondable()) {
                                // if user has a controllable unit selected

                                if (screenborder->isScreenCoordInsideMap(mouseX, mouseY)) {
                                    if (handleSelectedObjectsActionClick(context,
                                                                         screenborder->screen2MapX(mouseX),
                                                                         screenborder->screen2MapY(mouseY))) {
                                        indicatorFrame_      = 0;
                                        indicatorPosition_.x = screenborder->screen2worldX(mouseX);
                                        indicatorPosition_.y = screenborder->screen2worldY(mouseY);
                                    }
                                }
                            }
                        }
                    } break; // end of SDL_BUTTON_RIGHT
                    default: break;
                }
            } break;

            case SDL_EVENT_MOUSE_MOTION: {
                const auto* const mouse = &event.motion;

                pInterface_->handleMouseMovement(static_cast<int>(mouse->x), static_cast<int>(mouse->y));
            } break;

            case SDL_EVENT_MOUSE_BUTTON_UP: {
                const auto* const mouse = &event.button;

                switch (mouse->button) {
                    case SDL_BUTTON_LEFT: {
                        pInterface_->handleMouseLeft(static_cast<int>(mouse->x), static_cast<int>(mouse->y), false);
                    } break;

                    case SDL_BUTTON_RIGHT: {
                        pInterface_->handleMouseRight(static_cast<int>(mouse->x), static_cast<int>(mouse->y), false);
                    } break;
                    default: break;
                }

                if (selectionMode_ && (mouse->button == SDL_BUTTON_LEFT)) {
                    // this keeps the box on the map, and not over game bar
                    auto finalMouseX = mouse->x;
                    auto finalMouseY = mouse->y;

                    if (finalMouseX >= sideBarPos_.x) {
                        finalMouseX = sideBarPos_.x - 1;
                    }

                    if (finalMouseY < topBarPos_.y + topBarPos_.h) {
                        finalMouseY = topBarPos_.x + topBarPos_.h;
                    }

                    const auto* const screenborder = dune::globals::screenborder.get();

                    int rectFinishX = screenborder->screen2MapX(finalMouseX);
                    if (rectFinishX > (map_->getSizeX() - 1)) {
                        rectFinishX = map_->getSizeX() - 1;
                    }

                    const int rectFinishY = screenborder->screen2MapY(finalMouseY);

                    // convert start also to map coordinates
                    const int rectStartX = selectionRect_.x / TILESIZE;
                    const int rectStartY = selectionRect_.y / TILESIZE;

                    map_->selectObjects(dune::globals::pLocalHouse,
                                        rectStartX,
                                        rectStartY,
                                        rectFinishX,
                                        rectFinishY,
                                        screenborder->screen2worldX(finalMouseX),
                                        screenborder->screen2worldY(finalMouseY),
                                        SDL_GetModState() & SDL_KMOD_SHIFT);

                    if (selectedList_.size() == 1) {
                        const auto* pHarvester = objectManager_.getObject<Harvester>(*selectedList_.begin());
                        if (pHarvester != nullptr && pHarvester->getOwner() == dune::globals::pLocalHouse) {

                            auto harvesterMessage = std::string{_("@DUNE.ENG|226#Harvester")};

                            const auto percent = lround(100 * pHarvester->getAmountOfSpice() / HARVESTERMAXSPICE);
                            if (percent > 0) {
                                if (pHarvester->isAwaitingPickup()) {
                                    harvesterMessage +=
                                        fmt::sprintf(_("@DUNE.ENG|124#full and awaiting pickup"), percent);
                                } else if (pHarvester->isReturning()) {
                                    harvesterMessage += fmt::sprintf(_("@DUNE.ENG|123#full and returning"), percent);
                                } else if (pHarvester->isHarvesting()) {
                                    harvesterMessage += fmt::sprintf(_("@DUNE.ENG|122#full and harvesting"), percent);
                                } else {
                                    harvesterMessage += fmt::sprintf(_("@DUNE.ENG|121#full"), percent);
                                }

                            } else {
                                if (pHarvester->isAwaitingPickup()) {
                                    harvesterMessage += _("@DUNE.ENG|128#empty and awaiting pickup");
                                } else if (pHarvester->isReturning()) {
                                    harvesterMessage += _("@DUNE.ENG|127#empty and returning");
                                } else if (pHarvester->isHarvesting()) {
                                    harvesterMessage += _("@DUNE.ENG|126#empty and harvesting");
                                } else {
                                    harvesterMessage += _("@DUNE.ENG|125#empty");
                                }
                            }

                            if (!pInterface_->newsTickerHasMessage()) {
                                pInterface_->addToNewsTicker(harvesterMessage);
                            }
                        }
                    }
                }

                selectionMode_ = false;

            } break;

            case SDL_EVENT_WINDOW_PIXEL_SIZE_CHANGED: {
                auto& gui = GUIStyle::getInstance();

                gui.setLogicalSize(dune::globals::renderer.get(), event.window.data1, event.window.data2);

                resize();
            } break;

            case SDL_EVENT_QUIT: {
                bQuitGame_ = true;
            } break;

            default: break;
        }
    }

    if ((pInGameMenu_ == nullptr) && (pInGameMentat_ == nullptr) && (pWaitingForOtherPlayers_ == nullptr)
        && (SDL_GetWindowFlags(dune::globals::window.get()) & SDL_WINDOW_INPUT_FOCUS)) {

        const auto* keystate = SDL_GetKeyboardState(nullptr);
        // Update map scrolling speed state. Using both keyboard and mouse to scroll at the same time is intended to
        // accumulate together.
        mapVerticalScroll_ = (dune::globals::drawnMouseY >= getRendererHeight() - 1 - SCROLLBORDER ? 1 : 0)
                           + (keystate[SDL_SCANCODE_DOWN] ? 1 : 0)
                           + (dune::globals::drawnMouseY <= SCROLLBORDER ? -1 : 0)
                           + (keystate[SDL_SCANCODE_UP] ? -1 : 0);
        mapHorizontalScroll_ = (dune::globals::drawnMouseX <= SCROLLBORDER ? -1 : 0)
                             + (keystate[SDL_SCANCODE_LEFT] ? -1 : 0)
                             + (dune::globals::drawnMouseX >= getRendererWidth() - 1 - SCROLLBORDER ? 1 : 0)
                             + (keystate[SDL_SCANCODE_RIGHT] ? 1 : 0);

        // Holding down shift enables 3x fast speed scrolling.
        if (keystate[SDL_SCANCODE_LSHIFT] || keystate[SDL_SCANCODE_RSHIFT]) {
            mapVerticalScroll_ *= 3;
            mapHorizontalScroll_ *= 3;
        }
    } else {
        mapVerticalScroll_ = mapHorizontalScroll_ = 0;
    }

    if (sdl_handler_ && Window::isBroadcastEvent(event))
        sdl_handler_(event);
}

void Game::handleChatInput([[maybe_unused]] const GameContext& context, SDL_KeyboardEvent& keyboardEvent) {
    if (keyboardEvent.key == SDLK_ESCAPE) {
        chatMode_ = false;
    } else if (keyboardEvent.key == SDLK_RETURN) {
        if (typingChatMessage_.length() > 0) {
            std::array<unsigned char, 16> md5sum{};

            const auto* const code = reinterpret_cast<const unsigned char*>(typingChatMessage_.data());
            md5(code, typingChatMessage_.size(), md5sum.data());

            if ((!bCheatsEnabled_) && (std::ranges::equal(md5sum, CHEAT_MODE_ENABLE))) {
                bCheatsEnabled_ = true;
                pInterface_->getChatManager().addInfoMessage("Cheat mode enabled");
            } else if ((bCheatsEnabled_) && (std::ranges::equal(md5sum, CHEAT_MODE_ENABLE))) {
                pInterface_->getChatManager().addInfoMessage("Cheat mode already enabled");
            } else if ((bCheatsEnabled_) && (std::ranges::equal(md5sum, CHEAT_WIN_GAME))) {
                if (gameType != GameType::CustomMultiplayer) {
                    pInterface_->getChatManager().addInfoMessage("You win this game");
                    setGameWon();
                }
            } else if ((bCheatsEnabled_) && (std::ranges::equal(md5sum, CHEAT_START_DEBUGGING))) {
                if (dune::globals::debug) {
                    pInterface_->getChatManager().addInfoMessage("You are already in debug mode");
                } else if (gameType != GameType::CustomMultiplayer) {
                    pInterface_->getChatManager().addInfoMessage("Debug mode enabled");
                    dune::globals::debug = true;
                }
            } else if ((bCheatsEnabled_) && (std::ranges::equal(md5sum, CHEAT_STOP_DEBUGGING))) {
                if (!dune::globals::debug) {
                    pInterface_->getChatManager().addInfoMessage("You are not in debug mode");
                } else if (gameType != GameType::CustomMultiplayer) {
                    pInterface_->getChatManager().addInfoMessage("Debug mode disabled");
                    dune::globals::debug = false;
                }
            } else if ((bCheatsEnabled_) && (std::ranges::equal(md5sum, CHEAT_GIVE_CREDITS))) {
                if (gameType != GameType::CustomMultiplayer) {
                    pInterface_->getChatManager().addInfoMessage("You got some credits");
                    dune::globals::pLocalHouse->returnCredits(10000_fix);
                }
            } else {
                if (auto* const network_manager = dune::globals::pNetworkManager.get()) {
                    network_manager->sendChatMessage(typingChatMessage_);
                }
                pInterface_->getChatManager().addChatMessage(getLocalPlayerName(), typingChatMessage_);
            }
        }

        chatMode_ = false;
    } else if (keyboardEvent.key == SDLK_BACKSPACE) {
        if (typingChatMessage_.length() > 0) {
            typingChatMessage_ = utf8Substr(typingChatMessage_, 0, utf8Length(typingChatMessage_) - 1);
        }
    }
}

void Game::handleKeyInput(const GameContext& context, SDL_KeyboardEvent& keyboardEvent) {
    auto* const screenborder = dune::globals::screenborder.get();

    switch (keyboardEvent.key) {

        case SDLK_0: {
            // if ctrl and 0 remove selected units from all groups
            if (SDL_GetModState() & KMOD_CTRL) {
                std::vector<ObjectBase*> selected_objects;
                selected_objects.reserve(selectedList_.size());

                for (auto objectID : selectedList_) {
                    if (auto* object = objectManager_.getObject(objectID))
                        selected_objects.push_back(object);
                }

                for (auto* pObject : selected_objects) {
                    const auto objectID = pObject->getObjectID();

                    removeFromSelectionLists(pObject);
                    removeFromQuickSelectionLists(objectID);
                }
            } else {
                for (uint32_t objectID : selectedList_) {
                    if (auto* object = objectManager_.getObject(objectID))
                        object->setSelected(false);
                }
            }

            selectedList_.clear();
            selectionChanged();
            currentCursorMode = CursorMode_Normal;
        } break;

        case SDLK_1:
        case SDLK_2:
        case SDLK_3:
        case SDLK_4:
        case SDLK_5:
        case SDLK_6:
        case SDLK_7:
        case SDLK_8:
        case SDLK_9: {
            // for SDLK_1 to SDLK_9 select group with that number, if ctrl create group from selected obj
            const auto selectListIndex = keyboardEvent.key - SDLK_1;

            auto* const local_player = dune::globals::pLocalPlayer;

            if (SDL_GetModState() & KMOD_CTRL) {
                local_player->setGroupList(selectListIndex, selectedList_);

                pInterface_->updateObjectInterface();
            } else {
                const auto& groupList = local_player->getGroupList(selectListIndex);

                // find out if we are choosing a group with all items already selected
                bool bEverythingWasSelected = (selectedList_.size() == groupList.size());
                Coord averagePosition;
                for (auto objectID : groupList) {
                    auto* pObject          = objectManager_.getObject(objectID);
                    bEverythingWasSelected = bEverythingWasSelected && pObject->isSelected();
                    averagePosition += pObject->getLocation();
                }

                if (!groupList.empty()) {
                    averagePosition /= gsl::narrow<uint32_t>(groupList.size());
                }

                if (SDL_GetModState() & KMOD_SHIFT) {
                    // we add the items from this list to the list of selected items
                } else {
                    // we replace the list of the selected items with the items from this list
                    clearSelectedList();
                }

                // now we add the selected items
                for (auto objectID : groupList) {
                    auto* pObject = objectManager_.getObject(objectID);
                    if (pObject->getOwner() == dune::globals::pLocalHouse) {
                        pObject->setSelected(true);
                        selectedList_.insert(pObject->getObjectID());
                        selectionChanged();
                    }
                }

                if (bEverythingWasSelected && (!groupList.empty())) {
                    // we center around the newly selected units/structures
                    dune::globals::screenborder->setNewScreenCenter(averagePosition * TILESIZE);
                }
            }
            currentCursorMode = CursorMode_Normal;
        } break;

        case SDLK_KP_MINUS:
        case SDLK_MINUS: {
            if (gameType != GameType::CustomMultiplayer) {
                const auto& settings = dune::globals::settings;

                dune::globals::settings.gameOptions.gameSpeed =
                    std::min(settings.gameOptions.gameSpeed + 1, GAMESPEED_MAX);
                INIFile myINIFile(getConfigFilepath());
                myINIFile.setIntValue("Game Options", "Game Speed", settings.gameOptions.gameSpeed);
                if (!myINIFile.saveChangesTo(getConfigFilepath())) {
                    sdl2::log_error("Unable to save configuration file {}", getConfigFilepath().string());
                }
                addToNewsTicker(fmt::format("{}: {}", _("Game speed"), settings.gameOptions.gameSpeed));
            }
        } break;

        case SDLK_KP_PLUS:
        case SDLK_PLUS:
        case SDLK_EQUALS: {
            if (gameType != GameType::CustomMultiplayer) {
                const auto& settings = dune::globals::settings;

                dune::globals::settings.gameOptions.gameSpeed =
                    std::max(settings.gameOptions.gameSpeed - 1, GAMESPEED_MIN);
                INIFile myINIFile(getConfigFilepath());
                myINIFile.setIntValue("Game Options", "Game Speed", settings.gameOptions.gameSpeed);
                if (!myINIFile.saveChangesTo(getConfigFilepath())) {
                    sdl2::log_error("Unable to save configuration file {}", getConfigFilepath().string());
                }
                addToNewsTicker(fmt::format("{}: {}", _("Game speed"), settings.gameOptions.gameSpeed));
            }
        } break;

        case SDLK_c: {
            // set object to capture
            if (currentCursorMode != CursorMode_Capture) {
                for (uint32_t objectID : selectedList_) {
                    ObjectBase* pObject = objectManager_.getObject(objectID);
                    if (pObject->isAUnit() && (pObject->getOwner() == dune::globals::pLocalHouse)
                        && pObject->isRespondable() && pObject->canAttack() && pObject->isInfantry()) {
                        currentCursorMode = CursorMode_Capture;
                        break;
                    }
                }
            }
        } break;

        case SDLK_a: {
            // set object to attack
            if (currentCursorMode != CursorMode_Attack) {
                const auto* house = dune::globals::pLocalHouse;

                for (auto objectID : selectedList_) {
                    auto* pObject = objectManager_.getObject(objectID);
                    auto* pOwner  = pObject->getOwner();

                    if (pObject->isAUnit() && pOwner == house && pObject->isRespondable() && pObject->canAttack()) {
                        currentCursorMode = CursorMode_Attack;
                        break;
                    }

                    auto* const palace = dune_cast<Palace>(pObject);
                    if (palace == nullptr)
                        continue;

                    if (pOwner->getHouseID() == HOUSETYPE::HOUSE_HARKONNEN
                        || pOwner->getHouseID() == HOUSETYPE::HOUSE_SARDAUKAR) {
                        if (palace->isSpecialWeaponReady()) {
                            currentCursorMode = CursorMode_Attack;
                            break;
                        }
                    }
                }
            }
        } break;

        case SDLK_t: {
            bShowTime_ = !bShowTime_;
        } break;

        case SDLK_ESCAPE: {
            onOptions();
        } break;

        case SDLK_F1: {
            const auto oldCenterCoord       = screenborder->getCurrentCenter();
            dune::globals::currentZoomlevel = 0;
            screenborder->adjustScreenBorderToMapsize(map_->getSizeX(), map_->getSizeY());
            screenborder->setNewScreenCenter(oldCenterCoord);
        } break;

        case SDLK_F2: {
            const auto oldCenterCoord       = screenborder->getCurrentCenter();
            dune::globals::currentZoomlevel = 1;
            screenborder->adjustScreenBorderToMapsize(map_->getSizeX(), map_->getSizeY());
            screenborder->setNewScreenCenter(oldCenterCoord);
        } break;

        case SDLK_F3: {
            const auto oldCenterCoord       = screenborder->getCurrentCenter();
            dune::globals::currentZoomlevel = 2;
            screenborder->adjustScreenBorderToMapsize(map_->getSizeX(), map_->getSizeY());
            screenborder->setNewScreenCenter(oldCenterCoord);
        } break;

        case SDLK_F4: {
            // skip a 10 seconds
            if (gameType != GameType::CustomMultiplayer || bReplay_) {
                skipToGameCycle_ = gameCycleCount_ + (10 * 1000) / GAMESPEED_DEFAULT;
            }
        } break;

        case SDLK_F5: {
            // skip a 30 seconds
            if (gameType != GameType::CustomMultiplayer || bReplay_) {
                skipToGameCycle_ = gameCycleCount_ + (30 * 1000) / GAMESPEED_DEFAULT;
            }
        } break;

        case SDLK_F6: {
            // skip 2 minutes
            if (gameType != GameType::CustomMultiplayer || bReplay_) {
                skipToGameCycle_ = gameCycleCount_ + (120 * 1000) / GAMESPEED_DEFAULT;
            }
        } break;

        case SDLK_F10: {
            dune::globals::soundPlayer->toggleSound();
        } break;

        case SDLK_F11: {
            dune::globals::musicPlayer->toggleSound();
        } break;

        case SDLK_F12: {
            bShowFPS_ = !bShowFPS_;
        } break;

        case SDLK_m: {
            // set object to move
            if (currentCursorMode != CursorMode_Move) {
                for (const auto objectID : selectedList_) {
                    auto* const pObject = objectManager_.getObject(objectID);
                    if (pObject->isAUnit() && (pObject->getOwner() == dune::globals::pLocalHouse)
                        && pObject->isRespondable()) {
                        currentCursorMode = CursorMode_Move;
                        break;
                    }
                }
            }
        } break;

        case SDLK_g: {
            // select next construction yard
            dune::selected_set_type itemIDs;
            itemIDs.insert(Structure_ConstructionYard);
            selectNextStructureOfType(itemIDs);
        } break;

        case SDLK_f: {
            // select next factory
            dune::selected_set_type itemIDs;
            itemIDs.insert(Structure_Barracks);
            itemIDs.insert(Structure_WOR);
            itemIDs.insert(Structure_LightFactory);
            itemIDs.insert(Structure_HeavyFactory);
            itemIDs.insert(Structure_HighTechFactory);
            itemIDs.insert(Structure_StarPort);
            selectNextStructureOfType(itemIDs);
        } break;

        case SDLK_p: {
            if (SDL_GetModState() & KMOD_CTRL) {
                // fall through to SDLK_PRINT
            } else {
                // Place structure
                if (selectedList_.size() == 1) {
                    const auto object_id = *selectedList_.begin();

                    if (auto* const pConstructionYard = objectManager_.getObject<ConstructionYard>(object_id)) {
                        if (currentCursorMode == CursorMode_Placing) {
                            currentCursorMode = CursorMode_Normal;
                        } else if (pConstructionYard->isWaitingToPlace()) {
                            currentCursorMode = CursorMode_Placing;
                        }
                    }
                }

                break; // do not fall through
            }

        } // fall through

        case SDLK_PRINTSCREEN:
        case SDLK_SYSREQ: {
            if (SDL_GetModState() & KMOD_SHIFT) {
                takePeriodicScreenshots_ = !takePeriodicScreenshots_;
            } else {
                takeScreenshot();
            }
        } break;

        case SDLK_h: {
            for (uint32_t objectID : selectedList_) {
                if (auto* const pObject = objectManager_.getObject<Harvester>(objectID)) {
                    pObject->handleReturnClick(context);
                }
            }
        } break;

        case SDLK_r: {
            for (uint32_t objectID : selectedList_) {
                auto* const pObject = objectManager_.getObject(objectID);
                if (auto* const structure = dune_cast<StructureBase>(pObject)) {
                    structure->handleRepairClick();
                } else if (auto* const groundUnit = dune_cast<GroundUnit>(pObject);
                           groundUnit && groundUnit->getHealth() < pObject->getMaxHealth()) {
                    groundUnit->handleSendToRepairClick();
                }
            }
        } break;

        case SDLK_d: {
            if (currentCursorMode != CursorMode_CarryallDrop) {
                for (uint32_t objectID : selectedList_) {
                    auto* const pObject = objectManager_.getObject<GroundUnit>(objectID);
                    if (pObject && pObject->getOwner()->hasCarryalls()) {
                        currentCursorMode = CursorMode_CarryallDrop;
                    }
                }
            }

        } break;

        case SDLK_u: {
            for (uint32_t objectID : selectedList_) {
                if (auto* const pBuilder = objectManager_.getObject<BuilderBase>(objectID)) {
                    if (pBuilder->getHealth() >= pBuilder->getMaxHealth() && pBuilder->isAllowedToUpgrade()) {
                        pBuilder->handleUpgradeClick();
                    }
                }
            }
        } break;

        case SDLK_RETURN: {
            if (SDL_GetModState() & KMOD_ALT) {
                toggleFullscreen();
            } else {
                typingChatMessage_.clear();
                chatMode_ = true;
            }
        } break;

        case SDLK_TAB: {
            if (SDL_GetModState() & KMOD_ALT) {
                SDL_MinimizeWindow(dune::globals::window.get());
            }
        } break;

        case SDLK_SPACE: {
            if (gameType != GameType::CustomMultiplayer) {
                if (bPause_) {
                    resumeGame();
                } else {
                    pauseGame();
                }
            }
        } break;

        default: {
        } break;
    }
}

bool Game::handlePlacementClick(const GameContext& context, int xPos, int yPos) {
    const BuilderBase* pBuilder = nullptr;

    if (selectedList_.size() == 1) {
        pBuilder = objectManager_.getObject<BuilderBase>(*selectedList_.begin());
    }

    if (!pBuilder) {
        return false;
    }

    const auto* const currentGame = dune::globals::currentGame.get();
    using dune::globals::soundPlayer;

    const auto placeItem = pBuilder->getCurrentProducedItem();
    if (placeItem == ItemID_Invalid) {
        // We lost a race with another team member
        currentGame->addToNewsTicker(_("There is no item to place."));
        dune::globals::soundPlayer->playSound(Sound_enum::Sound_InvalidAction); // can't place noise
        currentCursorMode = CursorMode_Normal;
        return true;
    }

    const auto structuresize = getStructureSize(placeItem);

    if (placeItem == Structure_Slab1) {
        if ((map_->isWithinBuildRange(xPos, yPos, pBuilder->getOwner()))
            && (map_->okayToPlaceStructure(xPos, yPos, 1, 1, false, pBuilder->getOwner()))
            && (!map_->getTile(xPos, yPos)->isConcrete())) {
            getCommandManager().addCommand(Command(dune::globals::pLocalPlayer->getPlayerID(),
                                                   CMDTYPE::CMD_PLACE_STRUCTURE,
                                                   pBuilder->getObjectID(),
                                                   xPos,
                                                   yPos));
            // the user has tried to place and has been successful
            soundPlayer->playSound(Sound_enum::Sound_PlaceStructure);
            currentCursorMode = CursorMode_Normal;
            return true;
        }
        // the user has tried to place but clicked on impossible point
        currentGame->addToNewsTicker(_("@DUNE.ENG|135#Cannot place slab here."));
        soundPlayer->playSound(Sound_enum::Sound_InvalidAction); // can't place noise
        return false;
    }
    if (placeItem == Structure_Slab4) {
        if ((map_->isWithinBuildRange(xPos, yPos, pBuilder->getOwner())
             || map_->isWithinBuildRange(xPos + 1, yPos, pBuilder->getOwner())
             || map_->isWithinBuildRange(xPos + 1, yPos + 1, pBuilder->getOwner())
             || map_->isWithinBuildRange(xPos, yPos + 1, pBuilder->getOwner()))
            && ((map_->okayToPlaceStructure(xPos, yPos, 1, 1, false, pBuilder->getOwner())
                 || map_->okayToPlaceStructure(xPos + 1, yPos, 1, 1, false, pBuilder->getOwner())
                 || map_->okayToPlaceStructure(xPos + 1, yPos + 1, 1, 1, false, pBuilder->getOwner())
                 || map_->okayToPlaceStructure(xPos, yPos, 1, 1 + 1, false, pBuilder->getOwner())))
            && ((!map_->getTile(xPos, yPos)->isConcrete()) || (!map_->getTile(xPos + 1, yPos)->isConcrete())
                || (!map_->getTile(xPos, yPos + 1)->isConcrete())
                || (!map_->getTile(xPos + 1, yPos + 1)->isConcrete()))) {

            getCommandManager().addCommand(Command(dune::globals::pLocalPlayer->getPlayerID(),
                                                   CMDTYPE::CMD_PLACE_STRUCTURE,
                                                   pBuilder->getObjectID(),
                                                   xPos,
                                                   yPos));
            // the user has tried to place and has been successful
            soundPlayer->playSound(Sound_enum::Sound_PlaceStructure);
            currentCursorMode = CursorMode_Normal;
            return true;
        }
        // the user has tried to place but clicked on impossible point
        currentGame->addToNewsTicker(_("@DUNE.ENG|135#Cannot place slab here."));
        soundPlayer->playSound(Sound_enum::Sound_InvalidAction); // can't place noise
        return false;
    }
    if (map_->okayToPlaceStructure(xPos, yPos, structuresize.x, structuresize.y, false, pBuilder->getOwner())) {
        getCommandManager().addCommand(Command(dune::globals::pLocalPlayer->getPlayerID(),
                                               CMDTYPE::CMD_PLACE_STRUCTURE,
                                               pBuilder->getObjectID(),
                                               xPos,
                                               yPos));
        // the user has tried to place and has been successful
        soundPlayer->playSound(Sound_enum::Sound_PlaceStructure);
        currentCursorMode = CursorMode_Normal;
        return true;
    }
    // the user has tried to place but clicked on impossible point
    currentGame->addToNewsTicker(fmt::sprintf(_("@DUNE.ENG|134#Cannot place %%s here."), resolveItemName(placeItem)));
    soundPlayer->playSound(Sound_enum::Sound_InvalidAction); // can't place noise

    // is this building area only blocked by units?
    if (map_->okayToPlaceStructure(xPos, yPos, structuresize.x, structuresize.y, false, pBuilder->getOwner(), true)) {
        // then we try to move all units outside the building area

        // generate a independent temporal random number generator as we are in input handling code (and outside
        // game logic code)

        auto& uiRandom = dune::globals::pGFXManager->random();

        for (int y = yPos; y < yPos + structuresize.y; y++) {
            for (int x = xPos; x < xPos + structuresize.x; x++) {
                const auto* const pTile = map_->getTile(x, y);
                if (pTile->hasANonInfantryGroundObject()) {
                    auto* const pObject = pTile->getNonInfantryGroundObject(objectManager_);
                    if (pObject && pObject->getOwner() == pBuilder->getOwner()) {
                        if (auto* pUnit = dune_cast<UnitBase>(pObject)) {
                            const auto newDestination = map_->findDeploySpot(
                                pUnit, Coord(xPos, yPos), uiRandom, pUnit->getLocation(), structuresize);
                            pUnit->handleMoveClick(context, newDestination.x, newDestination.y);
                        }
                    }
                } else if (pTile->hasInfantry()) {
                    for (const auto objectID : pTile->getInfantryList()) {
                        auto* pInfantry = getObjectManager().getObject<InfantryBase>(objectID);
                        if ((pInfantry != nullptr) && (pInfantry->getOwner() == pBuilder->getOwner())) {
                            const auto newDestination = map_->findDeploySpot(
                                pInfantry, Coord(xPos, yPos), uiRandom, pInfantry->getLocation(), structuresize);
                            pInfantry->handleMoveClick(context, newDestination.x, newDestination.y);
                        }
                    }
                }
            }
        }
    }

    return false;
}

bool Game::handleSelectedObjectsAttackClick(const GameContext& context, int xPos, int yPos) {
    UnitBase* pResponder = nullptr;
    for (const auto objectID : selectedList_) {
        auto* const pObject = objectManager_.getObject(objectID);
        if (!pObject)
            continue;

        const auto* const pOwner = pObject->getOwner();
        if (pObject->isAUnit() && (pOwner == dune::globals::pLocalHouse) && pObject->isRespondable()) {
            pResponder = dune_cast<UnitBase>(pObject);
            if (pResponder)
                pResponder->handleAttackClick(context, xPos, yPos);
        } else if ((pObject->getItemID() == Structure_Palace)
                   && ((pOwner->getHouseID() == HOUSETYPE::HOUSE_HARKONNEN)
                       || (pOwner->getHouseID() == HOUSETYPE::HOUSE_SARDAUKAR))) {
            if (auto* const pPalace = dune_cast<Palace>(pObject)) {
                if (pPalace->isSpecialWeaponReady()) {
                    pPalace->handleDeathhandClick(context, xPos, yPos);
                }
            }
        }
    }

    currentCursorMode = CursorMode_Normal;
    if (pResponder) {
        pResponder->playConfirmSound();
        return true;
    }

    return false;
}

bool Game::handleSelectedObjectsMoveClick(const GameContext& context, int xPos, int yPos) {
    UnitBase* pResponder = nullptr;

    for (const auto objectID : selectedList_) {
        auto* const pObject = objectManager_.getObject<UnitBase>(objectID);
        if (pObject && (pObject->getOwner() == dune::globals::pLocalHouse) && pObject->isRespondable()) {
            pResponder = pObject;
            pResponder->handleMoveClick(context, xPos, yPos);
        }
    }

    currentCursorMode = CursorMode_Normal;
    if (pResponder) {
        pResponder->playConfirmSound();
        return true;
    }

    return false;
}

/**
    New method for transporting units quickly using carryalls
**/
bool Game::handleSelectedObjectsRequestCarryallDropClick(const GameContext& context, int xPos, int yPos) {

    UnitBase* pResponder = nullptr;

    /*
        If manual carryall mode isn't enabled then turn this off...
    */
    if (!getGameInitSettings().getGameOptions().manualCarryallDrops) {
        currentCursorMode = CursorMode_Normal;
        return false;
    }

    for (const auto objectID : selectedList_) {
        auto* const pObject = objectManager_.getObject<UnitBase>(objectID);
        if (pObject && pObject->isAGroundUnit() && (pObject->getOwner() == dune::globals::pLocalHouse)
            && pObject->isRespondable()) {
            pResponder = pObject;
            pResponder->handleRequestCarryallDropClick(context, xPos, yPos);
        }
    }

    currentCursorMode = CursorMode_Normal;
    if (pResponder) {
        pResponder->playConfirmSound();
        return true;
    }

    return false;
}

bool Game::handleSelectedObjectsCaptureClick(const GameContext& context, int xPos, int yPos) {
    const auto* const pTile = map_->tryGetTile(xPos, yPos);

    if (pTile == nullptr) {
        return false;
    }

    const auto* const pStructure = pTile->getGroundObject<StructureBase>(objectManager_);
    if ((pStructure != nullptr) && (pStructure->canBeCaptured())
        && (pStructure->getOwner()->getTeamID() != dune::globals::pLocalHouse->getTeamID())) {
        InfantryBase* pResponder = nullptr;

        for (const auto objectID : selectedList_) {
            auto* const pObject = objectManager_.getObject<InfantryBase>(objectID);
            if (pObject && (pObject->getOwner() == dune::globals::pLocalHouse) && pObject->isRespondable()) {
                pResponder = pObject;
                pResponder->handleCaptureClick(context, xPos, yPos);
            }
        }

        currentCursorMode = CursorMode_Normal;
        if (pResponder) {
            pResponder->playConfirmSound();
            return true;
        }

        return false;
    }

    return false;
}

bool Game::handleSelectedObjectsActionClick(const GameContext& context, int xPos, int yPos) {
    // let unit handle right click on map or target
    ObjectBase* pResponder = nullptr;
    for (const auto objectID : selectedList_) {
        auto* const pObject = objectManager_.getObject(objectID);
        if (pObject && pObject->getOwner() == dune::globals::pLocalHouse && pObject->isRespondable()) {
            pObject->handleActionClick(context, xPos, yPos);

            // if this object obey the command
            if ((pResponder == nullptr) && pObject->isRespondable())
                pResponder = pObject;
        }
    }

    if (pResponder) {
        pResponder->playConfirmSound();
        return true;
    }

    return false;
}

bool Game::onRadarClick(const GameContext& context, Coord worldPosition, bool bRightMouseButton, bool bDrag) {
    if (bRightMouseButton) {

        if (handleSelectedObjectsActionClick(context, worldPosition.x / TILESIZE, worldPosition.y / TILESIZE)) {
            indicatorFrame_    = 0;
            indicatorPosition_ = worldPosition;
        }

        return false;
    }
    if (bDrag) {
        dune::globals::screenborder->setNewScreenCenter(worldPosition);
        return true;
    }
    switch (currentCursorMode) {
        case CursorMode_Attack: {
            handleSelectedObjectsAttackClick(context, worldPosition.x / TILESIZE, worldPosition.y / TILESIZE);
            return false;
        }

        case CursorMode_Move: {
            handleSelectedObjectsMoveClick(context, worldPosition.x / TILESIZE, worldPosition.y / TILESIZE);
            return false;
        }

        case CursorMode_Capture: {
            handleSelectedObjectsCaptureClick(context, worldPosition.x / TILESIZE, worldPosition.y / TILESIZE);
            return false;
        }

        case CursorMode_CarryallDrop: {
            handleSelectedObjectsRequestCarryallDropClick(
                context, worldPosition.x / TILESIZE, worldPosition.y / TILESIZE);
            return false;
        }

        case CursorMode_Normal:
        default: {
            dune::globals::screenborder->setNewScreenCenter(worldPosition);
            return true;
        }
    }
}

bool Game::isOnRadarView(int mouseX, int mouseY) const {
    return pInterface_->getRadarView().isOnRadar(mouseX - (sideBarPos_.x + SIDEBAR_COLUMN_WIDTH),
                                                 mouseY - sideBarPos_.y);
}
