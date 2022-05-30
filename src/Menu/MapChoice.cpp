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
#include <FileClasses/INIFile.h>
#include <FileClasses/TextManager.h>
#include <FileClasses/music/MusicPlayer.h>

#include "Renderer/DuneRenderer.h"
#include "misc/DrawingRectHelper.h"
#include "misc/draw_util.h"
#include <misc/exceptions.h>
#include <misc/string_util.h>

#include <fmt/printf.h>

#include <algorithm>
#include <chrono>
#include <exception>
#include <memory>
#include <string>
#include <utility>

MapChoice::MapChoice(HOUSETYPE newHouse, unsigned int lastMission, uint32_t oldAlreadyPlayedRegions)
    : house(newHouse), lastScenario((lastMission + 1) / 3 + 1), alreadyPlayedRegions(oldAlreadyPlayedRegions) {
    // We are a top level window
    const auto size = getRendererSize();

    MapChoice::resize(size.w, size.h);

    MapChoice::setWindowWidget(&container_);

    disableQuitting(true);

    auto* const gfx = dune::globals::pGFXManager.get();

    // set up window
    const auto* const pBackground = gfx->getUIGraphic(UI_MapChoiceScreen, house);
    setBackground(pBackground);

    centerAreaRect.x = size.w / 2 - 320;
    centerAreaRect.y = size.h / 2 - 200;
    centerAreaRect.w = 640;
    centerAreaRect.h = 400;

    msgticker.resize(415, 44);
    container_.addWidget(&msgticker, {centerAreaRect.x + 112, centerAreaRect.y + 322}, msgticker.getSize());

    // load all data from ini
    loadINI();

    auto numSelectableRegions = 0;
    auto numRegions           = 0;
    for (auto i = 0; i < 4; i++) {
        const int regionNum = group[lastScenario].attackRegion[i].regionNum;
        if (regionNum > 0) {
            numRegions++;
            if ((alreadyPlayedRegions & (1 << regionNum)) == 0) {
                numSelectableRegions++;
            }
        }
    }

    if (numSelectableRegions < numRegions) {
        // we already were on this screen
        mapSurface     = nullptr;
        mapTexture     = nullptr;
        mapChoiceState = MAPCHOICESTATE_ARROWS;
        createMapSurfaceWithPieces(lastScenario + 1);
    } else {

        if (lastScenario == 1) {
            // first time we're on the map choice screen

            // create black rectangle
            mapSurface = convertSurfaceToDisplayFormat(gfx->getUIGraphicSurface(UI_MapChoicePlanet));
            constexpr SDL_Rect dest{16, 48, 608, 240};
            SDL_FillRect(mapSurface.get(), &dest, COLOR_BLACK);
            mapTexture =
                sdl2::texture_ptr{SDL_CreateTexture(dune::globals::renderer.get(), SCREEN_FORMAT,
                                                    SDL_TEXTUREACCESS_STREAMING, mapSurface->w, mapSurface->h)};
            SDL_SetTextureBlendMode(mapTexture.get(), SDL_BLENDMODE_BLEND);

            mapChoiceState = MAPCHOICESTATE_FADEINPLANET;

            msgticker.addMessage(_("@DUNE.ENG|283#Three Houses have come to Dune..."));
            msgticker.addMessage(_("@DUNE.ENG|284#...to take control of the land..."));
            msgticker.addMessage(_("@DUNE.ENG|285#...that has become divided."));
        } else {
            mapSurface     = nullptr;
            mapTexture     = nullptr;
            mapChoiceState = MAPCHOICESTATE_BLENDING;
            createMapSurfaceWithPieces(lastScenario);
        }
    }

    if (numSelectableRegions == 0) {
        // reset all selectable regions
        for (auto i = 0; i < 4; i++) {
            const int regionNum = group[lastScenario].attackRegion[i].regionNum;
            if (regionNum > 0) {
                alreadyPlayedRegions &= ~(1 << regionNum);
            }
        }
    }
}

MapChoice::~MapChoice() = default;

int MapChoice::showMenuImpl() {
    dune::globals::musicPlayer->changeMusic(MUSIC_MAPCHOICE);

    return parent::showMenuImpl();
}

int MapChoice::getSelectedMission() const {
    auto regionIndex = 0;
    for (regionIndex = 0; regionIndex < 4; regionIndex++) {
        if (group[lastScenario].attackRegion[regionIndex].regionNum == selectedRegion) {
            break;
        }
    }

    auto newMission = 0;
    if (lastScenario <= 7) {
        newMission = (lastScenario - 1) * 3 + 2 + regionIndex;
    } else if (lastScenario == 8) {
        newMission = (lastScenario - 1) * 3 - 1 + 2 + regionIndex;
    } else {
        THROW(std::runtime_error, "lastScenario = %u is no valid scenario number!", lastScenario);
    }
    return newMission;
}

void MapChoice::drawSpecificStuff() {
    using namespace std::chrono_literals;

    auto* const renderer = dune::globals::renderer.get();
    auto* const gfx      = dune::globals::pGFXManager.get();

    SDL_UpdateTexture(mapTexture.get(), nullptr, mapSurface->pixels, mapSurface->pitch);
    Dune_RenderCopy(renderer, mapTexture.get(), nullptr, &centerAreaRect);

    switch (mapChoiceState) {

        case MAPCHOICESTATE_FADEINPLANET: {
            if (curBlendBlitter == nullptr) {
                sdl2::surface_ptr pSurface =
                    convertSurfaceToDisplayFormat(gfx->getUIGraphicSurface(UI_MapChoicePlanet));
                SDL_Rect dest   = {0, 0, getWidth(pSurface.get()), getHeight(pSurface.get())};
                curBlendBlitter = std::make_unique<BlendBlitter>(std::move(pSurface), mapSurface.get(), dest);
            }

            if (curBlendBlitter != nullptr) {
                const int numSteps = bFastBlending ? 8 : 1;

                for (int i = 0; i < numSteps; i++) {
                    if (curBlendBlitter->nextStep() == 0) {
                        curBlendBlitter.reset();

                        stateSwitchTime = dune::dune_clock::now();
                        mapChoiceState  = MAPCHOICESTATE_SHOWPLANET;
                        break;
                    }
                }
            }
        } break;

        case MAPCHOICESTATE_SHOWPLANET: {
            if (dune::dune_clock::now() - stateSwitchTime > (bFastBlending ? 500ms : 4000ms)) {
                mapChoiceState = MAPCHOICESTATE_BLENDPLANET;
            }
        } break;

        case MAPCHOICESTATE_BLENDPLANET: {
            if (curBlendBlitter == nullptr) {
                sdl2::surface_ptr pSurface =
                    convertSurfaceToDisplayFormat(gfx->getUIGraphicSurface(UI_MapChoiceMapOnly));
                SDL_Rect dest   = {0, 0, getWidth(pSurface.get()), getHeight(pSurface.get())};
                curBlendBlitter = std::make_unique<BlendBlitter>(std::move(pSurface), mapSurface.get(), dest);
            }

            if (curBlendBlitter != nullptr) {
                const int numSteps = bFastBlending ? 8 : 1;

                for (auto i = 0; i < numSteps; i++) {
                    if (curBlendBlitter->nextStep() == 0) {
                        curBlendBlitter.reset();

                        stateSwitchTime = dune::dune_clock::now();
                        mapChoiceState  = MAPCHOICESTATE_SHOWMAPONLY;
                        break;
                    }
                }
            }
        } break;

        case MAPCHOICESTATE_SHOWMAPONLY: {
            if (dune::dune_clock::now() - stateSwitchTime > (bFastBlending ? 500ms : 4000ms)) {
                mapChoiceState = MAPCHOICESTATE_BLENDMAP;
            }
        } break;

        case MAPCHOICESTATE_BLENDMAP: {
            if (curBlendBlitter == nullptr) {
                sdl2::surface_ptr pSurface = convertSurfaceToDisplayFormat(gfx->getUIGraphicSurface(UI_MapChoiceMap));
                SDL_Rect dest              = {0, 0, getWidth(pSurface.get()), getHeight(pSurface.get())};
                curBlendBlitter = std::make_unique<BlendBlitter>(std::move(pSurface), mapSurface.get(), dest);
            }

            if (curBlendBlitter != nullptr) {
                const int numSteps = bFastBlending ? 8 : 1;

                for (auto i = 0; i < numSteps; i++) {
                    if (curBlendBlitter->nextStep() == 0) {
                        curBlendBlitter.reset();

                        createMapSurfaceWithPieces(lastScenario);
                        mapChoiceState = MAPCHOICESTATE_BLENDING;
                        break;
                    }
                }
            }
        } break;

        case MAPCHOICESTATE_BLENDING: {
            if (curBlendBlitter == nullptr) {
                const auto int_house = static_cast<int>(house);

                const auto region = [&] { return (static_cast<int>(curHouse2Blit) + int_house) % NUM_HOUSES; };

                const auto blitThreshold = [&] { return group[lastScenario].newRegion[region()].size(); };

                while (curHouse2Blit < HOUSETYPE::NUM_HOUSES && curRegion2Blit >= blitThreshold()) {
                    curRegion2Blit = 0;
                    curHouse2Blit  = static_cast<HOUSETYPE>(static_cast<int>(curHouse2Blit) + 1);
                }

                if (curHouse2Blit < HOUSETYPE::NUM_HOUSES && curRegion2Blit < blitThreshold()) {
                    // there is still some region to blend in
                    const auto pieceNum = group[lastScenario].newRegion[region()][curRegion2Blit];
                    auto* surface       = gfx->getMapChoicePieceSurface(pieceNum, static_cast<HOUSETYPE>(region()));
                    auto pPieceSurface  = convertSurfaceToDisplayFormat(surface);
                    auto dest =
                        calcDrawingRect(pPieceSurface.get(), piecePosition[pieceNum].x, piecePosition[pieceNum].y);
                    curBlendBlitter = std::make_unique<BlendBlitter>(std::move(pPieceSurface), mapSurface.get(), dest);
                    curRegion2Blit++;

                    // have to show some text?
                    for (const auto& ttext : group[lastScenario].text) {
                        if (ttext.region == pieceNum) {
                            msgticker.addMessage(ttext.message);
                        }
                    }

                } else {
                    msgticker.addMessage(_("@DUNE.ENG|286#Select next region"));
                    mapChoiceState = MAPCHOICESTATE_ARROWS;
                }
            }

            if (curBlendBlitter != nullptr) {
                const int numSteps = bFastBlending ? 8 : 1;

                for (auto i = 0; i < numSteps; i++) {
                    if (curBlendBlitter->nextStep() == 0) {
                        curBlendBlitter.reset();
                        break;
                    }
                }
            }
        } break;

        case MAPCHOICESTATE_ARROWS: {
            // Draw arrows
            for (int i = 0; i < 4; i++) {
                const int regionNum = group[lastScenario].attackRegion[i].regionNum;
                if (regionNum == 0) {
                    continue;
                }

                if (alreadyPlayedRegions & 1 << regionNum) {
                    continue;
                }

                const int arrowNum = std::max<int>(0, std::min<int>(8, group[lastScenario].attackRegion[i].arrowNum));
                const auto* const arrow =
                    gfx->getUIGraphic(static_cast<UIGraphics_Enum>(UI_MapChoiceArrow_None + arrowNum), house);
                const int arrowFrame =
                    static_cast<int>(dune::as_milliseconds(dune::dune_clock::now().time_since_epoch()) / 128U % 4U);
                assert(arrowFrame >= 0 && arrowFrame < 4);
                const auto src  = calcSpriteSourceRect(arrow, arrowFrame, 4);
                const auto dest = calcSpriteDrawingRectF(
                    arrow, group[lastScenario].attackRegion[i].arrowPosition.x + centerAreaRect.x,
                    group[lastScenario].attackRegion[i].arrowPosition.y + centerAreaRect.y, 4, 1);

                Dune_RenderCopyF(renderer, arrow, &src, &dest);
            }
        } break;

        case MAPCHOICESTATE_BLINKING: {
            if (dune::as_milliseconds(dune::dune_clock::now() - selectionTime) % 900 < 450) {
                if (const auto* const pieceTexture = gfx->getMapChoicePiece(selectedRegion, house)) {
                    pieceTexture->draw(renderer, piecePosition[selectedRegion].x + centerAreaRect.x,
                                       piecePosition[selectedRegion].y + centerAreaRect.y);
                }
            }

            for (auto i = 0; i < 4; i++) {
                if (group[lastScenario].attackRegion[i].regionNum != selectedRegion) {
                    continue;
                }

                const int arrowNum = std::max<int>(0, std::min<int>(8, group[lastScenario].attackRegion[i].arrowNum));
                const auto* const arrow =
                    gfx->getUIGraphic(static_cast<UIGraphics_Enum>(UI_MapChoiceArrow_None + arrowNum), house);
                const int arrowFrame =
                    static_cast<int>(dune::as_milliseconds(dune::dune_clock::now().time_since_epoch()) / 128 % 4);
                const auto src  = calcSpriteSourceRect(arrow, arrowFrame, 4);
                const auto dest = calcSpriteDrawingRectF(
                    arrow, group[lastScenario].attackRegion[i].arrowPosition.x + centerAreaRect.x,
                    group[lastScenario].attackRegion[i].arrowPosition.y + centerAreaRect.y, 4, 1);

                Dune_RenderCopyF(renderer, arrow, &src, &dest);
            }

            if (dune::dune_clock::now() - selectionTime > 2000ms) {
                quit();
            }
        } break;
    }
}

void MapChoice::doInputImpl(const SDL_Event& event) {
    if (event.type == SDL_MOUSEBUTTONUP && event.button.button == SDL_BUTTON_LEFT) {
        if (mapChoiceState == MAPCHOICESTATE_ARROWS) {
            const int x = event.button.x - centerAreaRect.x;
            const int y = event.button.y - centerAreaRect.y;

            if (x > 0 && x < centerAreaRect.w && y > 0 && y < centerAreaRect.h) {
                auto* const clickmap = dune::globals::pGFXManager->getUIGraphicSurface(UI_MapChoiceClickMap);

                uint8_t regionNum = 0;
                {
                    // Scope
                    sdl2::surface_lock lock{clickmap};

                    regionNum = static_cast<uint8_t*>(clickmap->pixels)[y * clickmap->pitch + x];
                }

                if (regionNum != 0 && (alreadyPlayedRegions & 1 << regionNum) == 0) {
                    for (int i = 0; i < 4; i++) {
                        if (group[lastScenario].attackRegion[i].regionNum == regionNum) {
                            mapChoiceState = MAPCHOICESTATE_BLINKING;
                            selectedRegion = static_cast<UIGraphics_Enum>(regionNum);
                            alreadyPlayedRegions |= 1 << selectedRegion;
                            selectionTime = dune::dune_clock::now();
                            break;
                        }
                    }
                }
            }
        } else {
            bFastBlending = true;
        }
    }

    parent::doInputImpl(event);
}

void MapChoice::resize(uint32_t width, uint32_t height) {
    parent::resize(width, height);

    centerAreaRect.x = static_cast<int>(width) / 2 - 320;
    centerAreaRect.y = static_cast<int>(height) / 2 - 200;

    container_.setWidgetGeometry(&msgticker, {centerAreaRect.x + 112, centerAreaRect.y + 322}, msgticker.getSize());
}

void MapChoice::createMapSurfaceWithPieces(unsigned int scenario) {
    auto* const gfx = dune::globals::pGFXManager.get();

    // Load map surface
    mapSurface = convertSurfaceToDisplayFormat(gfx->getUIGraphicSurface(UI_MapChoiceMap));
    mapTexture = sdl2::texture_ptr{SDL_CreateTexture(dune::globals::renderer.get(), SCREEN_FORMAT,
                                                     SDL_TEXTUREACCESS_STREAMING, mapSurface->w, mapSurface->h)};
    SDL_SetTextureBlendMode(mapTexture.get(), SDL_BLENDMODE_BLEND);

    if (group.size() < 2)
        return;

    for (unsigned int s = 1; s < scenario; s++) {
        const auto& g = group[s];

        for_each_housetype([&](const auto h) {
            for (auto pieceNum : g.newRegion[static_cast<int>(h)]) {
                auto* const pieceSurface = gfx->getMapChoicePieceSurface(pieceNum, h);
                SDL_Rect dest = calcDrawingRect(pieceSurface, piecePosition[pieceNum].x, piecePosition[pieceNum].y);
                SDL_BlitSurface(pieceSurface, nullptr, mapSurface.get(), &dest);
            }
        });
    }
}

void MapChoice::loadINI() {
    const auto filename = fmt::sprintf("REGION%c.INI", dune::globals::houseChar[static_cast<int>(house)]);

    INIFile RegionINI(dune::globals::pFileManager->openFile(filename).get());

    piecePosition[0].x = 0;
    piecePosition[0].y = 0;

    // read [PIECES]
    for (auto i = 1u; i < piecePosition.size(); i++) {
        const auto entry = RegionINI.getStringValue("PIECES", std::to_string(i));

        std::string strXPos;
        std::string strYPos;

        if (!splitString(entry, strXPos, strYPos)) {
            THROW(std::runtime_error, "File '%s' contains invalid value for key '%d'", filename, i);
        }

        auto& coord = piecePosition[i];

        coord.x = parseStringThrows<int>(strXPos);
        coord.y = parseStringThrows<int>(strYPos);
        coord.x += 8;
        coord.y += 24;
        coord.x *= 2;
        coord.y *= 2;
    }

    for (int i = 1; i <= 8; i++) {
        const auto strSection = fmt::format("GROUP{}", i);

        // read new regions
        for_each_housetype([&](const auto h) {
            std::string key;
            // clang-format off
            switch (h) {
                case HOUSETYPE::HOUSE_HARKONNEN: key = "HAR";
                    break;
                case HOUSETYPE::HOUSE_ATREIDES: key = "ATR";
                    break;
                case HOUSETYPE::HOUSE_ORDOS: key = "ORD";
                    break;
                case HOUSETYPE::HOUSE_FREMEN: key = "FRE";
                    break;
                case HOUSETYPE::HOUSE_SARDAUKAR: key = "SAR";
                    break;
                case HOUSETYPE::HOUSE_MERCENARY: key = "MER";
                    break;
            }
            // clang-format on

            const auto strValue = RegionINI.getStringValue(strSection, key);
            if (!strValue.empty()) {
                const auto strRegions = splitStringToStringVector(strValue);

                for (auto& strRegion : strRegions) {
                    int value;
                    if (!parseString(strRegion, value))
                        THROW(std::runtime_error, "File '%s' contains invalid region value '%s' for section [%s]!",
                              filename, strRegion, strSection);

                    group[i].newRegion[static_cast<int>(h)].push_back(static_cast<UIGraphics_Enum>(value));
                }
            }
        });

        // read attackRegion (REG1, REG2, REG3)
        for (int a = 0; a < 4; a++) {
            auto strKey = fmt::format("REG{}", a + 1);

            auto tmp = RegionINI.getStringValue(strSection, strKey);

            auto& attack_region = group[i].attackRegion[a];

            if (tmp.empty()) {
                attack_region.regionNum       = 0;
                attack_region.arrowNum        = 0;
                attack_region.arrowPosition.x = 0;
                attack_region.arrowPosition.y = 0;
            } else {
                auto strAttackRegion = splitStringToStringVector(tmp);

                if (strAttackRegion.size() < 4) {
                    THROW(std::runtime_error,
                          "File '%s' contains invalid value for key [%s]/%s; it has to consist of 4 numbers!", filename,
                          strSection, strKey);
                }

                attack_region.regionNum       = parseStringThrows<int>(strAttackRegion[0]);
                attack_region.arrowNum        = parseStringThrows<int>(strAttackRegion[1]);
                attack_region.arrowPosition.x = parseStringThrows<int>(strAttackRegion[2]);
                attack_region.arrowPosition.y = parseStringThrows<int>(strAttackRegion[3]);
                attack_region.arrowPosition.x *= 2;
                attack_region.arrowPosition.y *= 2;
            }
        }

        // read text
        for (auto j = 1; j < 28; j++) {
            auto key = _("LanguageFileExtension") + fmt::format("TXT{}", j);

            if (!RegionINI.hasKey(strSection, key)) {
                // Workaround for bug in REGIONO.INI / GROUP1 / GERTXT 6: Add space after TXT
                key = _("LanguageFileExtension") + fmt::format("TXT {}", j);
            }

            auto str = convertCP850ToUTF8(RegionINI.getStringValue(strSection, key));
            if (str.empty()) {
                // try TXT? without leading language
                key = fmt::sprintf("TXT%d", j);

                str = convertCP850ToUTF8(RegionINI.getStringValue(strSection, key));
                if (str.empty())
                    continue;
            }

#ifdef HAVE_PARENTHESIZED_INITIALIZATION_OF_AGGREGATES
            group[i].text.emplace_back(std::move(str), j);
#else
            group[i].text.push_back({std::move(str), j});
#endif
        }
    }
}
