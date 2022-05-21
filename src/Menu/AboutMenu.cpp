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

#include <Menu/AboutMenu.h>

#include <globals.h>

#include "misc/DrawingRectHelper.h"
#include <FileClasses/GFXManager.h>

AboutMenu::AboutMenu() {
    // set up window

    AboutMenu::setWindowWidget(&windowWidget);

    const auto* const gfx = dune::globals::pGFXManager.get();

    // set up pictures in the background
    const auto* const pPlanetBackground = gfx->getUIGraphic(UI_PlanetBackground);
    planetPicture.setTexture(pPlanetBackground);
    auto dest1 = calcAlignedDrawingRect(pPlanetBackground);
    dest1.y    = dest1.y - getHeight(pPlanetBackground) / 2 + 10;
    windowWidget.addWidget(&planetPicture, dest1);

    const auto* const pDuneLegacy = gfx->getUIGraphic(UI_DuneLegacy);
    duneLegacy.setTexture(pDuneLegacy);
    auto dest2 = calcAlignedDrawingRect(pDuneLegacy);
    dest2.y    = dest2.y + getHeight(pDuneLegacy) / 2 + 28;
    windowWidget.addWidget(&duneLegacy, dest2);

    const auto* pMenuButtonBorder = gfx->getUIGraphic(UI_MenuButtonBorder);
    buttonBorder.setTexture(pMenuButtonBorder);
    auto dest3 = calcAlignedDrawingRect(pMenuButtonBorder);
    dest3.y    = dest3.y + getHeight(pMenuButtonBorder) / 2 + 59;
    windowWidget.addWidget(&buttonBorder, dest3);

    text.setText("Written by\n   Anthony Cole,\n      Richard Schaller,\n         Stefan van der Wel\n            and "
                 "many others!\n");
    text.setAlignment(Alignment_Left);
    windowWidget.addWidget(&text, Point((getRendererWidth() - 160) / 2, getRendererHeight() / 2 + 70), Point(170, 110));
}

AboutMenu::~AboutMenu() = default;

void AboutMenu::resize(uint32_t width, uint32_t height) {
    parent::resize(width, height);

    const auto iWidth  = static_cast<int>(width);
    const auto iHeight = static_cast<int>(height);

    const auto planet_size = planetPicture.getSize();
    windowWidget.setWidgetGeometry(&planetPicture, {(iWidth - planet_size.x) / 2, iHeight / 2 - planet_size.y + 10},
                                   planet_size);

    const auto dune_size = duneLegacy.getSize();
    windowWidget.setWidgetGeometry(&duneLegacy, {(iWidth - dune_size.x) / 2, iHeight / 2 + 28}, dune_size);

    const auto border_size = buttonBorder.getSize();
    windowWidget.setWidgetGeometry(&buttonBorder, {(iWidth - border_size.x) / 2, iHeight / 2 + 59}, border_size);

    const auto text_size = text.getSize();
    windowWidget.setWidgetGeometry(&text, {(iWidth - 160) / 2, iHeight / 2 + 70}, text_size);
}

void AboutMenu::doInputImpl(const SDL_Event& event) {
    if (event.type == SDL_MOUSEBUTTONUP) {
        quit();
    }

    parent::doInputImpl(event);
}
