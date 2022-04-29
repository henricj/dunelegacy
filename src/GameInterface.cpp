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

void GameInterface::draw_indicator(SDL_Renderer* const renderer, const SDL_FRect& dest, float percent, uint32_t color) {

    auto yCount2 = static_cast<float>(dest.h) * percent;

    if (yCount2 > dest.h)
        yCount2 = dest.h;
    else if (yCount2 <= 0)
        return;

    static constexpr auto indicator_height = 6;

    render_rects_.clear();

    for (auto i = 0; i < yCount2 + indicator_height; i += indicator_height) {
        const auto high = std::min(static_cast<float>(i + indicator_height), yCount2);
        const auto low  = i + 2;

        if (high <= low)
            break;

        render_rects_.emplace_back(dest.x + 1, dest.y + dest.h - high, dest.w - 2, high - low);
    }

    if (render_rects_.empty())
        return;

    setRenderDrawColor(renderer, color);
    DuneFillRects(renderer, render_rects_);
}

void GameInterface::draw(Point position) {
    parent::draw(position);

    auto* const renderer = dune::globals::renderer.get();

    // draw Power Indicator and Spice indicator

    const auto [renderer_width, renderer_height] = getRendererSizePointF();

    const auto left = static_cast<float>(sideBar.getSize().x);

    const SDL_FRect powerIndicatorPos{renderer_width - left + 14, 146, 4, renderer_height - 146 - 2};
    const SDL_FRect spiceIndicatorPos{renderer_width - left + 20, 146, 4, renderer_height - 146 - 2};

    setRenderDrawColor(renderer, COLOR_BLACK);
    DuneFillRects(renderer, {powerIndicatorPos, spiceIndicatorPos});

    const auto* const local_house = dune::globals::pLocalHouse;

    // draw power level indicator
    const auto power_required = local_house->getPowerRequirement();
    const auto power_produced = local_house->getProducedPower();

    auto power_percent = 0.f;

    if (power_required == 0) {
        if (power_produced > 0) {
            power_percent = 1;
        } else {
            power_percent = 0.5f;
        }
    } else {
        power_percent = 0.5f * static_cast<float>(power_produced) / static_cast<float>(power_required);
    }

    draw_indicator(renderer, powerIndicatorPos, power_percent, COLOR_GREEN);

    // draw spice level indicator
    const auto spice_capacity = local_house->getCapacity();

    auto spice_percent = 0.f;

    if (spice_capacity > 0) {
        const auto spice_stored = local_house->getStoredCredits();

        spice_percent = spice_stored.toFloat() / static_cast<float>(spice_capacity);
    }

    draw_indicator(renderer, spiceIndicatorPos, spice_percent, COLOR_ORANGE);

    // draw credits
    const auto credits       = local_house->getCredits();
    const auto CreditsBuffer = std::to_string((credits < 0) ? 0 : credits);
    const auto NumDigits     = static_cast<int>(CreditsBuffer.length());

    auto* const digitsTex = dune::globals::pGFXManager->getUIGraphic(UI_CreditsDigits);

    for (int i = NumDigits - 1; i >= 0; i--) {
        auto source     = calcSpriteSourceRect(digitsTex, CreditsBuffer[i] - '0', 10);
        const auto x    = renderer_width - left + 49 + (6 - NumDigits + i) * 10;
        const auto dest = calcSpriteDrawingRect(digitsTex, x, 135, 10);
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
