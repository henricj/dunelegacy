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
#include <misc/exceptions.h>
#include <misc/format.h>

#include <sand.h>

MapChoice::MapChoice(int newHouse, unsigned int lastMission, Uint32 oldAlreadyPlayedRegions) : MenuBase() {
    disableQuiting(true);
    selectedRegion = -1;
    selectionTime = 0;
    stateSwitchTime = 0;

    bFastBlending = false;
    curHouse2Blit = 0;
    curRegion2Blit = 0;
    curBlendBlitter = nullptr;
    lastScenario = (lastMission + 1)/3 + 1;
    alreadyPlayedRegions = oldAlreadyPlayedRegions;
    house = newHouse;

    // set up window
    SDL_Texture *pBackground = pGFXManager->getUIGraphic(UI_MapChoiceScreen, house);
    setBackground(pBackground);
    resize(getTextureSize(pBackground));

    centerAreaRect.x = getRendererWidth()/2 - 320;
    centerAreaRect.y = getRendererHeight()/2 - 200;
    centerAreaRect.w = 640;
    centerAreaRect.h = 400;

    msgticker.resize(640,30);

    // load all data from ini
    loadINI();

    int numSelectableRegions = 0;
    int numRegions = 0;
    for(int i = 0; i < 4; i++) {
        const int regionNum = group[lastScenario].attackRegion[i].regionNum;
        if(regionNum > 0) {
            numRegions++;
            if((alreadyPlayedRegions & (1 << regionNum)) == 0) {
                numSelectableRegions++;
            }
        }
    }

    if(numSelectableRegions < numRegions) {
        // we already were on this screen
        mapSurface = nullptr;
        mapTexture = nullptr;
        mapChoiceState = MAPCHOICESTATE_ARROWS;
        createMapSurfaceWithPieces(lastScenario+1);
    } else {

        if(lastScenario == 1) {
            // first time we're on the map choice screen

            // create black rectangle
            mapSurface = convertSurfaceToDisplayFormat(pGFXManager->getUIGraphicSurface(UI_MapChoicePlanet));
            SDL_Rect dest = { 16, 48, 608, 240 };
            SDL_FillRect(mapSurface.get(), &dest, COLOR_BLACK);
            mapTexture = sdl2::texture_ptr{ SDL_CreateTexture(renderer, SCREEN_FORMAT, SDL_TEXTUREACCESS_STREAMING, mapSurface->w, mapSurface->h) };
            SDL_SetTextureBlendMode(mapTexture.get(), SDL_BLENDMODE_BLEND);

            mapChoiceState = MAPCHOICESTATE_FADEINPLANET;

            msgticker.addMessage(_("@DUNE.ENG|283#Three Houses have come to Dune..."));
            msgticker.addMessage(_("@DUNE.ENG|284#...to take control of the land..."));
            msgticker.addMessage(_("@DUNE.ENG|285#...that has become divided."));
        } else {
            mapSurface = nullptr;
            mapTexture = nullptr;
            mapChoiceState = MAPCHOICESTATE_BLENDING;
            createMapSurfaceWithPieces(lastScenario);
        }
    }

    if(numSelectableRegions == 0) {
        // reset all selectable regions
        for(int i = 0; i < 4; i++) {
            const int regionNum = group[lastScenario].attackRegion[i].regionNum;
            if(regionNum > 0) {
                alreadyPlayedRegions &= ~(1 << regionNum);
            }
        }
    }
}

MapChoice::~MapChoice() = default;

int MapChoice::showMenu()
{
    musicPlayer->changeMusic(MUSIC_MAPCHOICE);

    return MenuBase::showMenu();
}

void MapChoice::drawSpecificStuff() {
    SDL_UpdateTexture(mapTexture.get(), nullptr, mapSurface->pixels, mapSurface->pitch);
    SDL_RenderCopy(renderer, mapTexture.get(), nullptr, &centerAreaRect);

    switch(mapChoiceState) {

        case MAPCHOICESTATE_FADEINPLANET: {
            if(curBlendBlitter == nullptr) {
                sdl2::surface_ptr pSurface = convertSurfaceToDisplayFormat(pGFXManager->getUIGraphicSurface(UI_MapChoicePlanet));
                SDL_Rect dest = { 0, 0, getWidth(pSurface.get()), getHeight(pSurface.get()) };
                curBlendBlitter = std::make_unique<BlendBlitter>(std::move(pSurface), mapSurface.get(), dest);
            }

            if(curBlendBlitter != nullptr) {
                const int numSteps = bFastBlending ? 8 : 1;

                for(int i = 0; i < numSteps; i++) {
                    if(curBlendBlitter->nextStep() == 0) {
                        curBlendBlitter.reset();

                        stateSwitchTime = SDL_GetTicks();
                        mapChoiceState = MAPCHOICESTATE_SHOWPLANET;
                        break;
                    }
                }
            }
        } break;

        case MAPCHOICESTATE_SHOWPLANET: {
            if(SDL_GetTicks() - stateSwitchTime > (bFastBlending ? 500U : 4000U)) {
                mapChoiceState = MAPCHOICESTATE_BLENDPLANET;
            }
        } break;

        case MAPCHOICESTATE_BLENDPLANET: {
            if(curBlendBlitter == nullptr) {
                sdl2::surface_ptr pSurface = convertSurfaceToDisplayFormat(pGFXManager->getUIGraphicSurface(UI_MapChoiceMapOnly));
                SDL_Rect dest = { 0, 0, getWidth(pSurface.get()), getHeight(pSurface.get()) };
                curBlendBlitter = std::make_unique< BlendBlitter>(std::move(pSurface), mapSurface.get(), dest);
            }

            if(curBlendBlitter != nullptr) {
                const int numSteps = bFastBlending ? 8 : 1;

                for(int i = 0; i < numSteps; i++) {
                    if(curBlendBlitter->nextStep() == 0) {
                        curBlendBlitter.reset();

                        stateSwitchTime = SDL_GetTicks();
                        mapChoiceState = MAPCHOICESTATE_SHOWMAPONLY;
                        break;
                    }
                }
            }
        } break;

        case MAPCHOICESTATE_SHOWMAPONLY: {
            if(SDL_GetTicks() - stateSwitchTime > (bFastBlending ? 500U : 4000U)) {
                mapChoiceState = MAPCHOICESTATE_BLENDMAP;
            }
        } break;

        case MAPCHOICESTATE_BLENDMAP: {
            if(curBlendBlitter == nullptr) {
                sdl2::surface_ptr pSurface = convertSurfaceToDisplayFormat(pGFXManager->getUIGraphicSurface(UI_MapChoiceMap));
                SDL_Rect dest = { 0, 0, getWidth(pSurface.get()), getHeight(pSurface.get()) };
                curBlendBlitter = std::make_unique<BlendBlitter>(std::move(pSurface), mapSurface.get(), dest);
            }

            if(curBlendBlitter != nullptr) {
                const int numSteps = bFastBlending ? 8 : 1;

                for(int i = 0; i < numSteps; i++) {
                    if(curBlendBlitter->nextStep() == 0) {
                        curBlendBlitter.reset();

                        createMapSurfaceWithPieces(lastScenario);
                        mapChoiceState = MAPCHOICESTATE_BLENDING;
                        break;
                    }
                }
            }
        } break;

        case MAPCHOICESTATE_BLENDING: {
            if(curBlendBlitter == nullptr) {
                while(  (curHouse2Blit < NUM_HOUSES) &&
                        (curRegion2Blit >= group[lastScenario].newRegion[(curHouse2Blit + house) % NUM_HOUSES].size())) {
                        curRegion2Blit = 0;
                        curHouse2Blit++;
                }

                if((curHouse2Blit < NUM_HOUSES)&&(curRegion2Blit < group[lastScenario].newRegion[(curHouse2Blit + house) % NUM_HOUSES].size())) {
                    // there is still some region to blend in
                    const int pieceNum = (group[lastScenario].newRegion[(curHouse2Blit + house) % NUM_HOUSES])[curRegion2Blit];
                    sdl2::surface_ptr pPieceSurface = convertSurfaceToDisplayFormat(pGFXManager->getMapChoicePieceSurface(pieceNum,(curHouse2Blit + house) % NUM_HOUSES));
                    SDL_Rect dest = calcDrawingRect(pPieceSurface.get(), piecePosition[pieceNum].x, piecePosition[pieceNum].y);
                    curBlendBlitter = std::make_unique<BlendBlitter>(std::move(pPieceSurface), mapSurface.get(), dest);
                    curRegion2Blit++;

                    // have to show some text?
                    for(const TGroup::TText& ttext : group[lastScenario].text) {
                        if(ttext.region == pieceNum) {
                            msgticker.addMessage(ttext.message);
                        }
                    }

                } else {
                    msgticker.addMessage(_("@DUNE.ENG|286#Select next region"));
                    mapChoiceState = MAPCHOICESTATE_ARROWS;
                }
            }

            if(curBlendBlitter != nullptr) {
                const int numSteps = bFastBlending ? 8 : 1;

                for(int i = 0; i < numSteps; i++) {
                    if(curBlendBlitter->nextStep() == 0) {
                        curBlendBlitter.reset();
                        break;
                    }
                }
            }
        } break;

        case MAPCHOICESTATE_ARROWS:
        {
            // Draw arrows
            for(int i = 0; i < 4; i++) {
                const int regionNum = group[lastScenario].attackRegion[i].regionNum;
                if(regionNum == 0) {
                    continue;
                }

                if(alreadyPlayedRegions & (1 << regionNum)) {
                    continue;
                }

                const int arrowNum = std::max<int>(0, std::min<int>(8, group[lastScenario].attackRegion[i].arrowNum));
                SDL_Texture* arrow = pGFXManager->getUIGraphic(UI_MapChoiceArrow_None + arrowNum, house);
                const int arrowFrame = (SDL_GetTicks() / 128) % 4;
                SDL_Rect src = calcSpriteSourceRect(arrow, arrowFrame, 4);
                SDL_Rect dest = calcSpriteDrawingRect(  arrow,
                                                        group[lastScenario].attackRegion[i].arrowPosition.x + centerAreaRect.x,
                                                        group[lastScenario].attackRegion[i].arrowPosition.y + centerAreaRect.y,
                                                        4, 1);

                SDL_RenderCopy(renderer, arrow, &src, &dest);
            }
        } break;

        case MAPCHOICESTATE_BLINKING:
        {
            if(((SDL_GetTicks() - selectionTime) % 900) < 450) {
                SDL_Texture* pieceTexture = pGFXManager->getMapChoicePiece(selectedRegion,house);
                SDL_Rect dest = calcDrawingRect(pieceTexture, piecePosition[selectedRegion].x + centerAreaRect.x, piecePosition[selectedRegion].y + centerAreaRect.y);
                SDL_RenderCopy(renderer, pieceTexture, nullptr, &dest);
            }

            for(int i = 0; i < 4; i++) {
                if(group[lastScenario].attackRegion[i].regionNum != selectedRegion) {
                    continue;
                }

                const int arrowNum = std::max<int>(0, std::min<int>(8, group[lastScenario].attackRegion[i].arrowNum));
                SDL_Texture* arrow = pGFXManager->getUIGraphic(UI_MapChoiceArrow_None + arrowNum, house);
                const int arrowFrame = (SDL_GetTicks() / 128) % 4;
                SDL_Rect src = calcSpriteSourceRect(arrow, arrowFrame, 4);
                SDL_Rect dest = calcSpriteDrawingRect(  arrow,
                                                        group[lastScenario].attackRegion[i].arrowPosition.x + centerAreaRect.x,
                                                        group[lastScenario].attackRegion[i].arrowPosition.y + centerAreaRect.y,
                                                        4, 1);

                SDL_RenderCopy(renderer, arrow, &src, &dest);
            }

            if((SDL_GetTicks() - selectionTime) > 2000) {
                quit();
            }
        } break;

    }

    msgticker.draw(Point(centerAreaRect.x + 110, centerAreaRect.y + 320));
}

bool MapChoice::doInput(SDL_Event &event) {
    if((event.type == SDL_MOUSEBUTTONUP) && (event.button.button == SDL_BUTTON_LEFT)) {
        if(mapChoiceState == MAPCHOICESTATE_ARROWS) {
            int x = event.button.x-centerAreaRect.x;
            int y = event.button.y-centerAreaRect.y;

            if((x > 0) && (x < centerAreaRect.w) && (y > 0) && (y < centerAreaRect.h)) {
                SDL_Surface* clickmap = pGFXManager->getUIGraphicSurface(UI_MapChoiceClickMap);

                Uint8 regionNum;
                {
                    sdl2::surface_lock lock{ clickmap };

                    regionNum = static_cast<Uint8*>(clickmap->pixels)[y * clickmap->pitch + x];
                }

                if((regionNum != 0) && ((alreadyPlayedRegions & (1 << regionNum)) == 0)) {
                    for(int i = 0; i < 4; i++) {
                        if(group[lastScenario].attackRegion[i].regionNum == regionNum) {
                            mapChoiceState = MAPCHOICESTATE_BLINKING;
                            selectedRegion = regionNum;
                            alreadyPlayedRegions |= (1 << selectedRegion);
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

void MapChoice::createMapSurfaceWithPieces(unsigned int scenario) {
    // Load map surface
    mapSurface = convertSurfaceToDisplayFormat(pGFXManager->getUIGraphicSurface(UI_MapChoiceMap));
    mapTexture = sdl2::texture_ptr{ SDL_CreateTexture(renderer, SCREEN_FORMAT, SDL_TEXTUREACCESS_STREAMING, mapSurface->w, mapSurface->h) };
    SDL_SetTextureBlendMode(mapTexture.get(), SDL_BLENDMODE_BLEND);

    if (group.size() < 2)
        return;

    for(unsigned int s = 1; s < scenario; s++) {
        auto g = group[s];
        for(unsigned int h = 0; h < g.newRegion.size(); h++) {
            for (int pieceNum : g.newRegion[h]) {
                SDL_Surface* pieceSurface = pGFXManager->getMapChoicePieceSurface(pieceNum, h);
                SDL_Rect dest = calcDrawingRect(pieceSurface, piecePosition[pieceNum].x, piecePosition[pieceNum].y);
                SDL_BlitSurface(pieceSurface,nullptr,mapSurface.get(),&dest);
            }
        }
    }
}

void MapChoice::loadINI() {
    const std::string filename = fmt::sprintf("REGION%c.INI", houseChar[house]);

    INIFile RegionINI(pFileManager->openFile(filename).get());

    piecePosition[0].x = 0;
    piecePosition[0].y = 0;

    // read [PIECES]
    for(int i=1; i < 28; i++) {
        std::string entry = RegionINI.getStringValue("PIECES",std::to_string(i));

        std::string strXPos;
        std::string strYPos;

        if(splitString(entry, strXPos, strYPos) == false) {
            THROW(std::runtime_error, "File '%s' contains invalid value for key '%d'", filename, i);
        }

        piecePosition[i].x = atol(strXPos.c_str());
        piecePosition[i].y = atol(strYPos.c_str());
        piecePosition[i].x += 8;
        piecePosition[i].y += 24;
        piecePosition[i].x *= 2;
        piecePosition[i].y *= 2;
    }

    for(int i=1; i<=8; i++) {
        std::string strSection = "GROUP" + std::to_string(i);

        // read new regions
        for(int h = 0; h < NUM_HOUSES; h++) {
            std::string key;
            switch(h) {
                case HOUSE_HARKONNEN:   key = "HAR"; break;
                case HOUSE_ATREIDES:    key = "ATR"; break;
                case HOUSE_ORDOS:       key = "ORD"; break;
                case HOUSE_FREMEN:      key = "FRE"; break;
                case HOUSE_SARDAUKAR:   key = "SAR"; break;
                case HOUSE_MERCENARY:   key = "MER"; break;
            }

            std::string strValue = RegionINI.getStringValue(strSection,key);
            if(strValue != "") {
                std::vector<std::string> strRegions = splitStringToStringVector(strValue);

                for(unsigned int r = 0; r < strRegions.size(); r++) {
                    group[i].newRegion[h].push_back(atol(strRegions[r].c_str()));
                }
            }
        }

        // read attackRegion (REG1, REG2, REG3)
        for(int a = 0; a < 4; a++) {
            std::string strKey = "REG" + std::to_string(a+1);

            std::string tmp = RegionINI.getStringValue(strSection,strKey);
            if(tmp == "") {
                group[i].attackRegion[a].regionNum = 0;
                group[i].attackRegion[a].arrowNum = 0;
                group[i].attackRegion[a].arrowPosition.x = 0;
                group[i].attackRegion[a].arrowPosition.y = 0;
            } else {
                std::vector<std::string> strAttackRegion = splitStringToStringVector(tmp);

                if(strAttackRegion.size() < 4) {
                    THROW(std::runtime_error, "File '%s' contains invalid value for key [%s]/%s; it has to consist of 4 numbers!", filename, strSection, strKey);
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
            std::string key = _("LanguageFileExtension") + "TXT" + std::to_string(j);

            if(!RegionINI.hasKey(strSection, key)) {
                // Workaround for bug in REGIONO.INI / GROUP1 / GERTXT 6: Add space after TXT
                key = _("LanguageFileExtension") + "TXT " + std::to_string(j);
            }

            std::string str = convertCP850ToUTF8(RegionINI.getStringValue(strSection,key));
            if(!str.empty()) {
                TGroup::TText tmp;
                tmp.message = str;
                tmp.region = j;
                group[i].text.push_back(tmp);
            } else {
                // try TXT? without leading language
                std::string key = std::string("TXT") + std::to_string(j);

                std::string str = convertCP850ToUTF8(RegionINI.getStringValue(strSection,key));
                if(!str.empty()) {
                    TGroup::TText tmp;
                    tmp.message = str;
                    tmp.region = j;
                    group[i].text.push_back(tmp);
                }
            }
        }
    }
}
