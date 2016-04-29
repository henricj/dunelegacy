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

#include <GUI/dune/GameOptionsWindow.h>

#include <globals.h>

#include <FileClasses/TextManager.h>
#include <FileClasses/GFXManager.h>


GameOptionsWindow::GameOptionsWindow(SettingsClass::GameOptionsClass& initialGameOptions)
 : Window(50,50,400, 450), gameOptions(initialGameOptions) {

    setWindowWidget(&vbox);
    vbox.addWidget(VSpacer::create(6));

    captionlabel.setText(_("Game Options"));
    vbox.addWidget(&captionlabel);
    vbox.addWidget(VSpacer::create(3));
    vbox.addWidget(&hbox);
    vbox.addWidget(VSpacer::create(6));
    hbox.addWidget(Spacer::create());
    hbox.addWidget(&vbox2);
    vbox2.addWidget(Spacer::create());

    concreteRequiredCheckbox.setText(_("Concrete Required"));
    /* xgettext:no-c-format */
    concreteRequiredCheckbox.setTooltipText(_("If checked building on bare rock will result in 50% structure health penalty."));
    concreteRequiredCheckbox.setChecked(gameOptions.concreteRequired);
    vbox2.addWidget(&concreteRequiredCheckbox);
    vbox2.addWidget(VSpacer::create(4));

    structuresDegradeOnConcreteCheckbox.setText(_("Structures Degrade On Concrete"));
    structuresDegradeOnConcreteCheckbox.setTooltipText(_("If checked structures will degrade on power shortage even if built on concrete."));
    structuresDegradeOnConcreteCheckbox.setChecked(gameOptions.structuresDegradeOnConcrete);
    vbox2.addWidget(&structuresDegradeOnConcreteCheckbox);
    vbox2.addWidget(VSpacer::create(4));

    fogOfWarCheckbox.setText(_("Fog of War"));
    fogOfWarCheckbox.setTooltipText(_("If checked explored terrain will become foggy when no unit or structure is next to it."));
    fogOfWarCheckbox.setChecked(gameOptions.fogOfWar);
    vbox2.addWidget(&fogOfWarCheckbox);
    vbox2.addWidget(VSpacer::create(4));

    startWithExploredMapCheckbox.setText(_("Start with Explored Map"));
    startWithExploredMapCheckbox.setTooltipText(_("If checked the complete map is unhidden at the beginning of the game."));
    startWithExploredMapCheckbox.setChecked(gameOptions.startWithExploredMap);
    vbox2.addWidget(&startWithExploredMapCheckbox);
    vbox2.addWidget(VSpacer::create(4));

    instantBuildCheckbox.setText(_("Instant Build"));
    instantBuildCheckbox.setTooltipText(_("If checked the building of structures and units does not take any time."));
    instantBuildCheckbox.setChecked(gameOptions.instantBuild);
    vbox2.addWidget(&instantBuildCheckbox);
    vbox2.addWidget(VSpacer::create(4));

    onlyOnePalaceCheckbox.setText(_("Only One Palace per House"));
    onlyOnePalaceCheckbox.setTooltipText(_("If checked only one palace can be build per house."));
    onlyOnePalaceCheckbox.setChecked(gameOptions.onlyOnePalace);
    vbox2.addWidget(&onlyOnePalaceCheckbox);
    vbox2.addWidget(VSpacer::create(4));

    rocketTurretsNeedPowerCheckbox.setText(_("Rocket-Turrets Need Power"));
    rocketTurretsNeedPowerCheckbox.setTooltipText(_("If checked rocket turrets are dysfunctional on power shortage."));
    rocketTurretsNeedPowerCheckbox.setChecked(gameOptions.rocketTurretsNeedPower);
    vbox2.addWidget(&rocketTurretsNeedPowerCheckbox);
    vbox2.addWidget(VSpacer::create(4));

    sandwormsRespawnCheckbox.setText(_("Killed Sandworms Respawn"));
    sandwormsRespawnCheckbox.setTooltipText(_("If checked killed sandworms respawn after some time."));
    sandwormsRespawnCheckbox.setChecked(gameOptions.sandwormsRespawn);
    vbox2.addWidget(&sandwormsRespawnCheckbox);
    vbox2.addWidget(VSpacer::create(4));

    killedSandwormsDropSpiceCheckbox.setText(_("Killed Sandworms Drop Spice"));
    killedSandwormsDropSpiceCheckbox.setTooltipText(_("If checked killed sandworms drop some spice."));
    killedSandwormsDropSpiceCheckbox.setChecked(gameOptions.killedSandwormsDropSpice);
    vbox2.addWidget(&killedSandwormsDropSpiceCheckbox);
    vbox2.addWidget(VSpacer::create(4));

    manualCarryallDropsCheckbox.setText(_("Manual Carryall Drops"));
    manualCarryallDropsCheckbox.setTooltipText(_("If checked player can request carryall to transport units."));
    manualCarryallDropsCheckbox.setChecked(gameOptions.manualCarryallDrops);
    vbox2.addWidget(&manualCarryallDropsCheckbox);
    vbox2.addWidget(VSpacer::create(4));

    gameSpeedMinus.setTextures(pGFXManager->getUIGraphic(UI_Minus), false, pGFXManager->getUIGraphic(UI_Minus_Pressed), false);
    gameSpeedMinus.setOnClick(std::bind(&GameOptionsWindow::onGameSpeedMinus, this));
    gameSpeedHBox.addWidget(HSpacer::create(2));
    gameSpeedHBox.addWidget(&gameSpeedMinus);
    gameSpeedHBox.addWidget(HSpacer::create(2));

    gameSpeedBar.setText(_("Game speed"));
    gameSpeedHBox.addWidget(&gameSpeedBar);
    currentGameSpeed = gameOptions.gameSpeed;
    updateGameSpeedBar();
    gameSpeedPlus.setTextures(pGFXManager->getUIGraphic(UI_Plus), false, pGFXManager->getUIGraphic(UI_Plus_Pressed), false);
    gameSpeedPlus.setOnClick(std::bind(&GameOptionsWindow::onGameSpeedPlus, this));
    gameSpeedHBox.addWidget(HSpacer::create(2));
    gameSpeedHBox.addWidget(&gameSpeedPlus);
    gameSpeedHBox.addWidget(HSpacer::create(2));
    vbox2.addWidget(&gameSpeedHBox, 20);
    vbox2.addWidget(VSpacer::create(4));


    vbox2.addWidget(Spacer::create());

    vbox2.addWidget(VSpacer::create(4));

    okbutton.setText(_("OK"));
    okbutton.setOnClick(std::bind(&GameOptionsWindow::onOK, this));
    vbox2.addWidget(&okbutton, 30);
    vbox2.addWidget(VSpacer::create(4));
    hbox.addWidget(Spacer::create());
    captionlabel.setAlignment(Alignment_HCenter);

    int xpos = std::max(0,(getRendererWidth() - getSize().x)/2);
    int ypos = std::max(0,(getRendererHeight() - getSize().y)/2);

    setCurrentPosition(xpos,ypos,getSize().x,getSize().y);
}

GameOptionsWindow::~GameOptionsWindow() {
    ;
}

void GameOptionsWindow::onOK() {
    gameOptions.gameSpeed = currentGameSpeed;
    gameOptions.concreteRequired = concreteRequiredCheckbox.isChecked();
    gameOptions.structuresDegradeOnConcrete = structuresDegradeOnConcreteCheckbox.isChecked();
    gameOptions.fogOfWar = fogOfWarCheckbox.isChecked();
    gameOptions.startWithExploredMap = startWithExploredMapCheckbox.isChecked();
    gameOptions.instantBuild = instantBuildCheckbox.isChecked();
    gameOptions.onlyOnePalace = onlyOnePalaceCheckbox.isChecked();
    gameOptions.rocketTurretsNeedPower = rocketTurretsNeedPowerCheckbox.isChecked();
    gameOptions.sandwormsRespawn = sandwormsRespawnCheckbox.isChecked();
    gameOptions.killedSandwormsDropSpice = killedSandwormsDropSpiceCheckbox.isChecked();
    gameOptions.manualCarryallDrops = manualCarryallDropsCheckbox.isChecked();

    Window* pParentWindow = dynamic_cast<Window*>(getParent());
    if(pParentWindow != nullptr) {
        pParentWindow->closeChildWindow();
    }
}

void GameOptionsWindow::onGameSpeedMinus() {
    if(currentGameSpeed < GAMESPEED_MAX) {
        currentGameSpeed++;
        updateGameSpeedBar();
    }
}

void GameOptionsWindow::onGameSpeedPlus() {
    if(currentGameSpeed > GAMESPEED_MIN) {
        currentGameSpeed--;
        updateGameSpeedBar();
    }
}

void GameOptionsWindow::updateGameSpeedBar() {
    gameSpeedBar.setProgress( 100.0 - ((currentGameSpeed-GAMESPEED_MIN)*100.0)/(GAMESPEED_MAX - GAMESPEED_MIN) );
}
