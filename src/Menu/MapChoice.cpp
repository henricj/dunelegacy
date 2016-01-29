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

#include <Menu/MapChoice.h>

#include <globals.h>

#include <FileClasses/FileManager.h>
#include <FileClasses/GFXManager.h>
#include <FileClasses/TextManager.h>
#include <FileClasses/INIFile.h>
#include <FileClasses/music/MusicPlayer.h>

#include <misc/string_util.h>

#include <sand.h>

MapChoice::MapChoice(int newHouse, unsigned int LastMission) : MenuBase() {
	disableQuiting(true);
	selectedRegion = -1;

    bFastBlending = false;
	curHouse2Blit = 0;
	curRegion2Blit = 0;
	curBlendBlitter = NULL;
	lastScenario = (LastMission + 1)/3 + 1;
	house = newHouse;

	// set up window
	SDL_Surface *surf;
	surf = pGFXManager->getUIGraphic(UI_MapChoiceScreen,house);

	setBackground(surf,false);
    resize(surf->w,surf->h);

	centerAreaRect.x = screen->w/2 - 320;
	centerAreaRect.y = screen->h/2 - 200;
	centerAreaRect.w = 640;
	centerAreaRect.h = 400;

	msgticker.resize(640,30);

	// load all data from ini
	loadINI();

    if(lastScenario == 1) {
        // create black rectangle
        mapSurface = copySurface(pGFXManager->getUIGraphic(UI_MapChoicePlanet));
        SDL_Rect dest = { 16, 48, 608, 240 };
        SDL_FillRect(mapSurface, &dest, COLOR_BLACK);

        mapChoiceState = MAPCHOICESTATE_FADEINPLANET;

        msgticker.addMessage(_("@DUNE.ENG|283#Three Houses have come to Dune..."));
        msgticker.addMessage(_("@DUNE.ENG|284#...to take control of the land..."));
        msgticker.addMessage(_("@DUNE.ENG|285#...that has become divided."));
    } else {
        mapSurface = NULL;
        mapChoiceState = MAPCHOICESTATE_BLENDING;
        createMapSurfaceWithPieces();
    }
}

MapChoice::~MapChoice() {
	delete curBlendBlitter;
    curBlendBlitter = NULL;

	SDL_FreeSurface(mapSurface);
	mapSurface = NULL;
}

int MapChoice::showMenu()
{
    musicPlayer->changeMusic(MUSIC_MAPCHOICE);

    return MenuBase::showMenu();
}

void MapChoice::drawSpecificStuff() {
	SDL_BlitSurface(mapSurface,NULL,screen,&centerAreaRect);

	switch(mapChoiceState) {

        case MAPCHOICESTATE_FADEINPLANET: {
            if(curBlendBlitter == NULL) {
                SDL_Surface* pSurface = pGFXManager->getUIGraphic(UI_MapChoicePlanet);
                SDL_Rect dest = { 0, 0, static_cast<Uint16>(pSurface->w), static_cast<Uint16>(pSurface->h)};
                curBlendBlitter = new BlendBlitter(pSurface,mapSurface,dest);
            }

            if(curBlendBlitter != NULL) {
                int numSteps = bFastBlending ? 8 : 1;

                for(int i = 0; i < numSteps; i++) {
                    if(curBlendBlitter->nextStep() == 0) {
                        delete curBlendBlitter;
                        curBlendBlitter = NULL;

                        stateSwitchTime = SDL_GetTicks();
                        mapChoiceState = MAPCHOICESTATE_SHOWPLANET;
                        break;
                    }
                }
            }
        } break;

        case MAPCHOICESTATE_SHOWPLANET: {
            if(SDL_GetTicks() - stateSwitchTime > (bFastBlending ? 500 : 4000)) {
                mapChoiceState = MAPCHOICESTATE_BLENDPLANET;
            }
        } break;

        case MAPCHOICESTATE_BLENDPLANET: {
            if(curBlendBlitter == NULL) {
                SDL_Surface* pSurface = pGFXManager->getUIGraphic(UI_MapChoiceMapOnly);
                SDL_Rect dest = { 0, 0, static_cast<Uint16>(pSurface->w), static_cast<Uint16>(pSurface->h)};
                curBlendBlitter = new BlendBlitter(pSurface,mapSurface,dest);
            }

            if(curBlendBlitter != NULL) {
                int numSteps = bFastBlending ? 8 : 1;

                for(int i = 0; i < numSteps; i++) {
                    if(curBlendBlitter->nextStep() == 0) {
                        delete curBlendBlitter;
                        curBlendBlitter = NULL;

                        stateSwitchTime = SDL_GetTicks();
                        mapChoiceState = MAPCHOICESTATE_SHOWMAPONLY;
                        break;
                    }
                }
            }
        } break;

        case MAPCHOICESTATE_SHOWMAPONLY: {
            if(SDL_GetTicks() - stateSwitchTime > (bFastBlending ? 500 : 4000)) {
                mapChoiceState = MAPCHOICESTATE_BLENDMAP;
            }
        } break;

        case MAPCHOICESTATE_BLENDMAP: {
            if(curBlendBlitter == NULL) {
                SDL_Surface* pSurface = pGFXManager->getUIGraphic(UI_MapChoiceMap);
                SDL_Rect dest = { 0, 0, static_cast<Uint16>(pSurface->w), static_cast<Uint16>(pSurface->h)};
                curBlendBlitter = new BlendBlitter(pSurface,mapSurface,dest);
            }

            if(curBlendBlitter != NULL) {
                int numSteps = bFastBlending ? 8 : 1;

                for(int i = 0; i < numSteps; i++) {
                    if(curBlendBlitter->nextStep() == 0) {
                        delete curBlendBlitter;
                        curBlendBlitter = NULL;

                        createMapSurfaceWithPieces();
                        mapChoiceState = MAPCHOICESTATE_BLENDING;
                        break;
                    }
                }
            }
        } break;

        case MAPCHOICESTATE_BLENDING: {
            if(curBlendBlitter == NULL) {
                while(	(curHouse2Blit < NUM_HOUSES) &&
                        (curRegion2Blit >= group[lastScenario].newRegion[(curHouse2Blit + house) % NUM_HOUSES].size())) {
                        curRegion2Blit = 0;
                        curHouse2Blit++;
                }

                if((curHouse2Blit < NUM_HOUSES)&&(curRegion2Blit < group[lastScenario].newRegion[(curHouse2Blit + house) % NUM_HOUSES].size())) {
                    // there is still some region to blend in
                    int pieceNum = (group[lastScenario].newRegion[(curHouse2Blit + house) % NUM_HOUSES])[curRegion2Blit];
                    SDL_Surface* PieceSurface = pGFXManager->getMapChoicePiece(pieceNum,(curHouse2Blit + house) % NUM_HOUSES);
                    SDL_Rect dest = {	static_cast<Sint16>(piecePosition[pieceNum].x),static_cast<Sint16>(piecePosition[pieceNum].y),
                                        static_cast<Uint16>(PieceSurface->w), static_cast<Uint16>(PieceSurface->h)};
                    curBlendBlitter = new BlendBlitter(PieceSurface,mapSurface,dest);
                    curRegion2Blit++;

                    // have to show some text?
                    std::vector<TGroup::TText>::const_iterator iter;
                    for(iter = group[lastScenario].text.begin(); iter != group[lastScenario].text.end(); ++iter) {
                        if(iter->region == pieceNum) {
                            msgticker.addMessage(iter->message);
                        }
                    }

                } else {
                    msgticker.addMessage(_("@DUNE.ENG|286#Select next region"));
                    mapChoiceState = MAPCHOICESTATE_ARROWS;
                }
            }

            if(curBlendBlitter != NULL) {
                int numSteps = bFastBlending ? 8 : 1;

                for(int i = 0; i < numSteps; i++) {
                    if(curBlendBlitter->nextStep() == 0) {
                        delete curBlendBlitter;
                        curBlendBlitter = NULL;
                        break;
                    }
                }
            }
        } break;

        case MAPCHOICESTATE_ARROWS:
        {
            // Draw arrows
            for(int i = 0; i < 4; i++) {
                if(group[lastScenario].attackRegion[i].regionNum == 0) {
                    continue;
                }

                SDL_Surface* arrow = pGFXManager->getMapChoiceArrow(group[lastScenario].attackRegion[i].arrowNum);
                SDL_Rect dest = {	static_cast<Sint16>(group[lastScenario].attackRegion[i].arrowPosition.x + centerAreaRect.x),
                                    static_cast<Sint16>(group[lastScenario].attackRegion[i].arrowPosition.y + centerAreaRect.y),
                                    static_cast<Uint16>(arrow->w),static_cast<Uint16>(arrow->h)};

                SDL_BlitSurface(arrow,NULL,screen,&dest);
            }
        } break;

        case MAPCHOICESTATE_BLINKING:
        {
            if(((SDL_GetTicks() - selectionTime) % 900) < 450) {
                SDL_Surface* pieceSurface = pGFXManager->getMapChoicePiece(selectedRegion,house);
                SDL_Rect dest = {	static_cast<Sint16>(piecePosition[selectedRegion].x + centerAreaRect.x),
                                    static_cast<Sint16>(piecePosition[selectedRegion].y + centerAreaRect.y),
                                    static_cast<Uint16>(pieceSurface->w), static_cast<Uint16>(pieceSurface->h)};
                SDL_BlitSurface(pieceSurface,NULL,screen,&dest);
            }

            if((SDL_GetTicks() - selectionTime) > 2000) {
                int regionIndex;
                for(regionIndex = 0; regionIndex < 4; regionIndex++) {
                    if(group[lastScenario].attackRegion[regionIndex].regionNum == selectedRegion) {
                        break;
                    }
                }

                int newMission;
                if(lastScenario <= 7) {
                    newMission = (lastScenario-1) * 3 + 2 + regionIndex;
                } else if(lastScenario == 8) {
                    newMission = (lastScenario-1) * 3 - 1 + 2 + regionIndex;
                } else {
                    fprintf(stderr,"MapChoice::DrawSpecificStuff(): LastScenario (%d) is no valid scenario!\n", lastScenario);
                    exit(EXIT_FAILURE);
                }

                quit(newMission);
            }
        } break;

	}

	msgticker.draw(screen, Point(centerAreaRect.x + 110, centerAreaRect.y + 320));
}

bool MapChoice::doInput(SDL_Event &event) {
    if((event.type == SDL_MOUSEBUTTONUP) && (event.button.button == SDL_BUTTON_LEFT)) {
	    if(mapChoiceState == MAPCHOICESTATE_ARROWS) {
            int x = event.button.x-centerAreaRect.x;
            int y = event.button.y-centerAreaRect.y;

            if((x > 0) && (x < centerAreaRect.w) && (y > 0) && (y < centerAreaRect.h)) {
                SDL_Surface* clickmap = pGFXManager->getUIGraphic(UI_MapChoiceClickMap);

                if(SDL_LockSurface(clickmap) != 0) {
                    fprintf(stderr,"MapChoice::doInput(): Cannot lock image!\n");
                    exit(EXIT_FAILURE);
                }

                Uint8 regionNum = ((Uint8*)clickmap->pixels)[y * clickmap->pitch + x];

                SDL_UnlockSurface(clickmap);

                if(regionNum != 0) {
                    for(int i = 0; i < 4; i++) {
                        if(group[lastScenario].attackRegion[i].regionNum == regionNum) {
                            mapChoiceState = MAPCHOICESTATE_BLINKING;
                            selectedRegion = regionNum;
                            selectionTime = SDL_GetTicks();
                            break;
                        }
                    }
                }
            }
		} else {
            bFastBlending = true;
		}
	}
	return MenuBase::doInput(event);
}

void MapChoice::createMapSurfaceWithPieces() {
	if(mapSurface != NULL) {
        SDL_FreeSurface(mapSurface);
	}

	// Load map surface
	mapSurface = copySurface(pGFXManager->getUIGraphic(UI_MapChoiceMap));

	for(unsigned int s = 1; s < lastScenario; s++) {
		for(unsigned int h = 0; h < NUM_HOUSES; h++) {
			for(unsigned int p = 0; p < group[s].newRegion[h].size(); p++) {
				int pieceNum = (group[s].newRegion[h])[p];
				SDL_Surface* PieceSurface = pGFXManager->getMapChoicePiece(pieceNum,h);
				SDL_Rect dest = {	static_cast<Sint16>(piecePosition[pieceNum].x),static_cast<Sint16>(piecePosition[pieceNum].y),
									static_cast<Uint16>(PieceSurface->w), static_cast<Uint16>(PieceSurface->h)};
				SDL_BlitSurface(PieceSurface,NULL,mapSurface,&dest);
			}
		}
	}
}

void MapChoice::loadINI() {
	std::string filename = strprintf("REGION%c.INI", houseChar[house]);

	SDL_RWops* file;
	if((file = pFileManager->openFile(filename)) == NULL) {
		fprintf(stderr,"MapChoice::LoadINI(): Cannot open %s!\n",filename.c_str());
		exit(EXIT_FAILURE);
	}

	INIFile RegionINI(file);
	SDL_RWclose(file);


	piecePosition[0].x = 0;
	piecePosition[0].y = 0;

	// read [PIECES]
	for(int i=1; i < 28; i++) {
		char tmp[3];
		sprintf(tmp,"%d",i);
		std::string entry = RegionINI.getStringValue("PIECES",tmp);

		std::string strXPos;
		std::string strYPos;

		if(splitString(entry,2,&strXPos,&strYPos) == false) {
			fprintf(stderr,"MapChoice::LoadINI(): File %s is invalid!\n",filename.c_str());
			exit(EXIT_FAILURE);
		}

		piecePosition[i].x = atol(strXPos.c_str());
		piecePosition[i].y = atol(strYPos.c_str());
		piecePosition[i].x += 8;
		piecePosition[i].y += 24;
		piecePosition[i].x *= 2;
		piecePosition[i].y *= 2;
	}

	for(int i=1; i<=8; i++) {
		char strSection[8];
		sprintf(strSection,"GROUP%d",i);

		// read new regions
		for(int h = 0; h < NUM_HOUSES; h++) {
			std::string key;
			switch(h) {
				case HOUSE_HARKONNEN:	key = "HAR"; break;
				case HOUSE_ATREIDES:	key = "ATR"; break;
				case HOUSE_ORDOS:		key = "ORD"; break;
				case HOUSE_FREMEN:		key = "FRE"; break;
				case HOUSE_SARDAUKAR:	key = "SAR"; break;
				case HOUSE_MERCENARY:	key = "MER"; break;
			}

			std::string strValue = RegionINI.getStringValue(strSection,key);
			if(strValue != "") {
				std::vector<std::string> strRegions = splitString(strValue);

				for(unsigned int r = 0; r < strRegions.size(); r++) {
					group[i].newRegion[h].push_back(atol(strRegions[r].c_str()));
				}
			}
		}

		// read attackRegion (REG1, REG2, REG3)
		for(int a = 0; a < 4; a++) {
			char strKey[5];
			sprintf(strKey,"REG%d",a+1);

			std::string tmp = RegionINI.getStringValue(strSection,strKey);
			if(tmp == "") {
				group[i].attackRegion[a].regionNum = 0;
				group[i].attackRegion[a].arrowNum = 0;
				group[i].attackRegion[a].arrowPosition.x = 0;
				group[i].attackRegion[a].arrowPosition.y = 0;
			} else {
				std::vector<std::string> strAttackRegion = splitString(tmp);

				if(strAttackRegion.size() < 4) {
					fprintf(stderr,"MapChoice::LoadINI(): %s:[%s]/%s has to have 4 numbers!\n",filename.c_str(),strSection,strKey);
					exit(EXIT_FAILURE);
				}

				group[i].attackRegion[a].regionNum = atol(strAttackRegion[0].c_str());
				group[i].attackRegion[a].arrowNum = atol(strAttackRegion[1].c_str());
				group[i].attackRegion[a].arrowPosition.x = atol(strAttackRegion[2].c_str());
				group[i].attackRegion[a].arrowPosition.y = atol(strAttackRegion[3].c_str());
				group[i].attackRegion[a].arrowPosition.x *= 2;
				group[i].attackRegion[a].arrowPosition.y *= 2;
			}
		}

		// read text
		for(int j = 1; j < 28; j++) {
			char key[10];
			sprintf(key,"%sTXT%d",_("LanguageFileExtension").c_str(),j);

			std::string str = convertCP850ToISO8859_1(RegionINI.getStringValue(strSection,key));
			if(str != "") {
				TGroup::TText tmp;
				tmp.message = str;
				tmp.region = j;
				group[i].text.push_back(tmp);
			} else {
                // try TXT? without leading language
                sprintf(key,"TXT%d",j);

                std::string str = convertCP850ToISO8859_1(RegionINI.getStringValue(strSection,key));
                if(str != "") {
                    TGroup::TText tmp;
                    tmp.message = str;
                    tmp.region = j;
                    group[i].text.push_back(tmp);
                }
			}
		}
	}
}
