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

#include <FileClasses/FileManager.h>
#include <FileClasses/GFXManager.h>
#include <FileClasses/FontManager.h>
#include <FileClasses/TextManager.h>
#include <FileClasses/music/MusicPlayer.h>
#include <SoundPlayer.h>
#include <misc/IFileStream.h>
#include <misc/OFileStream.h>
#include <misc/IMemoryStream.h>
#include <misc/FileSystem.h>
#include <misc/fnkdat.h>
#include <misc/draw_util.h>
#include <misc/string_util.h>
#include <misc/md5.h>

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

#include <sstream>
#include <iomanip>
#include <SDL.h>

Game::Game() {
    currentZoomlevel = settings.video.preferredZoomLevel;

	whatNextParam = GAME_NOTHING;

	finished = false;
	bPause = false;
	bMenu = false;
	won = false;

	gameType = GAMETYPE_CAMPAIGN;
	techLevel = 0;

    currentCursorMode = CursorMode_Normal;

	chatMode = false;
	scrollDownMode = false;
	scrollLeftMode = false;
	scrollRightMode = false;
	scrollUpMode = false;

	bShowFPS = false;
	bShowTime = false;

	bCheatsEnabled = false;

	selectionMode = false;

	bQuitGame = false;

	bReplay = false;

	indicatorFrame = NONE;
	indicatorTime = 5;
	indicatorTimer = 0;

    pInterface = NULL;
	pInGameMenu = NULL;
	pInGameMentat = NULL;
	pWaitingForOtherPlayers = NULL;

	startWaitingForOtherPlayersTime = 0;

	bSelectionChanged = false;

	unitList.clear();   	//holds all the units
	structureList.clear();	//all the structures
	bulletList.clear();

    house.resize(NUM_HOUSES);

	gamespeed = GAMESPEED_DEFAULT;

	sideBarPos = calcAlignedDrawingRect(pGFXManager->getUIGraphic(UI_SideBar), HAlign::Right, VAlign::Top);
    topBarPos = calcAlignedDrawingRect(pGFXManager->getUIGraphic(UI_TopBar), HAlign::Left, VAlign::Top);

    gameState = START;

	gameCycleCount = 0;
	skipToGameCycle = 0;

	averageFrameTime = 31.25f;
	debug = false;

	powerIndicatorPos.x = 14;
	powerIndicatorPos.y = 146;
	spiceIndicatorPos.x = 20;
	spiceIndicatorPos.y = 146;
	powerIndicatorPos.w = spiceIndicatorPos.w = 4;
	powerIndicatorPos.h = spiceIndicatorPos.h = settings.video.height - 146 - 2;

	musicPlayer->changeMusic(MUSIC_PEACE);
	//////////////////////////////////////////////////////////////////////////
	SDL_Rect gameBoardRect = { 0, topBarPos.h, sideBarPos.x, getRendererHeight() - topBarPos.h };
	screenborder = new ScreenBorder(gameBoardRect);
}


/**
    The destructor frees up all the used memory.
*/
Game::~Game() {
	if(pNetworkManager != NULL) {
        pNetworkManager->setOnReceiveChatMessage(std::function<void (std::string, std::string)>());
        pNetworkManager->setOnReceiveCommandList(std::function<void (const std::string&, const CommandList&)>());
        pNetworkManager->setOnReceiveSelectionList(std::function<void (std::string, std::set<Uint32>, int)>());
        pNetworkManager->setOnPeerDisconnected(std::function<void (std::string, bool, int)>());
	}

    delete pInGameMenu;
    pInGameMenu = NULL;

    delete pInterface;
    pInterface = NULL;

    delete pWaitingForOtherPlayers;
    pWaitingForOtherPlayers = NULL;

    for(RobustList<StructureBase*>::const_iterator iter = structureList.begin(); iter != structureList.end(); ++iter) {
        delete *iter;
    }
    structureList.clear();

    for(RobustList<UnitBase*>::const_iterator iter = unitList.begin(); iter != unitList.end(); ++iter) {
        delete *iter;
    }
    unitList.clear();

	for(RobustList<Bullet*>::const_iterator iter = bulletList.begin(); iter != bulletList.end(); ++iter) {
	    delete *iter;
	}
	bulletList.clear();

    for(RobustList<Explosion*>::const_iterator iter = explosionList.begin(); iter != explosionList.end(); ++iter) {
	    delete *iter;
	}
	explosionList.clear();

	for(int i=0;i<NUM_HOUSES;i++) {
		delete house[i];
		house[i] = NULL;
	}

	delete currentGameMap;
	currentGameMap = NULL;
	delete screenborder;
	screenborder = NULL;
}


void Game::initGame(const GameInitSettings& newGameInitSettings) {
    gameInitSettings = newGameInitSettings;

    switch(gameInitSettings.getGameType()) {
        case GAMETYPE_LOAD_SAVEGAME: {
            if(loadSaveGame(gameInitSettings.getFilename()) == false) {
                throw std::runtime_error("Loading save game failed!");
            }
        } break;

        case GAMETYPE_LOAD_MULTIPLAYER: {
            IMemoryStream memStream(gameInitSettings.getFiledata().c_str(), gameInitSettings.getFiledata().size());

            if(loadSaveGame(memStream) == false) {
                throw std::runtime_error("Loading save game failed!");
            }
        } break;

        case GAMETYPE_CAMPAIGN:
        case GAMETYPE_SKIRMISH:
        case GAMETYPE_CUSTOM:
        case GAMETYPE_CUSTOM_MULTIPLAYER: {
            gameType = gameInitSettings.getGameType();
            randomGen.setSeed(gameInitSettings.getRandomSeed());

            objectData.loadFromINIFile("ObjectData.ini");

            if(gameInitSettings.getMission() != 0) {
                techLevel = ((gameInitSettings.getMission() + 1)/3) + 1 ;
            }

            gamespeed = gameInitSettings.getGameOptions().gameSpeed;

            INIMapLoader* pINIMapLoader = new INIMapLoader(this, gameInitSettings.getFilename(), gameInitSettings.getFiledata());
            delete pINIMapLoader;


            if(bReplay == false && gameInitSettings.getGameType() != GAMETYPE_CUSTOM && gameInitSettings.getGameType() != GAMETYPE_CUSTOM_MULTIPLAYER) {
                /* do briefing */
                fprintf(stdout,"Briefing...");
                fflush(stdout);
                BriefingMenu* pBriefing = new BriefingMenu(gameInitSettings.getHouseID(), gameInitSettings.getMission(),BRIEFING);
                pBriefing->showMenu();
                delete pBriefing;

                fprintf(stdout,"\t\t\tfinished\n");
                fflush(stdout);
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
		perror("Game::loadSaveGame()");
		exit(EXIT_FAILURE);
	}

	// read GameInitInfo
	GameInitSettings loadedGameInitSettings(fs);

	// load all commands
	cmdManager.load(fs);

	initGame(loadedGameInitSettings);

	// fs is closed by its destructor
}


void Game::processObjects()
{
	// update all tiles
    for(int y = 0; y < currentGameMap->getSizeY(); y++) {
		for(int x = 0; x < currentGameMap->getSizeX(); x++) {
            currentGameMap->getTile(x,y)->update();
		}
	}


    for(RobustList<StructureBase*>::iterator iter = structureList.begin(); iter != structureList.end(); ++iter) {
        StructureBase* tempStructure = *iter;
        tempStructure->update();
    }

	if ((currentCursorMode == CursorMode_Placing) && selectedList.empty()) {
		currentCursorMode = CursorMode_Normal;
	}

	for(RobustList<UnitBase*>::iterator iter = unitList.begin(); iter != unitList.end(); ++iter) {
		UnitBase* tempUnit = *iter;
		tempUnit->update();
	}

    for(RobustList<Bullet*>::iterator iter = bulletList.begin(); iter != bulletList.end(); ++iter) {
        (*iter)->update();
	}

    for(RobustList<Explosion*>::iterator iter = explosionList.begin(); iter != explosionList.end(); ++iter) {
        (*iter)->update();
	}
}


void Game::drawScreen()
{
    Coord TopLeftTile = screenborder->getTopLeftTile();
    Coord BottomRightTile = screenborder->getBottomRightTile();

    // extend the view a little bit to avoid graphical glitches
    TopLeftTile.x = std::max(0, TopLeftTile.x - 1);
    TopLeftTile.y = std::max(0, TopLeftTile.y - 1);
    BottomRightTile.x = std::min(currentGameMap->getSizeX()-1, BottomRightTile.x + 1);
    BottomRightTile.y = std::min(currentGameMap->getSizeY()-1, BottomRightTile.y + 1);

    Coord currentTile;

    /* draw ground */
	for(currentTile.y = TopLeftTile.y; currentTile.y <= BottomRightTile.y; currentTile.y++) {
		for(currentTile.x = TopLeftTile.x; currentTile.x <= BottomRightTile.x; currentTile.x++) {

			if (currentGameMap->tileExists(currentTile))	{
				Tile* pTile = currentGameMap->getTile(currentTile);
				pTile->blitGround( screenborder->world2screenX(currentTile.x*TILESIZE),
                                  screenborder->world2screenY(currentTile.y*TILESIZE));
			}
		}
	}

    /* draw structures */
	for(currentTile.y = TopLeftTile.y; currentTile.y <= BottomRightTile.y; currentTile.y++) {
		for(currentTile.x = TopLeftTile.x; currentTile.x <= BottomRightTile.x; currentTile.x++) {

			if (currentGameMap->tileExists(currentTile))	{
				Tile* pTile = currentGameMap->getTile(currentTile);
				pTile->blitStructures( screenborder->world2screenX(currentTile.x*TILESIZE),
                                      screenborder->world2screenY(currentTile.y*TILESIZE));
			}
		}
	}

    /* draw underground units */
	for(currentTile.y = TopLeftTile.y; currentTile.y <= BottomRightTile.y; currentTile.y++) {
		for(currentTile.x = TopLeftTile.x; currentTile.x <= BottomRightTile.x; currentTile.x++) {

			if (currentGameMap->tileExists(currentTile))	{
				Tile* pTile = currentGameMap->getTile(currentTile);
				pTile->blitUndergroundUnits( screenborder->world2screenX(currentTile.x*TILESIZE),
                                            screenborder->world2screenY(currentTile.y*TILESIZE));
			}
		}
	}

    /* draw dead objects */
	for(currentTile.y = TopLeftTile.y; currentTile.y <= BottomRightTile.y; currentTile.y++) {
		for(currentTile.x = TopLeftTile.x; currentTile.x <= BottomRightTile.x; currentTile.x++) {

			if (currentGameMap->tileExists(currentTile))	{
				Tile* pTile = currentGameMap->getTile(currentTile);
				pTile->blitDeadUnits( screenborder->world2screenX(currentTile.x*TILESIZE),
                                     screenborder->world2screenY(currentTile.y*TILESIZE));
			}
		}
	}

    /* draw infantry */
	for(currentTile.y = TopLeftTile.y; currentTile.y <= BottomRightTile.y; currentTile.y++) {
		for(currentTile.x = TopLeftTile.x; currentTile.x <= BottomRightTile.x; currentTile.x++) {

			if (currentGameMap->tileExists(currentTile))	{
				Tile* pTile = currentGameMap->getTile(currentTile);
				pTile->blitInfantry( screenborder->world2screenX(currentTile.x*TILESIZE),
                                    screenborder->world2screenY(currentTile.y*TILESIZE));
			}
		}
	}

    /* draw non-infantry ground units */
	for(currentTile.y = TopLeftTile.y; currentTile.y <= BottomRightTile.y; currentTile.y++) {
		for(currentTile.x = TopLeftTile.x; currentTile.x <= BottomRightTile.x; currentTile.x++) {

			if (currentGameMap->tileExists(currentTile))	{
				Tile* pTile = currentGameMap->getTile(currentTile);
				pTile->blitNonInfantryGroundUnits( screenborder->world2screenX(currentTile.x*TILESIZE),
                                                  screenborder->world2screenY(currentTile.y*TILESIZE));
			}
		}
	}

	/* draw bullets */
    for(RobustList<Bullet*>::const_iterator iter = bulletList.begin(); iter != bulletList.end(); ++iter) {
        Bullet* pBullet = *iter;
        pBullet->blitToScreen();
	}


	/* draw explosions */
	for(RobustList<Explosion*>::const_iterator iter = explosionList.begin(); iter != explosionList.end(); ++iter) {
        (*iter)->blitToScreen();
	}

    /* draw air units */
	for(currentTile.y = TopLeftTile.y; currentTile.y <= BottomRightTile.y; currentTile.y++) {
		for(currentTile.x = TopLeftTile.x; currentTile.x <= BottomRightTile.x; currentTile.x++) {

			if (currentGameMap->tileExists(currentTile))	{
				Tile* pTile = currentGameMap->getTile(currentTile);
				pTile->blitAirUnits(   screenborder->world2screenX(currentTile.x*TILESIZE),
                                      screenborder->world2screenY(currentTile.y*TILESIZE));
			}
		}
	}

    /* draw selection rectangles */
	for(currentTile.y = TopLeftTile.y; currentTile.y <= BottomRightTile.y; currentTile.y++) {
		for(currentTile.x = TopLeftTile.x; currentTile.x <= BottomRightTile.x; currentTile.x++) {

			if (currentGameMap->tileExists(currentTile))	{
				Tile* pTile = currentGameMap->getTile(currentTile);

				if(debug || pTile->isExplored(pLocalHouse->getHouseID())) {
					pTile->blitSelectionRects(   screenborder->world2screenX(currentTile.x*TILESIZE),
                                                screenborder->world2screenY(currentTile.y*TILESIZE));
				}
			}
		}
	}


//////////////////////////////draw unexplored/shade

	if(debug == false) {
	    int zoomedTileSize = world2zoomedWorld(TILESIZE);
		for(int x = screenborder->getTopLeftTile().x - 1; x <= screenborder->getBottomRightTile().x + 1; x++) {
			for (int y = screenborder->getTopLeftTile().y - 1; y <= screenborder->getBottomRightTile().y + 1; y++) {

				if((x >= 0) && (x < currentGameMap->getSizeX()) && (y >= 0) && (y < currentGameMap->getSizeY())) {
					Tile* pTile = currentGameMap->getTile(x, y);

					if(pTile->isExplored(pLocalHouse->getHouseID())) {
					    int hideTile = pTile->getHideTile(pLocalHouse->getHouseID());

						if(hideTile != 0) {
						    SDL_Texture** hiddenTex = pGFXManager->getObjPic(ObjPic_Terrain_Hidden);

						    SDL_Rect source = { hideTile*zoomedTileSize, 0, zoomedTileSize, zoomedTileSize };
						    SDL_Rect drawLocation = {   screenborder->world2screenX(x*TILESIZE), screenborder->world2screenY(y*TILESIZE),
                                                        zoomedTileSize, zoomedTileSize };
							SDL_RenderCopy(renderer, hiddenTex[currentZoomlevel], &source, &drawLocation);
						}

						if(gameInitSettings.getGameOptions().fogOfWar == true) {
                            int fogTile = pTile->getFogTile(pLocalHouse->getHouseID());

                            if(pTile->isFogged(pLocalHouse->getHouseID()) == true) {
                                fogTile = Terrain_HiddenFull;
                            }

                            if(fogTile != 0) {
                                SDL_Texture** hiddenFogTex = pGFXManager->getObjPic(ObjPic_Terrain_HiddenFog);

                                SDL_Rect source = { fogTile*zoomedTileSize, 0,
                                                    zoomedTileSize, zoomedTileSize };
                                SDL_Rect drawLocation = {   screenborder->world2screenX(x*TILESIZE), screenborder->world2screenY(y*TILESIZE),
                                                            zoomedTileSize, zoomedTileSize };

                                SDL_RenderCopy(renderer, hiddenFogTex[currentZoomlevel], &source, &drawLocation);
                            }
						}
					} else {
					    if(!debug) {
					        SDL_Texture** hiddenTex = pGFXManager->getObjPic(ObjPic_Terrain_Hidden);
					        SDL_Rect source = { zoomedTileSize*15, 0, zoomedTileSize, zoomedTileSize };
					        SDL_Rect drawLocation = {   screenborder->world2screenX(x*TILESIZE), screenborder->world2screenY(y*TILESIZE),
                                                        zoomedTileSize, zoomedTileSize };
                            SDL_RenderCopy(renderer, hiddenTex[currentZoomlevel], &source, &drawLocation);
					    }
					}
				} else {
                    // we are outside the map => draw complete hidden
                    SDL_Texture** hiddenTex = pGFXManager->getObjPic(ObjPic_Terrain_Hidden);
                    SDL_Rect source = { zoomedTileSize*15, 0, zoomedTileSize, zoomedTileSize };
                    SDL_Rect drawLocation = {   screenborder->world2screenX(x*TILESIZE), screenborder->world2screenY(y*TILESIZE),
                                                zoomedTileSize, zoomedTileSize };
                    SDL_RenderCopy(renderer, hiddenTex[currentZoomlevel], &source, &drawLocation);
				}
			}
		}
	}

/////////////draw placement position

	int mouse_x, mouse_y;

	SDL_GetMouseState(&mouse_x, &mouse_y);
    adjustMouseCoords(mouse_x, mouse_y);

	if(currentCursorMode == CursorMode_Placing) {
		//if user has selected to place a structure

		if(screenborder->isScreenCoordInsideMap(mouse_x, mouse_y)) {
		    //if mouse is not over game bar

			int	xPos = screenborder->screen2MapX(mouse_x);
			int yPos = screenborder->screen2MapY(mouse_y);

			bool withinRange = false;

			BuilderBase* builder = NULL;
			if(selectedList.size() == 1) {
			    builder = dynamic_cast<BuilderBase*>(objectManager.getObject(*selectedList.begin()));

                int placeItem = builder->getCurrentProducedItem();
                Coord structuresize = getStructureSize(placeItem);

                for (int i = xPos; i < (xPos + structuresize.x); i++) {
                    for (int j = yPos; j < (yPos + structuresize.y); j++) {
                        if (currentGameMap->isWithinBuildRange(i, j, builder->getOwner())) {
                            withinRange = true;			//find out if the structure is close enough to other buildings
                        }
                    }
                }

                SDL_Texture* validPlace = NULL;
                SDL_Texture* invalidPlace = NULL;

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
                        SDL_RenderCopy(renderer, image, NULL, &drawLocation);
                    }
                }
            }
		}
	}

///////////draw game selection rectangle
	if(selectionMode) {

		int finalMouseX = mouse_x;
		int finalMouseY = mouse_y;
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

	if((indicatorFrame != NONE) && (screenborder->isInsideScreen(indicatorPosition, Coord(TILESIZE,TILESIZE)) == true)) {
        SDL_Texture* pUIIndicator = pGFXManager->getUIGraphic(UI_Indicator);
        SDL_Rect source = calcSpriteSourceRect(pUIIndicator, indicatorFrame, 3);
        SDL_Rect drawLocation = calcSpriteDrawingRect(  pUIIndicator,
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
        SDL_Texture* pChatTexture = pFontManager->createTextureWithText("Chat: " + typingChatMessage + (((SDL_GetTicks() / 150) % 2 == 0) ? "_" : ""), COLOR_WHITE, FONT_STD12);
        SDL_Rect drawLocation = calcDrawingRect(pChatTexture, 20, getRendererHeight() - 40);
        SDL_RenderCopy(renderer, pChatTexture, NULL, &drawLocation);
        SDL_DestroyTexture(pChatTexture);
	}

	if(bShowFPS) {
		char	temp[50];
		snprintf(temp,50,"fps: %.1f ", 1000.0f/averageFrameTime);

		SDL_Texture* pFPSTexture = pFontManager->createTextureWithText(temp, COLOR_WHITE, FONT_STD12);
        SDL_Rect drawLocation = calcDrawingRect(pFPSTexture,sideBarPos.x - strlen(temp)*8, 60);
		SDL_RenderCopy(renderer, pFPSTexture, NULL, &drawLocation);
		SDL_DestroyTexture(pFPSTexture);
	}

	if(bShowTime) {
		char	temp[50];
		int     seconds = getGameTime() / 1000;
		snprintf(temp,50," %.2d:%.2d:%.2d", seconds / 3600, (seconds % 3600)/60, (seconds % 60) );

		SDL_Texture* pTimeTexture = pFontManager->createTextureWithText(temp, COLOR_WHITE, FONT_STD12);
        SDL_Rect drawLocation = calcAlignedDrawingRect(pTimeTexture, HAlign::Left, VAlign::Bottom);
        drawLocation.y++;
		SDL_RenderCopy(renderer, pTimeTexture, NULL, &drawLocation);
		SDL_DestroyTexture(pTimeTexture);
	}

	if(finished) {
		std::string message;

        if(won) {
            message = _("You Have Completed Your Mission.");
        } else {
            message = _("You Have Failed Your Mission.");
        }

		SDL_Texture* pFinishMessageTexture = pFontManager->createTextureWithText(message.c_str(), COLOR_WHITE, FONT_STD24);
        SDL_Rect drawLocation = calcDrawingRect(pFinishMessageTexture, sideBarPos.x/2, topBarPos.h + (getRendererHeight()-topBarPos.h)/2, HAlign::Center, VAlign::Center);
		SDL_RenderCopy(renderer, pFinishMessageTexture, NULL, &drawLocation);
		SDL_DestroyTexture(pFinishMessageTexture);
	}

	if(pWaitingForOtherPlayers != NULL) {
        pWaitingForOtherPlayers->draw();
	}

	if(pInGameMenu != NULL) {
		pInGameMenu->draw();
	} else if(pInGameMentat != NULL) {
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
			drawnMouseX = mouse->x;
			drawnMouseY = mouse->y;
		}

		if(pInGameMenu != NULL) {
			pInGameMenu->handleInput(event);

			if(bMenu == false) {
                delete pInGameMenu;
                pInGameMenu = NULL;
			}

		} else if(pInGameMentat != NULL) {
			pInGameMentat->doInput(event);

			if(bMenu == false) {
                delete pInGameMentat;
                pInGameMentat = NULL;
			}

		} else if(pWaitingForOtherPlayers != NULL) {
			pWaitingForOtherPlayers->handleInput(event);

			if(bMenu == false) {
                delete pWaitingForOtherPlayers;
                pWaitingForOtherPlayers = NULL;
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
                        std::string newText = convertUTF8ToISO8859_1(event.text.text);
                        if(typingChatMessage.length() + newText.length() <= 60) {
                            typingChatMessage += newText;
                        }
                    }
                } break;

                case SDL_MOUSEWHEEL: {
                    if (event.wheel.y != 0) {
                        int x, y;
                        SDL_GetMouseState(&x,&y);
                        pInterface->handleMouseWheel(x,y,(event.wheel.y > 0));
                    }
                } break;

                case SDL_MOUSEBUTTONDOWN: {
                    SDL_MouseButtonEvent* mouse = &event.button;

                    switch(mouse->button) {
                        case SDL_BUTTON_LEFT: {
                            pInterface->handleMouseLeft(mouse->x, mouse->y, true);
                        } break;

                        case SDL_BUTTON_RIGHT: {
                            pInterface->handleMouseRight(mouse->x, mouse->y, true);
                        } break;
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
                        } break;	//end of SDL_BUTTON_LEFT

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
                        } break;	//end of SDL_BUTTON_RIGHT
                    }
                } break;

                case SDL_MOUSEMOTION: {
                    SDL_MouseMotionEvent* mouse = &event.motion;

                    pInterface->handleMouseMovement(mouse->x,mouse->y);
                } break;

                case SDL_MOUSEBUTTONUP: {
                    SDL_MouseButtonEvent* mouse = &event.button;

                    switch(mouse->button) {
                        case SDL_BUTTON_LEFT: {
                            pInterface->handleMouseLeft(mouse->x, mouse->y, false);
                        } break;

                        case SDL_BUTTON_RIGHT: {
                            pInterface->handleMouseRight(mouse->x, mouse->y, false);
                        } break;
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
                        int	rectStartY = selectionRect.y/TILESIZE;

                        currentGameMap->selectObjects(  pLocalHouse->getHouseID(),
                                                        rectStartX, rectStartY, rectFinishX, rectFinishY,
                                                        screenborder->screen2worldX(finalMouseX),
                                                        screenborder->screen2worldY(finalMouseY),
                                                        SDL_GetModState() & KMOD_SHIFT);

                        if(selectedList.size() == 1) {
                            ObjectBase* pObject = objectManager.getObject( *selectedList.begin());
                            if(pObject != NULL && pObject->getOwner() == pLocalHouse && pObject->getItemID() == Unit_Harvester) {
                                Harvester* pHarvester = (Harvester*) pObject;

                                std::string harvesterMessage = _("@DUNE.ENG|226#Harvester");

                                int percent = lround(100 * pHarvester->getAmountOfSpice() / HARVESTERMAXSPICE);
                                if(percent > 0) {
                                    if(pHarvester->isawaitingPickup()) {
                                        harvesterMessage += strprintf(_("@DUNE.ENG|124#full and awaiting pickup"), percent);
                                    } else if(pHarvester->isReturning()) {
                                        harvesterMessage += strprintf(_("@DUNE.ENG|123#full and returning"), percent);
                                    } else if(pHarvester->isHarvesting()) {
                                        harvesterMessage += strprintf(_("@DUNE.ENG|122#full and harvesting"), percent);
                                    } else {
                                        harvesterMessage += strprintf(_("@DUNE.ENG|121#full"), percent);
                                    }

                                } else {
                                    if(pHarvester->isawaitingPickup()) {
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

	if((pInGameMenu == NULL) && (pInGameMentat == NULL) && (pWaitingForOtherPlayers == NULL) && (SDL_GetWindowFlags(window) & SDL_WINDOW_MOUSE_FOCUS)) {

	    const Uint8 *keystate = SDL_GetKeyboardState(NULL);
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


void Game::drawCursor()
{
    if(!(SDL_GetWindowFlags(window) & SDL_WINDOW_MOUSE_FOCUS)) {
        return;
    }

	SDL_Texture* pCursor = NULL;
    SDL_Rect dest = { 0, 0, 0, 0};
	if(scrollLeftMode || scrollRightMode || scrollUpMode || scrollDownMode) {
        if(scrollLeftMode && !scrollRightMode) {
	        pCursor = pGFXManager->getUIGraphic(UI_CursorLeft);
	        dest = calcDrawingRect(pCursor, drawnMouseX, drawnMouseY-5, HAlign::Left, VAlign::Top);
	    } else if(scrollRightMode && !scrollLeftMode) {
            pCursor = pGFXManager->getUIGraphic(UI_CursorRight);
	        dest = calcDrawingRect(pCursor, drawnMouseX, drawnMouseY-5, HAlign::Center, VAlign::Top);
	    }

        if(pCursor == NULL) {
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
	    if( (pInGameMenu != NULL) || (pInGameMentat != NULL) || (pWaitingForOtherPlayers != NULL) || (((drawnMouseX >= sideBarPos.x) || (drawnMouseY < topBarPos.h)) && (isOnRadarView(drawnMouseX, drawnMouseY) == false))) {
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

                        Tile* pTile = currentGameMap->getTile(xPos, yPos);

                        if(pTile->isExplored(pLocalHouse->getHouseID())) {

                            StructureBase* pStructure = dynamic_cast<StructureBase*>(pTile->getGroundObject());

                            if((pStructure != NULL) && (pStructure->canBeCaptured()) && (pStructure->getOwner()->getTeam() != pLocalHouse->getTeam())) {
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
                    throw std::runtime_error("Game::drawCursor(): Unknown cursor mode");
                };
            }
	    }
	}

	SDL_RenderCopy(renderer, pCursor, NULL, &dest);
}

void Game::setupView()
{
	int i = 0;
	int j = 0;
	int count = 0;

	//setup start location/view
	i = j = count = 0;

    RobustList<UnitBase*>::const_iterator unitIterator;
	for(unitIterator = unitList.begin(); unitIterator != unitList.end(); ++unitIterator) {
		UnitBase* pUnit = *unitIterator;
		if(pUnit->getOwner() == pLocalHouse) {
			i += pUnit->getX();
			j += pUnit->getY();
			count++;
		}
	}

    RobustList<StructureBase*>::const_iterator structureIterator;
	for(structureIterator = structureList.begin(); structureIterator != structureList.end(); ++structureIterator) {
		StructureBase* pStructure = *structureIterator;
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
	printf("Starting game...\n");
	fflush(stdout);

    // add interface
	if(pInterface == NULL) {
        pInterface = new GameInterface();
        if(gameState == LOADING) {
            // when loading a save game we set radar directly
            pInterface->getRadarView().setRadarMode(pLocalHouse->hasRadarOn());
        } else if(pLocalHouse->hasRadarOn()) {
            // when starting a new game we switch the radar on with an animation if appropriate
            pInterface->getRadarView().switchRadarMode(true);
        }
	}

    gameState = BEGUN;

	//setup endlevel conditions
	finishedLevel = false;

	bShowTime = winFlags & WINLOSEFLAGS_TIMEOUT;

	// Check if a player has lost
	for(int j = 0; j < NUM_HOUSES; j++) {
		if(house[j] != NULL) {
			if(!house[j]->isAlive()) {
				house[j]->lose(true);
			}
		}
	}

	if(bReplay) {
		cmdManager.setReadOnly(true);
	} else {
		char tmp[FILENAME_MAX];
		fnkdat("replay/auto.rpl", tmp, FILENAME_MAX, FNKDAT_USER | FNKDAT_CREAT);
		std::string replayname(tmp);

		OFileStream* pStream = new OFileStream();
		pStream->open(replayname);
		gameInitSettings.save(*pStream);

        // when this game was loaded we have to save the old commands to the replay file first
		cmdManager.save(*pStream);

		// now all new commands might be added
		cmdManager.setStream(pStream);

		// flush stream
		pStream->flush();
	}

	if(pNetworkManager != NULL) {
        pNetworkManager->setOnReceiveChatMessage(std::bind(&ChatManager::addChatMessage, &(pInterface->getChatManager()), std::placeholders::_1, std::placeholders::_2));
        pNetworkManager->setOnReceiveCommandList(std::bind(&CommandManager::addCommandList, &cmdManager, std::placeholders::_1, std::placeholders::_2));
        pNetworkManager->setOnReceiveSelectionList(std::bind(&Game::onReceiveSelectionList, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
        pNetworkManager->setOnPeerDisconnected(std::bind(&Game::onPeerDisconnected, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));

        cmdManager.setNetworkCycleBuffer( MILLI2CYCLES(pNetworkManager->getMaxPeerRoundTripTime()) + 5 );
	}

	// Change music to ingame music
	musicPlayer->changeMusic(MUSIC_PEACE);


	int		frameStart = SDL_GetTicks();
	int     frameEnd = 0;
	int     frameTime = 0;
	int     numFrames = 0;

    //fprintf(stderr, "Random Seed (GameCycle %d): 0x%0X\n", GameCycleCount, RandomGen.getSeed());

	//main game loop
    do {
        SDL_SetRenderTarget(renderer, screenTexture);

        // clear whole screen
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderClear(renderer);

        drawScreen();

        SDL_RenderPresent(renderer);

        SDL_SetRenderTarget(renderer, NULL);
        SDL_RenderCopy(renderer, screenTexture, NULL, NULL);
        SDL_RenderPresent(renderer);

        frameEnd = SDL_GetTicks();

        if(frameEnd == frameStart) {
            SDL_Delay(1);
        }

        frameTime += frameEnd - frameStart; // find difference to get frametime
        frameStart = SDL_GetTicks();

        numFrames++;

        if (bShowFPS) {
            averageFrameTime = 0.999f * averageFrameTime + 0.001f * frameTime;
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


        while( (frameTime > gamespeed) || (!finished && (gameCycleCount < skipToGameCycle)) )	{

            bool bWaitForNetwork = false;

            if(pNetworkManager != NULL) {
                pNetworkManager->update();

                // test if we need to wait for data to arrive
                std::list<std::string> peerList = pNetworkManager->getConnectedPeers();
                std::list<std::string>::iterator iter;
                for(iter = peerList.begin(); iter != peerList.end(); ++iter) {
                    HumanPlayer* pPlayer = dynamic_cast<HumanPlayer*>(getPlayerByName(*iter));
                    if(pPlayer != NULL) {
                        if(pPlayer->nextExpectedCommandsCycle <= gameCycleCount) {
                            //fprintf(stderr, "Cycle %d: Waiting for player '%s' to send data for cycle %d...\n", GameCycleCount, pPlayer->getPlayername().c_str(), pPlayer->nextExpectedCommandsCycle);
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

                            if(pWaitingForOtherPlayers == NULL) {
                                pWaitingForOtherPlayers = new WaitingForOtherPlayers();
                                bMenu = true;
                            }
                        }
                    }

                    SDL_Delay(10);
                } else {
                    startWaitingForOtherPlayersTime = 0;
                    delete pWaitingForOtherPlayers;
                    pWaitingForOtherPlayers = NULL;
                }
            }

            doInput();
            pInterface->updateObjectInterface();

            if(pNetworkManager != NULL) {
                if(bSelectionChanged) {
                    pNetworkManager->sendSelectedList(selectedList);

                    bSelectionChanged = false;
                }
            }

            if(pInGameMentat != NULL) {
                pInGameMentat->update();
            }

            if(pWaitingForOtherPlayers != NULL) {
                pWaitingForOtherPlayers->update();
            }

            cmdManager.update();

            if(!bWaitForNetwork && !bPause)	{
                pInterface->getRadarView().update();
                cmdManager.executeCommands(gameCycleCount);

#ifdef TEST_SYNC
                // add every gamecycles one test sync command
                if(bReplay == false) {
                    cmdManager.addCommand(Command(pLocalPlayer->getPlayerID(), CMD_TEST_SYNC, randomGen.getSeed()));
                }
#endif

                for (int i = 0; i < NUM_HOUSES; i++) {
                    if (house[i] != NULL) {
                        house[i]->update();
                    }
                }

                screenborder->update();

                triggerManager.trigger(gameCycleCount);

                processObjects();

                if ((indicatorFrame != NONE) && (--indicatorTimer <= 0)) {
                    indicatorTimer = indicatorTime;

                    if (++indicatorFrame > 2) {
                        indicatorFrame = NONE;
                    }
                }

                gameCycleCount++;
            }

            if(gameCycleCount <= skipToGameCycle) {
                frameTime = 0;
            } else {
                frameTime -= gamespeed;
            }
        }

        musicPlayer->musicCheck();	//if song has finished, start playing next one
    } while (!bQuitGame && !finishedLevel);//not sure if we need this extra bool



	// Game is finished

	if(bReplay == false && currentGame->won == true) {
        // save replay
		char tmp[FILENAME_MAX];

		std::string mapnameBase = getBasename(gameInitSettings.getFilename(), true);
		fnkdat(std::string("replay/" + mapnameBase + ".rpl").c_str(), tmp, FILENAME_MAX, FNKDAT_USER | FNKDAT_CREAT);
		std::string replayname(tmp);

		OFileStream* pStream = new OFileStream();
		pStream->open(replayname);
		gameInitSettings.save(*pStream);
		cmdManager.save(*pStream);
        delete pStream;
	}

	if(pNetworkManager != NULL) {
        pNetworkManager->disconnect();
	}

    gameState = DEINITIALIZE;
	printf("Game finished!\n");
	fflush(stdout);
}


void Game::resumeGame()
{
	bMenu = false;
	if(gameType != GAMETYPE_CUSTOM_MULTIPLAYER) {
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
        pInGameMenu = new InGameMenu((gameType == GAMETYPE_CUSTOM_MULTIPLAYER), color);
        bMenu = true;
        pauseGame();
    }
}


void Game::onMentat()
{
	pInGameMentat = new MentatHelp(pLocalHouse->getHouseID(), techLevel, gameInitSettings.getMission());
	bMenu = true;
    pauseGame();
}


GameInitSettings Game::getNextGameInitSettings()
{
	if(nextGameInitSettings.getGameType() != GAMETYPE_INVALID) {
		// return the prepared game init settings (load game or restart mission)
		return nextGameInitSettings;
	}

	switch(gameInitSettings.getGameType()) {
		case GAMETYPE_CAMPAIGN: {
			/* do map choice */
			fprintf(stdout,"Map Choice...");
			fflush(stdout);
			MapChoice* pMapChoice = new MapChoice(gameInitSettings.getHouseID(), gameInitSettings.getMission());
			int nextMission = pMapChoice->showMenu();
			delete pMapChoice;

			fprintf(stdout,"\t\t\tfinished\n");
			fflush(stdout);

            return GameInitSettings(gameInitSettings, nextMission);
		} break;

		default: {
			fprintf(stderr,"Game::getNextGameInitClass(): Wrong gameType for next Game.\n");
			fflush(stderr);
			return GameInitSettings();
		} break;
	}

	return GameInitSettings();
}


int Game::whatNext()
{
	if(whatNextParam != GAME_NOTHING) {
		int tmp = whatNextParam;
		whatNextParam = GAME_NOTHING;
		return tmp;
	}

	if(nextGameInitSettings.getGameType() != GAMETYPE_INVALID) {
		return GAME_LOAD;
	}

	switch(gameType) {
		case GAMETYPE_CAMPAIGN: {
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
                // copy old init class to init class for next game
                nextGameInitSettings = gameInitSettings;

				return GAME_DEBRIEFING_LOST;
			}
		} break;

		case GAMETYPE_SKIRMISH: {
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

		case GAMETYPE_CUSTOM:
		case GAMETYPE_CUSTOM_MULTIPLAYER: {
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


bool Game::loadSaveGame(std::string filename) {
    IFileStream fs;

    if(fs.open(filename) == false) {
		return false;
	}

    bool ret = loadSaveGame(fs);

    fs.close();

    return ret;
}

bool Game::loadSaveGame(InputStream& stream) {
	gameState = LOADING;

	Uint32 magicNum = stream.readUint32();
	if (magicNum != SAVEMAGIC) {
		fprintf(stderr,"Game::loadSaveGame(): No valid savegame! Expected magic number %.8X, but got %.8X!\n", SAVEMAGIC, magicNum);
		return false;
	}

	Uint32 savegameVersion = stream.readUint32();
	if (savegameVersion != SAVEGAMEVERSION) {
		fprintf(stderr,"Game::loadSaveGame(): No valid savegame! Expected savegame version %d, but got %d!\n", SAVEGAMEVERSION, savegameVersion);
		return false;
	}

	std::string duneVersion = stream.readString();

	// if this is a multiplayer load we need to save some information before we overwrite gameInitSettings with the settings saved in the savegame
	bool bMultiplayerLoad = (gameInitSettings.getGameType() == GAMETYPE_LOAD_MULTIPLAYER);
	GameInitSettings::HouseInfoList oldHouseInfoList = gameInitSettings.getHouseInfoList();

	// read gameInitSettings
	gameInitSettings = GameInitSettings(stream);

    // read the actual house setup choosen at the beginning of the game
    Uint32 numHouseInfo = stream.readUint32();
	for(Uint32 i=0;i<numHouseInfo;i++) {
        houseInfoListSetup.push_back(GameInitSettings::HouseInfo(stream));
	}

	//read map size
	short mapSizeX = stream.readUint32();
	short mapSizeY = stream.readUint32();

	//create the new map
	currentGameMap = new Map(mapSizeX, mapSizeY);

	//read GameCycleCount
	gameCycleCount = stream.readUint32();

	// read some settings
	gameType = (GAMETYPE) stream.readSint8();
	techLevel = stream.readUint8();
	gamespeed = stream.readUint32();
	randomGen.setSeed(stream.readUint32());

    // read in the unit/structure data
    objectData.load(stream);

	//load the house(s) info
	for(int i=0; i<NUM_HOUSES; i++) {
		if (stream.readBool() == true) {
		    //house in game
	        house[i] = new House(stream);
		}
	}

	// we have to set the local player
	if(bMultiplayerLoad) {
        // get it from the gameInitSettings that started the game (not the one saved in the savegame)
        GameInitSettings::HouseInfoList::iterator iter;
        for(iter = oldHouseInfoList.begin(); iter != oldHouseInfoList.end(); ++iter) {

            // find the right house
            for(int i=0;i<NUM_HOUSES;i++) {
                if((house[i] != NULL) && (house[i]->getHouseID() == iter->houseID)) {
                    // iterate over all players

                    const std::list<std::shared_ptr<Player> >& players = house[i]->getPlayerList();
                    GameInitSettings::HouseInfo::PlayerInfoList playerInfoList = iter->playerInfoList;

                    std::list<std::shared_ptr<Player> >::const_iterator playerIter = players.begin();
                    GameInitSettings::HouseInfo::PlayerInfoList::iterator playerInfoListIter;

                    for(playerInfoListIter = playerInfoList.begin(); playerInfoListIter != playerInfoList.end(); ++playerInfoListIter) {
                        if(playerInfoListIter->playerClass == HUMANPLAYERCLASS) {
                            while(playerIter != players.end()) {

                                std::shared_ptr<HumanPlayer> humanPlayer = std::dynamic_pointer_cast<HumanPlayer>(*playerIter);
                                if(humanPlayer.get() != NULL) {
                                    // we have actually found a human player and now assign the first unused name to it
                                    std::string playername = playerInfoListIter->playerName;
                                    unregisterPlayer(humanPlayer.get());
                                    humanPlayer->setPlayername(playername);
                                    registerPlayer(humanPlayer.get());

                                    if(playername == settings.general.playerName) {
                                        pLocalHouse = house[i];
                                        pLocalPlayer = humanPlayer.get();
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
        Uint8 localPlayerID = stream.readUint8();
        pLocalPlayer = dynamic_cast<HumanPlayer*>(getPlayerByID(localPlayerID));
        pLocalHouse = house[pLocalPlayer->getHouse()->getHouseID()];
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
		explosionList.push_back(new Explosion(stream));
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


bool Game::saveGame(std::string filename)
{
	OFileStream fs;

	if(fs.open(filename) == false) {
		perror("Game::saveGame()");
		currentGame->addToNewsTicker(std::string("Game NOT saved: Cannot open \"") + filename + "\".");
		return false;
	}

	fs.writeUint32(SAVEMAGIC);

	fs.writeUint32(SAVEGAMEVERSION);

	fs.writeString(VERSIONSTRING);

	// write gameInitSettings
	gameInitSettings.save(fs);

    fs.writeUint32(houseInfoListSetup.size());
	GameInitSettings::HouseInfoList::const_iterator iter;
	for(iter = houseInfoListSetup.begin(); iter != houseInfoListSetup.end(); ++iter) {
        iter->save(fs);
	}

	//write the map size
	fs.writeUint32(currentGameMap->getSizeX());
	fs.writeUint32(currentGameMap->getSizeY());

	// write GameCycleCount
	fs.writeUint32(gameCycleCount);

	// write some settings
	fs.writeSint8(gameType);
	fs.writeUint8(techLevel);
    fs.writeUint32(gamespeed);
	fs.writeUint32(randomGen.getSeed());

    // write out the unit/structure data
    objectData.save(fs);

	//write the house(s) info
	for(int i=0; i<NUM_HOUSES; i++) {
		fs.writeBool(house[i] != NULL);

		if(house[i] != NULL) {
			house[i]->save(fs);
		}
	}

    if(gameInitSettings.getGameType() != GAMETYPE_CUSTOM_MULTIPLAYER) {
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
	for(RobustList<Bullet*>::const_iterator iter = bulletList.begin(); iter != bulletList.end(); ++iter) {
		(*iter)->save(fs);
	}

	fs.writeUint32(explosionList.size());
	for(RobustList<Explosion*>::const_iterator iter = explosionList.begin(); iter != explosionList.end(); ++iter) {
		(*iter)->save(fs);
	}

    if(gameInitSettings.getGameType() != GAMETYPE_CUSTOM_MULTIPLAYER) {
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


void Game::saveObject(OutputStream& stream, ObjectBase* obj) {
	if(obj == NULL)
		return;

	stream.writeUint32(obj->getItemID());
	obj->save(stream);
}


ObjectBase* Game::loadObject(InputStream& stream, Uint32 objectID)
{
	Uint32 itemID;

	itemID = stream.readUint32();

	ObjectBase* newObject = ObjectBase::loadObject(stream, itemID, objectID);
	if(newObject == NULL) {
		fprintf(stderr,"Game::LoadObject(): ObjectBase::loadObject() returned NULL!\n");
		exit(EXIT_FAILURE);
	}

	return newObject;
}


void Game::selectAll(std::set<Uint32>& aList)
{
    std::set<Uint32>::iterator iter;
    for(iter = aList.begin(); iter != aList.end(); ++iter) {
        ObjectBase *tempObject = objectManager.getObject(*iter);
        tempObject->setSelected(true);
    }
}


void Game::unselectAll(std::set<Uint32>& aList)
{
    std::set<Uint32>::iterator iter;
    for(iter = aList.begin(); iter != aList.end(); ++iter) {
        ObjectBase *tempObject = objectManager.getObject(*iter);
        tempObject->setSelected(false);
    }
}

void Game::onReceiveSelectionList(std::string name, std::set<Uint32> newSelectionList, int groupListIndex)
{
    HumanPlayer* pHumanPlayer = dynamic_cast<HumanPlayer*>(getPlayerByName(name));

    if(pHumanPlayer == NULL) {
        return;
    }

    if(groupListIndex == -1) {
        // the other player controlling the same house has selected some units

        if(pHumanPlayer->getHouse() != pLocalHouse) {
            return;
        }

        std::set<Uint32>::iterator iter;
        for(iter = selectedByOtherPlayerList.begin(); iter != selectedByOtherPlayerList.end(); ++iter) {
            ObjectBase* pObject = objectManager.getObject(*iter);
            if(pObject != NULL) {
                pObject->setSelectedByOtherPlayer(false);
            }
        }

        selectedByOtherPlayerList = newSelectionList;

        for(iter = selectedByOtherPlayerList.begin(); iter != selectedByOtherPlayerList.end(); ++iter) {
            ObjectBase* pObject = objectManager.getObject(*iter);
            if(pObject != NULL) {
                pObject->setSelectedByOtherPlayer(true);
            }
        }
    } else {
        // some other player has assigned a number to a list of units
        pHumanPlayer->setGroupList(groupListIndex, newSelectionList);
    }
}

void Game::onPeerDisconnected(std::string name, bool bHost, int cause) {
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


bool Game::isOnRadarView(int mouseX, int mouseY) {
    return pInterface->getRadarView().isOnRadar(mouseX - (sideBarPos.x + SIDEBAR_COLUMN_WIDTH), mouseY - sideBarPos.y);
}


void Game::handleChatInput(SDL_KeyboardEvent& keyboardEvent) {
    if(keyboardEvent.keysym.sym == SDLK_ESCAPE) {
        chatMode = false;
    } else if(keyboardEvent.keysym.sym == SDLK_RETURN) {
        if(typingChatMessage.length() > 0) {
            unsigned char md5sum[16];

            md5((const unsigned char*) typingChatMessage.c_str(), typingChatMessage.size(), md5sum);

            std::stringstream md5stream;
            md5stream << std::setfill('0') << std::hex << std::uppercase << "0x";
            for(int i=0;i<16;i++) {
                md5stream << std::setw(2) << (int) md5sum[i];
            }

            std::string md5string = md5stream.str();

            if((bCheatsEnabled == false) && (md5string == "0xB8766C8EC7A61036B69893FC17AAF21E")) {
                bCheatsEnabled = true;
                pInterface->getChatManager().addInfoMessage("Cheat mode enabled");
            } else if((bCheatsEnabled == true) && (md5string == "0xB8766C8EC7A61036B69893FC17AAF21E")) {
                pInterface->getChatManager().addInfoMessage("Cheat mode already enabled");
            } else if((bCheatsEnabled == true) && (md5string == "0x57583291CB37F8167EDB0611D8D19E58")) {
                if (gameType != GAMETYPE_CUSTOM_MULTIPLAYER) {
                    pInterface->getChatManager().addInfoMessage("You win this game");
                    setGameWon();
                }
            } else if((bCheatsEnabled == true) && (md5string == "0x1A12BE3DBE54C5A504CAA6EE9782C1C8")) {
                if(debug == true) {
                    pInterface->getChatManager().addInfoMessage("You are already in debug mode");
                } else if (gameType != GAMETYPE_CUSTOM_MULTIPLAYER) {
                    pInterface->getChatManager().addInfoMessage("Debug mode enabled");
                    debug = true;
                }
            } else if((bCheatsEnabled == true) && (md5string == "0x54F68155FC64A5BC66DCD50C1E925C0B")) {
                if(debug == false) {
                    pInterface->getChatManager().addInfoMessage("You are not in debug mode");
                } else if (gameType != GAMETYPE_CUSTOM_MULTIPLAYER) {
                    pInterface->getChatManager().addInfoMessage("Debug mode disabled");
                    debug = false;
                }
            } else if((bCheatsEnabled == true) && (md5string == "0xCEF1D26CE4B145DE985503CA35232ED8")) {
                if (gameType != GAMETYPE_CUSTOM_MULTIPLAYER) {
                    pInterface->getChatManager().addInfoMessage("You got some credits");
                    pLocalHouse->returnCredits(10000);
                }
            } else {
                if(pNetworkManager != NULL) {
                    pNetworkManager->sendChatMessage(typingChatMessage);
                }
                pInterface->getChatManager().addChatMessage(settings.general.playerName, typingChatMessage);
            }
        }

        chatMode = false;
    } else if(keyboardEvent.keysym.sym == SDLK_BACKSPACE) {
        if(typingChatMessage.length() > 0) {
            typingChatMessage.resize(typingChatMessage.length() - 1);
        }
    }
}


void Game::handleKeyInput(SDL_KeyboardEvent& keyboardEvent) {
    switch(keyboardEvent.keysym.sym) {

        case SDLK_0: {
            //if ctrl and 0 remove selected units from all groups
            if(SDL_GetModState() & KMOD_CTRL) {
                std::set<Uint32>::iterator iter;
                for(iter = selectedList.begin(); iter != selectedList.end(); ++iter) {
                    ObjectBase* object = objectManager.getObject(*iter);
                    object->setSelected(false);
                    object->removeFromSelectionLists();
                }
                selectedList.clear();
                currentGame->selectionChanged();
                currentCursorMode = CursorMode_Normal;
            } else {
                std::set<Uint32>::iterator iter;
                for(iter = selectedList.begin(); iter != selectedList.end(); ++iter) {
                    ObjectBase* object = objectManager.getObject(*iter);
                    object->setSelected(false);
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
            int selectListIndex = keyboardEvent.keysym.sym - SDLK_1;

            if(SDL_GetModState() & KMOD_CTRL) {
                pLocalPlayer->setGroupList(selectListIndex, selectedList);

                pInterface->updateObjectInterface();
            } else {
                std::set<Uint32>& groupList = pLocalPlayer->getGroupList(selectListIndex);

                // find out if we are choosing a group with all items already selected
                bool bEverythingWasSelected = (selectedList.size() == groupList.size());
                Coord averagePosition;
                for(std::set<Uint32>::iterator iter = groupList.begin(); iter != groupList.end(); ++iter) {
                    ObjectBase* object = objectManager.getObject(*iter);
                    bEverythingWasSelected = bEverythingWasSelected && object->isSelected();
                    averagePosition += object->getLocation();
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
                for(std::set<Uint32>::iterator iter = groupList.begin(); iter != groupList.end(); ++iter) {
                    ObjectBase* object = objectManager.getObject(*iter);
                    object->setSelected(true);
                    selectedList.insert(object->getObjectID());
                    currentGame->selectionChanged();
                }

                if(bEverythingWasSelected && (groupList.empty() == false)) {
                    // we center around the newly selected units/structures
                    screenborder->setNewScreenCenter(averagePosition*TILESIZE);
                }
            }
            currentCursorMode = CursorMode_Normal;
        } break;

        case SDLK_MINUS: {
            if(gameType != GAMETYPE_CUSTOM_MULTIPLAYER) {
                if(++gamespeed > GAMESPEED_MAX) {
                    gamespeed = GAMESPEED_MAX;
                }
                currentGame->addToNewsTicker(strprintf(_("Game speed") + ": %d", gamespeed));
            }
        } break;

        case SDLK_EQUALS: {
            //PLUS
            if(gameType != GAMETYPE_CUSTOM_MULTIPLAYER) {
                if(--gamespeed < GAMESPEED_MIN) {
                    gamespeed = GAMESPEED_MIN;
                }
                currentGame->addToNewsTicker(strprintf(_("Game speed") + ": %d", gamespeed));
            }
        } break;

        case SDLK_a: {
            //set object to attack
            if(currentCursorMode != CursorMode_Attack) {
                std::set<Uint32>::iterator iter;
                for(iter = selectedList.begin(); iter != selectedList.end(); ++iter) {

                    ObjectBase* tempObject = objectManager.getObject(*iter);
                    if(tempObject->isAUnit() && (tempObject->getOwner() == pLocalHouse)
                        && tempObject->isRespondable() && tempObject->canAttack()) {

                        currentCursorMode = CursorMode_Attack;
                        break;
                    } else if((tempObject->getItemID() == Structure_Palace)
                                && ((tempObject->getOwner()->getHouseID() == HOUSE_HARKONNEN) || (tempObject->getOwner()->getHouseID() == HOUSE_SARDAUKAR))) {
                        if(((Palace*) tempObject)->isSpecialWeaponReady()) {
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
            Coord oldCenterCoord = screenborder->getCurrentCenter();
            currentZoomlevel = 0;
            screenborder->adjustScreenBorderToMapsize(currentGameMap->getSizeX(), currentGameMap->getSizeY());
            screenborder->setNewScreenCenter(oldCenterCoord);
        } break;

        case SDLK_F2: {
            Coord oldCenterCoord = screenborder->getCurrentCenter();
            currentZoomlevel = 1;
            screenborder->adjustScreenBorderToMapsize(currentGameMap->getSizeX(), currentGameMap->getSizeY());
            screenborder->setNewScreenCenter(oldCenterCoord);
        } break;

        case SDLK_F3: {
            Coord oldCenterCoord = screenborder->getCurrentCenter();
            currentZoomlevel = 2;
            screenborder->adjustScreenBorderToMapsize(currentGameMap->getSizeX(), currentGameMap->getSizeY());
            screenborder->setNewScreenCenter(oldCenterCoord);
        } break;

        case SDLK_F5: {
            // skip a 30 seconds
            if(gameType != GAMETYPE_CUSTOM_MULTIPLAYER || bReplay) {
                skipToGameCycle = gameCycleCount + (30*1000)/GAMESPEED_DEFAULT;
            }
        } break;

        case SDLK_F6: {
            // skip 2 minutes
            if(gameType != GAMETYPE_CUSTOM_MULTIPLAYER || bReplay) {
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
                std::set<Uint32>::iterator iter;
                for(iter = selectedList.begin(); iter != selectedList.end(); ++iter) {

                    ObjectBase* tempObject = objectManager.getObject(*iter);
                    if(tempObject->isAUnit() && (tempObject->getOwner() == pLocalHouse)
                        && tempObject->isRespondable()) {

                        currentCursorMode = CursorMode_Move;
                        break;
                    }
                }
            }
        } break;

        case SDLK_g: {
            // select next construction yard
            std::set<Uint32> itemIDs;
            itemIDs.insert(Structure_ConstructionYard);
            selectNextStructureOfType(itemIDs);
        } break;

        case SDLK_f: {
            // select next factory
            std::set<Uint32> itemIDs;
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
                    if(pConstructionYard != NULL) {
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
            std::string screenshotFilename;
            int i = 1;
            do {
                screenshotFilename = "Screenshot" + stringify(i) + ".bmp";
                i++;
            } while(existsFile(screenshotFilename) == true);

            SDL_Surface* pCurrentScreen = renderReadSurface(renderer);
            SDL_SaveBMP(pCurrentScreen, screenshotFilename.c_str());
            currentGame->addToNewsTicker(_("Screenshot saved") + ": '" + screenshotFilename + "'");
            SDL_FreeSurface(pCurrentScreen);
        } break;

        case SDLK_h: {
            std::set<Uint32>::iterator iter;
            for(iter = selectedList.begin(); iter != selectedList.end(); ++iter) {
                ObjectBase *tempObject = objectManager.getObject(*iter);
                if(tempObject->getItemID() == Unit_Harvester) {
                    ((Harvester*)tempObject)->handleReturnClick();
                }
            }
        } break;


        case SDLK_r: {
            std::set<Uint32>::iterator iter;
            for(iter = selectedList.begin(); iter != selectedList.end(); ++iter) {
                ObjectBase *tempObject = objectManager.getObject(*iter);
                if(tempObject->isAStructure()) {
                    ((StructureBase*)tempObject)->handleRepairClick();
                } else if(tempObject->isAGroundUnit() && tempObject->getHealth() < tempObject->getMaxHealth()) {
                    ((GroundUnit*)tempObject)->handleSendToRepairClick();
                }
            }
        } break;


        case SDLK_d: {
            if(currentCursorMode != CursorMode_CarryallDrop){
                std::set<Uint32>::iterator iter;
                for(iter = selectedList.begin(); iter != selectedList.end(); ++iter) {
                    ObjectBase *tempObject = objectManager.getObject(*iter);
                    if(tempObject->isAGroundUnit() && tempObject->getOwner()->hasCarryalls()) {
                        currentCursorMode = CursorMode_CarryallDrop;
                    }
                }
            }

        } break;

        case SDLK_u: {
            std::set<Uint32>::iterator iter;
            for(iter = selectedList.begin(); iter != selectedList.end(); ++iter) {
                ObjectBase *tempObject = objectManager.getObject(*iter);
                if(tempObject->isABuilder()) {
                    BuilderBase* pBuilder = dynamic_cast<BuilderBase*>(tempObject);

                    if(pBuilder->getHealth() >= pBuilder->getMaxHealth() && pBuilder->isAllowedToUpgrade()) {
                        pBuilder->handleUpgradeClick();
                    }
                }
            }
        } break;

        case SDLK_RETURN: {
            if(SDL_GetModState() & KMOD_ALT) {
                SDL_SetWindowFullscreen(window, (SDL_GetWindowFlags(window) ^ SDL_WINDOW_FULLSCREEN_DESKTOP));
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
            if(gameType != GAMETYPE_CUSTOM_MULTIPLAYER) {
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
    BuilderBase* pBuilder = NULL;

    if(selectedList.size() == 1) {
        pBuilder = dynamic_cast<BuilderBase*>(objectManager.getObject(*selectedList.begin()));
    }

    int placeItem = pBuilder->getCurrentProducedItem();
    Coord structuresize = getStructureSize(placeItem);

    if(placeItem == Structure_Slab1) {
        if((currentGameMap->isWithinBuildRange(xPos, yPos, pBuilder->getOwner()))
            && (currentGameMap->okayToPlaceStructure(xPos, yPos, 1, 1, false, pBuilder->getOwner()))
            && (currentGameMap->getTile(xPos, yPos)->isConcrete() == false)) {
            getCommandManager().addCommand(Command(pLocalPlayer->getPlayerID(), CMD_PLACE_STRUCTURE,pBuilder->getObjectID(), xPos, yPos));
            //the user has tried to place and has been successful
            soundPlayer->playSound(PlaceStructure);
            currentCursorMode = CursorMode_Normal;
            return true;
        } else {
            //the user has tried to place but clicked on impossible point
            currentGame->addToNewsTicker(_("@DUNE.ENG|135#Cannot place slab here."));
            soundPlayer->playSound(InvalidAction);	//can't place noise
            return false;
        }
    } else if(placeItem == Structure_Slab4) {
        if(	(currentGameMap->isWithinBuildRange(xPos, yPos, pBuilder->getOwner()) || currentGameMap->isWithinBuildRange(xPos+1, yPos, pBuilder->getOwner())
                || currentGameMap->isWithinBuildRange(xPos+1, yPos+1, pBuilder->getOwner()) || currentGameMap->isWithinBuildRange(xPos, yPos+1, pBuilder->getOwner()))
            && ((currentGameMap->okayToPlaceStructure(xPos, yPos, 1, 1, false, pBuilder->getOwner())
                || currentGameMap->okayToPlaceStructure(xPos+1, yPos, 1, 1, false, pBuilder->getOwner())
                || currentGameMap->okayToPlaceStructure(xPos+1, yPos+1, 1, 1, false, pBuilder->getOwner())
                || currentGameMap->okayToPlaceStructure(xPos, yPos, 1, 1+1, false, pBuilder->getOwner())))
            && ((currentGameMap->getTile(xPos, yPos)->isConcrete() == false) || (currentGameMap->getTile(xPos+1, yPos)->isConcrete() == false)
                || (currentGameMap->getTile(xPos, yPos+1)->isConcrete() == false) || (currentGameMap->getTile(xPos+1, yPos+1)->isConcrete() == false)) ) {

            getCommandManager().addCommand(Command(pLocalPlayer->getPlayerID(), CMD_PLACE_STRUCTURE,pBuilder->getObjectID(), xPos, yPos));
            //the user has tried to place and has been successful
            soundPlayer->playSound(PlaceStructure);
            currentCursorMode = CursorMode_Normal;
            return true;
        } else {
            //the user has tried to place but clicked on impossible point
            currentGame->addToNewsTicker(_("@DUNE.ENG|135#Cannot place slab here."));
            soundPlayer->playSound(InvalidAction);	//can't place noise
            return false;
        }
    } else {
        if(currentGameMap->okayToPlaceStructure(xPos, yPos, structuresize.x, structuresize.y, false, pBuilder->getOwner())) {
            getCommandManager().addCommand(Command(pLocalPlayer->getPlayerID(), CMD_PLACE_STRUCTURE,pBuilder->getObjectID(), xPos, yPos));
            //the user has tried to place and has been successful
            soundPlayer->playSound(PlaceStructure);
            currentCursorMode = CursorMode_Normal;
            return true;
        } else {
            //the user has tried to place but clicked on impossible point
            currentGame->addToNewsTicker(strprintf(_("@DUNE.ENG|134#Cannot place %%s here."), resolveItemName(placeItem).c_str()));
            soundPlayer->playSound(InvalidAction);	//can't place noise

            // is this building area only blocked by units?
            if(currentGameMap->okayToPlaceStructure(xPos, yPos, structuresize.x, structuresize.y, false, pBuilder->getOwner(), true)) {
                // then we try to move all units outside the building area
                for(int y = yPos; y < yPos + structuresize.y; y++) {
                    for(int x = xPos; x < xPos + structuresize.x; x++) {
                        Tile* pTile = currentGameMap->getTile(x,y);
                        if(pTile->hasANonInfantryGroundObject()) {
                            ObjectBase* pObject = pTile->getNonInfantryGroundObject();
                            if(pObject->isAUnit() && pObject->getOwner() == pBuilder->getOwner()) {
                                UnitBase* pUnit = dynamic_cast<UnitBase*>(pObject);
                                Coord newDestination = currentGameMap->findDeploySpot(pUnit, Coord(xPos, yPos), pUnit->getLocation(), structuresize);
                                pUnit->handleMoveClick(newDestination.x, newDestination.y);
                            }
                        } else if(pTile->hasInfantry()) {
                            std::list<Uint32>::const_iterator iter;
                            for(iter = pTile->getInfantryList().begin(); iter != pTile->getInfantryList().end(); ++iter) {
                                InfantryBase* pInfantry = dynamic_cast<InfantryBase*>(getObjectManager().getObject(*iter));
                                if((pInfantry != NULL) && (pInfantry->getOwner() == pBuilder->getOwner())) {
                                    Coord newDestination = currentGameMap->findDeploySpot(pInfantry, Coord(xPos, yPos), pInfantry->getLocation(), structuresize);
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
    UnitBase* responder = NULL;

    std::set<Uint32>::iterator iter;
    for(iter = selectedList.begin(); iter != selectedList.end(); ++iter) {
        ObjectBase *tempObject = objectManager.getObject(*iter);
        if(tempObject->isAUnit() && (tempObject->getOwner() == pLocalHouse) && tempObject->isRespondable()) {
            responder = (UnitBase*) tempObject;
            responder->handleAttackClick(xPos,yPos);
        } else if((tempObject->getItemID() == Structure_Palace)
                    && ((tempObject->getOwner()->getHouseID() == HOUSE_HARKONNEN) || (tempObject->getOwner()->getHouseID() == HOUSE_SARDAUKAR))) {

            if(((Palace*) tempObject)->isSpecialWeaponReady()) {
                ((Palace*) tempObject)->handleDeathhandClick(xPos, yPos);
            }
        }
    }

    currentCursorMode = CursorMode_Normal;
    if(responder) {
        responder->playConfirmSound();
        return true;
    } else {
        return false;
    }
}

bool Game::handleSelectedObjectsMoveClick(int xPos, int yPos) {
    UnitBase* responder = NULL;

    std::set<Uint32>::iterator iter;
    for(iter = selectedList.begin(); iter != selectedList.end(); ++iter) {
        ObjectBase *tempObject = objectManager.getObject(*iter);
        if (tempObject->isAUnit() && (tempObject->getOwner() == pLocalHouse) && tempObject->isRespondable()) {
            responder = (UnitBase*) tempObject;
            responder->handleMoveClick(xPos,yPos);
        }
    }

    currentCursorMode = CursorMode_Normal;
    if(responder) {
        responder->playConfirmSound();
        return true;
    } else {
        return false;
    }
}

/**
    New method for transporting units quickly using carryalls
**/
bool Game::handleSelectedObjectsRequestCarryallDropClick(int xPos, int yPos) {

    UnitBase* responder = NULL;

    /*
        If manual carryall mode isn't enabled then turn this off...
    */
    if(!getGameInitSettings().getGameOptions().manualCarryallDrops) {
        currentCursorMode = CursorMode_Normal;
        return false;
    }


    std::set<Uint32>::iterator iter;
    for(iter = selectedList.begin(); iter != selectedList.end(); ++iter) {
        ObjectBase *tempObject = objectManager.getObject(*iter);
        if (tempObject->isAGroundUnit() && (tempObject->getOwner() == pLocalHouse) && tempObject->isRespondable()) {
            responder = (UnitBase*) tempObject;
            responder->handleRequestCarryallDropClick(xPos,yPos);
        }
    }

    currentCursorMode = CursorMode_Normal;
    if(responder) {
        responder->playConfirmSound();
        return true;
    } else {
        return false;
    }
}



bool Game::handleSelectedObjectsCaptureClick(int xPos, int yPos) {
    Tile* pTile = currentGameMap->getTile(xPos, yPos);

    if(pTile == NULL) {
        return false;
    }

    StructureBase* pStructure = dynamic_cast<StructureBase*>(pTile->getGroundObject());

    if((pStructure != NULL) && (pStructure->canBeCaptured()) && (pStructure->getOwner()->getTeam() != pLocalHouse->getTeam())) {
        InfantryBase* responder = NULL;

        std::set<Uint32>::iterator iter;
        for(iter = selectedList.begin(); iter != selectedList.end(); ++iter) {
            ObjectBase *tempObject = objectManager.getObject(*iter);
            if (tempObject->isInfantry() && (tempObject->getOwner() == pLocalHouse) && tempObject->isRespondable()) {
                responder = (InfantryBase*) tempObject;
                responder->handleCaptureClick(xPos,yPos);
            }
        }

        currentCursorMode = CursorMode_Normal;
        if(responder) {
            responder->playConfirmSound();
            return true;
        } else {
            return false;
        }
    }

    return false;
}


bool Game::handleSelectedObjectsActionClick(int xPos, int yPos) {
    //let unit handle right click on map or target
    ObjectBase	*responder = NULL;
    ObjectBase	*tempObject = NULL;

    std::set<Uint32>::iterator iter;
    for(iter = selectedList.begin(); iter != selectedList.end(); ++iter) {
        tempObject = objectManager.getObject(*iter);

        if(tempObject->getOwner() == pLocalHouse && tempObject->isRespondable()) {
            tempObject->handleActionClick(xPos, yPos);

            //if this object obey the command
            if((responder == NULL) && tempObject->isRespondable())
                responder = tempObject;
        }
    }

    if(responder) {
        responder->playConfirmSound();
        return true;
    } else {
        return false;
    }
}

void Game::selectNextStructureOfType(const std::set<Uint32>& itemIDs) {
    bool bSelectNext = true;

    if(selectedList.size() == 1) {
        ObjectBase* pObject = getObjectManager().getObject(*selectedList.begin());
        if((pObject != NULL) && (itemIDs.count(pObject->getItemID()) == 1)) {
            bSelectNext = false;
        }
    }

    StructureBase* pStructure2Select = NULL;

    for(RobustList<StructureBase*>::const_iterator iter = structureList.begin(); iter != structureList.end(); ++iter) {
        StructureBase* pStructure = *iter;

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

    if(pStructure2Select == NULL) {
        // start over at the beginning
        for(RobustList<StructureBase*>::const_iterator iter = structureList.begin(); iter != structureList.end(); ++iter) {
            StructureBase* pStructure = *iter;
            if( (itemIDs.count(pStructure->getItemID()) == 1) && (pStructure->getOwner() == pLocalHouse) && !pStructure->isSelected() ) {
                pStructure2Select = pStructure;
                break;
            }
        }
    }

    if(pStructure2Select != NULL) {
        unselectAll(selectedList);
        selectedList.clear();

        pStructure2Select->setSelected(true);
        selectedList.insert(pStructure2Select->getObjectID());
        currentGame->selectionChanged();

        // we center around the newly selected construction yard
        screenborder->setNewScreenCenter(pStructure2Select->getLocation()*TILESIZE);
    }
}
