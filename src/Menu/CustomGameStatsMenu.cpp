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

#include <misc/string_util.h>
#include <misc/FileSystem.h>

#include <FileClasses/GFXManager.h>
#include <FileClasses/TextManager.h>

#include <Game.h>
#include <House.h>
#include <sand.h>

#include <algorithm>


CustomGameStatsMenu::CustomGameStatsMenu() : MenuBase()
{
	// set up window
	SDL_Surface *surf;
	surf = pGFXManager->getUIGraphic(UI_MenuBackground);

	setBackground(surf,false);
	resize(surf->w,surf->h);

	setWindowWidget(&windowWidget);

	Uint32 localHouseColor = SDL2RGB(palette[houseToPaletteIndex[pLocalHouse->getHouseID()]]);

	windowWidget.addWidget(&mainVBox, Point(24,23),	Point(screen->w - 48, screen->h - 32));

    captionLabel.setText(getBasename(currentGame->getGameInitSettings().getFilename(), true));
    captionLabel.setTextColor(localHouseColor + 3);
    captionLabel.setAlignment(Alignment_HCenter);
    mainVBox.addWidget(&captionLabel, 24);
    mainVBox.addWidget(VSpacer::create(24));

    mainVBox.addWidget(Spacer::create(), 0.05);

    mainVBox.addWidget(&mainHBox, 0.80);

    mainHBox.addWidget(Spacer::create(), 0.4);
    mainHBox.addWidget(&playerStatListVBox, 0.2);

    headerHBox.addWidget(&headerLabelDummy, 130);
    headerHBox.addWidget(Spacer::create(), 10);
    headerLabel1.setText(_("Built Object"));
    headerLabel1.setAlignment(Alignment_HCenter);
    headerLabel1.setTextColor(localHouseColor + 3);
    headerHBox.addWidget(&headerLabel1, 132);
    headerHBox.addWidget(Spacer::create(), 30);
    headerLabel2.setText(_("Destroyed"));
    headerLabel2.setAlignment(Alignment_HCenter);
    headerLabel2.setTextColor(localHouseColor + 3);
    headerHBox.addWidget(&headerLabel2, 132);
    headerLabel3.setText(_("Harvested Spice"));
    headerLabel3.setAlignment(Alignment_HCenter);
    headerLabel3.setTextColor(localHouseColor + 3);
    headerHBox.addWidget(Spacer::create(), 30);
    headerHBox.addWidget(&headerLabel3, 132);

    playerStatListVBox.addWidget(&headerHBox, 25);

    playerStatListVBox.addWidget(VSpacer::create(15));

    int maxBuiltValue = 0;
    int maxDestroyedValue = 0;
    float maxSpiceHarvested = 0.0;

    for(int i=0;i<NUM_HOUSES;i++) {
        House* pHouse = currentGame->getHouse(i);

        if(pHouse != NULL) {
            maxBuiltValue = std::max(maxBuiltValue, pHouse->getBuiltValue());
            maxDestroyedValue = std::max(maxDestroyedValue, pHouse->getDestroyedValue());
            maxSpiceHarvested = std::max(maxSpiceHarvested, pHouse->getHarvestedSpice().toFloat());
        }
    }

    for(int i=0;i<NUM_HOUSES;i++) {
        HouseStat& curHouseStat = houseStat[i];
        House* pHouse = currentGame->getHouse(i);

        if(pHouse != NULL) {
            Uint32 color = SDL2RGB(palette[houseToPaletteIndex[i]]);

            curHouseStat.houseName.setText(_("House") + " " + getHouseNameByNumber((HOUSETYPE) i));
            curHouseStat.houseName.setTextColor(color + 3);
            curHouseStat.houseHBox.addWidget(&curHouseStat.houseName, 130);
            curHouseStat.houseHBox.addWidget(Spacer::create(), 10);

            curHouseStat.value1.setText( stringify(pHouse->getBuiltValue()*100));
            curHouseStat.value1.setTextFont(FONT_STD10);
            curHouseStat.value1.setAlignment(Alignment_Right);
            curHouseStat.value1.setTextColor(color + 3);
            curHouseStat.houseHBox.addWidget(&curHouseStat.value1, 50);
            curHouseStat.houseHBox.addWidget(HSpacer::create(2));
            curHouseStat.progressBar1.setProgress( (maxBuiltValue == 0) ? 0.0 : (pHouse->getBuiltValue() * 100.0f / maxBuiltValue));
            curHouseStat.progressBar1.setDrawShadow(true);
            curHouseStat.progressBar1.setColor(color + 1);
            curHouseStat.vBox1.addWidget(Spacer::create(), 0.5);
            curHouseStat.vBox1.addWidget(&curHouseStat.progressBar1, 12);
            curHouseStat.vBox1.addWidget(Spacer::create(), 0.5);
            curHouseStat.houseHBox.addWidget(&curHouseStat.vBox1, 80);

            curHouseStat.houseHBox.addWidget(Spacer::create(), 30);

            curHouseStat.value2.setText( stringify(pHouse->getDestroyedValue()*100));
            curHouseStat.value2.setTextFont(FONT_STD10);
            curHouseStat.value2.setAlignment(Alignment_Right);
            curHouseStat.value2.setTextColor(color + 3);
            curHouseStat.houseHBox.addWidget(&curHouseStat.value2, 50);
            curHouseStat.houseHBox.addWidget(HSpacer::create(2));
            curHouseStat.progressBar2.setProgress( (maxDestroyedValue == 0) ? 0.0 : (pHouse->getDestroyedValue() * 100.0f / maxDestroyedValue));
            curHouseStat.progressBar2.setDrawShadow(true);
            curHouseStat.progressBar2.setColor(color + 1);
            curHouseStat.vBox2.addWidget(Spacer::create(), 0.5);
            curHouseStat.vBox2.addWidget(&curHouseStat.progressBar2, 12);
            curHouseStat.vBox2.addWidget(Spacer::create(), 0.5);
            curHouseStat.houseHBox.addWidget(&curHouseStat.vBox2, 80);

            curHouseStat.houseHBox.addWidget(Spacer::create(), 30);

            curHouseStat.value3.setText( stringify(lround(pHouse->getHarvestedSpice())));
            curHouseStat.value3.setTextFont(FONT_STD10);
            curHouseStat.value3.setAlignment(Alignment_Right);
            curHouseStat.value3.setTextColor(color + 3);
            curHouseStat.houseHBox.addWidget(&curHouseStat.value3, 50);
            curHouseStat.houseHBox.addWidget(HSpacer::create(2));
            curHouseStat.progressBar3.setProgress( (maxSpiceHarvested == 0.0) ? 0.0 : (pHouse->getHarvestedSpice().toFloat() * 100.0f / maxSpiceHarvested));
            curHouseStat.progressBar3.setDrawShadow(true);
            curHouseStat.progressBar3.setColor(color + 1);
            curHouseStat.vBox3.addWidget(Spacer::create(), 0.5);
            curHouseStat.vBox3.addWidget(&curHouseStat.progressBar3, 12);
            curHouseStat.vBox3.addWidget(Spacer::create(), 0.5);
            curHouseStat.houseHBox.addWidget(&curHouseStat.vBox3, 80);

            playerStatListVBox.addWidget(&curHouseStat.houseHBox, 20);

            playerStatListVBox.addWidget(VSpacer::create(15));
        }
    }

    mainHBox.addWidget(Spacer::create(), 0.4);

    mainVBox.addWidget(Spacer::create(), 0.05);

    mainVBox.addWidget(VSpacer::create(20));
	mainVBox.addWidget(&buttonHBox, 24);
	mainVBox.addWidget(VSpacer::create(14), 0.0);

    buttonHBox.addWidget(HSpacer::create(70));
    int totalTime = currentGame->getGameTime()/1000;
    timeLabel.setText(strprintf(_("@DUNE.ENG|22#Time: %d:%02d"), totalTime/3600, (totalTime%3600)/60));
    timeLabel.setTextColor(localHouseColor + 3);
	buttonHBox.addWidget(&timeLabel, 0.2);

	buttonHBox.addWidget(Spacer::create(), 0.0625);
	buttonHBox.addWidget(Spacer::create(), 0.475);
	buttonHBox.addWidget(Spacer::create(), 0.0625);

    okButton.setText(_("OK"));
    okButton.setTextColor(localHouseColor + 3);
	okButton.setOnClick(std::bind(&CustomGameStatsMenu::onOK, this));
	buttonHBox.addWidget(&okButton, 0.2);
	buttonHBox.addWidget(HSpacer::create(90));
}

CustomGameStatsMenu::~CustomGameStatsMenu()
{
	;
}

void CustomGameStatsMenu::onOK()
{
    quit();
}

