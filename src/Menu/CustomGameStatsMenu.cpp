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

#include <Menu/CustomGameStatsMenu.h>

#include <globals.h>

#include <fmt/printf.h>
#include <misc/FileSystem.h>

#include <FileClasses/GFXManager.h>
#include <FileClasses/TextManager.h>

#include "GUI/Spacer.h"

#include <Game.h>
#include <House.h>
#include <sand.h>

#include <algorithm>

CustomGameStatsMenu::CustomGameStatsMenu() {
    // set up window

    CustomGameStatsMenu::setWindowWidget(&windowWidget);

    const Uint32 localHouseColor = SDL2RGB(
        dune::globals::palette[houseToPaletteIndex[static_cast<int>(dune::globals::pLocalHouse->getHouseID())] + 3]);

    windowWidget.addWidget(&mainVBox, Point(24, 23), Point(getRendererWidth() - 48, getRendererHeight() - 32));

    auto* const game = dune::globals::currentGame.get();

    captionLabel.setText(
        reinterpret_cast<const char*>(getBasename(game->getGameInitSettings().getFilename(), true).u8string().c_str()));
    captionLabel.setTextColor(localHouseColor);
    captionLabel.setAlignment(Alignment_HCenter);
    mainVBox.addWidget(&captionLabel, 24);
    mainVBox.addWidget(Widget::create<VSpacer>(24).release());

    mainVBox.addWidget(Widget::create<Spacer>().release(), 0.05);

    mainVBox.addWidget(&mainHBox, 0.80);

    mainHBox.addWidget(Widget::create<Spacer>().release(), 0.4);
    mainHBox.addWidget(&playerStatListVBox, 0.2);

    headerHBox.addWidget(&headerLabelDummy, 130);
    headerHBox.addWidget(Widget::create<Spacer>().release(), 10);
    headerLabel1.setText(_("Built Objects"));
    headerLabel1.setAlignment(Alignment_HCenter);
    headerLabel1.setTextColor(localHouseColor);
    headerHBox.addWidget(&headerLabel1, 132);
    headerHBox.addWidget(Widget::create<Spacer>().release(), 30);
    headerLabel2.setText(_("Destroyed"));
    headerLabel2.setAlignment(Alignment_HCenter);
    headerLabel2.setTextColor(localHouseColor);
    headerHBox.addWidget(&headerLabel2, 132);
    headerLabel3.setText(_("Harvested Spice"));
    headerLabel3.setAlignment(Alignment_HCenter);
    headerLabel3.setTextColor(localHouseColor);
    headerHBox.addWidget(Widget::create<Spacer>().release(), 30);
    headerHBox.addWidget(&headerLabel3, 132);

    playerStatListVBox.addWidget(&headerHBox, 25);

    playerStatListVBox.addWidget(Widget::create<VSpacer>(15).release());

    int maxBuiltValue       = 0;
    int maxDestroyedValue   = 0;
    float maxSpiceHarvested = 0.0;

    game->for_each_house([&](auto& house) {
        maxBuiltValue     = std::max(maxBuiltValue, house.getBuiltValue());
        maxDestroyedValue = std::max(maxDestroyedValue, house.getDestroyedValue());
        maxSpiceHarvested = std::max(maxSpiceHarvested, house.getHarvestedSpice().toFloat());
    });

    for_each_stat(game, [&](const auto i, auto& house, auto& curHouseStat) {
        const auto textcolor     = SDL2RGB(dune::globals::palette[houseToPaletteIndex[i] + 3]);
        const auto progresscolor = SDL2RGB(dune::globals::palette[houseToPaletteIndex[i] + 1]);

        curHouseStat.houseName.setText(_("House") + " " + getHouseNameByNumber(static_cast<HOUSETYPE>(i)));
        curHouseStat.houseName.setTextColor(textcolor);
        curHouseStat.houseHBox.addWidget(&curHouseStat.houseName, 145);
        curHouseStat.houseHBox.addWidget(Widget::create<Spacer>().release(), 5);

        curHouseStat.value1.setText(std::to_string(house.getBuiltValue()));
        curHouseStat.value1.setTextFontSize(12);
        curHouseStat.value1.setAlignment(Alignment_Right);
        curHouseStat.value1.setTextColor(textcolor);
        curHouseStat.houseHBox.addWidget(&curHouseStat.value1, 50);
        curHouseStat.houseHBox.addWidget(Widget::create<HSpacer>(2).release());
        curHouseStat.progressBar1.setProgress(maxBuiltValue == 0 ? 0.0f
                                                                 : house.getBuiltValue() * 100.0f / maxBuiltValue);
        curHouseStat.progressBar1.setDrawShadow(true);
        curHouseStat.progressBar1.setColor(progresscolor);
        curHouseStat.vBox1.addWidget(Widget::create<Spacer>().release(), 0.5);
        curHouseStat.vBox1.addWidget(&curHouseStat.progressBar1, 12);
        curHouseStat.vBox1.addWidget(Widget::create<Spacer>().release(), 0.5);
        curHouseStat.houseHBox.addWidget(&curHouseStat.vBox1, 80);

        curHouseStat.houseHBox.addWidget(Widget::create<Spacer>().release(), 25);

        curHouseStat.value2.setText(std::to_string(house.getDestroyedValue() * 100));
        curHouseStat.value2.setTextFontSize(12);
        curHouseStat.value2.setAlignment(Alignment_Right);
        curHouseStat.value2.setTextColor(textcolor);
        curHouseStat.houseHBox.addWidget(&curHouseStat.value2, 50);
        curHouseStat.houseHBox.addWidget(Widget::create<HSpacer>(2).release());
        curHouseStat.progressBar2.setProgress(
            maxDestroyedValue == 0 ? 0.0f : house.getDestroyedValue() * 100.0f / maxDestroyedValue);
        curHouseStat.progressBar2.setDrawShadow(true);
        curHouseStat.progressBar2.setColor(progresscolor);
        curHouseStat.vBox2.addWidget(Widget::create<Spacer>().release(), 0.5);
        curHouseStat.vBox2.addWidget(&curHouseStat.progressBar2, 12);
        curHouseStat.vBox2.addWidget(Widget::create<Spacer>().release(), 0.5);
        curHouseStat.houseHBox.addWidget(&curHouseStat.vBox2, 80);

        curHouseStat.houseHBox.addWidget(Widget::create<Spacer>().release(), 25);

        curHouseStat.value3.setText(std::to_string(lround(house.getHarvestedSpice())));
        curHouseStat.value3.setTextFontSize(12);
        curHouseStat.value3.setAlignment(Alignment_Right);
        curHouseStat.value3.setTextColor(textcolor);
        curHouseStat.houseHBox.addWidget(&curHouseStat.value3, 50);
        curHouseStat.houseHBox.addWidget(Widget::create<HSpacer>(2).release());
        curHouseStat.progressBar3.setProgress(
            maxSpiceHarvested == 0.0f ? 0.0f : house.getHarvestedSpice().toFloat() * 100.0f / maxSpiceHarvested);
        curHouseStat.progressBar3.setDrawShadow(true);
        curHouseStat.progressBar3.setColor(progresscolor);
        curHouseStat.vBox3.addWidget(Widget::create<Spacer>().release(), 0.5);
        curHouseStat.vBox3.addWidget(&curHouseStat.progressBar3, 12);
        curHouseStat.vBox3.addWidget(Widget::create<Spacer>().release(), 0.5);
        curHouseStat.houseHBox.addWidget(&curHouseStat.vBox3, 80);

        playerStatListVBox.addWidget(&curHouseStat.houseHBox, 20);

        playerStatListVBox.addWidget(Widget::create<VSpacer>(15).release());
    });

    mainHBox.addWidget(Widget::create<Spacer>().release(), 0.4);

    mainVBox.addWidget(Widget::create<Spacer>().release(), 0.05);

    mainVBox.addWidget(Widget::create<VSpacer>(20).release());
    mainVBox.addWidget(&buttonHBox, 24);
    mainVBox.addWidget(Widget::create<VSpacer>(14).release(), 0.0);

    buttonHBox.addWidget(Widget::create<HSpacer>(70).release());
    const int totalTime = game->getGameTime() / 1000;
    timeLabel.setText(fmt::sprintf(_("@DUNE.ENG|22#Time: %d:%02d"), totalTime / 3600, totalTime % 3600 / 60));
    timeLabel.setTextColor(localHouseColor);
    buttonHBox.addWidget(&timeLabel, 0.2);

    buttonHBox.addWidget(Widget::create<Spacer>().release(), 0.0625);
    buttonHBox.addWidget(Widget::create<Spacer>().release(), 0.475);
    buttonHBox.addWidget(Widget::create<Spacer>().release(), 0.0625);

    okButton.setText(_("OK"));
    okButton.setTextColor(localHouseColor);
    okButton.setOnClick([this] { onOK(); });
    buttonHBox.addWidget(&okButton, 0.2);
    buttonHBox.addWidget(Widget::create<HSpacer>(90).release());
}

CustomGameStatsMenu::~CustomGameStatsMenu() = default;

void CustomGameStatsMenu::onOK() {
    quit();
}
