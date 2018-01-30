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
#include <config.h>
#include <main.h>

#include <FileClasses/FileManager.h>
#include <FileClasses/GFXManager.h>
#include <FileClasses/FontManager.h>
#include <FileClasses/TextManager.h>
#include <FileClasses/music/MusicPlayer.h>
#include <FileClasses/LoadSavePNG.h>
#include <SoundPlayer.h>
#include <misc/IFileStream.h>
#include <misc/OFileStream.h>
#include <misc/IMemoryStream.h>
#include <misc/FileSystem.h>
#include <misc/fnkdat.h>
#include <misc/draw_util.h>
#include <misc/md5.h>
#include <misc/exceptions.h>
#include <misc/format.h>
#include <misc/SDL2pp.h>

#include <players/HumanPlayer.h>

#include <Network/NetworkManager.h>

#include <GUI/dune/InGameMenu.h>
#include <GUI/dune/WaitingForOtherPlayers.h>
#include <Menu/MentatHelp.h>
#include <Menu/BriefingMenu.h>
#include <Menu/MapChoice.h>

#include <House.h>
#include <Map.h>
#include <Bullet.h>
#include <Explosion.h>
#include <GameInitSettings.h>
#include <ScreenBorder.h>
#include <sand.h>

#include <structures/StructureBase.h>
#include <structures/ConstructionYard.h>
#include <units/UnitBase.h>
#include <structures/BuilderBase.h>
#include <structures/Palace.h>
#include <units/Harvester.h>
#include <units/InfantryBase.h>

#include <algorithm>
#include <sstream>
#include <iomanip>

Game::Game() {
    currentZoomlevel = settings.video.preferredZoomLevel;

    localPlayerName = settings.general.playerName;

    unitList.clear();       //holds all the units
    structureList.clear();  //all the structures
    bulletList.clear();

    sideBarPos = calcAlignedDrawingRect(pGFXManager->getUIGraphic(UI_SideBar), HAlign::Right, VAlign::Top);
    topBarPos = calcAlignedDrawingRect(pGFXManager->getUIGraphic(UI_TopBar), HAlign::Left, VAlign::Top);

    // set to true for now
    debug = false;

    powerIndicatorPos.h = spiceIndicatorPos.h = settings.video.height - 146 - 2;

    musicPlayer->changeMusic(MUSIC_PEACE);
    //////////////////////////////////////////////////////////////////////////
    const SDL_Rect gameBoardRect = { 0, topBarPos.h, sideBarPos.x, getRendererHeight() - topBarPos.h };
    screenborder = new ScreenBorder(gameBoardRect);
}


/**
    The destructor frees up all the used memory.
*/
Game::~Game() {
    if(pNetworkManager != nullptr) {
        pNetworkManager->setOnReceiveChatMessage(std::function<void (const std::string&, const std::string&)>());
        pNetworkManager->setOnReceiveCommandList(std::function<void (const std::string&, const CommandList&)>());
        pNetworkManager->setOnReceiveSelectionList(std::function<void (const std::string&, const Dune::selected_set_type&, int)>());
        pNetworkManager->setOnPeerDisconnected(std::function<void (const std::string&, bool, int)>());
    }

    for(StructureBase* pStructure : structureList) {
        delete pStructure;
    }
    structureList.clear();

    for(UnitBase* pUnit : unitList) {
        delete pUnit;
    }
    unitList.clear();

    for(Bullet* pBullet : bulletList) {
        delete pBullet;
    }
    bulletList.clear();

    for(Explosion* pExplosion : explosionList) {
        delete pExplosion;
    }
    explosionList.clear();

    delete currentGameMap;
    currentGameMap = nullptr;
    delete screenborder;
    screenborder = nullptr;
}


void Game::initGame(const GameInitSettings& newGameInitSettings) {
    gameInitSettings = newGameInitSettings;

    switch(gameInitSettings.getGameType()) {
        case GameType::LoadSavegame: {
            if(loadSaveGame(gameInitSettings.getFilename()) == false) {
                THROW(std::runtime_error, "Loading save game failed!");
            }
        } break;

        case GameType::LoadMultiplayer: {
            IMemoryStream memStream(gameInitSettings.getFiledata().c_str(), gameInitSettings.getFiledata().size());

            if(loadSaveGame(memStream) == false) {
                THROW(std::runtime_error, "Loading save game failed!");
            }
        } break;

        case GameType::Campaign:
        case GameType::Skirmish:
        case GameType::CustomGame:
        case GameType::CustomMultiplayer: {
            gameType = gameInitSettings.getGameType();
            randomGen.setSeed(gameInitSettings.getRandomSeed());

            objectData.loadFromINIFile("ObjectData.ini");

            if(gameInitSettings.getMission() != 0) {
                techLevel = ((gameInitSettings.getMission() + 1)/3) + 1 ;
            }

            INIMapLoader(this, gameInitSettings.getFilename(), gameInitSettings.getFiledata());

            if(bReplay == false && gameInitSettings.getGameType() != GameType::CustomGame && gameInitSettings.getGameType() != GameType::CustomMultiplayer) {
                /* do briefing */
                SDL_Log("Briefing...");
                BriefingMenu(gameInitSettings.getHouseID(), gameInitSettings.getMission(),BRIEFING).showMenu();
            }
        } break;

        default: {
        } break;
    }
}

void Game::initReplay(const std::string& filename) {
    bReplay = true;

    IFileStream fs;

    if(fs.open(filename) == false) {
        THROW(io_error, "Error while opening '%s'!", filename);
    }

    // override local player name as it was when the replay was created
    localPlayerName = fs.readString();

    // read GameInitInfo
    const GameInitSettings loadedGameInitSettings(fs);

    // load all commands
    cmdManager.load(fs);

    initGame(loadedGameInitSettings);
}


void Game::processObjects()
{
    // update all tiles
    currentGameMap->for_all([](Tile& t) { t.update(); });

    for(auto pStructure : structureList) {
        pStructure->update();
    }

    if ((currentCursorMode == CursorMode_Placing) && selectedList.empty()) {
        currentCursorMode = CursorMode_Normal;
    }

    for(auto pUnit : unitList) {
        pUnit->update();
    }

    for(auto pBullet : bulletList) {
        pBullet->update();
    }

    explosionList.erase(std::remove_if(explosionList.begin(), explosionList.end(), [](std::unique_ptr<Explosion>& e) { return e->update(); }), explosionList.end());
}


void Game::drawScreen()
{
    const auto top_left = screenborder->getTopLeftTile();
    const auto bottom_right = screenborder->getBottomRightTile();

    auto TopLeftTile = top_left;
    auto BottomRightTile = bottom_right;

    // extend the view a little bit to avoid graphical glitches
    TopLeftTile.x = std::max(0, TopLeftTile.x - 1);
    TopLeftTile.y = std::max(0, TopLeftTile.y - 1);
    BottomRightTile.x = std::min(currentGameMap->getSizeX() - 1, BottomRightTile.x + 1);
    BottomRightTile.y = std::min(currentGameMap->getSizeY() - 1, BottomRightTile.y + 1);

    const auto x1 = TopLeftTile.x;
    const auto y1 = TopLeftTile.y;
    const auto x2 = BottomRightTile.x + 1;
    const auto y2 = BottomRightTile.y + 1;

    const auto tiles = currentGameMap;

    const auto x1 = TopLeftTile.x;
    const auto y1 = TopLeftTile.y;
    const auto x2 = BottomRightTile.x + 1;
    const auto y2 = BottomRightTile.y + 1;

    /* draw ground */

    currentGameMap->for_each(x1, y1, x2, y2,
        [](Tile& t) {
            t.blitGround(screenborder->world2screenX(t.getLocation().x*TILESIZE),
                screenborder->world2screenY(t.getLocation().y*TILESIZE));
        });

    /* draw structures */
    currentGameMap->for_each(x1, y1, x2, y2,
        [](Tile& t) {
            t.blitStructures(screenborder->world2screenX(t.getLocation().x*TILESIZE),
                screenborder->world2screenY(t.getLocation().y*TILESIZE));
        });

    /* draw underground units */
    currentGameMap->for_each(x1, y1, x2, y2,
        [](Tile& t) {
            t.blitUndergroundUnits(screenborder->world2screenX(t.getLocation().x*TILESIZE),
                screenborder->world2screenY(t.getLocation().y*TILESIZE));
        });

    /* draw dead objects */
    currentGameMap->for_each(x1, y1, x2, y2,
        [](Tile& t) {
            t.blitDeadUnits(screenborder->world2screenX(t.getLocation().x*TILESIZE),
                screenborder->world2screenY(t.getLocation().y*TILESIZE));
        });

    /* draw infantry */
    currentGameMap->for_each(x1, y1, x2, y2,
        [](Tile& t) {
            t.blitInfantry(screenborder->world2screenX(t.getLocation().x*TILESIZE),
                screenborder->world2screenY(t.getLocation().y*TILESIZE));
        });

    /* draw non-infantry ground units */
    currentGameMap->for_each(x1, y1, x2, y2,
        [](Tile& t) {
            t.blitNonInfantryGroundUnits(screenborder->world2screenX(t.getLocation().x*TILESIZE),
                screenborder->world2screenY(t.getLocation().y*TILESIZE));
        });

    /* draw bullets */
    for (const auto pBullet : bulletList) {
        pBullet->blitToScreen();
    }


    /* draw explosions */
    for (const auto& pExplosion : explosionList) {
        pExplosion->blitToScreen();
    }

    /* draw air units */
    currentGameMap->for_each(x1, y1, x2, y2,
        [&](Tile& t) {
            t.blitAirUnits(screenborder->world2screenX(t.getLocation().x*TILESIZE),
                screenborder->world2screenY(t.getLocation().y*TILESIZE));
        });

    // draw the gathering point line if a structure is selected
    if (selectedList.size() == 1) {
        const auto pStructure = dynamic_cast<StructureBase*>(getObjectManager().getObject(*selectedList.begin()));
        if (pStructure != nullptr) {
            pStructure->drawGatheringPointLine();
        }
    }

    /* draw selection rectangles */
    currentGameMap->for_each(x1, y1, x2, y2,
        [](Tile& t) {
            if (debug || t.isExploredByTeam(pLocalHouse->getTeamID())) {
                t.blitSelectionRects(screenborder->world2screenX(t.getLocation().x*TILESIZE),
                    screenborder->world2screenY(t.getLocation().y*TILESIZE));
            }
        });


//////////////////////////////draw unexplored/shade

    if(debug == false) {
        SDL_Texture* hiddenTexZoomed = pGFXManager->getZoomedObjPic(ObjPic_Terrain_Hidden, currentZoomlevel);
        SDL_Texture* hiddenFogTexZoomed = pGFXManager->getZoomedObjPic(ObjPic_Terrain_HiddenFog, currentZoomlevel);
        int zoomedTileSize = world2zoomedWorld(TILESIZE);

        const auto fogOfWar = gameInitSettings.getGameOptions().fogOfWar;
        const auto zoomedTileSize = world2zoomedWorld(TILESIZE);
        const auto hideTile = pTile->getHideTile(pLocalHouse->getTeamID());

        tiles->for_each(top_left.x - 1, top_left.y - 1, bottom_right.x + 2, bottom_right.y + 1,
            [=](Tile& t) {
                const auto x = t.getLocation().x;
                const auto y = t.getLocation().y;

                 if(pTile->isExploredByTeam(pLocalHouse->getTeamID())) {

                        if(hideTile != 0) {
                            SDL_Rect source = { hideTile*zoomedTileSize, 0, zoomedTileSize, zoomedTileSize };
                            SDL_Rect drawLocation = {   screenborder->world2screenX(x*TILESIZE), screenborder->world2screenY(y*TILESIZE),
                                                        zoomedTileSize, zoomedTileSize };
                            SDL_RenderCopy(renderer, hiddenTexZoomed, &source, &drawLocation);
                        }

                        if(gameInitSettings.getGameOptions().fogOfWar == true) {
                            int fogTile = pTile->getFogTile(pLocalHouse->getTeamID());

                            if(pTile->isFoggedByTeam(pLocalHouse->getTeamID()) == true) {
                                fogTile = Terrain_HiddenFull;
                            }

                            if(fogTile != 0) {
                                SDL_Rect source = { fogTile*zoomedTileSize, 0,
                                                    zoomedTileSize, zoomedTileSize };
                        SDL_RenderCopy(renderer, hiddenTex, &source, &drawLocation);
                    }

                                SDL_RenderCopy(renderer, hiddenFogTexZoomed, &source, &drawLocation);
                            }
                        }
                    } else {
                        if(!debug) {
                            SDL_Rect source = { zoomedTileSize*15, 0, zoomedTileSize, zoomedTileSize };
                            SDL_Rect drawLocation = {   screenborder->world2screenX(x*TILESIZE), screenborder->world2screenY(y*TILESIZE),
                                                        zoomedTileSize, zoomedTileSize };
                            SDL_RenderCopy(renderer, hiddenTexZoomed, &source, &drawLocation);
                        }
                    }
                } else {
                    // we are outside the map => draw complete hidden
                    SDL_Rect source = { zoomedTileSize*15, 0, zoomedTileSize, zoomedTileSize };
                    SDL_Rect drawLocation = {   screenborder->world2screenX(x*TILESIZE), screenborder->world2screenY(y*TILESIZE),
                                                zoomedTileSize, zoomedTileSize };
                    SDL_RenderCopy(renderer, hiddenTexZoomed, &source, &drawLocation);
                }
                else {
                    if (!debug) {
                        const SDL_Rect source = { zoomedTileSize * 15, 0, zoomedTileSize, zoomedTileSize };
                        const SDL_Rect drawLocation = { screenborder->world2screenX(x*TILESIZE), screenborder->world2screenY(y*TILESIZE),
                                                    zoomedTileSize, zoomedTileSize };
                        SDL_RenderCopy(renderer, hiddenTex, &source, &drawLocation);
                    }
                }
            });

        const auto size_x = std::min(tiles->getSizeX(), bottom_right.x + 1);
        const auto size_y = std::min(tiles->getSizeY(), bottom_right.y + 1);
        for (auto x = top_left.x - 1; x <= bottom_right.x + 1; ++x) {
            for (auto y = top_left.y - 1; y <= bottom_right.y + 1; ++y) {
                if (x >= 0 && x < size_x)
                    x = size_x;
                if (y >= 0 && y < size_y)
                    y = size_y;

                // we are outside the map => draw complete hidden
                const SDL_Rect source = { zoomedTileSize * 15, 0, zoomedTileSize, zoomedTileSize };
                const SDL_Rect drawLocation = { screenborder->world2screenX(x*TILESIZE), screenborder->world2screenY(y*TILESIZE),
                    zoomedTileSize, zoomedTileSize };
                SDL_RenderCopy(renderer, hiddenTex, &source, &drawLocation);
            }
        }
    }

/////////////draw placement position

    if(currentCursorMode == CursorMode_Placing) {
        //if user has selected to place a structure

        if(screenborder->isScreenCoordInsideMap(drawnMouseX, drawnMouseY)) {
            //if mouse is not over game bar

            const int xPos = screenborder->screen2MapX(drawnMouseX);
            const int yPos = screenborder->screen2MapY(drawnMouseY);

            if(selectedList.size() == 1) {
                auto pBuilder = dynamic_cast<BuilderBase*>(objectManager.getObject(*selectedList.begin()));
                if(pBuilder) {
                    int placeItem = pBuilder->getCurrentProducedItem();
                    Coord structuresize = getStructureSize(placeItem);

                    bool withinRange = false;
                    for (int i = xPos; i < (xPos + structuresize.x); i++) {
                        for (int j = yPos; j < (yPos + structuresize.y); j++) {
                            if (currentGameMap->isWithinBuildRange(i, j, pBuilder->getOwner())) {
                                withinRange = true;         //find out if the structure is close enough to other buildings
                            }
                        }
                    }

                    SDL_Texture* validPlace = nullptr;
                    SDL_Texture* invalidPlace = nullptr;

                    switch(currentZoomlevel) {
                        case 0: {
                            validPlace = pGFXManager->getUIGraphic(UI_ValidPlace_Zoomlevel0);
                            invalidPlace = pGFXManager->getUIGraphic(UI_InvalidPlace_Zoomlevel0);
                        } break;

                        case 1: {
                            validPlace = pGFXManager->getUIGraphic(UI_ValidPlace_Zoomlevel1);
                            invalidPlace = pGFXManager->getUIGraphic(UI_InvalidPlace_Zoomlevel1);
                        } break;

                        case 2:
                        default: {
                            validPlace = pGFXManager->getUIGraphic(UI_ValidPlace_Zoomlevel2);
                            invalidPlace = pGFXManager->getUIGraphic(UI_InvalidPlace_Zoomlevel2);
                        } break;

                    }

                    for(int i = xPos; i < (xPos + structuresize.x); i++) {
                        for(int j = yPos; j < (yPos + structuresize.y); j++) {
                            SDL_Texture* image;

                            if(!withinRange || !currentGameMap->tileExists(i,j) || !currentGameMap->getTile(i,j)->isRock()
                                || currentGameMap->getTile(i,j)->isMountain() || currentGameMap->getTile(i,j)->hasAGroundObject()
                                || (((placeItem == Structure_Slab1) || (placeItem == Structure_Slab4)) && currentGameMap->getTile(i,j)->isConcrete())) {
                                image = invalidPlace;
                            } else {
                                image = validPlace;
                            }

                            SDL_Rect drawLocation = calcDrawingRect(image, screenborder->world2screenX(i*TILESIZE), screenborder->world2screenY(j*TILESIZE));
                            SDL_RenderCopy(renderer, image, nullptr, &drawLocation);
                        }
                    }
                }
            }
        }
    }

///////////draw game selection rectangle
    if(selectionMode) {

        int finalMouseX = drawnMouseX;
        int finalMouseY = drawnMouseY;
        if(finalMouseX >= sideBarPos.x) {
            //this keeps the box on the map, and not over game bar
            finalMouseX = sideBarPos.x-1;
        }

        if(finalMouseY < topBarPos.y+topBarPos.h) {
            finalMouseY = topBarPos.x+topBarPos.h;
        }

        // draw the mouse selection rectangle
        renderDrawRect( renderer,
                        screenborder->world2screenX(selectionRect.x),
                        screenborder->world2screenY(selectionRect.y),
                        finalMouseX,
                        finalMouseY,
                        COLOR_WHITE);
    }



///////////draw action indicator

    if((indicatorFrame != NONE_ID) && (screenborder->isInsideScreen(indicatorPosition, Coord(TILESIZE,TILESIZE)) == true)) {
        const auto pUIIndicator = pGFXManager->getUIGraphic(UI_Indicator);
        auto source = calcSpriteSourceRect(pUIIndicator, indicatorFrame, 3);
        auto drawLocation = calcSpriteDrawingRect(  pUIIndicator,
                                                        screenborder->world2screenX(indicatorPosition.x),
                                                        screenborder->world2screenY(indicatorPosition.y),
                                                        3, 1,
                                                        HAlign::Center, VAlign::Center);
        SDL_RenderCopy(renderer, pUIIndicator, &source, &drawLocation);
    }


///////////draw game bar
    pInterface->draw(Point(0,0));
    pInterface->drawOverlay(Point(0,0));

    // draw chat message currently typed
    if(chatMode) {
        sdl2::texture_ptr pChatTexture = pFontManager->createTextureWithText("Chat: " + typingChatMessage + (((SDL_GetTicks() / 150) % 2 == 0) ? "_" : ""), COLOR_WHITE, 14);
        SDL_Rect drawLocation = calcDrawingRect(pChatTexture.get(), 20, getRendererHeight() - 40);
        SDL_RenderCopy(renderer, pChatTexture.get(), nullptr, &drawLocation);
    }

    if(bShowFPS) {
        const auto strFPS = fmt::sprintf("fps: %.1f ", 1000.0f/averageFrameTime);

        sdl2::texture_ptr pFPSTexture = pFontManager->createTextureWithText(strFPS, COLOR_WHITE, 14);
        SDL_Rect drawLocation = calcDrawingRect(pFPSTexture.get(),sideBarPos.x - strFPS.length()*8, 60);
        SDL_RenderCopy(renderer, pFPSTexture.get(), nullptr, &drawLocation);
    }

    if(bShowTime) {
        const int seconds = getGameTime() / 1000;
        const auto strTime = fmt::sprintf(" %.2d:%.2d:%.2d", seconds / 3600, (seconds % 3600)/60, (seconds % 60) );

        sdl2::texture_ptr pTimeTexture = pFontManager->createTextureWithText(strTime, COLOR_WHITE, 14);
        SDL_Rect drawLocation = calcAlignedDrawingRect(pTimeTexture.get(), HAlign::Left, VAlign::Bottom);
        drawLocation.y++;
        SDL_RenderCopy(renderer, pTimeTexture.get(), nullptr, &drawLocation);
    }

    if(finished) {
        std::string message;

        if(won) {
            message = _("You Have Completed Your Mission.");
        } else {
            message = _("You Have Failed Your Mission.");
        }

        sdl2::texture_ptr pFinishMessageTexture = pFontManager->createTextureWithText(message.c_str(), COLOR_WHITE, 28);
        SDL_Rect drawLocation = calcDrawingRect(pFinishMessageTexture.get(), sideBarPos.x/2, topBarPos.h + (getRendererHeight()-topBarPos.h)/2, HAlign::Center, VAlign::Center);
        SDL_RenderCopy(renderer, pFinishMessageTexture.get(), nullptr, &drawLocation);
    }

    if(pWaitingForOtherPlayers != nullptr) {
        pWaitingForOtherPlayers->draw();
    }

    if(pInGameMenu != nullptr) {
        pInGameMenu->draw();
    } else if(pInGameMentat != nullptr) {
        pInGameMentat->draw();
    }

    drawCursor();
}


void Game::doInput()
{
    SDL_Event event;
    while(SDL_PollEvent(&event)) {
        // check for a key press

        // first of all update mouse
        if(event.type == SDL_MOUSEMOTION) {
            SDL_MouseMotionEvent* mouse = &event.motion;
            drawnMouseX = std::max(0, std::min(mouse->x, settings.video.width-1));
            drawnMouseY = std::max(0, std::min(mouse->y, settings.video.height-1));
        }

        if(pInGameMenu != nullptr) {
            pInGameMenu->handleInput(event);

            if(bMenu == false) {
                pInGameMenu.reset();
            }

        } else if(pInGameMentat != nullptr) {
            pInGameMentat->doInput(event);

            if(bMenu == false) {
                pInGameMentat.reset();
            }

        } else if(pWaitingForOtherPlayers != nullptr) {
            pWaitingForOtherPlayers->handleInput(event);

            if(bMenu == false) {
                pWaitingForOtherPlayers.reset();
            }

        } else {
            /* Look for a keypress */
            switch (event.type) {

                case SDL_KEYDOWN: {
                    if(chatMode) {
                        handleChatInput(event.key);
                    } else {
                        handleKeyInput(event.key);
                    }
                } break;

                case SDL_TEXTINPUT: {
                    if(chatMode) {
                        const auto newText = event.text.text;
                        if(utf8Length(typingChatMessage) + utf8Length(newText) <= 60) {
                            typingChatMessage += newText;
                        }
                    }
                } break;

                case SDL_MOUSEWHEEL: {
                    if (event.wheel.y != 0) {
                        pInterface->handleMouseWheel(drawnMouseX,drawnMouseY,(event.wheel.y > 0));
                    }
                } break;

                case SDL_MOUSEBUTTONDOWN: {
                    const auto mouse = &event.button;

                    switch(mouse->button) {
                        case SDL_BUTTON_LEFT: {
                            pInterface->handleMouseLeft(mouse->x, mouse->y, true);
                        } break;

                        case SDL_BUTTON_RIGHT: {
                            pInterface->handleMouseRight(mouse->x, mouse->y, true);
                        } break;

                        default:
                            break;
                    }

                    switch(mouse->button) {

                        case SDL_BUTTON_LEFT: {

                            switch(currentCursorMode) {

                                case CursorMode_Placing: {
                                    if(screenborder->isScreenCoordInsideMap(mouse->x, mouse->y) == true) {
                                        handlePlacementClick(screenborder->screen2MapX(mouse->x), screenborder->screen2MapY(mouse->y));
                                    }
                                } break;

                                case CursorMode_Attack: {

                                    if(screenborder->isScreenCoordInsideMap(mouse->x, mouse->y) == true) {
                                        handleSelectedObjectsAttackClick(screenborder->screen2MapX(mouse->x), screenborder->screen2MapY(mouse->y));
                                    }

                                } break;

                                case CursorMode_Move: {

                                    if(screenborder->isScreenCoordInsideMap(mouse->x, mouse->y) == true) {
                                        handleSelectedObjectsMoveClick(screenborder->screen2MapX(mouse->x), screenborder->screen2MapY(mouse->y));
                                    }

                                } break;

                                case CursorMode_CarryallDrop: {

                                    if(screenborder->isScreenCoordInsideMap(mouse->x, mouse->y) == true) {
                                        handleSelectedObjectsRequestCarryallDropClick(screenborder->screen2MapX(mouse->x), screenborder->screen2MapY(mouse->y));
                                    }

                                } break;

                                case CursorMode_Capture: {

                                    if(screenborder->isScreenCoordInsideMap(mouse->x, mouse->y) == true) {
                                        handleSelectedObjectsCaptureClick(screenborder->screen2MapX(mouse->x), screenborder->screen2MapY(mouse->y));
                                    }

                                } break;

                                case CursorMode_Normal:
                                default: {

                                    if (mouse->x < sideBarPos.x && mouse->y >= topBarPos.h) {
                                        // it isn't on the gamebar

                                        if(!selectionMode) {
                                            // if we have started the selection rectangle
                                            // the starting point of the selection rectangele
                                            selectionRect.x = screenborder->screen2worldX(mouse->x);
                                            selectionRect.y = screenborder->screen2worldY(mouse->y);
                                        }
                                        selectionMode = true;

                                    }
                                } break;
                            }
                        } break;    //end of SDL_BUTTON_LEFT

                        case SDL_BUTTON_RIGHT: {
                            //if the right mouse button is pressed

                            if(currentCursorMode != CursorMode_Normal) {
                                //cancel special cursor mode
                                currentCursorMode = CursorMode_Normal;
                            } else if((!selectedList.empty()
                                            && (((objectManager.getObject(*selectedList.begin()))->getOwner() == pLocalHouse))
                                            && (((objectManager.getObject(*selectedList.begin()))->isRespondable())) ) )
                            {
                                //if user has a controlable unit selected

                                if(screenborder->isScreenCoordInsideMap(mouse->x, mouse->y) == true) {
                                    if(handleSelectedObjectsActionClick(screenborder->screen2MapX(mouse->x), screenborder->screen2MapY(mouse->y))) {
                                        indicatorFrame = 0;
                                        indicatorPosition.x = screenborder->screen2worldX(mouse->x);
                                        indicatorPosition.y = screenborder->screen2worldY(mouse->y);
                                    }
                                }
                            }
                        } break;    //end of SDL_BUTTON_RIGHT
                        default:
                            break;
                    }
                } break;

                case SDL_MOUSEMOTION: {
                    const auto mouse = &event.motion;

                    pInterface->handleMouseMovement(mouse->x,mouse->y);
                } break;

                case SDL_MOUSEBUTTONUP: {
                    const auto mouse = &event.button;

                    switch(mouse->button) {
                        case SDL_BUTTON_LEFT: {
                            pInterface->handleMouseLeft(mouse->x, mouse->y, false);
                        } break;

                        case SDL_BUTTON_RIGHT: {
                            pInterface->handleMouseRight(mouse->x, mouse->y, false);
                        } break;
                    default:
                        break;
                    }

                    if(selectionMode && (mouse->button == SDL_BUTTON_LEFT)) {
                        //this keeps the box on the map, and not over game bar
                        int finalMouseX = mouse->x;
                        int finalMouseY = mouse->y;

                        if(finalMouseX >= sideBarPos.x) {
                            finalMouseX = sideBarPos.x-1;
                        }

                        if(finalMouseY < topBarPos.y+topBarPos.h) {
                            finalMouseY = topBarPos.x+topBarPos.h;
                        }

                        int rectFinishX = screenborder->screen2MapX(finalMouseX);
                        if(rectFinishX > (currentGameMap->getSizeX()-1)) {
                            rectFinishX = currentGameMap->getSizeX()-1;
                        }

                        int rectFinishY = screenborder->screen2MapY(finalMouseY);

                        // convert start also to map coordinates
                        int rectStartX = selectionRect.x/TILESIZE;
                        int rectStartY = selectionRect.y/TILESIZE;

                        currentGameMap->selectObjects(  pLocalHouse,
                                                        rectStartX, rectStartY, rectFinishX, rectFinishY,
                                                        screenborder->screen2worldX(finalMouseX),
                                                        screenborder->screen2worldY(finalMouseY),
                                                        SDL_GetModState() & KMOD_SHIFT);

                        if(selectedList.size() == 1) {
                            auto pObject = objectManager.getObject( *selectedList.begin());
                            if(pObject != nullptr && pObject->getOwner() == pLocalHouse && pObject->getItemID() == Unit_Harvester) {
                                auto pHarvester = static_cast<Harvester*>(pObject);

                                auto harvesterMessage = _("@DUNE.ENG|226#Harvester");

                                int percent = lround(100 * pHarvester->getAmountOfSpice() / HARVESTERMAXSPICE);
                                if(percent > 0) {
                                    if(pHarvester->isAwaitingPickup()) {
                                        harvesterMessage += fmt::sprintf(_("@DUNE.ENG|124#full and awaiting pickup"), percent);
                                    } else if(pHarvester->isReturning()) {
                                        harvesterMessage += fmt::sprintf(_("@DUNE.ENG|123#full and returning"), percent);
                                    } else if(pHarvester->isHarvesting()) {
                                        harvesterMessage += fmt::sprintf(_("@DUNE.ENG|122#full and harvesting"), percent);
                                    } else {
                                        harvesterMessage += fmt::sprintf(_("@DUNE.ENG|121#full"), percent);
                                    }

                                } else {
                                    if(pHarvester->isAwaitingPickup()) {
                                        harvesterMessage += _("@DUNE.ENG|128#empty and awaiting pickup");
                                    } else if(pHarvester->isReturning()) {
                                        harvesterMessage += _("@DUNE.ENG|127#empty and returning");
                                    } else if(pHarvester->isHarvesting()) {
                                        harvesterMessage += _("@DUNE.ENG|126#empty and harvesting");
                                    } else {
                                        harvesterMessage += _("@DUNE.ENG|125#empty");
                                    }
                                }

                                if(!pInterface->newsTickerHasMessage()) {
                                    pInterface->addToNewsTicker(harvesterMessage);
                                }
                            }
                        }
                    }

                    selectionMode = false;

                } break;

                case (SDL_QUIT): {
                    bQuitGame = true;
                } break;

                default:
                    break;
            }
        }
    }

    if((pInGameMenu == nullptr) && (pInGameMentat == nullptr) && (pWaitingForOtherPlayers == nullptr) && (SDL_GetWindowFlags(window) & SDL_WINDOW_MOUSE_FOCUS)) {

        auto keystate = SDL_GetKeyboardState(nullptr);
        scrollDownMode =  (drawnMouseY >= getRendererHeight()-1-SCROLLBORDER) || keystate[SDL_SCANCODE_DOWN];
        scrollLeftMode = (drawnMouseX <= SCROLLBORDER) || keystate[SDL_SCANCODE_LEFT];
        scrollRightMode = (drawnMouseX >= getRendererWidth()-1-SCROLLBORDER) || keystate[SDL_SCANCODE_RIGHT];
        scrollUpMode = (drawnMouseY <= SCROLLBORDER) || keystate[SDL_SCANCODE_UP];

        if(scrollLeftMode && scrollRightMode) {
            // do nothing
        } else if(scrollLeftMode) {
            scrollLeftMode = screenborder->scrollLeft();
        } else if(scrollRightMode) {
            scrollRightMode = screenborder->scrollRight();
        }

        if(scrollDownMode && scrollUpMode) {
            // do nothing
        } else if(scrollDownMode) {
            scrollDownMode = screenborder->scrollDown();
        } else if(scrollUpMode) {
            scrollUpMode = screenborder->scrollUp();
        }
    } else {
        scrollDownMode = false;
        scrollLeftMode = false;
        scrollRightMode = false;
        scrollUpMode = false;
    }
}


void Game::drawCursor() const
{
    if(!(SDL_GetWindowFlags(window) & SDL_WINDOW_MOUSE_FOCUS)) {
        return;
    }

    SDL_Texture* pCursor = nullptr;
    SDL_Rect dest = { 0, 0, 0, 0};
    if(scrollLeftMode || scrollRightMode || scrollUpMode || scrollDownMode) {
        if(scrollLeftMode && !scrollRightMode) {
            pCursor = pGFXManager->getUIGraphic(UI_CursorLeft);
            dest = calcDrawingRect(pCursor, drawnMouseX, drawnMouseY-5, HAlign::Left, VAlign::Top);
        } else if(scrollRightMode && !scrollLeftMode) {
            pCursor = pGFXManager->getUIGraphic(UI_CursorRight);
            dest = calcDrawingRect(pCursor, drawnMouseX, drawnMouseY-5, HAlign::Center, VAlign::Top);
        }

        if(pCursor == nullptr) {
            if(scrollUpMode && !scrollDownMode) {
                pCursor = pGFXManager->getUIGraphic(UI_CursorUp);
                dest = calcDrawingRect(pCursor, drawnMouseX-5, drawnMouseY, HAlign::Left, VAlign::Top);
            } else if(scrollDownMode && !scrollUpMode) {
                pCursor = pGFXManager->getUIGraphic(UI_CursorDown);
                dest = calcDrawingRect(pCursor, drawnMouseX-5, drawnMouseY, HAlign::Left, VAlign::Center);
            } else {
                pCursor = pGFXManager->getUIGraphic(UI_CursorNormal);
                dest = calcDrawingRect(pCursor, drawnMouseX, drawnMouseY, HAlign::Left, VAlign::Top);
            }
        }
    } else {
        if( (pInGameMenu != nullptr) || (pInGameMentat != nullptr) || (pWaitingForOtherPlayers != nullptr) || (((drawnMouseX >= sideBarPos.x) || (drawnMouseY < topBarPos.h)) && (isOnRadarView(drawnMouseX, drawnMouseY) == false))) {
            // Menu mode or Mentat Menu or Waiting for other players or outside of game screen but not inside minimap
            pCursor = pGFXManager->getUIGraphic(UI_CursorNormal);
            dest = calcDrawingRect(pCursor, drawnMouseX, drawnMouseY, HAlign::Left, VAlign::Top);
        } else {

            switch(currentCursorMode) {
                case CursorMode_Normal:
                case CursorMode_Placing: {
                    pCursor = pGFXManager->getUIGraphic(UI_CursorNormal);
                    dest = calcDrawingRect(pCursor, drawnMouseX, drawnMouseY, HAlign::Left, VAlign::Top);
                } break;

                case CursorMode_Move: {
                    switch(currentZoomlevel) {
                        case 0:     pCursor = pGFXManager->getUIGraphic(UI_CursorMove_Zoomlevel0); break;
                        case 1:     pCursor = pGFXManager->getUIGraphic(UI_CursorMove_Zoomlevel1); break;
                        case 2:
                        default:    pCursor = pGFXManager->getUIGraphic(UI_CursorMove_Zoomlevel2); break;
                    }

                    dest = calcDrawingRect(pCursor, drawnMouseX, drawnMouseY, HAlign::Center, VAlign::Center);
                } break;

                case CursorMode_Attack: {
                    switch(currentZoomlevel) {
                        case 0:     pCursor = pGFXManager->getUIGraphic(UI_CursorAttack_Zoomlevel0); break;
                        case 1:     pCursor = pGFXManager->getUIGraphic(UI_CursorAttack_Zoomlevel1); break;
                        case 2:
                        default:    pCursor = pGFXManager->getUIGraphic(UI_CursorAttack_Zoomlevel2); break;
                    }

                    dest = calcDrawingRect(pCursor, drawnMouseX, drawnMouseY, HAlign::Center, VAlign::Center);
                } break;

                case CursorMode_Capture: {
                    switch(currentZoomlevel) {
                        case 0:     pCursor = pGFXManager->getUIGraphic(UI_CursorCapture_Zoomlevel0); break;
                        case 1:     pCursor = pGFXManager->getUIGraphic(UI_CursorCapture_Zoomlevel1); break;
                        case 2:
                        default:    pCursor = pGFXManager->getUIGraphic(UI_CursorCapture_Zoomlevel2); break;
                    }

                    dest = calcDrawingRect(pCursor, drawnMouseX, drawnMouseY, HAlign::Center, VAlign::Bottom);

                    int xPos = INVALID_POS;
                    int yPos = INVALID_POS;

                    if(screenborder->isScreenCoordInsideMap(drawnMouseX, drawnMouseY) == true) {
                        xPos = screenborder->screen2MapX(drawnMouseX);
                        yPos = screenborder->screen2MapY(drawnMouseY);
                    } else if(isOnRadarView(drawnMouseX, drawnMouseY)) {
                        Coord position = pInterface->getRadarView().getWorldCoords(drawnMouseX - (sideBarPos.x + SIDEBAR_COLUMN_WIDTH), drawnMouseY - sideBarPos.y);

                        xPos = position.x / TILESIZE;
                        yPos = position.y / TILESIZE;
                    }

                    if((xPos != INVALID_POS) && (yPos != INVALID_POS)) {
                        const auto pTile = currentGameMap->getTile(xPos, yPos);

                        if(pTile->isExploredByTeam(pLocalHouse->getTeamID())) {

                            const auto pStructure = dynamic_cast<StructureBase*>(pTile->getGroundObject());

                            if((pStructure != nullptr) && (pStructure->canBeCaptured()) && (pStructure->getOwner()->getTeamID() != pLocalHouse->getTeamID())) {
                                dest.y += ((getGameCycleCount() / 10) % 5);
                            }
                        }
                    }

                } break;

                case CursorMode_CarryallDrop: {
                    switch(currentZoomlevel) {
                        case 0:     pCursor = pGFXManager->getUIGraphic(UI_CursorCarryallDrop_Zoomlevel0); break;
                        case 1:     pCursor = pGFXManager->getUIGraphic(UI_CursorCarryallDrop_Zoomlevel1); break;
                        case 2:
                        default:    pCursor = pGFXManager->getUIGraphic(UI_CursorCarryallDrop_Zoomlevel2); break;
                    }

                    dest = calcDrawingRect(pCursor, drawnMouseX, drawnMouseY, HAlign::Center, VAlign::Bottom);
                } break;


                default: {
                    THROW(std::runtime_error, "Game::drawCursor(): Unknown cursor mode");
                };
            }
        }
    }

    SDL_RenderCopy(renderer, pCursor, nullptr, &dest);
}

void Game::setupView() const
{
    int i = 0;
    int j = 0;
    int count = 0;

    //setup start location/view
    i = j = count = 0;

    for(const UnitBase* pUnit : unitList) {
        if((pUnit->getOwner() == pLocalHouse) && (pUnit->getItemID() != Unit_Sandworm)) {
            i += pUnit->getX();
            j += pUnit->getY();
            count++;
        }
    }

    for(const StructureBase* pStructure : structureList) {
        if(pStructure->getOwner() == pLocalHouse) {
            i += pStructure->getX();
            j += pStructure->getY();
            count++;
        }
    }

    if(count == 0) {
        i = currentGameMap->getSizeX()*TILESIZE/2-1;
        j = currentGameMap->getSizeY()*TILESIZE/2-1;
    } else {
        i = i*TILESIZE/count;
        j = j*TILESIZE/count;
    }

    screenborder->setNewScreenCenter(Coord(i,j));
}


void Game::runMainLoop() {
    SDL_Log("Starting game...");

    // add interface
    if(pInterface == nullptr) {
        pInterface = std::make_unique<GameInterface>();
        if(gameState == GameState::Loading) {
            // when loading a save game we set radar directly
            pInterface->getRadarView().setRadarMode(pLocalHouse->hasRadarOn());
        } else if(pLocalHouse->hasRadarOn()) {
            // when starting a new game we switch the radar on with an animation if appropriate
            pInterface->getRadarView().switchRadarMode(true);
        }
    }

    gameState = GameState::Running;

    //setup endlevel conditions
    finishedLevel = false;

    bShowTime = winFlags & WINLOSEFLAGS_TIMEOUT;

    // Check if a player has lost
    std::for_each(house.begin(), house.end(), [](std::unique_ptr<House>& h) { if (h && !h->isAlive()) h->lose(true); });

    if(bReplay) {
        cmdManager.setReadOnly(true);
    } else {
        char tmp[FILENAME_MAX];
        fnkdat("replay/auto.rpl", tmp, FILENAME_MAX, FNKDAT_USER | FNKDAT_CREAT);
        const std::string replayname(tmp);

        auto pStream = std::make_unique<OFileStream>();

        if (pStream->open(replayname)) {
            pStream->writeString(getLocalPlayerName());

            gameInitSettings.save(*pStream);

            // when this game was loaded we have to save the old commands to the replay file first
            cmdManager.save(*pStream);

            // flush stream
            pStream->flush();

            // now all new commands might be added
            cmdManager.setStream(std::move(pStream));
        }
        else
        {
            // Should we throw instead?
            // TODO: Report problem to user...?
            quitGame();
        }
    }

    if(pNetworkManager != nullptr) {
        pNetworkManager->setOnReceiveChatMessage(std::bind(&ChatManager::addChatMessage, &(pInterface->getChatManager()), std::placeholders::_1, std::placeholders::_2));
        pNetworkManager->setOnReceiveCommandList(std::bind(&CommandManager::addCommandList, &cmdManager, std::placeholders::_1, std::placeholders::_2));
        pNetworkManager->setOnReceiveSelectionList(std::bind(&Game::onReceiveSelectionList, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
        pNetworkManager->setOnPeerDisconnected(std::bind(&Game::onPeerDisconnected, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));

        cmdManager.setNetworkCycleBuffer( MILLI2CYCLES(pNetworkManager->getMaxPeerRoundTripTime()) + 5 );
    }

    // Change music to ingame music
    musicPlayer->changeMusic(MUSIC_PEACE);


    int     frameStart = SDL_GetTicks();
    int     frameTime = 0;
    int     numFrames = 0;

    //SDL_Log("Random Seed (GameCycle %d): 0x%0X", GameCycleCount, RandomGen.getSeed());

    //main game loop
    do {
        SDL_SetRenderTarget(renderer, screenTexture);

        // clear whole screen
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderClear(renderer);

        drawScreen();

        SDL_RenderPresent(renderer);

        SDL_SetRenderTarget(renderer, nullptr);
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderClear(renderer);
        SDL_RenderCopy(renderer, screenTexture, nullptr, nullptr);
        SDL_RenderPresent(renderer);

        const int frameEnd = SDL_GetTicks();

        if(frameEnd == frameStart) {
            SDL_Delay(1);
        }

        frameTime += frameEnd - frameStart; // find difference to get frametime
        frameStart = SDL_GetTicks();

        numFrames++;

        if (bShowFPS) {
            averageFrameTime = 0.99f * averageFrameTime + 0.01f * frameTime;
        }

        if(settings.video.frameLimit == true) {
            if(frameTime < 32) {
                SDL_Delay(32 - frameTime);
            }
        }

        if(finished) {
            // end timer for the ending message
            if(SDL_GetTicks() - finishedLevelTime > END_WAIT_TIME) {
                finishedLevel = true;
            }
        }

        if(takePeriodicalScreenshots && ((gameCycleCount % (MILLI2CYCLES(10*1000))) == 0)) {
            takeScreenshot();
        }


        while( (frameTime > getGameSpeed()) || (!finished && (gameCycleCount < skipToGameCycle)) )  {

            bool bWaitForNetwork = false;

            if(pNetworkManager != nullptr) {
                pNetworkManager->update();

                // test if we need to wait for data to arrive
                for(const auto& playername : pNetworkManager->getConnectedPeers()) {
                    auto pPlayer = dynamic_cast<HumanPlayer*>(getPlayerByName(playername));
                    if(pPlayer != nullptr) {
                        if(pPlayer->nextExpectedCommandsCycle <= gameCycleCount) {
                            //SDL_Log("Cycle %d: Waiting for player '%s' to send data for cycle %d...", GameCycleCount, pPlayer->getPlayername().c_str(), pPlayer->nextExpectedCommandsCycle);
                            bWaitForNetwork = true;
                        }
                    }
                }

                if(bWaitForNetwork == true) {
                    if(startWaitingForOtherPlayersTime == 0) {
                        // we just started waiting
                        startWaitingForOtherPlayersTime = SDL_GetTicks();
                    } else {
                        if(SDL_GetTicks() - startWaitingForOtherPlayersTime > 1000) {
                            // we waited for more than one second

                            if(pWaitingForOtherPlayers == nullptr) {
                                pWaitingForOtherPlayers = std::make_unique<WaitingForOtherPlayers>();
                                bMenu = true;
                            }
                        }
                    }

                    SDL_Delay(10);
                } else {
                    startWaitingForOtherPlayersTime = 0;
                    pWaitingForOtherPlayers.reset();
                }
            }

            doInput();
            pInterface->updateObjectInterface();

            if(pNetworkManager != nullptr) {
                if(bSelectionChanged) {
                    pNetworkManager->sendSelectedList(selectedList);

                    bSelectionChanged = false;
                }
            }

            if(pInGameMentat != nullptr) {
                pInGameMentat->update();
            }

            if(pWaitingForOtherPlayers != nullptr) {
                pWaitingForOtherPlayers->update();
            }

            cmdManager.update();

            if(!bWaitForNetwork && !bPause) {
                pInterface->getRadarView().update();
                cmdManager.executeCommands(gameCycleCount);

//              SDL_Log("cycle %d : %d", gameCycleCount, currentGame->randomGen.getSeed());

#ifdef TEST_SYNC
                // add every gamecycles one test sync command
                if(bReplay == false) {
                    cmdManager.addCommand(Command(pLocalPlayer->getPlayerID(), CMD_TEST_SYNC, randomGen.getSeed()));
                }
#endif

                std::for_each(house.begin(), house.end(), [](std::unique_ptr<House>& h) { if (h) h->update(); });

                screenborder->update();

                triggerManager.trigger(gameCycleCount);

                processObjects();

                if ((indicatorFrame != NONE_ID) && (--indicatorTimer <= 0)) {
                    indicatorTimer = indicatorTime;

                    if (++indicatorFrame > 2) {
                        indicatorFrame = NONE_ID;
                    }
                }

                gameCycleCount++;
            }

            if(gameCycleCount <= skipToGameCycle) {
                frameTime = 0;
            } else {
                if (frameTime > 200)
                    frameTime = 0;
                else
                    frameTime -= getGameSpeed();
            }
        }

        musicPlayer->musicCheck();  //if song has finished, start playing next one
    } while (!bQuitGame && !finishedLevel);//not sure if we need this extra bool



    // Game is finished

    if(bReplay == false && currentGame->won == true) {
        // save replay
        char tmp[FILENAME_MAX];

        std::string mapnameBase = getBasename(gameInitSettings.getFilename(), true);
        fnkdat(std::string("replay/" + mapnameBase + ".rpl").c_str(), tmp, FILENAME_MAX, FNKDAT_USER | FNKDAT_CREAT);
        std::string replayname(tmp);

        OFileStream replystream;
        replystream.open(replayname);
        replystream.writeString(getLocalPlayerName());
        gameInitSettings.save(replystream);
        cmdManager.save(replystream);
    }

    if(pNetworkManager != nullptr) {
        pNetworkManager->disconnect();
    }

    gameState = GameState::Deinitialize;
    SDL_Log("Game finished!");
}


void Game::resumeGame()
{
    bMenu = false;
    if(gameType != GameType::CustomMultiplayer) {
        bPause = false;
    }
}


void Game::onOptions()
{
    if(bReplay == true) {
        // don't show menu
        quitGame();
    } else {
        Uint32 color = SDL2RGB(palette[houseToPaletteIndex[pLocalHouse->getHouseID()] + 3]);
        pInGameMenu = std::make_unique<InGameMenu>((gameType == GameType::CustomMultiplayer), color);
        bMenu = true;
        pauseGame();
    }
}


void Game::onMentat()
{
    pInGameMentat = std::make_unique<MentatHelp>(pLocalHouse->getHouseID(), techLevel, gameInitSettings.getMission());
    bMenu = true;
    pauseGame();
}


GameInitSettings Game::getNextGameInitSettings()
{
    if(nextGameInitSettings.getGameType() != GameType::Invalid) {
        // return the prepared game init settings (load game or restart mission)
        return nextGameInitSettings;
    }

    switch(gameInitSettings.getGameType()) {
        case GameType::Campaign: {
            int currentMission = gameInitSettings.getMission();
            if(!won) {
                currentMission -= (currentMission >= 22) ? 1 : 3;
            }
            int nextMission = gameInitSettings.getMission();
            Uint32 alreadyPlayedRegions = gameInitSettings.getAlreadyPlayedRegions();
            if(currentMission >= -1) {
                // do map choice
                SDL_Log("Map Choice...");
                MapChoice mapChoice(gameInitSettings.getHouseID(), currentMission, alreadyPlayedRegions);
                mapChoice.showMenu();
                nextMission = mapChoice.getSelectedMission();
                alreadyPlayedRegions = mapChoice.getAlreadyPlayedRegions();
            }

            Uint32 alreadyShownTutorialHints = won ? pLocalPlayer->getAlreadyShownTutorialHints() : gameInitSettings.getAlreadyShownTutorialHints();
            return GameInitSettings(gameInitSettings, nextMission, alreadyPlayedRegions, alreadyShownTutorialHints);
        } break;

        default: {
            SDL_Log("Game::getNextGameInitClass(): Wrong gameType for next Game.");
            return GameInitSettings();
        } break;
    }
}


int Game::whatNext()
{
    if(whatNextParam != GAME_NOTHING) {
        int tmp = whatNextParam;
        whatNextParam = GAME_NOTHING;
        return tmp;
    }

    if(nextGameInitSettings.getGameType() != GameType::Invalid) {
        return GAME_LOAD;
    }

    switch(gameType) {
        case GameType::Campaign: {
            if(bQuitGame == true) {
                return GAME_RETURN_TO_MENU;
            } else if(won == true) {
                if(gameInitSettings.getMission() == 22) {
                    // there is no mission after this mission
                    whatNextParam = GAME_RETURN_TO_MENU;
                } else {
                    // there is a mission after this mission
                    whatNextParam = GAME_NEXTMISSION;
                }
                return GAME_DEBRIEFING_WIN;
            } else {
                // we need to play this mission again
                whatNextParam = GAME_NEXTMISSION;

                return GAME_DEBRIEFING_LOST;
            }
        } break;

        case GameType::Skirmish: {
            if(bQuitGame == true) {
                return GAME_RETURN_TO_MENU;
            } else if(won == true) {
                whatNextParam = GAME_RETURN_TO_MENU;
                return GAME_DEBRIEFING_WIN;
            } else {
                whatNextParam = GAME_RETURN_TO_MENU;
                return GAME_DEBRIEFING_LOST;
            }
        } break;

        case GameType::CustomGame:
        case GameType::CustomMultiplayer: {
            if(bQuitGame == true) {
                return GAME_RETURN_TO_MENU;
            } else {
                whatNextParam = GAME_RETURN_TO_MENU;
                return GAME_CUSTOM_GAME_STATS;
            }
        } break;

        default: {
            return GAME_RETURN_TO_MENU;
        } break;
    }
}


bool Game::loadSaveGame(const std::string& filename) {
    IFileStream fs;

    if(fs.open(filename) == false) {
        return false;
    }

    bool ret = loadSaveGame(fs);

    fs.close();

    return ret;
}

bool Game::loadSaveGame(InputStream& stream) {
    gameState = GameState::Loading;

    Uint32 magicNum = stream.readUint32();
    if (magicNum != SAVEMAGIC) {
        SDL_Log("Game::loadSaveGame(): No valid savegame! Expected magic number %.8X, but got %.8X!", SAVEMAGIC, magicNum);
        return false;
    }

    Uint32 savegameVersion = stream.readUint32();
    if (savegameVersion != SAVEGAMEVERSION) {
        SDL_Log("Game::loadSaveGame(): No valid savegame! Expected savegame version %d, but got %d!", SAVEGAMEVERSION, savegameVersion);
        return false;
    }

    std::string duneVersion = stream.readString();

    // if this is a multiplayer load we need to save some information before we overwrite gameInitSettings with the settings saved in the savegame
    bool bMultiplayerLoad = (gameInitSettings.getGameType() == GameType::LoadMultiplayer);
    GameInitSettings::HouseInfoList oldHouseInfoList = gameInitSettings.getHouseInfoList();

    // read gameInitSettings
    gameInitSettings = GameInitSettings(stream);

    // read the actual house setup choosen at the beginning of the game
    Uint32 numHouseInfo = stream.readUint32();
    for(Uint32 i=0;i<numHouseInfo;i++) {
        houseInfoListSetup.push_back(GameInitSettings::HouseInfo(stream));
    }

    //read map size
    const short mapSizeX = stream.readUint32();
    const short mapSizeY = stream.readUint32();

    //create the new map
    currentGameMap = new Map(mapSizeX, mapSizeY);

    //read GameCycleCount
    gameCycleCount = stream.readUint32();

    // read some settings
    gameType = static_cast<GameType>(stream.readSint8());
    techLevel = stream.readUint8();
    randomGen.setSeed(stream.readUint32());

    // read in the unit/structure data
    objectData.load(stream);

    //load the house(s) info
    for(auto i=0; i<NUM_HOUSES; i++) {
        if (stream.readBool() == true) {
            //house in game
            house[i] = std::make_unique<House>(stream);
        }
    }

    // we have to set the local player
    if(bMultiplayerLoad) {
        // get it from the gameInitSettings that started the game (not the one saved in the savegame)
        for(const auto& houseInfo : oldHouseInfoList) {

            // find the right house
            for(int i=0;i<NUM_HOUSES;i++) {
                if((house[i] != nullptr) && (house[i]->getHouseID() == houseInfo.houseID)) {
                    // iterate over all players
                    auto& players = house[i]->getPlayerList();
                    auto playerIter = players.cbegin();
                    for(const auto& playerInfo : houseInfo.playerInfoList) {
                        if(playerInfo.playerClass == HUMANPLAYERCLASS) {
                            while(playerIter != players.cend()) {

                                const auto pHumanPlayer = dynamic_cast<HumanPlayer*>(playerIter->get());
                                if(pHumanPlayer) {
                                    // we have actually found a human player and now assign the first unused name to it
                                    unregisterPlayer(pHumanPlayer);
                                    pHumanPlayer->setPlayername(playerInfo.playerName);
                                    registerPlayer(pHumanPlayer);

                                    if(playerInfo.playerName == getLocalPlayerName()) {
                                        pLocalHouse = house[i].get();
                                        pLocalPlayer = pHumanPlayer;
                                    }

                                    ++playerIter;
                                    break;
                                } else {
                                    ++playerIter;
                                }
                            }
                        }
                    }
                }
            }
        }
    } else {
        // it is stored in the savegame, so set it up
        const auto localPlayerID = stream.readUint8();
        pLocalPlayer = dynamic_cast<HumanPlayer*>(getPlayerByID(localPlayerID));
        pLocalHouse = house[pLocalPlayer->getHouse()->getHouseID()].get();
    }

    debug = stream.readBool();
    bCheatsEnabled = stream.readBool();

    winFlags = stream.readUint32();
    loseFlags = stream.readUint32();

    currentGameMap->load(stream);

    //load the structures and units
    objectManager.load(stream);

    int numBullets = stream.readUint32();
    for(int i = 0; i < numBullets; i++) {
        bulletList.push_back(new Bullet(stream));
    }

    int numExplosions = stream.readUint32();
    for(int i = 0; i < numExplosions; i++) {
        addExplosion(stream);
    }

    if(bMultiplayerLoad) {
        screenborder->adjustScreenBorderToMapsize(currentGameMap->getSizeX(), currentGameMap->getSizeY());

        screenborder->setNewScreenCenter(pLocalHouse->getCenterOfMainBase()*TILESIZE);

    } else {
        //load selection list
        selectedList = stream.readUint32Set();

        //load the screenborder info
        screenborder->adjustScreenBorderToMapsize(currentGameMap->getSizeX(), currentGameMap->getSizeY());
        screenborder->load(stream);
    }

    // load triggers
    triggerManager.load(stream);

    // CommandManager is at the very end of the file. DO NOT CHANGE THIS!
    cmdManager.load(stream);

    finished = false;

    return true;
}


bool Game::saveGame(const std::string& filename)
{
    OFileStream fs;

    if(fs.open(filename) == false) {
        SDL_Log("Game::saveGame(): %s", strerror(errno));
        currentGame->addToNewsTicker(std::string("Game NOT saved: Cannot open \"") + filename + "\".");
        return false;
    }

    fs.writeUint32(SAVEMAGIC);

    fs.writeUint32(SAVEGAMEVERSION);

    fs.writeString(VERSIONSTRING);

    // write gameInitSettings
    gameInitSettings.save(fs);

    fs.writeUint32(houseInfoListSetup.size());
    for(const GameInitSettings::HouseInfo& houseInfo : houseInfoListSetup) {
        houseInfo.save(fs);
    }

    //write the map size
    fs.writeUint32(currentGameMap->getSizeX());
    fs.writeUint32(currentGameMap->getSizeY());

    // write GameCycleCount
    fs.writeUint32(gameCycleCount);

    // write some settings
    fs.writeSint8(static_cast<Sint8>(gameType));
    fs.writeUint8(techLevel);
    fs.writeUint32(randomGen.getSeed());

    // write out the unit/structure data
    objectData.save(fs);

    //write the house(s) info
    for(int i=0; i<NUM_HOUSES; i++) {
        fs.writeBool(house[i] != nullptr);

        if(house[i] != nullptr) {
            house[i]->save(fs);
        }
    }

    if(gameInitSettings.getGameType() != GameType::CustomMultiplayer) {
        fs.writeUint8(pLocalPlayer->getPlayerID());
    }

    fs.writeBool(debug);
    fs.writeBool(bCheatsEnabled);

    fs.writeUint32(winFlags);
    fs.writeUint32(loseFlags);

    currentGameMap->save(fs);

    // save the structures and units
    objectManager.save(fs);

    fs.writeUint32(bulletList.size());
    for(const Bullet* pBullet : bulletList) {
        pBullet->save(fs);
    }

    fs.writeUint32(explosionList.size());
    for(const auto& pExplosion : explosionList) {
        pExplosion->save(fs);
    }

    if(gameInitSettings.getGameType() != GameType::CustomMultiplayer) {
        // save selection lists

        // write out selected units list
        fs.writeUint32Set(selectedList);

        // write the screenborder info
        screenborder->save(fs);
    }

    // save triggers
    triggerManager.save(fs);

    // CommandManager is at the very end of the file. DO NOT CHANGE THIS!
    cmdManager.save(fs);

    fs.close();

    return true;
}


void Game::saveObject(OutputStream& stream, ObjectBase* obj) const
{
    if(obj == nullptr)
        return;

    stream.writeUint32(obj->getItemID());
    obj->save(stream);
}


ObjectBase* Game::loadObject(InputStream& stream, Uint32 objectID) const
{
    const auto itemID = stream.readUint32();

    const auto newObject = ObjectBase::loadObject(stream, itemID, objectID);
    if(newObject == nullptr) {
        THROW(std::runtime_error, "Error while loading an object!");
    }

    return newObject;
}


void Game::selectAll(const Dune::selected_set_type& aList) const
{
    for(auto objectID : aList) {
        objectManager.getObject(objectID)->setSelected(true);
    }
}


void Game::unselectAll(const Dune::selected_set_type& aList) const
{
    for(auto objectID : aList) {
        objectManager.getObject(objectID)->setSelected(false);
    }
}

void Game::onReceiveSelectionList(const std::string& name, const Dune::selected_set_type& newSelectionList, int groupListIndex)
{
    auto pHumanPlayer = dynamic_cast<HumanPlayer*>(getPlayerByName(name));

    if(pHumanPlayer == nullptr) {
        return;
    }

    if(groupListIndex == -1) {
        // the other player controlling the same house has selected some units

        if(pHumanPlayer->getHouse() != pLocalHouse) {
            return;
        }

        for(auto objectID : selectedByOtherPlayerList) {
            auto pObject = objectManager.getObject(objectID);
            if(pObject != nullptr) {
                pObject->setSelectedByOtherPlayer(false);
            }
        }

        selectedByOtherPlayerList = newSelectionList;

        for(Uint32 objectID : selectedByOtherPlayerList) {
            ObjectBase* pObject = objectManager.getObject(objectID);
            if(pObject != nullptr) {
                pObject->setSelectedByOtherPlayer(true);
            }
        }
    } else {
        // some other player has assigned a number to a list of units
        pHumanPlayer->setGroupList(groupListIndex, newSelectionList);
    }
}

void Game::onPeerDisconnected(const std::string& name, bool bHost, int cause) const {
    pInterface->getChatManager().addInfoMessage(name + " disconnected!");
}

void Game::setGameWon() {
    if(!bQuitGame && !finished) {
        won = true;
        finished = true;
        finishedLevelTime = SDL_GetTicks();
        soundPlayer->playVoice(YourMissionIsComplete,pLocalHouse->getHouseID());
    }
}


void Game::setGameLost() {
    if(!bQuitGame && !finished) {
        won = false;
        finished = true;
        finishedLevelTime = SDL_GetTicks();
        soundPlayer->playVoice(YouHaveFailedYourMission,pLocalHouse->getHouseID());
    }
}


bool Game::onRadarClick(Coord worldPosition, bool bRightMouseButton, bool bDrag) {
    if(bRightMouseButton) {

        if(handleSelectedObjectsActionClick(worldPosition.x / TILESIZE, worldPosition.y / TILESIZE)) {
            indicatorFrame = 0;
            indicatorPosition = worldPosition;
        }

        return false;
    } else {

        if(bDrag) {
            screenborder->setNewScreenCenter(worldPosition);
            return true;
        } else {

            switch(currentCursorMode) {
                case CursorMode_Attack: {
                    handleSelectedObjectsAttackClick(worldPosition.x / TILESIZE, worldPosition.y / TILESIZE);
                    return false;
                } break;

                case CursorMode_Move: {
                    handleSelectedObjectsMoveClick(worldPosition.x / TILESIZE, worldPosition.y / TILESIZE);
                    return false;
                } break;

                case CursorMode_Capture: {
                    handleSelectedObjectsCaptureClick(worldPosition.x / TILESIZE, worldPosition.y / TILESIZE);
                    return false;
                } break;

                case CursorMode_CarryallDrop: {
                    handleSelectedObjectsRequestCarryallDropClick(worldPosition.x / TILESIZE, worldPosition.y / TILESIZE);
                    return false;
                } break;

                case CursorMode_Normal:
                default: {
                    screenborder->setNewScreenCenter(worldPosition);
                    return true;
                } break;
            }
        }
    }
}


bool Game::isOnRadarView(int mouseX, int mouseY) const {
    return pInterface->getRadarView().isOnRadar(mouseX - (sideBarPos.x + SIDEBAR_COLUMN_WIDTH), mouseY - sideBarPos.y);
}


void Game::handleChatInput(SDL_KeyboardEvent& keyboardEvent) {
    if(keyboardEvent.keysym.sym == SDLK_ESCAPE) {
        chatMode = false;
    } else if(keyboardEvent.keysym.sym == SDLK_RETURN) {
        if(typingChatMessage.length() > 0) {
            unsigned char md5sum[16];

            md5(reinterpret_cast<const unsigned char*>(typingChatMessage.c_str()), typingChatMessage.size(), md5sum);

            std::stringstream md5stream;
            md5stream << std::setfill('0') << std::hex << std::uppercase << "0x";
            for(int i : md5sum) {
                md5stream << std::setw(2) << i;
            }

            const std::string md5string = md5stream.str();

            if((bCheatsEnabled == false) && (md5string == "0xB8766C8EC7A61036B69893FC17AAF21E")) {
                bCheatsEnabled = true;
                pInterface->getChatManager().addInfoMessage("Cheat mode enabled");
            } else if((bCheatsEnabled == true) && (md5string == "0xB8766C8EC7A61036B69893FC17AAF21E")) {
                pInterface->getChatManager().addInfoMessage("Cheat mode already enabled");
            } else if((bCheatsEnabled == true) && (md5string == "0x57583291CB37F8167EDB0611D8D19E58")) {
                if (gameType != GameType::CustomMultiplayer) {
                    pInterface->getChatManager().addInfoMessage("You win this game");
                    setGameWon();
                }
            } else if((bCheatsEnabled == true) && (md5string == "0x1A12BE3DBE54C5A504CAA6EE9782C1C8")) {
                if(debug == true) {
                    pInterface->getChatManager().addInfoMessage("You are already in debug mode");
                } else if (gameType != GameType::CustomMultiplayer) {
                    pInterface->getChatManager().addInfoMessage("Debug mode enabled");
                    debug = true;
                }
            } else if((bCheatsEnabled == true) && (md5string == "0x54F68155FC64A5BC66DCD50C1E925C0B")) {
                if(debug == false) {
                    pInterface->getChatManager().addInfoMessage("You are not in debug mode");
                } else if (gameType != GameType::CustomMultiplayer) {
                    pInterface->getChatManager().addInfoMessage("Debug mode disabled");
                    debug = false;
                }
            } else if((bCheatsEnabled == true) && (md5string == "0xCEF1D26CE4B145DE985503CA35232ED8")) {
                if (gameType != GameType::CustomMultiplayer) {
                    pInterface->getChatManager().addInfoMessage("You got some credits");
                    pLocalHouse->returnCredits(10000);
                }
            } else {
                if(pNetworkManager != nullptr) {
                    pNetworkManager->sendChatMessage(typingChatMessage);
                }
                pInterface->getChatManager().addChatMessage(getLocalPlayerName(), typingChatMessage);
            }
        }

        chatMode = false;
    } else if(keyboardEvent.keysym.sym == SDLK_BACKSPACE) {
        if(typingChatMessage.length() > 0) {
            typingChatMessage = utf8Substr(typingChatMessage, 0, utf8Length(typingChatMessage) - 1);
        }
    }
}


void Game::handleKeyInput(SDL_KeyboardEvent& keyboardEvent) {
    switch(keyboardEvent.keysym.sym) {

        case SDLK_0: {
            //if ctrl and 0 remove selected units from all groups
            if(SDL_GetModState() & KMOD_CTRL) {
                for(Uint32 objectID : selectedList) {
                    ObjectBase* pObject = objectManager.getObject(objectID);
                    pObject->setSelected(false);
                    pObject->removeFromSelectionLists();
                    for(int i=0; i < NUMSELECTEDLISTS; i++) {
                        pLocalPlayer->getGroupList(i).erase(objectID);
                    }
                }
                selectedList.clear();
                currentGame->selectionChanged();
                currentCursorMode = CursorMode_Normal;
            } else {
                for(Uint32 objectID : selectedList) {
                    objectManager.getObject(objectID)->setSelected(false);
                }
                selectedList.clear();
                currentGame->selectionChanged();
                currentCursorMode = CursorMode_Normal;
            }
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
            //for SDLK_1 to SDLK_9 select group with that number, if ctrl create group from selected obj
            const auto selectListIndex = keyboardEvent.keysym.sym - SDLK_1;

            if(SDL_GetModState() & KMOD_CTRL) {
                pLocalPlayer->setGroupList(selectListIndex, selectedList);

                pInterface->updateObjectInterface();
            } else {
                auto& groupList = pLocalPlayer->getGroupList(selectListIndex);

                // find out if we are choosing a group with all items already selected
                bool bEverythingWasSelected = (selectedList.size() == groupList.size());
                Coord averagePosition;
                for(auto objectID : groupList) {
                    auto pObject = objectManager.getObject(objectID);
                    bEverythingWasSelected = bEverythingWasSelected && pObject->isSelected();
                    averagePosition += pObject->getLocation();
                }

                if(groupList.empty() == false) {
                    averagePosition /= groupList.size();
                }


                if(SDL_GetModState() & KMOD_SHIFT) {
                    // we add the items from this list to the list of selected items
                } else {
                    // we replace the list of the selected items with the items from this list
                    unselectAll(selectedList);
                    selectedList.clear();
                    currentGame->selectionChanged();
                }

                // now we add the selected items
                for(auto objectID : groupList) {
                    auto pObject = objectManager.getObject(objectID);
                    if(pObject->getOwner() == pLocalHouse) {
                        pObject->setSelected(true);
                        selectedList.insert(pObject->getObjectID());
                        currentGame->selectionChanged();
                    }
                }

                if(bEverythingWasSelected && (groupList.empty() == false)) {
                    // we center around the newly selected units/structures
                    screenborder->setNewScreenCenter(averagePosition*TILESIZE);
                }
            }
            currentCursorMode = CursorMode_Normal;
        } break;

        case SDLK_KP_MINUS:
        case SDLK_MINUS: {
            if(gameType != GameType::CustomMultiplayer) {
                settings.gameOptions.gameSpeed = std::min(settings.gameOptions.gameSpeed+1,GAMESPEED_MAX);
                INIFile myINIFile(getConfigFilepath());
                myINIFile.setIntValue("Game Options","Game Speed", settings.gameOptions.gameSpeed);
                myINIFile.saveChangesTo(getConfigFilepath());
                currentGame->addToNewsTicker(fmt::sprintf(_("Game speed") + ": %d", settings.gameOptions.gameSpeed));
            }
        } break;

        case SDLK_KP_PLUS:
        case SDLK_PLUS:
        case SDLK_EQUALS: {
            if(gameType != GameType::CustomMultiplayer) {
                settings.gameOptions.gameSpeed = std::max(settings.gameOptions.gameSpeed-1,GAMESPEED_MIN);
                INIFile myINIFile(getConfigFilepath());
                myINIFile.setIntValue("Game Options","Game Speed", settings.gameOptions.gameSpeed);
                myINIFile.saveChangesTo(getConfigFilepath());
                currentGame->addToNewsTicker(fmt::sprintf(_("Game speed") + ": %d", settings.gameOptions.gameSpeed));
            }
        } break;

        case SDLK_c: {
            //set object to capture
            if(currentCursorMode != CursorMode_Capture) {
                for(Uint32 objectID : selectedList) {
                    ObjectBase* pObject = objectManager.getObject(objectID);
                    if(pObject->isAUnit() && (pObject->getOwner() == pLocalHouse) && pObject->isRespondable() && pObject->canAttack() && pObject->isInfantry()) {
                        currentCursorMode = CursorMode_Capture;
                        break;
                    }
                }
            }
        } break;

        case SDLK_a: {
            //set object to attack
            if(currentCursorMode != CursorMode_Attack) {
                for(Uint32 objectID : selectedList) {
                    ObjectBase* pObject = objectManager.getObject(objectID);
                    House* pOwner = pObject->getOwner();
                    if(pObject->isAUnit() && (pOwner == pLocalHouse) && pObject->isRespondable() && pObject->canAttack()) {
                        currentCursorMode = CursorMode_Attack;
                        break;
                    } else if((pObject->getItemID() == Structure_Palace) && ((pOwner->getHouseID() == HOUSE_HARKONNEN) || (pOwner->getHouseID() == HOUSE_SARDAUKAR))) {
                        if(static_cast<Palace*>(pObject)->isSpecialWeaponReady()) {
                            currentCursorMode = CursorMode_Attack;
                            break;
                        }
                    }
                }
            }
        } break;

        case SDLK_t: {
            bShowTime = !bShowTime;
        } break;

        case SDLK_ESCAPE: {
            onOptions();
        } break;

        case SDLK_F1: {
            const auto oldCenterCoord = screenborder->getCurrentCenter();
            currentZoomlevel = 0;
            screenborder->adjustScreenBorderToMapsize(currentGameMap->getSizeX(), currentGameMap->getSizeY());
            screenborder->setNewScreenCenter(oldCenterCoord);
        } break;

        case SDLK_F2: {
            const auto oldCenterCoord = screenborder->getCurrentCenter();
            currentZoomlevel = 1;
            screenborder->adjustScreenBorderToMapsize(currentGameMap->getSizeX(), currentGameMap->getSizeY());
            screenborder->setNewScreenCenter(oldCenterCoord);
        } break;

        case SDLK_F3: {
            const auto oldCenterCoord = screenborder->getCurrentCenter();
            currentZoomlevel = 2;
            screenborder->adjustScreenBorderToMapsize(currentGameMap->getSizeX(), currentGameMap->getSizeY());
            screenborder->setNewScreenCenter(oldCenterCoord);
        } break;

        case SDLK_F4: {
            // skip a 30 seconds
            if(gameType != GameType::CustomMultiplayer || bReplay) {
                skipToGameCycle = gameCycleCount + (10*1000)/GAMESPEED_DEFAULT;
            }
        } break;

        case SDLK_F5: {
            // skip a 30 seconds
            if(gameType != GameType::CustomMultiplayer || bReplay) {
                skipToGameCycle = gameCycleCount + (30*1000)/GAMESPEED_DEFAULT;
            }
        } break;

        case SDLK_F6: {
            // skip 2 minutes
            if(gameType != GameType::CustomMultiplayer || bReplay) {
                skipToGameCycle = gameCycleCount + (120*1000)/GAMESPEED_DEFAULT;
            }
        } break;

        case SDLK_F10: {
            soundPlayer->toggleSound();
        } break;

        case SDLK_F11: {
            musicPlayer->toggleSound();
        } break;

        case SDLK_F12: {
            bShowFPS = !bShowFPS;
        } break;

        case SDLK_m: {
            //set object to move
            if(currentCursorMode != CursorMode_Move) {
                for(const auto objectID : selectedList) {
                    const auto pObject = objectManager.getObject(objectID);
                    if(pObject->isAUnit() && (pObject->getOwner() == pLocalHouse) && pObject->isRespondable()) {
                        currentCursorMode = CursorMode_Move;
                        break;
                    }
                }
            }
        } break;

        case SDLK_g: {
            // select next construction yard
            Dune::selected_set_type itemIDs;
            itemIDs.insert(Structure_ConstructionYard);
            selectNextStructureOfType(itemIDs);
        } break;

        case SDLK_f: {
            // select next factory
            Dune::selected_set_type itemIDs;
            itemIDs.insert(Structure_Barracks);
            itemIDs.insert(Structure_WOR);
            itemIDs.insert(Structure_LightFactory);
            itemIDs.insert(Structure_HeavyFactory);
            itemIDs.insert(Structure_HighTechFactory);
            itemIDs.insert(Structure_StarPort);
            selectNextStructureOfType(itemIDs);
        } break;

        case SDLK_p: {
            if(SDL_GetModState() & KMOD_CTRL) {
                // fall through to SDLK_PRINT
            } else {
                // Place structure
                if(selectedList.size() == 1) {
                    ConstructionYard* pConstructionYard = dynamic_cast<ConstructionYard*>(objectManager.getObject(*selectedList.begin()));
                    if(pConstructionYard != nullptr) {
                        if(currentCursorMode == CursorMode_Placing) {
                            currentCursorMode = CursorMode_Normal;
                        } else if(pConstructionYard->isWaitingToPlace()) {
                            currentCursorMode = CursorMode_Placing;
                        }
                    }
                }

                break;  // do not fall through
            }

        } // fall through

        case SDLK_PRINTSCREEN:
        case SDLK_SYSREQ: {
            if(SDL_GetModState() & KMOD_SHIFT) {
                takePeriodicalScreenshots = !takePeriodicalScreenshots;
            } else {
                takeScreenshot();
            }
        } break;

        case SDLK_h: {
            for(Uint32 objectID : selectedList) {
                ObjectBase* pObject = objectManager.getObject(objectID);
                if(pObject->getItemID() == Unit_Harvester) {
                    static_cast<Harvester*>(pObject)->handleReturnClick();
                }
            }
        } break;


        case SDLK_r: {
            for(Uint32 objectID : selectedList) {
                ObjectBase* pObject = objectManager.getObject(objectID);
                if(pObject->isAStructure()) {
                    static_cast<StructureBase*>(pObject)->handleRepairClick();
                } else if(pObject->isAGroundUnit() && pObject->getHealth() < pObject->getMaxHealth()) {
                    static_cast<GroundUnit*>(pObject)->handleSendToRepairClick();
                }
            }
        } break;


        case SDLK_d: {
            if(currentCursorMode != CursorMode_CarryallDrop){
                for(Uint32 objectID : selectedList) {
                    ObjectBase* pObject = objectManager.getObject(objectID);
                    if(pObject->isAGroundUnit() && pObject->getOwner()->hasCarryalls()) {
                        currentCursorMode = CursorMode_CarryallDrop;
                    }
                }
            }

        } break;

        case SDLK_u: {
            for(Uint32 objectID : selectedList) {
                ObjectBase* pObject = objectManager.getObject(objectID);
                if(pObject->isABuilder()) {
                    BuilderBase* pBuilder = static_cast<BuilderBase*>(pObject);
                    if(pBuilder->getHealth() >= pBuilder->getMaxHealth() && pBuilder->isAllowedToUpgrade()) {
                        pBuilder->handleUpgradeClick();
                    }
                }
            }
        } break;

        case SDLK_RETURN: {
            if(SDL_GetModState() & KMOD_ALT) {
                toogleFullscreen();
            } else {
                typingChatMessage = "";
                chatMode = true;
            }
        } break;

        case SDLK_TAB: {
            if(SDL_GetModState() & KMOD_ALT) {
                SDL_MinimizeWindow(window);
            }
        } break;

        case SDLK_SPACE: {
            if(gameType != GameType::CustomMultiplayer) {
                if(bPause) {
                    resumeGame();
                    pInterface->getChatManager().addInfoMessage(_("Game resumed!"));
                } else {
                    pauseGame();
                    pInterface->getChatManager().addInfoMessage(_("Game paused!"));
                }
            }
        } break;

        default: {
        } break;
    }
}


bool Game::handlePlacementClick(int xPos, int yPos) {
    BuilderBase* pBuilder = nullptr;

    if(selectedList.size() == 1) {
        pBuilder = dynamic_cast<BuilderBase*>(objectManager.getObject(*selectedList.begin()));
    }

    if(!pBuilder) {
        return false;
    }

    int placeItem = pBuilder->getCurrentProducedItem();
    Coord structuresize = getStructureSize(placeItem);

    if(placeItem == Structure_Slab1) {
        if((currentGameMap->isWithinBuildRange(xPos, yPos, pBuilder->getOwner()))
            && (currentGameMap->okayToPlaceStructure(xPos, yPos, 1, 1, false, pBuilder->getOwner()))
            && (currentGameMap->getTile(xPos, yPos)->isConcrete() == false)) {
            getCommandManager().addCommand(Command(pLocalPlayer->getPlayerID(), CMD_PLACE_STRUCTURE,pBuilder->getObjectID(), xPos, yPos));
            //the user has tried to place and has been successful
            soundPlayer->playSound(Sound_PlaceStructure);
            currentCursorMode = CursorMode_Normal;
            return true;
        } else {
            //the user has tried to place but clicked on impossible point
            currentGame->addToNewsTicker(_("@DUNE.ENG|135#Cannot place slab here."));
            soundPlayer->playSound(Sound_InvalidAction);    //can't place noise
            return false;
        }
    } else if(placeItem == Structure_Slab4) {
        if( (currentGameMap->isWithinBuildRange(xPos, yPos, pBuilder->getOwner()) || currentGameMap->isWithinBuildRange(xPos+1, yPos, pBuilder->getOwner())
                || currentGameMap->isWithinBuildRange(xPos+1, yPos+1, pBuilder->getOwner()) || currentGameMap->isWithinBuildRange(xPos, yPos+1, pBuilder->getOwner()))
            && ((currentGameMap->okayToPlaceStructure(xPos, yPos, 1, 1, false, pBuilder->getOwner())
                || currentGameMap->okayToPlaceStructure(xPos+1, yPos, 1, 1, false, pBuilder->getOwner())
                || currentGameMap->okayToPlaceStructure(xPos+1, yPos+1, 1, 1, false, pBuilder->getOwner())
                || currentGameMap->okayToPlaceStructure(xPos, yPos, 1, 1+1, false, pBuilder->getOwner())))
            && ((currentGameMap->getTile(xPos, yPos)->isConcrete() == false) || (currentGameMap->getTile(xPos+1, yPos)->isConcrete() == false)
                || (currentGameMap->getTile(xPos, yPos+1)->isConcrete() == false) || (currentGameMap->getTile(xPos+1, yPos+1)->isConcrete() == false)) ) {

            getCommandManager().addCommand(Command(pLocalPlayer->getPlayerID(), CMD_PLACE_STRUCTURE,pBuilder->getObjectID(), xPos, yPos));
            //the user has tried to place and has been successful
            soundPlayer->playSound(Sound_PlaceStructure);
            currentCursorMode = CursorMode_Normal;
            return true;
        } else {
            //the user has tried to place but clicked on impossible point
            currentGame->addToNewsTicker(_("@DUNE.ENG|135#Cannot place slab here."));
            soundPlayer->playSound(Sound_InvalidAction);    //can't place noise
            return false;
        }
    } else {
        if(currentGameMap->okayToPlaceStructure(xPos, yPos, structuresize.x, structuresize.y, false, pBuilder->getOwner())) {
            getCommandManager().addCommand(Command(pLocalPlayer->getPlayerID(), CMD_PLACE_STRUCTURE,pBuilder->getObjectID(), xPos, yPos));
            //the user has tried to place and has been successful
            soundPlayer->playSound(Sound_PlaceStructure);
            currentCursorMode = CursorMode_Normal;
            return true;
        } else {
            //the user has tried to place but clicked on impossible point
            currentGame->addToNewsTicker(fmt::sprintf(_("@DUNE.ENG|134#Cannot place %%s here."), resolveItemName(placeItem)));
            soundPlayer->playSound(Sound_InvalidAction);    //can't place noise

            // is this building area only blocked by units?
            if(currentGameMap->okayToPlaceStructure(xPos, yPos, structuresize.x, structuresize.y, false, pBuilder->getOwner(), true)) {
                // then we try to move all units outside the building area

                // generate a independent temporal random number generator as we are in input handling code (and outside game logic code)
                Random tempRandomGen(getGameCycleCount());

                for(int y = yPos; y < yPos + structuresize.y; y++) {
                    for(int x = xPos; x < xPos + structuresize.x; x++) {
                        Tile* pTile = currentGameMap->getTile(x,y);
                        if(pTile->hasANonInfantryGroundObject()) {
                            ObjectBase* pObject = pTile->getNonInfantryGroundObject();
                            if(pObject->isAUnit() && pObject->getOwner() == pBuilder->getOwner()) {
                                UnitBase* pUnit = static_cast<UnitBase*>(pObject);
                                Coord newDestination = currentGameMap->findDeploySpot(pUnit, Coord(xPos, yPos), tempRandomGen, pUnit->getLocation(), structuresize);
                                pUnit->handleMoveClick(newDestination.x, newDestination.y);
                            }
                        } else if(pTile->hasInfantry()) {
                            for(Uint32 objectID : pTile->getInfantryList()) {
                                InfantryBase* pInfantry = dynamic_cast<InfantryBase*>(getObjectManager().getObject(objectID));
                                if((pInfantry != nullptr) && (pInfantry->getOwner() == pBuilder->getOwner())) {
                                    Coord newDestination = currentGameMap->findDeploySpot(pInfantry, Coord(xPos, yPos), tempRandomGen, pInfantry->getLocation(), structuresize);
                                    pInfantry->handleMoveClick(newDestination.x, newDestination.y);
                                }
                            }
                        }
                    }
                }
            }

            return false;
        }
    }
}


bool Game::handleSelectedObjectsAttackClick(int xPos, int yPos) {
    UnitBase* pResponder = nullptr;
    for(Uint32 objectID : selectedList) {
        ObjectBase* pObject = objectManager.getObject(objectID);
        House* pOwner = pObject->getOwner();
        if(pObject->isAUnit() && (pOwner == pLocalHouse) && pObject->isRespondable()) {
            pResponder = static_cast<UnitBase*>(pObject);
            pResponder->handleAttackClick(xPos,yPos);
        } else if((pObject->getItemID() == Structure_Palace) && ((pOwner->getHouseID() == HOUSE_HARKONNEN) || (pOwner->getHouseID() == HOUSE_SARDAUKAR))) {
            Palace* pPalace = static_cast<Palace*>(pObject);
            if(pPalace->isSpecialWeaponReady()) {
                pPalace->handleDeathhandClick(xPos, yPos);
            }
        }
    }

    currentCursorMode = CursorMode_Normal;
    if(pResponder) {
        pResponder->playConfirmSound();
        return true;
    } else {
        return false;
    }
}

bool Game::handleSelectedObjectsMoveClick(int xPos, int yPos) {
    UnitBase* pResponder = nullptr;

    for(Uint32 objectID : selectedList) {
        ObjectBase* pObject = objectManager.getObject(objectID);
        if (pObject->isAUnit() && (pObject->getOwner() == pLocalHouse) && pObject->isRespondable()) {
            pResponder = static_cast<UnitBase*>(pObject);
            pResponder->handleMoveClick(xPos,yPos);
        }
    }

    currentCursorMode = CursorMode_Normal;
    if(pResponder) {
        pResponder->playConfirmSound();
        return true;
    } else {
        return false;
    }
}

/**
    New method for transporting units quickly using carryalls
**/
bool Game::handleSelectedObjectsRequestCarryallDropClick(int xPos, int yPos) {

    UnitBase* pResponder = nullptr;

    /*
        If manual carryall mode isn't enabled then turn this off...
    */
    if(!getGameInitSettings().getGameOptions().manualCarryallDrops) {
        currentCursorMode = CursorMode_Normal;
        return false;
    }

    for(Uint32 objectID : selectedList) {
        ObjectBase* pObject = objectManager.getObject(objectID);
        if (pObject->isAGroundUnit() && (pObject->getOwner() == pLocalHouse) && pObject->isRespondable()) {
            pResponder = static_cast<UnitBase*>(pObject);
            pResponder->handleRequestCarryallDropClick(xPos,yPos);
        }
    }

    currentCursorMode = CursorMode_Normal;
    if(pResponder) {
        pResponder->playConfirmSound();
        return true;
    } else {
        return false;
    }
}



bool Game::handleSelectedObjectsCaptureClick(int xPos, int yPos) {
    Tile* pTile = currentGameMap->getTile(xPos, yPos);

    if(pTile == nullptr) {
        return false;
    }

    StructureBase* pStructure = dynamic_cast<StructureBase*>(pTile->getGroundObject());
    if((pStructure != nullptr) && (pStructure->canBeCaptured()) && (pStructure->getOwner()->getTeamID() != pLocalHouse->getTeamID())) {
        InfantryBase* pResponder = nullptr;

        for(Uint32 objectID : selectedList) {
            ObjectBase* pObject = objectManager.getObject(objectID);
            if (pObject->isInfantry() && (pObject->getOwner() == pLocalHouse) && pObject->isRespondable()) {
                pResponder = static_cast<InfantryBase*>(pObject);
                pResponder->handleCaptureClick(xPos,yPos);
            }
        }

        currentCursorMode = CursorMode_Normal;
        if(pResponder) {
            pResponder->playConfirmSound();
            return true;
        } else {
            return false;
        }
    }

    return false;
}


bool Game::handleSelectedObjectsActionClick(int xPos, int yPos) {
    //let unit handle right click on map or target
    ObjectBase  *pResponder = nullptr;
    for(Uint32 objectID : selectedList) {
        ObjectBase* pObject = objectManager.getObject(objectID);
        if(pObject->getOwner() == pLocalHouse && pObject->isRespondable()) {
            pObject->handleActionClick(xPos, yPos);

            //if this object obey the command
            if((pResponder == nullptr) && pObject->isRespondable())
                pResponder = pObject;
        }
    }

    if(pResponder) {
        pResponder->playConfirmSound();
        return true;
    } else {
        return false;
    }
}


void Game::takeScreenshot() const {
    std::string screenshotFilename;
    int i = 1;
    do {
        screenshotFilename = "Screenshot" + std::to_string(i) + ".png";
        i++;
    } while(existsFile(screenshotFilename) == true);

    sdl2::surface_ptr pCurrentScreen = renderReadSurface(renderer);
    SavePNG(pCurrentScreen.get(), screenshotFilename.c_str());
    currentGame->addToNewsTicker(_("Screenshot saved") + ": '" + screenshotFilename + "'");
}


void Game::selectNextStructureOfType(const Dune::selected_set_type& itemIDs) {
    bool bSelectNext = true;

    if(selectedList.size() == 1) {
        const auto pObject = getObjectManager().getObject(*selectedList.begin());
        if((pObject != nullptr) && (itemIDs.count(pObject->getItemID()) == 1)) {
            bSelectNext = false;
        }
    }

    StructureBase* pStructure2Select = nullptr;

    for(const auto pStructure : structureList) {
        if(bSelectNext) {
            if( (itemIDs.count(pStructure->getItemID()) == 1) && (pStructure->getOwner() == pLocalHouse) ) {
                pStructure2Select = pStructure;
                break;
            }
        } else {
            if(selectedList.size() == 1 && pStructure->isSelected()) {
                bSelectNext = true;
            }
        }
    }

    if(pStructure2Select == nullptr) {
        // start over at the beginning
        for(auto pStructure : structureList) {
            if( (itemIDs.count(pStructure->getItemID()) == 1) && (pStructure->getOwner() == pLocalHouse) && !pStructure->isSelected() ) {
                pStructure2Select = pStructure;
                break;
            }
        }
    }

    if(pStructure2Select != nullptr) {
        unselectAll(selectedList);
        selectedList.clear();

        pStructure2Select->setSelected(true);
        selectedList.insert(pStructure2Select->getObjectID());
        currentGame->selectionChanged();

        // we center around the newly selected construction yard
        screenborder->setNewScreenCenter(pStructure2Select->getLocation()*TILESIZE);
    }
}

int Game::getGameSpeed() const {
    if(gameType == GameType::CustomMultiplayer) {
        return gameInitSettings.getGameOptions().gameSpeed;
    } else {
        return settings.gameOptions.gameSpeed;
    }
}
