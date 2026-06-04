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

#include <Bullet.h>
#include <FileClasses/GFXManager.h>
#include <FileClasses/LoadSavePNG.h>
#include <GUI/GUIStyle.h>
#include <GUI/dune/InGameMenu.h>
#include <GUI/dune/WaitingForOtherPlayers.h>
#include <GameInterface.h>
#include <Map.h>
#include <Menu/MentatHelp.h>
#include <ScreenBorder.h>
#include <misc/DrawingRectHelper.h>
#include <misc/draw_util.h>
#include <misc/dune_clock.h>
#include <misc/dune_sdl.h>
#include <misc/exceptions.h>
#include <sand.h>
#include <structures/BuilderBase.h>
#include <structures/StructureBase.h>

#include <fmt/format.h>

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
                             zoomedTileSize * map_->getSizeX(),
                             zoomedTileSize * map_->getSizeY()};
    auto& board_rect = screenborder->getGameBoard();
    const SDL_Rect game_board_rect{static_cast<int>(std::ceil(board_rect.x)),
                                   static_cast<int>(std::ceil(board_rect.y)),
                                   static_cast<int>(std::floor(board_rect.w)),
                                   static_cast<int>(std::floor(board_rect.h))};
    SDL_Rect on_screen_rect;
    SDL_GetRectIntersection(&game_board_rect, &tile_rect, &on_screen_rect);

    SDL_SetRenderClipRect(renderer, &on_screen_rect);

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
        auto* const pStructure = getObjectManager().getObject<StructureBase>(*selectedList_.begin());
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
                                         screenborder->world2screenY(y * TILESIZE),
                                         static_cast<float>(zoomedTileSize),
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

    SDL_SetRenderClipRect(renderer, nullptr);

    /////////////draw placement position

    if (currentCursorMode == CursorMode_Placing) {
        // if user has selected to place a structure

        if (screenborder->isScreenCoordInsideMap(dune::globals::drawnMouseX, dune::globals::drawnMouseY)) {
            // if mouse is not over game bar

            const int xPos = screenborder->screen2MapX(dune::globals::drawnMouseX);
            const int yPos = screenborder->screen2MapY(dune::globals::drawnMouseY);

            if (selectedList_.size() == 1) {
                if (auto* pBuilder = objectManager_.getObject<BuilderBase>(*selectedList_.begin())) {
                    const auto placeItem = pBuilder->getCurrentProducedItem();
                    auto structureSize   = getStructureSize(placeItem);

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

                    map_->for_each(xPos,
                                   yPos,
                                   xPos + structureSize.x,
                                   yPos + structureSize.y,
                                   [withinRange,
                                    invalidPlace,
                                    validPlace,
                                    is_slab,
                                    renderer     = dune::globals::renderer.get(),
                                    screenborder = dune::globals::screenborder.get()](auto& t) {
                                       const DuneTexture* image = nullptr;

                                       if (!withinRange || !t.isRock() || t.isMountain() || t.hasAGroundObject()
                                           || (is_slab && t.isConcrete())) {
                                           image = invalidPlace;
                                       } else {
                                           image = validPlace;
                                       }

                                       image->draw(renderer,
                                                   screenborder->world2screenX(t.getLocation().x * TILESIZE),
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
        auto drawLocation              = calcSpriteDrawingRectF(pUIIndicator,
                                                   screenborder->world2screenX(indicatorPosition_.x),
                                                   screenborder->world2screenY(indicatorPosition_.y),
                                                   3,
                                                   1,
                                                   HAlign::Center,
                                                   VAlign::Center);
        Dune_RenderCopyF(renderer, pUIIndicator, &source, &drawLocation);
    }

    ///////////draw game bar
    uiController_.getGameInterface()->draw({});
    uiController_.getGameInterface()->drawOverlay({});

    const auto& gui = GUIStyle::getInstance();

    const auto renderer_height = static_cast<float>(getRendererHeight());

    // draw chat message currently typed
    if (chatMode_) {
        const auto pChatTexture = gui.createText(
            renderer,
            "Chat: " + typingChatMessage_
                + (((dune::as_milliseconds(dune::dune_clock::now().time_since_epoch()) / 150) % 2 == 0) ? "_" : ""),
            COLOR_WHITE,
            14);

        pChatTexture.draw(renderer, 20.f, renderer_height - 40.f);
    }

    if (bShowFPS_) {
        const auto str = fmt::sprintf("fps: %4.1f\nrenderer: %4.1fms\nupdate: %4.1fms",
                                      1000.0f / averageFrameTime_,
                                      averageRenderTime_,
                                      averageUpdateTime_);

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

        SDL_RenderFillRects(renderer, rects.data(), static_cast<int>(rects.size()));
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

    if (uiController_.getWaitingForOtherPlayers() != nullptr) {
        uiController_.getWaitingForOtherPlayers()->draw();
    }

    if (uiController_.getInGameMenu() != nullptr) {
        uiController_.getInGameMenu()->draw();
    } else if (uiController_.getInGameMentat() != nullptr) {
        uiController_.getInGameMentat()->draw();
    }

    drawCursor(on_screen_rect);
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
        if ((uiController_.getInGameMenu() != nullptr) || (uiController_.getInGameMentat() != nullptr)
            || (uiController_.getWaitingForOtherPlayers() != nullptr)
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

                    dest = calcDrawingRect(
                        pCursor, dune::globals::drawnMouseX, dune::globals::drawnMouseY, HAlign::Center, VAlign::Bottom);

                    int xPos = INVALID_POS;
                    int yPos = INVALID_POS;

                    const auto* const screenborder = dune::globals::screenborder.get();

                    if (screenborder->isScreenCoordInsideMap(dune::globals::drawnMouseX, dune::globals::drawnMouseY)) {
                        xPos = screenborder->screen2MapX(dune::globals::drawnMouseX);
                        yPos = screenborder->screen2MapY(dune::globals::drawnMouseY);
                    } else if (isOnRadarView(dune::globals::drawnMouseX, dune::globals::drawnMouseY)) {
                        const auto position = uiController_.getGameInterface()->getRadarView().getWorldCoords(
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
        SDL_HideCursor();

        pCursor->draw(dune::globals::renderer.get(), dest.x, dest.y);
    } else {
        if (!hardware_cursor)
            hardware_cursor = gfx->getDefaultCursor();

        if (hardware_cursor != SDL_GetCursor())
            SDL_SetCursor(hardware_cursor);

        SDL_ShowCursor();
    }
}

void Game::saveScreenshot() {
    pendingScreenshot_ = false;

    auto [ok, path] = SaveScreenshot();

    if (ok && path.has_value()) {
        const std::string filename{reinterpret_cast<const char*>(path.value().filename().u8string().c_str())};
        dune::globals::currentGame->addToNewsTicker(fmt::format("{}: '{}'", _("Screenshot saved"), filename));
    }
}
