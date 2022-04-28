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

#include <GameInterface.h>

#include <globals.h>

#include <FileClasses/GFXManager.h>
#include <Game.h>
#include <House.h>

#include "GUI/Spacer.h"
#include <GUI/ObjectInterfaces/MultiUnitInterface.h>
#include <GUI/ObjectInterfaces/ObjectInterface.h>
#include <ObjectBase.h>

#include <misc/SDL2pp.h>
#include <misc/draw_util.h>

GameInterface::GameInterface(const GameContext& context) : Window{0, 0, 0, 0}, context_{context} {
    GameInterface::setTransparentBackground(true);

    const auto width  = getRendererWidth();
    const auto height = getRendererHeight();

    GameInterface::setCurrentPosition(0, 0, width, height);

    GameInterface::setWindowWidget(&windowWidget);

    const auto* const gfx = dune::globals::pGFXManager.get();

    const auto house_id = dune::globals::pLocalHouse->getHouseID();

    // top bar
    const auto* const pTopBarTex = gfx->getUIGraphic(UI_TopBar, house_id);
    topBar.setTexture(pTopBarTex);
    windowWidget.addWidget(
        &topBar, {},
        {static_cast<int>(std::ceil(getWidth(pTopBarTex))), static_cast<int>(std::ceil(getHeight(pTopBarTex))) - 12});

    // side bar
    const auto* const pSideBarTex = gfx->getUIGraphic(UI_SideBar, house_id);
    sideBar.setTexture(pSideBarTex);
    const auto dest = calcAlignedDrawingRect(pSideBarTex, HAlign::Right, VAlign::Top);
    windowWidget.addWidget(&sideBar, dest);

    // add buttons
    windowWidget.addWidget(&topBarHBox, {5, 5}, {width - sideBar.getSize().x, topBar.getSize().y - 10});

    topBarHBox.addWidget(&newsticker);

    topBarHBox.addWidget(Spacer::create());

    optionsButton.setTextures(gfx->getUIGraphic(UI_Options, house_id), gfx->getUIGraphic(UI_Options_Pressed, house_id));
    optionsButton.setOnClick([] { dune::globals::currentGame->onOptions(); });
    topBarHBox.addWidget(&optionsButton);

    topBarHBox.addWidget(Spacer::create());

    mentatButton.setTextures(gfx->getUIGraphic(UI_Mentat, house_id), gfx->getUIGraphic(UI_Mentat_Pressed, house_id));
    mentatButton.setOnClick([] { dune::globals::currentGame->onMentat(); });
    topBarHBox.addWidget(&mentatButton);

    topBarHBox.addWidget(Spacer::create());

    // add radar
    windowWidget.addWidget(&radarView, {width - sideBar.getSize().x + SIDEBAR_COLUMN_WIDTH, 0},
                           radarView.getMinimumSize());
    radarView.setOnRadarClick([&](Coord worldPosition, bool bRightMouseButton, bool bDrag) {
        return context_.game.onRadarClick(context_, worldPosition, bRightMouseButton, bDrag);
    });

    // add chat manager
    windowWidget.addWidget(&chatManager, {20, 60}, {width - sideBar.getSize().x, 360});
}

GameInterface::~GameInterface() = default;

void GameInterface::draw(Point position) {
    parent::draw(position);

    auto* const renderer = dune::globals::renderer.get();

    // draw Power Indicator and Spice indicator

    const SDL_Rect powerIndicatorPos = {getRendererWidth() - sideBar.getSize().x + 14, 146, 4,
                                        getRendererHeight() - 146 - 2};
    renderFillRect(renderer, &powerIndicatorPos, COLOR_BLACK);

    const SDL_Rect spiceIndicatorPos = {getRendererWidth() - sideBar.getSize().x + 20, 146, 4,
                                        getRendererHeight() - 146 - 2};
    renderFillRect(renderer, &spiceIndicatorPos, COLOR_BLACK);

    int xCount = 0;

    int yCount  = 0;
    int yCount2 = 0;

    const auto* const local_house = dune::globals::pLocalHouse;

    // draw power level indicator
    if (local_house->getPowerRequirement() == 0) {
        if (local_house->getProducedPower() > 0) {
            yCount2 = powerIndicatorPos.h + 1;
        } else {
            yCount2 = powerIndicatorPos.h / 2;
        }
    } else {
        yCount2 = lround(static_cast<double>(local_house->getProducedPower())
                         / static_cast<double>(local_house->getPowerRequirement())
                         * static_cast<double>(powerIndicatorPos.h / 2));
    }

    if (yCount2 > powerIndicatorPos.h + 1) {
        yCount2 = powerIndicatorPos.h + 1;
    }

    setRenderDrawColor(renderer, COLOR_GREEN);
    render_points_.clear();
    for (yCount = 0; yCount < yCount2; yCount++) {
        for (xCount = 1; xCount < powerIndicatorPos.w - 1; xCount++) {
            if (((yCount / 2) % 3) != 0) {
                render_points_.emplace_back(xCount + powerIndicatorPos.x,
                                            powerIndicatorPos.y + powerIndicatorPos.h - yCount);
            }
        }
    }
    if (!render_points_.empty())
        SDL_RenderDrawPoints(renderer, render_points_.data(), static_cast<int>(render_points_.size()));

    // draw spice level indicator
    if (local_house->getCapacity() == 0) {
        yCount2 = 0;
    } else {
        yCount2 = lround((local_house->getStoredCredits() / local_house->getCapacity()) * spiceIndicatorPos.h);
    }

    if (yCount2 > spiceIndicatorPos.h + 1) {
        yCount2 = spiceIndicatorPos.h + 1;
    }

    setRenderDrawColor(renderer, COLOR_ORANGE);
    render_points_.clear();
    for (yCount = 0; yCount < yCount2; yCount++) {
        for (xCount = 1; xCount < spiceIndicatorPos.w - 1; xCount++) {
            if (yCount / 2 % 3 == 0)
                continue;

            render_points_.emplace_back(xCount + spiceIndicatorPos.x,
                                        spiceIndicatorPos.y + spiceIndicatorPos.h - yCount);
        }
    }
    if (!render_points_.empty())
        SDL_RenderDrawPoints(renderer, render_points_.data(), static_cast<int>(render_points_.size()));

    // draw credits
    const auto credits       = local_house->getCredits();
    const auto CreditsBuffer = std::to_string((credits < 0) ? 0 : credits);
    const auto NumDigits     = static_cast<int>(CreditsBuffer.length());

    auto* const digitsTex = dune::globals::pGFXManager->getUIGraphic(UI_CreditsDigits);

    for (int i = NumDigits - 1; i >= 0; i--) {
        auto source = calcSpriteSourceRect(digitsTex, CreditsBuffer[i] - '0', 10);
        auto dest   = calcSpriteDrawingRect(
              digitsTex, getRendererWidth() - sideBar.getSize().x + 49 + (6 - NumDigits + i) * 10, 135, 10);
        Dune_RenderCopyF(renderer, digitsTex, &source, &dest);
    }
}

void GameInterface::updateObjectInterface() {
    auto* const game = dune::globals::currentGame.get();

    const auto& selected = game->getSelectedList();

    if (selected.empty()) {
        removeOldContainer();
        return;
    }

    const auto size = selected.size();

    const auto renderer_width = getRendererWidth();

    if (size == 1) {
        const auto selected_object_id = *selected.begin();

        auto* pObject = game->getObjectManager().getObject(selected_object_id);
        if (!pObject) {
            sdl2::log_error("The selected object %d cannot be found!", selected_object_id);
            game->clearSelectedList();
            return;
        }

        if (selected_object_id != objectID) {
            removeOldContainer();

            pObjectContainer = pObject->getInterfaceContainer(context_);

            if (pObjectContainer != nullptr) {
                objectID = selected_object_id;

                windowWidget.addWidget(pObjectContainer.get(), {renderer_width - sideBar.getSize().x + 24, 146},
                                       {sideBar.getSize().x - 25, getRendererHeight() - 148});
            }

        } else {
            if (!pObjectContainer->update()) {
                removeOldContainer();
            }
        }

        return;
    }

    if ((pObjectContainer == nullptr) || (objectID != NONE_ID)) {
        // either there was nothing selected before or exactly one unit

        removeOldContainer();

        pObjectContainer = MultiUnitInterface::create(context_);

        windowWidget.addWidget(pObjectContainer.get(), {renderer_width - sideBar.getSize().x + 24, 146},
                               {sideBar.getSize().x - 25, getRendererHeight() - 148});
    } else {
        if (!pObjectContainer->update()) {
            removeOldContainer();
        }
    }
}

void GameInterface::removeOldContainer() {
    if (pObjectContainer == nullptr)
        return;

    pObjectContainer.reset();
    objectID = NONE_ID;
}
