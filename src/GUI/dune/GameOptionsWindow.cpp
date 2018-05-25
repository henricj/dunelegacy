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
 : Window(50,50,626,340), gameOptions(initialGameOptions) {

    setWindowWidget(&vbox);
    vbox.addWidget(VSpacer::create(6));

    captionlabel.setText(_("Game Options"));
    captionlabel.setAlignment(Alignment_HCenter);
    vbox.addWidget(&captionlabel);
    vbox.addWidget(VSpacer::create(3));
    vbox.addWidget(&hbox);
    vbox.addWidget(VSpacer::create(6));
    hbox.addWidget(Spacer::create());
    hbox.addWidget(&vboxLeft);
    hbox.addWidget(HSpacer::create(20));
    hbox.addWidget(&vboxRight);
    hbox.addWidget(Spacer::create());
    vboxLeft.addWidget(Spacer::create());

    startWithExploredMapCheckbox.setText(_("Start with Explored Map"));
    startWithExploredMapCheckbox.setTooltipText(_("If checked the complete map is unhidden at the beginning of the game."));
    startWithExploredMapCheckbox.setChecked(gameOptions.startWithExploredMap);
    vboxLeft.addWidget(&startWithExploredMapCheckbox);
    vboxLeft.addWidget(VSpacer::create(6));

    structuresDegradeOnConcreteCheckbox.setText(_("Structures Degrade On Concrete"));
    structuresDegradeOnConcreteCheckbox.setTooltipText(_("If checked structures will degrade on power shortage even if built on concrete."));
    structuresDegradeOnConcreteCheckbox.setChecked(gameOptions.structuresDegradeOnConcrete);
    vboxLeft.addWidget(&structuresDegradeOnConcreteCheckbox);
    vboxLeft.addWidget(VSpacer::create(6));

    sandwormsRespawnCheckbox.setText(_("Killed Sandworms Respawn"));
    sandwormsRespawnCheckbox.setTooltipText(_("If checked killed sandworms respawn after some time."));
    sandwormsRespawnCheckbox.setChecked(gameOptions.sandwormsRespawn);
    vboxLeft.addWidget(&sandwormsRespawnCheckbox);
    vboxLeft.addWidget(VSpacer::create(6));

    killedSandwormsDropSpiceCheckbox.setText(_("Killed Sandworms Drop Spice"));
    killedSandwormsDropSpiceCheckbox.setTooltipText(_("If checked killed sandworms drop some spice."));
    killedSandwormsDropSpiceCheckbox.setChecked(gameOptions.killedSandwormsDropSpice);
    vboxLeft.addWidget(&killedSandwormsDropSpiceCheckbox);
    vboxLeft.addWidget(VSpacer::create(6));

    manualCarryallDropsCheckbox.setText(_("Manual Carryall Drops"));
    manualCarryallDropsCheckbox.setTooltipText(_("If checked player can request carryall to transport units."));
    manualCarryallDropsCheckbox.setChecked(gameOptions.manualCarryallDrops);
    vboxLeft.addWidget(&manualCarryallDropsCheckbox);
    vboxLeft.addWidget(VSpacer::create(6));

    maxUnitsOverrideCheckbox.setText(_("Override max. number of units"));
    maxUnitsOverrideCheckbox.setTooltipText(_("If checked the maximum number of units per house can be overridden; otherwise it is map dependent."));
    maxUnitsOverrideCheckbox.setChecked(gameOptions.maximumNumberOfUnitsOverride >= 0);
    maxUnitsOverrideCheckbox.setOnClick([this]() { maxUnitsOverrideTextBox.setVisible(maxUnitsOverrideCheckbox.isChecked()); });
    maxUnitsOverrideHBox.addWidget(&maxUnitsOverrideCheckbox);
    maxUnitsOverrideTextBox.setMinMax(0,999);
    maxUnitsOverrideTextBox.setValue( (gameOptions.maximumNumberOfUnitsOverride < 0) ? 25 : gameOptions.maximumNumberOfUnitsOverride );
    maxUnitsOverrideTextBox.setVisible(gameOptions.maximumNumberOfUnitsOverride >= 0);
    maxUnitsOverrideHBox.addWidget(&maxUnitsOverrideTextBox);
    vboxLeft.addWidget(&maxUnitsOverrideHBox, 24);
    vboxLeft.addWidget(VSpacer::create(6));

    vboxLeft.addWidget(VSpacer::create(62));


    concreteRequiredCheckbox.setText(_("Concrete Required"));
    /* xgettext:no-c-format */
    concreteRequiredCheckbox.setTooltipText(_("If checked building on bare rock will result in 50% structure health penalty."));
    concreteRequiredCheckbox.setChecked(gameOptions.concreteRequired);
    vboxRight.addWidget(&concreteRequiredCheckbox);
    vboxRight.addWidget(VSpacer::create(6));

    fogOfWarCheckbox.setText(_("Fog of War"));
    fogOfWarCheckbox.setTooltipText(_("If checked explored terrain will become foggy when no unit or structure is next to it."));
    fogOfWarCheckbox.setChecked(gameOptions.fogOfWar);
    vboxRight.addWidget(&fogOfWarCheckbox);
    vboxRight.addWidget(VSpacer::create(6));

    instantBuildCheckbox.setText(_("Instant Build"));
    instantBuildCheckbox.setTooltipText(_("If checked the building of structures and units does not take any time."));
    instantBuildCheckbox.setChecked(gameOptions.instantBuild);
    vboxRight.addWidget(&instantBuildCheckbox);
    vboxRight.addWidget(VSpacer::create(6));

    rocketTurretsNeedPowerCheckbox.setText(_("Rocket-Turrets Need Power"));
    rocketTurretsNeedPowerCheckbox.setTooltipText(_("If checked rocket turrets are dysfunctional on power shortage."));
    rocketTurretsNeedPowerCheckbox.setChecked(gameOptions.rocketTurretsNeedPower);
    vboxRight.addWidget(&rocketTurretsNeedPowerCheckbox);
    vboxRight.addWidget(VSpacer::create(6));

    onlyOnePalaceCheckbox.setText(_("Only One Palace per House"));
    onlyOnePalaceCheckbox.setTooltipText(_("If checked only one palace can be build per house."));
    onlyOnePalaceCheckbox.setChecked(gameOptions.onlyOnePalace);
    vboxRight.addWidget(&onlyOnePalaceCheckbox);
    vboxRight.addWidget(VSpacer::create(6));

    gameSpeedMinus.setTextures(pGFXManager->getUIGraphic(UI_Minus), pGFXManager->getUIGraphic(UI_Minus_Pressed));
    gameSpeedMinus.setOnClick(std::bind(&GameOptionsWindow::onGameSpeedMinus, this));
    gameSpeedHBox.addWidget(HSpacer::create(4));
    gameSpeedHBox.addWidget(&gameSpeedMinus);
    gameSpeedHBox.addWidget(HSpacer::create(2));

    gameSpeedBar.setText(_("Game speed"));
    gameSpeedHBox.addWidget(&gameSpeedBar);
    currentGameSpeed = gameOptions.gameSpeed;
    updateGameSpeedBar();
    gameSpeedPlus.setTextures(pGFXManager->getUIGraphic(UI_Plus), pGFXManager->getUIGraphic(UI_Plus_Pressed));
    gameSpeedPlus.setOnClick(std::bind(&GameOptionsWindow::onGameSpeedPlus, this));
    gameSpeedHBox.addWidget(HSpacer::create(2));
    gameSpeedHBox.addWidget(&gameSpeedPlus);
    gameSpeedHBox.addWidget(HSpacer::create(4));
    vboxRight.addWidget(&gameSpeedHBox, 24);
    vboxRight.addWidget(VSpacer::create(6));

    vboxRight.addWidget(VSpacer::create(20));

    vboxRight.addWidget(VSpacer::create(6));

    okbutton.setText(_("OK"));
    okbutton.setOnClick(std::bind(&GameOptionsWindow::onOK, this));
    vboxRight.addWidget(&okbutton, 30);
    vboxRight.addWidget(VSpacer::create(6));


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
    gameOptions.maximumNumberOfUnitsOverride = maxUnitsOverrideCheckbox.isChecked() ? maxUnitsOverrideTextBox.getValue() : -1;

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
