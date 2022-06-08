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

#include <config.h>
#include <globals.h>
#include <main.h>

#include <FileClasses/FileManager.h>
#include <FileClasses/FontManager.h>
#include <FileClasses/GFXManager.h>
#include <FileClasses/LoadSavePNG.h>
#include <FileClasses/TextManager.h>
#include <FileClasses/music/MusicPlayer.h>
#include <SoundPlayer.h>
#include <misc/FileSystem.h>
#include <misc/IFileStream.h>
#include <misc/IMemoryStream.h>
#include <misc/OFileStream.h>
#include <misc/SDL2pp.h>
#include <misc/draw_util.h>
#include <misc/dune_events.h>
#include <misc/dune_timer_resolution.h>
#include <misc/exceptions.h>
#include <misc/fnkdat.h>
#include <misc/md5.h>
#include <misc/string_error.h>

#include <players/HumanPlayer.h>

#include <Network/NetworkManager.h>

#include <GUI/dune/InGameMenu.h>
#include <GUI/dune/WaitingForOtherPlayers.h>
#include <Menu/BriefingMenu.h>
#include <Menu/MapChoice.h>
#include <Menu/MentatHelp.h>

#include <Bullet.h>
#include <Explosion.h>
#include <GameInitSettings.h>
#include <House.h>
#include <Map.h>
#include <ScreenBorder.h>
#include <sand.h>

#include <structures/BuilderBase.h>
#include <structures/ConstructionYard.h>
#include <structures/Palace.h>
#include <structures/StructureBase.h>
#include <units/Harvester.h>
#include <units/InfantryBase.h>
#include <units/UnitBase.h>

#include <SDL2/SDL_render.h>

#include <fmt/format.h>

#include <algorithm>
#include <iomanip>
#include <sstream>

Game::Game() : localPlayerName_(dune::globals::settings.general.playerName) {
    dune::globals::currentZoomlevel = dune::globals::settings.video.preferredZoomLevel;

    dune::globals::unitList.clear();      // holds all the units
    dune::globals::structureList.clear(); // all the structures
    dune::globals::bulletList.clear();

    dune::globals::musicPlayer->changeMusic(MUSIC_PEACE);

    dune::globals::debug = false;

    resize();
}

/**
    The destructor frees up all the used memory.
*/
Game::~Game() {
    if (auto* const network_manager = dune::globals::pNetworkManager.get()) {
        network_manager->setOnReceiveChatMessage({});
        network_manager->setOnReceiveCommandList({});
        network_manager->setOnReceiveSelectionList({});
        network_manager->setOnPeerDisconnected({});
    }

    dune::globals::structureList.clear();

    dune::globals::unitList.clear();

    dune::globals::bulletList.clear();

    explosionList_.clear();

    dune::globals::currentGameMap = nullptr;
    map_.reset();

    dune::globals::screenborder.reset();
}

void Game::resize() {
    const auto* const gfx = dune::globals::pGFXManager.get();

    sideBarPos_ = calcAlignedDrawingRect(gfx->getUIGraphic(UI_SideBar), HAlign::Right, VAlign::Top);
    topBarPos_  = calcAlignedDrawingRect(gfx->getUIGraphic(UI_TopBar), HAlign::Left, VAlign::Top);

    const auto renderer_size   = getRendererSize();
    const auto renderer_width  = static_cast<float>(renderer_size.w);
    const auto renderer_height = static_cast<float>(renderer_size.h);

    powerIndicatorPos_.h = spiceIndicatorPos_.h = renderer_height - 146 - 2;

    //////////////////////////////////////////////////////////////////////////
    const SDL_FRect gameBoardRect{0, topBarPos_.h, sideBarPos_.x, renderer_height - topBarPos_.h};

    dune::globals::screenborder = std::make_unique<ScreenBorder>(gameBoardRect);

    if (map_)
        dune::globals::screenborder->adjustScreenBorderToMapsize(map_->getSizeX(), map_->getSizeY());

    if (pInterface_)
        pInterface_->resize(renderer_width, renderer_height);
}

void Game::initGame(const GameInitSettings& newGameInitSettings) {
    gameInitSettings_ = newGameInitSettings;

    switch (gameInitSettings_.getGameType()) {
        case GameType::LoadSavegame: {
            if (!loadSaveGame(gameInitSettings_.getFilename())) {
                THROW(std::runtime_error, "Loading save game failed!");
            }
        } break;

        case GameType::LoadMultiplayer: {
            IMemoryStream memStream(gameInitSettings_.getFiledata().data(), gameInitSettings_.getFiledata().size());

            if (!loadSaveGame(memStream)) {
                THROW(std::runtime_error, "Loading save game failed!");
            }
        } break;

        case GameType::Campaign:
        case GameType::Skirmish:
        case GameType::CustomGame:
        case GameType::CustomMultiplayer: {
            gameType = gameInitSettings_.getGameType();
            randomFactory.setSeed({gameInitSettings_.getRandomSeed()});

            randomGen = randomFactory.create("Game");

            objectData.loadFromINIFile("ObjectData.ini");

            if (gameInitSettings_.getMission() != 0) {
                techLevel = ((gameInitSettings_.getMission() + 1) / 3) + 1;
            }

            INIMapLoader loader{this, gameInitSettings_.getFilename(), gameInitSettings_.getFiledata()};

            map_                          = loader.load();
            dune::globals::currentGameMap = map_.get();

            if (!bReplay_ && gameInitSettings_.getGameType() != GameType::CustomGame
                && gameInitSettings_.getGameType() != GameType::CustomMultiplayer) {
                /* do briefing */
                sdl2::log_info("Briefing...");
                BriefingMenu(gameInitSettings_.getHouseID(), gameInitSettings_.getMission(), BRIEFING)
                    .showMenu(sdl_handler_);
            }
        } break;

        default: {
        } break;
    }
}

void Game::initReplay(const std::filesystem::path& filename) {
    bReplay_ = true;

    IFileStream fs;

    if (!fs.open(filename)) {
        THROW(io_error, "Error while opening '%s'!", filename.string());
    }

    // override local player name as it was when the replay was created
    localPlayerName_ = fs.readString();

    // read GameInitInfo
    const GameInitSettings loadedGameInitSettings(fs);

    // load all commands
    cmdManager_.load(fs);

    initGame(loadedGameInitSettings);
}

void Game::processObjects() {
    // update all tiles
    map_->for_all([](Tile& t) { t.update(); });

    const GameContext context{*this, *dune::globals::currentGameMap, objectManager_};

    for (auto* pStructure : dune::globals::structureList) {
        pStructure->update(context);
    }

    if ((currentCursorMode == CursorMode_Placing) && selectedList_.empty()) {
        currentCursorMode = CursorMode_Normal;
    }

    for (auto* pUnit : dune::globals::unitList) {
        pUnit->update(context);
    }

    auto selection_changed = false;

    map_->consume_removed_objects([&](uint32_t objectID) {
        auto* object = objectManager_.getObject(objectID);

        if (!object)
            return;

        if (removeFromSelectionLists(object))
            selection_changed = true;
    });

    objectManager_.consume_pending_deletes([&](auto& object) {
        object->cleanup(context, dune::globals::pLocalPlayer);

        if (removeFromSelectionLists(object.get()))
            selection_changed = true;

        removeFromQuickSelectionLists(object->getObjectID());
    });

    if (selection_changed)
        selectionChanged();

    std::erase_if(dune::globals::bulletList, [&](auto& b) { return b->update(context); });

    std::erase_if(explosionList_, [](auto& e) { return e->update(); });
}

void Game::drawScreen() {
    auto* const screenborder = dune::globals::screenborder.get();
    auto* const renderer     = dune::globals::renderer.get();

    const auto top_left     = screenborder->getTopLeftTile();
    const auto bottom_right = screenborder->getBottomRightTile();

    auto TopLeftTile     = top_left;
    auto BottomRightTile = bottom_right;

    // extend the view a little bit to avoid graphical glitches
    TopLeftTile.x     = std::max(0, TopLeftTile.x - 1);
    TopLeftTile.y     = std::max(0, TopLeftTile.y - 1);
    BottomRightTile.x = std::min(map_->getSizeX() - 1, BottomRightTile.x + 1);
    BottomRightTile.y = std::min(map_->getSizeY() - 1, BottomRightTile.y + 1);

    const auto x1 = TopLeftTile.x;
    const auto y1 = TopLeftTile.y;
    const auto x2 = BottomRightTile.x + 1;
    const auto y2 = BottomRightTile.y + 1;

    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);

    const auto zoomedTileSize = world2zoomedWorld(TILESIZE);
    const SDL_Rect tile_rect{static_cast<int>((std::ceil(screenborder->world2screenX(0)))),
                             static_cast<int>(std::ceil(screenborder->world2screenY(0))),
                             zoomedTileSize * map_->getSizeX(), zoomedTileSize * map_->getSizeY()};
    auto& board_rect = screenborder->getGameBoard();
    const SDL_Rect game_board_rect{static_cast<int>(std::ceil(board_rect.x)), static_cast<int>(std::ceil(board_rect.y)),
                                   static_cast<int>(std::floor(board_rect.w)),
                                   static_cast<int>(std::floor(board_rect.h))};
    SDL_Rect on_screen_rect;
    SDL_IntersectRect(&game_board_rect, &tile_rect, &on_screen_rect);

    SDL_RenderSetClipRect(renderer, &on_screen_rect);

    /* draw ground */
    map_->for_each(x1, y1, x2, y2, [&](Tile& t) { t.blitGround(this); });

    /* draw structures */
    map_->for_each(x1, y1, x2, y2, [&](Tile& t) { t.blitStructures(this); });

    /* draw underground units */
    map_->for_each(x1, y1, x2, y2, [&](Tile& t) { t.blitUndergroundUnits(this); });

    /* draw dead objects */
    map_->for_each(x1, y1, x2, y2, [&](Tile& t) { t.blitDeadUnits(this); });

    /* draw infantry */
    map_->for_each(x1, y1, x2, y2, [&](Tile& t) { t.blitInfantry(this); });

    /* draw non-infantry ground units */
    map_->for_each(x1, y1, x2, y2, [&](Tile& t) { t.blitNonInfantryGroundUnits(this); });

    /* draw bullets */
    for (const auto& pBullet : dune::globals::bulletList) {
        pBullet->blitToScreen(gameCycleCount_);
    }

    /* draw explosions */
    for (const auto& pExplosion : explosionList_) {
        pExplosion->blitToScreen();
    }

    /* draw air units */
    map_->for_each(x1, y1, x2, y2, [&](Tile& t) { t.blitAirUnits(this); });

    // draw the gathering point line if a structure is selected
    if (selectedList_.size() == 1) {
        auto* const pStructure = dynamic_cast<StructureBase*>(getObjectManager().getObject(*selectedList_.begin()));
        if (pStructure != nullptr) {
            pStructure->drawGatheringPointLine();
        }
    }

    /* draw selection rectangles */
    map_->for_each(x1, y1, x2, y2, [&](Tile& t) {
        if (dune::globals::debug || t.isExploredByTeam(this, dune::globals::pLocalHouse->getTeamID())) {
            t.blitSelectionRects(this);
        }
    });

    //////////////////////////////draw unexplored/shade

    const auto zoom = dune::globals::currentZoomlevel;

    auto* const gfx = dune::globals::pGFXManager.get();

    if (!dune::globals::debug) {
        auto* const hiddenTexZoomed    = gfx->getZoomedObjPic(ObjPic_Terrain_Hidden, zoom);
        auto* const hiddenFogTexZoomed = gfx->getZoomedObjPic(ObjPic_Terrain_HiddenFog, zoom);

        const auto fogOfWar = gameInitSettings_.getGameOptions().fogOfWar;

        map_->for_each(top_left.x - 1, top_left.y - 1, bottom_right.x + 1, bottom_right.y + 1, [&](Tile& t) {
            const auto x = t.getLocation().x;
            const auto y = t.getLocation().y;

            const auto* const pTile = &t;

            const auto team_id = dune::globals::pLocalHouse->getTeamID();

            const SDL_FRect drawLocation{screenborder->world2screenX(x * TILESIZE),
                                         screenborder->world2screenY(y * TILESIZE), static_cast<float>(zoomedTileSize),
                                         static_cast<float>(zoomedTileSize)};

            if (pTile->isExploredByTeam(this, team_id)) {
                const auto hideTile = t.getHideTile(this, team_id);

                if (hideTile != 0) {
                    const SDL_Rect source{hideTile * zoomedTileSize, 0, zoomedTileSize, zoomedTileSize};
                    Dune_RenderCopyF(renderer, hiddenTexZoomed, &source, &drawLocation);
                }

                if (fogOfWar) {
                    const auto fogTile = pTile->isFoggedByTeam(this, team_id)
                                           ? static_cast<int>(HIDDENTYPE::Terrain_HiddenFull)
                                           : pTile->getFogTile(this, team_id);

                    if (fogTile != 0) {
                        const SDL_Rect source{fogTile * zoomedTileSize, 0, zoomedTileSize, zoomedTileSize};
                        Dune_RenderCopyF(renderer, hiddenFogTexZoomed, &source, &drawLocation);
                    }
                }
            } else {
                if (!dune::globals::debug) {
                    const SDL_Rect source{zoomedTileSize * 15, 0, zoomedTileSize, zoomedTileSize};
                    Dune_RenderCopyF(renderer, hiddenTexZoomed, &source, &drawLocation);
                }
            }
        });
    }

    SDL_RenderSetClipRect(renderer, nullptr);

    /////////////draw placement position

    if (currentCursorMode == CursorMode_Placing) {
        // if user has selected to place a structure

        if (screenborder->isScreenCoordInsideMap(dune::globals::drawnMouseX, dune::globals::drawnMouseY)) {
            // if mouse is not over game bar

            const int xPos = screenborder->screen2MapX(dune::globals::drawnMouseX);
            const int yPos = screenborder->screen2MapY(dune::globals::drawnMouseY);

            if (selectedList_.size() == 1) {
                if (auto* pBuilder = dune_cast<BuilderBase>(objectManager_.getObject(*selectedList_.begin()))) {
                    const auto placeItem = pBuilder->getCurrentProducedItem();
                    Coord structureSize  = getStructureSize(placeItem);

                    bool withinRange = false;
                    for (int i = xPos; i < (xPos + structureSize.x); i++) {
                        for (int j = yPos; j < (yPos + structureSize.y); j++) {
                            if (map_->isWithinBuildRange(i, j, pBuilder->getOwner())) {
                                withinRange = true; // find out if the structure is close enough to other buildings
                                break;
                            }
                        }
                    }

                    const DuneTexture* validPlace   = nullptr;
                    const DuneTexture* invalidPlace = nullptr;

                    switch (zoom) {
                        case 0: {
                            validPlace   = gfx->getUIGraphic(UI_ValidPlace_Zoomlevel0);
                            invalidPlace = gfx->getUIGraphic(UI_InvalidPlace_Zoomlevel0);
                        } break;

                        case 1: {
                            validPlace   = gfx->getUIGraphic(UI_ValidPlace_Zoomlevel1);
                            invalidPlace = gfx->getUIGraphic(UI_InvalidPlace_Zoomlevel1);
                        } break;

                        case 2:
                        default: {
                            validPlace   = gfx->getUIGraphic(UI_ValidPlace_Zoomlevel2);
                            invalidPlace = gfx->getUIGraphic(UI_InvalidPlace_Zoomlevel2);
                        } break;
                    }

                    const auto is_slab = placeItem == Structure_Slab1 || placeItem == Structure_Slab4;

                    map_->for_each(xPos, yPos, xPos + structureSize.x, yPos + structureSize.y,
                                   [withinRange, invalidPlace, validPlace, is_slab,
                                    renderer     = dune::globals::renderer.get(),
                                    screenborder = dune::globals::screenborder.get()](auto& t) {
                                       const DuneTexture* image = nullptr;

                                       if (!withinRange || !t.isRock() || t.isMountain() || t.hasAGroundObject()
                                           || (is_slab && t.isConcrete())) {
                                           image = invalidPlace;
                                       } else {
                                           image = validPlace;
                                       }

                                       image->draw(renderer, screenborder->world2screenX(t.getLocation().x * TILESIZE),
                                                   screenborder->world2screenY(t.getLocation().y * TILESIZE));
                                   });
                }
            }
        }
    }

    ///////////draw game selection rectangle
    if (selectionMode_) {
        auto finalMouseX = static_cast<float>(dune::globals::drawnMouseX);
        auto finalMouseY = static_cast<float>(dune::globals::drawnMouseY);

        if (finalMouseX >= sideBarPos_.x) {
            // this keeps the box on the map, and not over game bar
            finalMouseX = sideBarPos_.x - 1;
        }

        if (finalMouseY < topBarPos_.y + topBarPos_.h) {
            finalMouseY = topBarPos_.x + topBarPos_.h;
        }

        const auto screen_x = screenborder->world2screenX(selectionRect_.x);
        const auto screen_y = screenborder->world2screenY(selectionRect_.y);

        // draw the mouse selection rectangle
        renderDrawRectF(renderer, screen_x, screen_y, finalMouseX, finalMouseY, COLOR_WHITE);
    }

    ///////////draw action indicator

    if ((indicatorFrame_ != NONE_ID) && (screenborder->isInsideScreen(indicatorPosition_, Coord(TILESIZE, TILESIZE)))) {
        const auto* const pUIIndicator = gfx->getUIGraphic(UI_Indicator);
        auto source                    = calcSpriteSourceRect(pUIIndicator, indicatorFrame_, 3);
        auto drawLocation = calcSpriteDrawingRectF(pUIIndicator, screenborder->world2screenX(indicatorPosition_.x),
                                                   screenborder->world2screenY(indicatorPosition_.y), 3, 1,
                                                   HAlign::Center, VAlign::Center);
        Dune_RenderCopyF(renderer, pUIIndicator, &source, &drawLocation);
    }

    ///////////draw game bar
    pInterface_->draw({});
    pInterface_->drawOverlay({});

    const auto& gui = GUIStyle::getInstance();

    const auto renderer_height = static_cast<float>(getRendererHeight());

    // draw chat message currently typed
    if (chatMode_) {
        const auto pChatTexture = gui.createText(
            renderer,
            "Chat: " + typingChatMessage_
                + (((dune::as_milliseconds(dune::dune_clock::now().time_since_epoch()) / 150) % 2 == 0) ? "_" : ""),
            COLOR_WHITE, 14);

        pChatTexture.draw(renderer, 20.f, renderer_height - 40.f);
    }

    if (bShowFPS_) {
        const auto str = fmt::sprintf("fps: %4.1f\nrenderer: %4.1fms\nupdate: %4.1fms", 1000.0f / averageFrameTime_,
                                      averageRenderTime_, averageUpdateTime_);

        auto pTexture = gui.createMultilineText(renderer, str, COLOR_WHITE, 14);

        pTexture.draw(renderer, sideBarPos_.x - 14.f * 8.f, 60.f);

        dune::defer_destroy_texture(std::move(pTexture));
    }

    if (bShowTime_) {
        const int seconds  = static_cast<int>(getGameTime()) / 1000;
        const auto strTime = fmt::sprintf(" %.2d:%.2d:%.2d", seconds / 3600, (seconds % 3600) / 60, (seconds % 60));

        auto pTimeTexture = gui.createText(renderer, strTime, COLOR_WHITE, 14);

        pTimeTexture.draw(renderer, 0.f, renderer_height - pTimeTexture.height_);

        dune::defer_destroy_texture(std::move(pTimeTexture));
    }

    if (bPause_) {
        SDL_SetRenderDrawColor(renderer, 0, 242, 0, 128);
        SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
        const auto rects = std::to_array<SDL_FRect>(
            {{10, renderer_height - 20 - 36, 12, 36}, {10 + 12 + 8, renderer_height - 20 - 36, 12, 36}});

        SDL_RenderFillRectsF(renderer, rects.data(), rects.size());
    } else if (gameCycleCount_ < skipToGameCycle_) {
        // Cache this texture...
        auto pTexture = gui.createText(renderer, ">>", COLOR_RGBA(0, 242, 0, 128), 48);

        pTexture.draw(renderer, 10.f, renderer_height - pTexture.height_ - 12);

        dune::defer_destroy_texture(std::move(pTexture));
    }

    if (finished_) {
        std::string message;

        if (won_) {
            message = _("You Have Completed Your Mission.");
        } else {
            message = _("You Have Failed Your Mission.");
        }

        auto pFinishMessageTexture = gui.createText(renderer, message, COLOR_WHITE, 28);

        const auto x = (sideBarPos_.x - pFinishMessageTexture.width_) / 2;
        const auto y = topBarPos_.h + (renderer_height - topBarPos_.h - pFinishMessageTexture.height_) / 2;

        pFinishMessageTexture.draw(renderer, x, y);

        dune::defer_destroy_texture(std::move(pFinishMessageTexture));
    }

    if (pWaitingForOtherPlayers_ != nullptr) {
        pWaitingForOtherPlayers_->draw();
    }

    if (pInGameMenu_ != nullptr) {
        pInGameMenu_->draw();
    } else if (pInGameMentat_ != nullptr) {
        pInGameMentat_->draw();
    }

    drawCursor(on_screen_rect);
}

void Game::doInput(const GameContext& context, SDL_Event& event) {
    // check for a key press

    // first of all update mouse
    if (event.type == SDL_MOUSEMOTION) {
        const SDL_MouseMotionEvent* mouse = &event.motion;

        const auto& video = dune::globals::settings.video;

        dune::globals::drawnMouseX = std::max(0, std::min(mouse->x, video.width - 1));
        dune::globals::drawnMouseY = std::max(0, std::min(mouse->y, video.height - 1));
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

            case SDL_KEYDOWN: {
                if (chatMode_) {
                    handleChatInput(context, event.key);
                } else {
                    handleKeyInput(context, event.key);
                }
            } break;

            case SDL_TEXTINPUT: {
                if (chatMode_) {
                    const auto* const newText = event.text.text;
                    if (utf8Length(typingChatMessage_) + utf8Length(newText) <= 60) {
                        typingChatMessage_ += newText;
                    }
                }
            } break;

            case SDL_MOUSEWHEEL: {
                if (event.wheel.y != 0) {
                    pInterface_->handleMouseWheel(dune::globals::drawnMouseX, dune::globals::drawnMouseY,
                                                  (event.wheel.y > 0));
                }
            } break;

            case SDL_MOUSEBUTTONDOWN: {
                const auto* const mouse = &event.button;

                switch (mouse->button) {
                    case SDL_BUTTON_LEFT: {
                        pInterface_->handleMouseLeft(mouse->x, mouse->y, true);
                    } break;

                    case SDL_BUTTON_RIGHT: {
                        pInterface_->handleMouseRight(mouse->x, mouse->y, true);
                    } break;

                    default: break;
                }

                auto* const screenborder = dune::globals::screenborder.get();

                const auto mouseX = static_cast<float>(mouse->x);
                const auto mouseY = static_cast<float>(mouse->y);

                switch (mouse->button) {

                    case SDL_BUTTON_LEFT: {

                        switch (currentCursorMode) {

                            case CursorMode_Placing: {
                                if (screenborder->isScreenCoordInsideMap(mouseX, mouseY)) {
                                    handlePlacementClick(context, screenborder->screen2MapX(mouseX),
                                                         screenborder->screen2MapY(mouseY));
                                }
                            } break;

                            case CursorMode_Attack: {

                                if (screenborder->isScreenCoordInsideMap(mouseX, mouseY)) {
                                    handleSelectedObjectsAttackClick(context, screenborder->screen2MapX(mouseX),
                                                                     screenborder->screen2MapY(mouseY));
                                }

                            } break;

                            case CursorMode_Move: {

                                if (screenborder->isScreenCoordInsideMap(mouseX, mouseY)) {
                                    handleSelectedObjectsMoveClick(context, screenborder->screen2MapX(mouseX),
                                                                   screenborder->screen2MapY(mouseY));
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
                                    handleSelectedObjectsCaptureClick(context, screenborder->screen2MapX(mouseX),
                                                                      screenborder->screen2MapY(mouseY));
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
                        } else if ((!selectedList_.empty()
                                    && (((objectManager_.getObject(*selectedList_.begin()))->getOwner()
                                         == dune::globals::pLocalHouse))
                                    && (((objectManager_.getObject(*selectedList_.begin()))->isRespondable())))) {
                            // if user has a controllable unit selected

                            if (screenborder->isScreenCoordInsideMap(mouse->x, mouse->y)) {
                                if (handleSelectedObjectsActionClick(context, screenborder->screen2MapX(mouse->x),
                                                                     screenborder->screen2MapY(mouse->y))) {
                                    indicatorFrame_      = 0;
                                    indicatorPosition_.x = screenborder->screen2worldX(mouse->x);
                                    indicatorPosition_.y = screenborder->screen2worldY(mouse->y);
                                }
                            }
                        }
                    } break; // end of SDL_BUTTON_RIGHT
                    default: break;
                }
            } break;

            case SDL_MOUSEMOTION: {
                const auto* const mouse = &event.motion;

                pInterface_->handleMouseMovement(mouse->x, mouse->y);
            } break;

            case SDL_MOUSEBUTTONUP: {
                const auto* const mouse = &event.button;

                switch (mouse->button) {
                    case SDL_BUTTON_LEFT: {
                        pInterface_->handleMouseLeft(mouse->x, mouse->y, false);
                    } break;

                    case SDL_BUTTON_RIGHT: {
                        pInterface_->handleMouseRight(mouse->x, mouse->y, false);
                    } break;
                    default: break;
                }

                if (selectionMode_ && (mouse->button == SDL_BUTTON_LEFT)) {
                    // this keeps the box on the map, and not over game bar
                    auto finalMouseX = static_cast<float>(mouse->x);
                    auto finalMouseY = static_cast<float>(mouse->y);

                    if (finalMouseX >= sideBarPos_.x) {
                        finalMouseX = sideBarPos_.x - 1;
                    }

                    if (finalMouseY < topBarPos_.y + topBarPos_.h) {
                        finalMouseY = topBarPos_.x + topBarPos_.h;
                    }

                    auto* const screenborder = dune::globals::screenborder.get();

                    int rectFinishX = screenborder->screen2MapX(finalMouseX);
                    if (rectFinishX > (map_->getSizeX() - 1)) {
                        rectFinishX = map_->getSizeX() - 1;
                    }

                    const int rectFinishY = screenborder->screen2MapY(finalMouseY);

                    // convert start also to map coordinates
                    const int rectStartX = selectionRect_.x / TILESIZE;
                    const int rectStartY = selectionRect_.y / TILESIZE;

                    map_->selectObjects(dune::globals::pLocalHouse, rectStartX, rectStartY, rectFinishX, rectFinishY,
                                        screenborder->screen2worldX(finalMouseX),
                                        screenborder->screen2worldY(finalMouseY), SDL_GetModState() & KMOD_SHIFT);

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

            case SDL_WINDOWEVENT: {
                switch (event.window.event) {
                    case SDL_WINDOWEVENT_SIZE_CHANGED: {
                        auto& gui = GUIStyle::getInstance();

                        gui.setLogicalSize(dune::globals::renderer.get(), event.window.data1, event.window.data2);

                        resize();
                    } break;

                    default: break;
                }
            } break;
            case SDL_QUIT: {
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

void Game::scrollViewport() {
    using namespace std::chrono_literals;

    if (mapVerticalScroll_ || mapHorizontalScroll_) {
        static constexpr auto scrollInterval = 60ms; // TODO: This might possibly go into an ingame menu setting.
        const auto now                       = dune::dune_clock::now();

        if (now > lastScrollTime_ + scrollInterval || lastScrollTime_ > now) {
            auto* const screenborder = dune::globals::screenborder.get();

            // N.b. Currently the game implements a "quantized" scrolling mode that is faithful to the original Dune 2
            // game, i.e. the viewport will scroll by ~1 grid unit jumps, rather than in a smooth pixel perfect fashion.
            // If smooth scrolling were to be desired, the above scrollInterval timer could be removed and instead the
            // following functions could be adjusted to scroll the viewport by much smaller increments, resulting in a
            // more modern style scrolling.
            for (int i = 0; i < mapVerticalScroll_; ++i)
                if (!screenborder->scrollDown())
                    mapVerticalScroll_ = 0;
            for (int i = 0; i > mapVerticalScroll_; --i)
                if (!screenborder->scrollUp())
                    mapVerticalScroll_ = 0;

            for (int i = 0; i < mapHorizontalScroll_; ++i)
                if (!screenborder->scrollRight())
                    mapHorizontalScroll_ = 0;
            for (int i = 0; i > mapHorizontalScroll_; --i)
                if (!screenborder->scrollLeft())
                    mapHorizontalScroll_ = 0;

            lastScrollTime_ = now;
        }
    }
}

void Game::drawCursor(const SDL_Rect& map_rect) const {
    if (!(SDL_GetWindowFlags(dune::globals::window.get()) & SDL_WINDOW_MOUSE_FOCUS)) {
        return;
    }

    const auto* const gfx = dune::globals::pGFXManager.get();

    SDL_Cursor* hardware_cursor = nullptr;
    const DuneTexture* pCursor  = nullptr;
    SDL_FRect dest{};

    if (mapHorizontalScroll_ < 0) {
        hardware_cursor = gfx->getCursor(UI_CursorLeft);
    } else if (mapHorizontalScroll_ > 0) {
        hardware_cursor = gfx->getCursor(UI_CursorRight);
    } else if (mapVerticalScroll_ < 0) {
        hardware_cursor = gfx->getCursor(UI_CursorUp);
    } else if (mapVerticalScroll_ > 0) {
        hardware_cursor = gfx->getCursor(UI_CursorDown);
    } else {
        const SDL_Point mouse_point{dune::globals::drawnMouseX, dune::globals::drawnMouseY};
        if ((pInGameMenu_ != nullptr) || (pInGameMentat_ != nullptr) || (pWaitingForOtherPlayers_ != nullptr)
            || ((!SDL_PointInRect(&mouse_point, &map_rect))
                && (!isOnRadarView(dune::globals::drawnMouseX, dune::globals::drawnMouseY)))) {
            // Menu mode or Mentat Menu or Waiting for other players or outside of game screen but not inside minimap
            hardware_cursor = gfx->getCursor(UI_CursorNormal);
        } else {

            switch (currentCursorMode) {
                case CursorMode_Normal:
                case CursorMode_Placing: {
                    hardware_cursor = gfx->getCursor(UI_CursorNormal);
                } break;

                case CursorMode_Move: {
                    const auto scale = GUIStyle::getInstance().getActualScale();

                    if (scale >= 2)
                        hardware_cursor = gfx->getCursor(UI_CursorMove_Zoomlevel2);
                    else if (scale > 1)
                        hardware_cursor = gfx->getCursor(UI_CursorMove_Zoomlevel1);
                    else
                        hardware_cursor = gfx->getCursor(UI_CursorMove_Zoomlevel0);
                } break;

                case CursorMode_Attack: {
                    const auto scale = GUIStyle::getInstance().getActualScale();

                    if (scale >= 2)
                        hardware_cursor = gfx->getCursor(UI_CursorAttack_Zoomlevel2);
                    else if (scale > 1)
                        hardware_cursor = gfx->getCursor(UI_CursorAttack_Zoomlevel1);
                    else
                        hardware_cursor = gfx->getCursor(UI_CursorAttack_Zoomlevel0);
                } break;

                case CursorMode_Capture: {
                    switch (dune::globals::currentZoomlevel) {
                        case 0: pCursor = gfx->getUIGraphic(UI_CursorCapture_Zoomlevel0); break;
                        case 1: pCursor = gfx->getUIGraphic(UI_CursorCapture_Zoomlevel1); break;
                        case 2:
                        default: pCursor = gfx->getUIGraphic(UI_CursorCapture_Zoomlevel2); break;
                    }

                    dest = calcDrawingRect(pCursor, dune::globals::drawnMouseX, dune::globals::drawnMouseY,
                                           HAlign::Center, VAlign::Bottom);

                    int xPos = INVALID_POS;
                    int yPos = INVALID_POS;

                    const auto* const screenborder = dune::globals::screenborder.get();

                    if (screenborder->isScreenCoordInsideMap(dune::globals::drawnMouseX, dune::globals::drawnMouseY)) {
                        xPos = screenborder->screen2MapX(dune::globals::drawnMouseX);
                        yPos = screenborder->screen2MapY(dune::globals::drawnMouseY);
                    } else if (isOnRadarView(dune::globals::drawnMouseX, dune::globals::drawnMouseY)) {
                        const auto position = pInterface_->getRadarView().getWorldCoords(
                            dune::globals::drawnMouseX - (sideBarPos_.x + SIDEBAR_COLUMN_WIDTH),
                            dune::globals::drawnMouseY - sideBarPos_.y);

                        xPos = position.x / TILESIZE;
                        yPos = position.y / TILESIZE;
                    }

                    if ((xPos != INVALID_POS) && (yPos != INVALID_POS)) {
                        const auto* const pTile = map_->getTile(xPos, yPos);

                        const auto team_id = dune::globals::pLocalHouse->getTeamID();

                        if (pTile->isExploredByTeam(this, team_id)) {

                            if (const auto* const pStructure = pTile->getGroundObject<StructureBase>(objectManager_)) {
                                if (pStructure->canBeCaptured() && pStructure->getOwner()->getTeamID() != team_id) {
                                    dest.y += static_cast<int>(getGameCycleCount() / 10) % 5;
                                }
                            }
                        }
                    }

                } break;

                case CursorMode_CarryallDrop: {
                    const auto scale = GUIStyle::getInstance().getActualScale();

                    if (scale >= 2)
                        hardware_cursor = gfx->getCursor(UI_CursorCarryallDrop_Zoomlevel2);
                    else if (scale > 1)
                        hardware_cursor = gfx->getCursor(UI_CursorCarryallDrop_Zoomlevel1);
                    else
                        hardware_cursor = gfx->getCursor(UI_CursorCarryallDrop_Zoomlevel0);
                } break;

                default: {
                    THROW(std::runtime_error, "Game::drawCursor(): Unknown cursor mode");
                }
            }
        }
    }

    if (pCursor) {
        SDL_ShowCursor(SDL_DISABLE);

        pCursor->draw(dune::globals::renderer.get(), dest.x, dest.y);
    } else {
        if (!hardware_cursor)
            hardware_cursor = gfx->getDefaultCursor();

        if (hardware_cursor != SDL_GetCursor())
            SDL_SetCursor(hardware_cursor);

        SDL_ShowCursor(SDL_ENABLE);
    }
}

void Game::setupView(const GameContext& context) const {
    // setup start location/view

    int i     = 0;
    int j     = 0;
    int count = 0;

    const auto* const house = dune::globals::pLocalHouse;

    for (const auto* pUnit : dune::globals::unitList) {
        if ((pUnit->getOwner() == house) && (pUnit->getItemID() != Unit_Sandworm)) {
            i += pUnit->getX();
            j += pUnit->getY();
            count++;
        }
    }

    for (const auto* pStructure : dune::globals::structureList) {
        if (pStructure->getOwner() == house) {
            i += pStructure->getX();
            j += pStructure->getY();
            count++;
        }
    }

    if (count == 0) {
        i = context.map.getSizeX() * TILESIZE / 2 - 1;
        j = context.map.getSizeY() * TILESIZE / 2 - 1;
    } else {
        i = i * TILESIZE / count;
        j = j * TILESIZE / count;
    }

    dune::globals::screenborder->setNewScreenCenter(Coord(i, j));
}

void Game::serviceNetwork(bool& bWaitForNetwork) {
    auto* const network_manager = dune::globals::pNetworkManager.get();

    network_manager->update();

    // test if we need to wait for data to arrive
    for (const auto& playername : network_manager->getConnectedPeers()) {
        const auto* const pPlayer = dynamic_cast<HumanPlayer*>(getPlayerByName(playername));
        if (pPlayer != nullptr) {
            if (pPlayer->nextExpectedCommandsCycle <= gameCycleCount_) {
                // sdl2::log_info("Cycle %d: Waiting for player '%s' to send data for cycle %d...", GameCycleCount,
                // pPlayer->getPlayername(), pPlayer->nextExpectedCommandsCycle);
                bWaitForNetwork = true;
                break;
            }
        }
    }

    if (bWaitForNetwork) {
        if (startWaitingForOtherPlayersTime_ == dune::dune_clock::time_point{}) {
            // we just started waiting
            startWaitingForOtherPlayersTime_ = dune::dune_clock::now();
        } else {
            using namespace std::chrono_literals;

            if (dune::dune_clock::now() - startWaitingForOtherPlayersTime_ > 1000ms) {
                // we waited for more than one second

                if (pWaitingForOtherPlayers_ == nullptr) {
                    pWaitingForOtherPlayers_ = std::make_unique<WaitingForOtherPlayers>();
                    bMenu_                   = true;
                }
            }
        }

        SDL_Delay(10);
    } else {
        startWaitingForOtherPlayersTime_ = dune::dune_clock::time_point{};
        pWaitingForOtherPlayers_.reset();
    }
}

void Game::updateGame(const GameContext& context) {
    pInterface_->getRadarView().update();
    cmdManager_.executeCommands(context, gameCycleCount_);

    // sdl2::log_info("cycle %d : %d", gameCycleCount, context.game.randomGen.getSeed());

#ifdef TEST_SYNC
    // add every gamecycles one test sync command
    if (bReplay == false) {
        cmdManager.addCommand(Command(pLocalPlayer->getPlayerID(), CMD_TEST_SYNC, randomGen.getSeed()));
    }
#endif

    std::ranges::for_each(house_, [](auto& h) {
        if (h)
            h->update();
    });

    dune::globals::screenborder->update(dune::globals::pGFXManager->random());

    triggerManager_.trigger(context, gameCycleCount_);

    processObjects();

    if ((indicatorFrame_ != NONE_ID) && (--indicatorTimer_ <= 0)) {
        indicatorTimer_ = indicatorTime_;

        if (++indicatorFrame_ > 2) {
            indicatorFrame_ = NONE_ID;
        }
    }

    gameCycleCount_++;
}

void Game::doEventsUntil(const GameContext& context, const dune::dune_clock::time_point until) {
    using namespace std::chrono_literals;

    SDL_Event event{};

    while (!bQuitGame_ && !finishedLevel_) {
        const auto remaining = until - dune::dune_clock::now();

        if (remaining <= dune::dune_clock::duration::zero() || remaining >= 100ms)
            return;

        const auto timeout = dune::as_milliseconds<int>(remaining);

        if (timeout < 1)
            return;

        if (dune::Dune_WaitEvent(&event, timeout)) {
            doInput(context, event);

            while (SDL_PollEvent(&event)) {
                doInput(context, event);
            }
        }
    }
}

void Game::runMainLoop(const GameContext& context, MenuBase::event_handler_type handler) {
    using namespace std::chrono_literals;

    sdl2::log_info("Starting game...");

    sdl_handler_         = handler;
    auto cleanup_handler = gsl::finally([&] { sdl_handler_ = {}; });

    // add interface
    if (pInterface_ == nullptr) {
        pInterface_ = std::make_unique<GameInterface>(context);

        const auto has_radar_on = dune::globals::pLocalHouse->hasRadarOn();

        if (gameState == GameState::Loading) {
            // when loading a save game we set radar directly
            pInterface_->getRadarView().setRadarMode(has_radar_on);
        } else if (has_radar_on) {
            // when starting a new game we switch the radar on with an animation if appropriate
            pInterface_->getRadarView().switchRadarMode(true);
        }
    }

    sdl2::log_info("Sizes: Tile %d UnitBase %d StructureBase %d Harvester %d ConstructionYard %d Palace %d",
                   sizeof(Tile), sizeof(UnitBase), sizeof(StructureBase), sizeof(Harvester), sizeof(ConstructionYard),
                   sizeof(Palace));

    gameState = GameState::Running;

    // setup endlevel conditions
    finishedLevel_ = false;

    bShowTime_ = winFlags & WINLOSEFLAGS_TIMEOUT;

    // Check if a player has lost
    std::ranges::for_each(house_, [](auto& h) {
        if (h && !h->isAlive())
            h->lose(true);
    });

    if (bReplay_) {
        cmdManager_.setReadOnly(true);
    } else {
        auto pStream = std::make_unique<OFileStream>();

        const auto [ok, replayname] = fnkdat("replay/auto.rpl", FNKDAT_USER | FNKDAT_CREAT);

        auto isOpen = ok && pStream->open(replayname);

        if (!isOpen) {
            const std::error_code replay_error{errno, std::generic_category()};

            sdl2::log_error(SDL_LOG_CATEGORY_APPLICATION, "Unable to open the default replay file: %s  Retrying...",
                            replay_error.message());

            auto& uiRandom = dune::globals::pGFXManager->random();

            for (auto i = 0; i < 10; ++i) {
                const auto [ok2, replayname2] =
                    fnkdat(fmt::format("replay/auto-{}.rpl", uiRandom.rand()), FNKDAT_USER | FNKDAT_CREAT);

                if (pStream->open(replayname2)) {
                    isOpen = true;
                    break;
                }

                const std::error_code replay2_error{errno, std::generic_category()};

                sdl2::log_error(SDL_LOG_CATEGORY_APPLICATION, "Unable to open the replay file %s: %s",
                                reinterpret_cast<const char*>(replayname2.filename().u8string().c_str()),
                                replay2_error.message());
            }
        }

        if (isOpen) {
            pStream->writeString(getLocalPlayerName());

            gameInitSettings_.save(*pStream);

            // when this game was loaded we have to save the old commands to the replay file first
            cmdManager_.save(*pStream);

            // flush stream
            pStream->flush();

            // now all new commands might be added
            cmdManager_.setStream(std::move(pStream));
        } else {
            // This can happen if another instance of the game is running or if the disk is full.
            // TODO: Report problem to user...?
            sdl2::log_error(SDL_LOG_CATEGORY_APPLICATION, "Unable to open the replay log file.");

            SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, std::string{_("Replay Log")}.c_str(),
                                     std::string{_("Unable to open the replay log file.")}.c_str(),
                                     dune::globals::window.get());

            quitGame();
        }
    }

    auto* const network_manager = dune::globals::pNetworkManager.get();
    if (network_manager) {
        network_manager->setOnReceiveChatMessage(
            [cm = &pInterface_->getChatManager()](const auto& username, const auto& message) {
                cm->addChatMessage(username, message);
            });
        network_manager->setOnReceiveCommandList([cm = &cmdManager_](const auto& playername, const auto& commands) {
            cm->addCommandList(playername, commands);
        });
        network_manager->setOnReceiveSelectionList(
            [this](const auto& name, const auto& newSelectionList, auto groupListIndex) {
                this->onReceiveSelectionList(name, newSelectionList, groupListIndex);
            });
        network_manager->setOnPeerDisconnected(
            [this](const auto& name, auto bHost, auto cause) { onPeerDisconnected(name, bHost, cause); });

        cmdManager_.setNetworkCycleBuffer(MILLI2CYCLES(network_manager->getMaxPeerRoundTripTime()) + 5);
    }

    // Change music to ingame music
    dune::globals::musicPlayer->changeMusic(MUSIC_PEACE);

    const auto gameStart = dune::dune_clock::now();

    int numFrames = 0;

    auto targetGameCycle = gameCycleCount_;

    lastTargetGameCycleTime_ = gameStart;

    auto previousFrameStart = gameStart;

    SDL_Event event;

    // sdl2::log_info("Random Seed (GameCycle %d): 0x%0X", GameCycleCount, RandomGen.getSeed());

    dune::DuneTimerResolution timer_handle;

    auto* const renderer = dune::globals::renderer.get();

    // main game loop
    do {
        auto now = dune::dune_clock::now();

        const auto frameStart = now;

        const auto frameElapsed = frameStart - previousFrameStart;

        previousFrameStart = frameStart;

        if (bShowFPS_) {
            averageFrameTime_ = 0.97f * averageFrameTime_ + 0.03f * dune::as_milliseconds<float>(frameElapsed);
        }

        if (finished_ && !bPause_) {
            // end timer for the ending message
            if (now - finishedLevelTime_ > END_WAIT_TIME) {
                finishedLevel_ = true;
            }
        }

        const auto renderStart = dune::dune_clock::now();

        // clear whole screen
        SDL_SetRenderDrawColor(renderer, 100, 50, 0, 255);
        SDL_RenderClear(renderer);

        drawScreen();

        // Apparently this must be done after drawing, but before render present.
        // https://discourse.libsdl.org/t/sdl-renderreadpixels-always-returns-black-rectangle/20371/6
        if (pendingScreenshot_)
            saveScreenshot();

        Dune_RenderPresent(renderer);

        updateFullscreen();

        const auto renderElapsed = dune::dune_clock::now() - renderStart;

        if (bShowFPS_) {
            averageRenderTime_ = 0.97f * (averageRenderTime_) + 0.03f * dune::as_milliseconds<float>(renderElapsed);
        }

        numFrames++;

        const auto gameSpeed = getGameSpeed();

        bool bWaitForNetwork = false;

        if (network_manager != nullptr) {
            serviceNetwork(bWaitForNetwork);
        }

        while (SDL_PollEvent(&event))
            doInput(context, event);

        pInterface_->updateObjectInterface();

        if (network_manager != nullptr) {
            if (bSelectionChanged_) {
                network_manager->sendSelectedList(selectedList_);

                bSelectionChanged_ = false;
            }
        }

        if (pInGameMentat_ != nullptr) {
            pInGameMentat_->update();
        }

        if (pWaitingForOtherPlayers_ != nullptr) {
            pWaitingForOtherPlayers_->update();
        }

        cmdManager_.update();

        while (!bWaitForNetwork && !bPause_) {
            now = dune::dune_clock::now();

            if (gameCycleCount_ < skipToGameCycle_) {
                targetGameCycle          = skipToGameCycle_;
                lastTargetGameCycleTime_ = now;
            } else {
                auto pendingTicks = now - lastTargetGameCycleTime_;

                // Watch for discontinuities...
                if (pendingTicks > 2500ms) {
                    pendingTicks             = 2 * gameSpeed;
                    lastTargetGameCycleTime_ = now - pendingTicks;
                }

                while (pendingTicks >= gameSpeed) {
                    pendingTicks -= gameSpeed;
                    lastTargetGameCycleTime_ += gameSpeed;
                    ++targetGameCycle;
                }
            }

            if (gameCycleCount_ >= targetGameCycle)
                break;

            // Reset in case of some massive discontinuity (e.g., computer sleep or debugger break).
            if (targetGameCycle - gameCycleCount_ > 250)
                targetGameCycle = gameCycleCount_;

            const auto updateStart = dune::dune_clock::now();

            updateGame(context);

            const auto updateElapsed = dune::dune_clock::now() - updateStart;

            if (bShowFPS_) {
                averageUpdateTime_ = 0.97f * averageUpdateTime_ + 0.03f * dune::as_milliseconds<float>(updateElapsed);
            }

            if (takePeriodicScreenshots_ && ((gameCycleCount_ % (MILLI2CYCLES(10 * 1000))) == 0)) {
                takeScreenshot();
            }

            now = dune::dune_clock::now();
            // Don't block the UI for more than 75ms, even if we are behind.
            if (now - frameStart > 75ms)
                break;
        }

        while (SDL_PollEvent(&event))
            doInput(context, event);

        // Process ingame map scrolling.
        scrollViewport();

        dune::globals::musicPlayer->musicCheck(); // if song has finished, start playing next one

        if (bWaitForNetwork || bPause_ || gameCycleCount_ >= skipToGameCycle_) {
            const auto until = (dune::globals::settings.video.frameLimit ? 16ms : 5ms) + frameStart;

            doEventsUntil(context, until);

            const auto stop = dune::dune_clock::now();

            if (stop > until + 50ms) {
                static auto count = 0;
                ++count;
            }
        }
    } while (!bQuitGame_ && !finishedLevel_); // not sure if we need this extra bool

    // Game is finished

    if (!bReplay_ && context.game.won_) {
        // save replay

        auto mapnameBase = getBasename(gameInitSettings_.getFilename(), true);
        mapnameBase += ".rpl";
        const auto rplName    = std::filesystem::path{"replay"} / mapnameBase;
        auto [ok, replayname] = fnkdat(rplName, FNKDAT_USER | FNKDAT_CREAT);

        OFileStream replystream;
        replystream.open(replayname);
        replystream.writeString(getLocalPlayerName());
        gameInitSettings_.save(replystream);
        cmdManager_.save(replystream);
    }

    if (network_manager != nullptr) {
        network_manager->disconnect();
    }

    gameState = GameState::Deinitialize;
    sdl2::log_info("Game finished!");
}

void Game::pauseGame() {
    if (gameType != GameType::CustomMultiplayer) {
        bPause_        = true;
        pauseGameTime_ = dune::dune_clock::now();
    }
}

void Game::resumeGame() {
    bMenu_ = false;
    if (gameType != GameType::CustomMultiplayer) {
        bPause_ = false;

        // Remove the time spent paused from the targetGameCycle update.
        const auto now       = dune::dune_clock::now();
        const auto pauseTime = now - pauseGameTime_;
        if (pauseTime > dune::dune_clock::duration::zero())
            lastTargetGameCycleTime_ += pauseTime;
        else
            lastTargetGameCycleTime_ = now;
    }
}

void Game::onOptions() {
    if (bReplay_) {
        // don't show menu
        quitGame();
    } else {
        const auto color = SDL2RGB(
            dune::globals::palette
                [dune::globals::houseToPaletteIndex[static_cast<int>(dune::globals::pLocalHouse->getHouseID())] + 3]);
        pInGameMenu_ = std::make_unique<InGameMenu>((gameType == GameType::CustomMultiplayer), color);
        bMenu_       = true;
        pauseGame();
    }
}

void Game::onMentat() {
    pInGameMentat_ = std::make_unique<MentatHelp>(dune::globals::pLocalHouse->getHouseID(), techLevel,
                                                  gameInitSettings_.getMission());
    bMenu_         = true;
    pauseGame();
}

GameInitSettings Game::getNextGameInitSettings() {
    if (nextGameInitSettings_.getGameType() != GameType::Invalid) {
        // return the prepared game init settings (load game or restart mission)
        return nextGameInitSettings_;
    }

    switch (gameInitSettings_.getGameType()) {
        case GameType::Campaign: {
            int currentMission = gameInitSettings_.getMission();
            if (!won_) {
                currentMission -= (currentMission >= 22) ? 1 : 3;
            }
            int nextMission               = gameInitSettings_.getMission();
            uint32_t alreadyPlayedRegions = gameInitSettings_.getAlreadyPlayedRegions();
            if (currentMission >= -1) {
                // do map choice
                sdl2::log_info("Map Choice...");
                MapChoice mapChoice(gameInitSettings_.getHouseID(), currentMission, alreadyPlayedRegions);
                mapChoice.showMenu(sdl_handler_);
                nextMission          = mapChoice.getSelectedMission();
                alreadyPlayedRegions = mapChoice.getAlreadyPlayedRegions();
            }

            const uint32_t alreadyShownTutorialHints = won_
                                                         ? dune::globals::pLocalPlayer->getAlreadyShownTutorialHints()
                                                         : gameInitSettings_.getAlreadyShownTutorialHints();
            return GameInitSettings(gameInitSettings_, nextMission, alreadyPlayedRegions, alreadyShownTutorialHints);
        }

        default: {
            sdl2::log_info("Game::getNextGameInitClass(): Wrong gameType for next Game.");
            return {};
        }
    }
}

int Game::whatNext() {
    if (whatNextParam_ != GAME_NOTHING) {
        const int tmp  = whatNextParam_;
        whatNextParam_ = GAME_NOTHING;
        return tmp;
    }

    if (nextGameInitSettings_.getGameType() != GameType::Invalid) {
        return GAME_LOAD;
    }

    switch (gameType) {
        case GameType::Campaign: {
            if (bQuitGame_) {
                return GAME_RETURN_TO_MENU;
            }
            if (won_) {
                if (gameInitSettings_.getMission() == 22) {
                    // there is no mission after this mission
                    whatNextParam_ = GAME_RETURN_TO_MENU;
                } else {
                    // there is a mission after this mission
                    whatNextParam_ = GAME_NEXTMISSION;
                }
                return GAME_DEBRIEFING_WIN;
            }
            // we need to play this mission again
            whatNextParam_ = GAME_NEXTMISSION;

            return GAME_DEBRIEFING_LOST;
        }

        case GameType::Skirmish: {
            if (bQuitGame_) {
                return GAME_RETURN_TO_MENU;
            }
            if (won_) {
                whatNextParam_ = GAME_RETURN_TO_MENU;
                return GAME_DEBRIEFING_WIN;
            }
            whatNextParam_ = GAME_RETURN_TO_MENU;
            return GAME_DEBRIEFING_LOST;
        }

        case GameType::CustomGame:
        case GameType::CustomMultiplayer: {
            if (bQuitGame_) {
                return GAME_RETURN_TO_MENU;
            }
            whatNextParam_ = GAME_RETURN_TO_MENU;
            return GAME_CUSTOM_GAME_STATS;
        }

        default: {
            return GAME_RETURN_TO_MENU;
        }
    }
}

bool Game::loadSaveGame(const std::filesystem::path& filename) {
    IFileStream fs;

    if (!fs.open(filename)) {
        return false;
    }

    const bool ret = loadSaveGame(fs);

    fs.close();

    return ret;
}

bool Game::loadSaveGame(InputStream& stream) {
    gameState = GameState::Loading;

    uint32_t magicNum = stream.readUint32();
    if (magicNum != SAVEMAGIC) {
        sdl2::log_info("Game::loadSaveGame(): No valid savegame! Expected magic number %.8X, but got %.8X!", SAVEMAGIC,
                       magicNum);
        return false;
    }

    uint32_t savegameVersion = stream.readUint32();
    if (savegameVersion != SAVEGAMEVERSION) {
        sdl2::log_info("Game::loadSaveGame(): No valid savegame! Expected savegame version %d, but got %d!",
                       SAVEGAMEVERSION, savegameVersion);
        return false;
    }

    std::string duneVersion = stream.readString();

    // if this is a multiplayer load we need to save some information before we overwrite gameInitSettings with
    // the settings saved in the savegame
    const bool bMultiplayerLoad = (gameInitSettings_.getGameType() == GameType::LoadMultiplayer);
    const GameInitSettings::HouseInfoList oldHouseInfoList = gameInitSettings_.getHouseInfoList();

    // read gameInitSettings
    gameInitSettings_ = GameInitSettings(stream);

    // read the actual house setup chosen at the beginning of the game
    const auto numHouseInfo = stream.readUint32();
    houseInfoListSetup_.reserve(numHouseInfo);
    for (uint32_t i = 0; i < numHouseInfo; i++) {
        houseInfoListSetup_.push_back(GameInitSettings::HouseInfo(stream));
    }

    // read map size
    const auto mapSizeX = static_cast<short>(stream.readUint32());
    const auto mapSizeY = static_cast<short>(stream.readUint32());

    // create the new map
    map_                          = std::make_unique<Map>(*this, mapSizeX, mapSizeY);
    dune::globals::currentGameMap = map_.get();

    const GameContext context{*this, *map_, this->getObjectManager()};

    // read GameCycleCount
    gameCycleCount_ = stream.readUint32();

    // read some settings
    gameType  = static_cast<GameType>(stream.readSint8());
    techLevel = stream.readUint8();
    randomFactory.setSeed(stream.readUint8Vector());
    auto seed = stream.readUint8Vector();
    randomGen.setState(seed);

    // read in the unit/structure data
    objectData.load(stream);

    // load the house(s) info
    for (auto i = 0; i < NUM_HOUSES; i++) {
        if (stream.readBool()) {
            // house in game
            house_[i] = std::make_unique<House>(context, stream);
        }
    }

    // we have to set the local player
    if (bMultiplayerLoad) {
        // get it from the gameInitSettings that started the game (not the one saved in the savegame)
        for (const auto& houseInfo : oldHouseInfoList) {

            // find the right house
            for (int i = 0; i < NUM_HOUSES; i++) {
                if ((house_[i] != nullptr) && (house_[i]->getHouseID() == houseInfo.houseID)) {
                    // iterate over all players
                    const auto& players = house_[i]->getPlayerList();
                    auto playerIter     = players.cbegin();
                    for (const auto& playerInfo : houseInfo.playerInfoList) {
                        if (playerInfo.playerClass == HUMANPLAYERCLASS) {
                            while (playerIter != players.cend()) {

                                auto* const pHumanPlayer = dynamic_cast<HumanPlayer*>(playerIter->get());
                                if (pHumanPlayer) {
                                    // we have actually found a human player and now assign the first unused
                                    // name to it
                                    unregisterPlayer(pHumanPlayer);
                                    pHumanPlayer->setPlayername(playerInfo.playerName);
                                    registerPlayer(pHumanPlayer);

                                    if (playerInfo.playerName == getLocalPlayerName()) {
                                        dune::globals::pLocalHouse  = house_[i].get();
                                        dune::globals::pLocalPlayer = pHumanPlayer;
                                    }

                                    ++playerIter;
                                    break;
                                }
                                ++playerIter;
                            }
                        }
                    }
                }
            }
        }
    } else {
        // it is stored in the savegame, so set it up
        const auto localPlayerID    = stream.readUint8();
        dune::globals::pLocalPlayer = dynamic_cast<HumanPlayer*>(getPlayerByID(localPlayerID));
        if (!dune::globals::pLocalPlayer) {
            sdl2::log_info("Game::loadSaveGame(): No invalid playerID (%d)!", localPlayerID);

            return false;
        }

        dune::globals::pLocalHouse =
            house_[static_cast<int>(dune::globals::pLocalPlayer->getHouse()->getHouseID())].get();
    }

    dune::globals::debug = stream.readBool();
    bCheatsEnabled_      = stream.readBool();

    winFlags  = stream.readUint32();
    loseFlags = stream.readUint32();

    map_->load(stream);

    // load the structures and units
    objectManager_.load(stream);

    const auto numBullets = stream.readUint32();
    dune::globals::bulletList.reserve(numBullets);
    for (auto i = 0u; i < numBullets; i++) {
        map_->add_bullet(stream);
    }

    const auto numExplosions = stream.readUint32();
    explosionList_.reserve(numExplosions);
    for (auto i = 0u; i < numExplosions; i++) {
        addExplosion(stream);
    }

    auto* const screenborder = dune::globals::screenborder.get();

    if (bMultiplayerLoad) {
        screenborder->adjustScreenBorderToMapsize(map_->getSizeX(), map_->getSizeY());

        screenborder->setNewScreenCenter(dune::globals::pLocalHouse->getCenterOfMainBase() * TILESIZE);

    } else {
        // load selection list
        selectedList_ = stream.readUint32Set();

        // load the screenborder info
        screenborder->adjustScreenBorderToMapsize(map_->getSizeX(), map_->getSizeY());
        screenborder->load(stream);
    }

    // load triggers
    triggerManager_.load(stream);

    // CommandManager is at the very end of the file. DO NOT CHANGE THIS!
    cmdManager_.load(stream);

    finished_ = false;

    return true;
}

bool Game::saveGame(const std::filesystem::path& filename) {
    OFileStream fs;

    if (!fs.open(filename)) {
        sdl2::log_info("Game::saveGame(): %s", dune::string_error(errno));
        dune::globals::currentGame->addToNewsTicker(std::string("Game NOT saved: Cannot open \"")
                                                    + reinterpret_cast<const char*>(filename.u8string().c_str())
                                                    + "\".");
        return false;
    }

    fs.writeUint32(SAVEMAGIC);

    fs.writeUint32(SAVEGAMEVERSION);

    fs.writeString(VERSIONSTRING);

    // write gameInitSettings
    gameInitSettings_.save(fs);

    fs.writeUint32(houseInfoListSetup_.size());
    for (const GameInitSettings::HouseInfo& houseInfo : houseInfoListSetup_) {
        houseInfo.save(fs);
    }

    // write the map size
    fs.writeUint32(map_->getSizeX());
    fs.writeUint32(map_->getSizeY());

    // write GameCycleCount
    fs.writeUint32(gameCycleCount_);

    // write some settings
    fs.writeSint8(static_cast<int8_t>(gameType));
    fs.writeUint8(static_cast<uint8_t>(techLevel));
    fs.writeUint8Vector(randomFactory.getSeed());
    fs.writeUint8Vector(randomGen.getState());

    // write out the unit/structure data
    objectData.save(fs);

    // write the house(s) info
    for (int i = 0; i < NUM_HOUSES; i++) {
        fs.writeBool(house_[i] != nullptr);

        if (house_[i] != nullptr) {
            house_[i]->save(fs);
        }
    }

    if (gameInitSettings_.getGameType() != GameType::CustomMultiplayer) {
        fs.writeUint8(dune::globals::pLocalPlayer->getPlayerID());
    }

    fs.writeBool(dune::globals::debug);
    fs.writeBool(bCheatsEnabled_);

    fs.writeUint32(winFlags);
    fs.writeUint32(loseFlags);

    map_->save(fs, getGameCycleCount());

    // save the structures and units
    objectManager_.save(fs);

    fs.writeUint32(dune::globals::bulletList.size());
    for (const auto& pBullet : dune::globals::bulletList) {
        pBullet->save(fs);
    }

    fs.writeUint32(explosionList_.size());
    for (const auto& pExplosion : explosionList_) {
        pExplosion->save(fs);
    }

    if (gameInitSettings_.getGameType() != GameType::CustomMultiplayer) {
        // save selection lists

        // write out selected units list
        fs.writeUint32Set(selectedList_);

        // write the screenborder info
        dune::globals::screenborder->save(fs);
    }

    // save triggers
    triggerManager_.save(fs);

    // CommandManager is at the very end of the file. DO NOT CHANGE THIS!
    cmdManager_.save(fs);

    fs.close();

    return true;
}

void Game::saveObject(OutputStream& stream, ObjectBase* obj) {
    if (obj == nullptr)
        return;

    stream.writeUint32(obj->getItemID());
    obj->save(stream);
}

void Game::selectAll(const dune::selected_set_type& aList) const {
    for (const auto objectID : aList) {
        if (auto* object = objectManager_.getObject(objectID))
            object->setSelected(true);
    }
}

void Game::unselectAll(const dune::selected_set_type& aList) const {
    for (const auto objectID : aList) {
        if (auto* object = objectManager_.getObject(objectID))
            object->setSelected(false);
    }
}

void Game::onReceiveSelectionList(const std::string& name, const dune::selected_set_type& newSelectionList,
                                  int groupListIndex) {
    auto* pHumanPlayer = dynamic_cast<HumanPlayer*>(getPlayerByName(name));

    if (pHumanPlayer == nullptr) {
        return;
    }

    if (groupListIndex == -1) {
        // the other player controlling the same house has selected some units

        if (pHumanPlayer->getHouse() != dune::globals::pLocalHouse) {
            return;
        }

        for (const auto objectID : selectedByOtherPlayerList_) {
            auto* pObject = objectManager_.getObject(objectID);
            if (pObject != nullptr) {
                pObject->setSelectedByOtherPlayer(false);
            }
        }

        selectedByOtherPlayerList_ = newSelectionList;

        for (const uint32_t objectID : selectedByOtherPlayerList_) {
            ObjectBase* pObject = objectManager_.getObject(objectID);
            if (pObject != nullptr) {
                pObject->setSelectedByOtherPlayer(true);
            }
        }
    } else {
        // some other player has assigned a number to a list of units
        pHumanPlayer->setGroupList(groupListIndex, newSelectionList);
    }
}

void Game::onPeerDisconnected(const std::string& name, [[maybe_unused]] bool bHost, [[maybe_unused]] int cause) const {
    pInterface_->getChatManager().addInfoMessage(name + " disconnected!");
}

void Game::setGameWon() {
    if (!bQuitGame_ && !finished_) {
        won_               = true;
        finished_          = true;
        finishedLevelTime_ = dune::dune_clock::now();
        dune::globals::soundPlayer->playVoice(Voice_enum::YourMissionIsComplete,
                                              dune::globals::pLocalHouse->getHouseID());
    }
}

void Game::setGameLost() {
    if (!bQuitGame_ && !finished_) {
        won_               = false;
        finished_          = true;
        finishedLevelTime_ = dune::dune_clock::now();
        dune::globals::soundPlayer->playVoice(Voice_enum::YouHaveFailedYourMission,
                                              dune::globals::pLocalHouse->getHouseID());
    }
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
            handleSelectedObjectsRequestCarryallDropClick(context, worldPosition.x / TILESIZE,
                                                          worldPosition.y / TILESIZE);
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

void Game::handleChatInput([[maybe_unused]] const GameContext& context, SDL_KeyboardEvent& keyboardEvent) {
    if (keyboardEvent.keysym.sym == SDLK_ESCAPE) {
        chatMode_ = false;
    } else if (keyboardEvent.keysym.sym == SDLK_RETURN) {
        if (typingChatMessage_.length() > 0) {
            unsigned char md5sum[16];

            md5(reinterpret_cast<const unsigned char*>(typingChatMessage_.c_str()), typingChatMessage_.size(), md5sum);

            std::stringstream md5stream;
            md5stream << std::setfill('0') << std::hex << std::uppercase << "0x";
            for (const int i : md5sum) {
                md5stream << std::setw(2) << i;
            }

            const std::string md5string = md5stream.str();

            if ((!bCheatsEnabled_) && (md5string == "0xB8766C8EC7A61036B69893FC17AAF21E")) {
                bCheatsEnabled_ = true;
                pInterface_->getChatManager().addInfoMessage("Cheat mode enabled");
            } else if ((bCheatsEnabled_) && (md5string == "0xB8766C8EC7A61036B69893FC17AAF21E")) {
                pInterface_->getChatManager().addInfoMessage("Cheat mode already enabled");
            } else if ((bCheatsEnabled_) && (md5string == "0x57583291CB37F8167EDB0611D8D19E58")) {
                if (gameType != GameType::CustomMultiplayer) {
                    pInterface_->getChatManager().addInfoMessage("You win this game");
                    setGameWon();
                }
            } else if ((bCheatsEnabled_) && (md5string == "0x1A12BE3DBE54C5A504CAA6EE9782C1C8")) {
                if (dune::globals::debug) {
                    pInterface_->getChatManager().addInfoMessage("You are already in debug mode");
                } else if (gameType != GameType::CustomMultiplayer) {
                    pInterface_->getChatManager().addInfoMessage("Debug mode enabled");
                    dune::globals::debug = true;
                }
            } else if ((bCheatsEnabled_) && (md5string == "0x54F68155FC64A5BC66DCD50C1E925C0B")) {
                if (!dune::globals::debug) {
                    pInterface_->getChatManager().addInfoMessage("You are not in debug mode");
                } else if (gameType != GameType::CustomMultiplayer) {
                    pInterface_->getChatManager().addInfoMessage("Debug mode disabled");
                    dune::globals::debug = false;
                }
            } else if ((bCheatsEnabled_) && (md5string == "0xCEF1D26CE4B145DE985503CA35232ED8")) {
                if (gameType != GameType::CustomMultiplayer) {
                    pInterface_->getChatManager().addInfoMessage("You got some credits");
                    dune::globals::pLocalHouse->returnCredits(10000);
                }
            } else {
                if (auto* const network_manager = dune::globals::pNetworkManager.get()) {
                    network_manager->sendChatMessage(typingChatMessage_);
                }
                pInterface_->getChatManager().addChatMessage(getLocalPlayerName(), typingChatMessage_);
            }
        }

        chatMode_ = false;
    } else if (keyboardEvent.keysym.sym == SDLK_BACKSPACE) {
        if (typingChatMessage_.length() > 0) {
            typingChatMessage_ = utf8Substr(typingChatMessage_, 0, utf8Length(typingChatMessage_) - 1);
        }
    }
}

bool Game::removeFromSelectionLists(ObjectBase* pObject) {
    if (!pObject->isSelected() && !pObject->isSelectedByOtherPlayer())
        return false;

    const auto objectID = pObject->getObjectID();

    assert(getSelectedList().contains(objectID) || getSelectedByOtherPlayerList().contains(objectID));

    getSelectedList().erase(objectID);
    getSelectedByOtherPlayerList().erase(objectID);

    pObject->setSelected(false);

    return true;
}

void Game::removeFromQuickSelectionLists(uint32_t objectID) {
    auto* const local_player = dune::globals::pLocalPlayer;

    for (int i = 0; i < NUMSELECTEDLISTS; i++) {
        local_player->getGroupList(i).erase(objectID);
    }
}

void Game::clearSelectedList() {
    unselectAll(selectedList_);
    selectedList_.clear();
    selectionChanged();
}

void Game::handleKeyInput(const GameContext& context, SDL_KeyboardEvent& keyboardEvent) {
    auto* const screenborder = dune::globals::screenborder.get();

    switch (keyboardEvent.keysym.sym) {

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
            const auto selectListIndex = keyboardEvent.keysym.sym - SDLK_1;

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
                    averagePosition /= groupList.size();
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
                    sdl2::log_error(SDL_LOG_CATEGORY_APPLICATION, "Unable to save configuration file %s",
                                    reinterpret_cast<const char*>(getConfigFilepath().u8string().c_str()));
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
                    sdl2::log_error(SDL_LOG_CATEGORY_APPLICATION, "Unable to save configuration file %s",
                                    reinterpret_cast<const char*>(getConfigFilepath().u8string().c_str()));
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
                    ConstructionYard* pConstructionYard =
                        dynamic_cast<ConstructionYard*>(objectManager_.getObject(*selectedList_.begin()));
                    if (pConstructionYard != nullptr) {
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
        pBuilder = dynamic_cast<BuilderBase*>(objectManager_.getObject(*selectedList_.begin()));
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
                                                   CMDTYPE::CMD_PLACE_STRUCTURE, pBuilder->getObjectID(), xPos, yPos));
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
                                                   CMDTYPE::CMD_PLACE_STRUCTURE, pBuilder->getObjectID(), xPos, yPos));
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
        getCommandManager().addCommand(Command(dune::globals::pLocalPlayer->getPlayerID(), CMDTYPE::CMD_PLACE_STRUCTURE,
                                               pBuilder->getObjectID(), xPos, yPos));
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
                            const auto newDestination = map_->findDeploySpot(pUnit, Coord(xPos, yPos), uiRandom,
                                                                             pUnit->getLocation(), structuresize);
                            pUnit->handleMoveClick(context, newDestination.x, newDestination.y);
                        }
                    }
                } else if (pTile->hasInfantry()) {
                    for (const auto objectID : pTile->getInfantryList()) {
                        auto* pInfantry = getObjectManager().getObject<InfantryBase>(objectID);
                        if ((pInfantry != nullptr) && (pInfantry->getOwner() == pBuilder->getOwner())) {
                            const auto newDestination = map_->findDeploySpot(pInfantry, Coord(xPos, yPos), uiRandom,
                                                                             pInfantry->getLocation(), structuresize);
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

void Game::saveScreenshot() {
    pendingScreenshot_ = false;

    auto [ok, path] = SaveScreenshot();

    if (ok && path.has_value()) {
        const std::string filename{reinterpret_cast<const char*>(path.value().filename().u8string().c_str())};
        dune::globals::currentGame->addToNewsTicker(fmt::format("{}: '{}'", _("Screenshot saved"), filename));
    }
}

void Game::selectNextStructureOfType(const dune::selected_set_type& itemIDs) {
    bool bSelectNext = true;

    if (selectedList_.size() == 1) {
        if (const auto* const pObject = getObjectManager().getObject(*selectedList_.begin()))
            if (itemIDs.count(pObject->getItemID()) == 1) {
                bSelectNext = false;
            }
    }

    StructureBase* pStructure2Select = nullptr;

    for (auto* const pStructure : dune::globals::structureList) {
        if (bSelectNext) {
            if ((itemIDs.count(pStructure->getItemID()) == 1)
                && (pStructure->getOwner() == dune::globals::pLocalHouse)) {
                pStructure2Select = pStructure;
                break;
            }
        } else {
            if (selectedList_.size() == 1 && pStructure->isSelected()) {
                bSelectNext = true;
            }
        }
    }

    if (pStructure2Select == nullptr) {
        // start over at the beginning
        for (auto* pStructure : dune::globals::structureList) {
            if ((itemIDs.count(pStructure->getItemID()) == 1) && (pStructure->getOwner() == dune::globals::pLocalHouse)
                && !pStructure->isSelected()) {
                pStructure2Select = pStructure;
                break;
            }
        }
    }

    if (pStructure2Select != nullptr) {
        unselectAll(selectedList_);
        selectedList_.clear();

        pStructure2Select->setSelected(true);
        selectedList_.insert(pStructure2Select->getObjectID());
        selectionChanged();

        // we center around the newly selected construction yard
        dune::globals::screenborder->setNewScreenCenter(pStructure2Select->getLocation() * TILESIZE);
    }
}

dune::dune_clock::duration Game::getGameSpeed() const {
    if (gameType == GameType::CustomMultiplayer) {
        return dune::as_dune_clock_duration(gameInitSettings_.getGameOptions().gameSpeed);
    }
    return dune::as_dune_clock_duration(dune::globals::settings.gameOptions.gameSpeed);
}
