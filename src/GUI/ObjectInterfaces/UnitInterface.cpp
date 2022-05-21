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

#include "GUI/ObjectInterfaces/UnitInterface.h"

#include "GUI/Spacer.h"

#include <FileClasses/GFXManager.h>
#include <FileClasses/TextManager.h>

#include <units/Devastator.h>
#include <units/Harvester.h>
#include <units/MCV.h>
#include <units/UnitBase.h>

std::unique_ptr<UnitInterface> UnitInterface::create(const GameContext& context, int objectID) {
    std::unique_ptr<UnitInterface> tmp{new UnitInterface{context, objectID}};
    tmp->pAllocated_ = true;
    return tmp;
}

UnitInterface::UnitInterface(const GameContext& context, int objectID) : DefaultObjectInterface(context, objectID) {
    const auto color = SDL2RGB(
        dune::globals::palette[houseToPaletteIndex[static_cast<int>(dune::globals::pLocalHouse->getHouseID())] + 3]);

    mainHBox.addWidget(HSpacer::create(4));

    buttonVBox.addWidget(VSpacer::create(6));

    auto* const gfx = dune::globals::pGFXManager.get();

    moveButton.setSymbol(gfx->getUIGraphicSurface(UI_CursorMove_Zoomlevel0));
    moveButton.setTooltipText(_("Move to a position (Hotkey: M)"));
    moveButton.setToggleButton(true);
    moveButton.setOnClick([] { onMove(); });
    actionHBox.addWidget(&moveButton);

    actionHBox.addWidget(HSpacer::create(2));

    attackButton.setSymbol(gfx->getUIGraphicSurface(UI_CursorAttack_Zoomlevel0));
    attackButton.setTooltipText(_("Attack a unit, structure or position (Hotkey: A)"));
    attackButton.setToggleButton(true);
    attackButton.setOnClick([] { onAttack(); });
    actionHBox.addWidget(&attackButton);

    actionHBox.addWidget(HSpacer::create(2));

    carryallDropButton.setSymbol(gfx->getUIGraphicSurface(UI_CursorCarryallDrop_Zoomlevel0));
    carryallDropButton.setTooltipText(_("Request Carryall drop to a position (Hotkey: D)"));
    carryallDropButton.setToggleButton(true);
    carryallDropButton.setOnClick([] { onCarryallDrop(); });
    actionHBox.addWidget(&carryallDropButton);

    actionHBox.addWidget(HSpacer::create(2));

    captureButton.setSymbol(gfx->getUIGraphicSurface(UI_CursorCapture_Zoomlevel0));
    captureButton.setTooltipText(_("Capture a building (Hotkey: C)"));
    captureButton.setVisible((itemID == Unit_Soldier) || (itemID == Unit_Trooper));
    captureButton.setToggleButton(true);
    captureButton.setOnClick([] { onCapture(); });
    actionHBox.addWidget(&captureButton);

    buttonVBox.addWidget(&actionHBox, 26);

    buttonVBox.addWidget(VSpacer::create(2));

    returnButton.setSymbol(gfx->getUIGraphicSurface(UI_ReturnIcon));
    returnButton.setTooltipText(_("Return harvester to refinery (Hotkey: H)"));
    returnButton.setVisible((itemID == Unit_Harvester));
    returnButton.setOnClick([this] { onReturn(); });
    commandHBox.addWidget(&returnButton);

    commandHBox.addWidget(HSpacer::create(2));

    deployButton.setSymbol(gfx->getUIGraphicSurface(UI_DeployIcon));
    deployButton.setTooltipText(_("Build a new construction yard"));
    deployButton.setVisible((itemID == Unit_MCV));
    deployButton.setOnClick([this] { onDeploy(); });
    commandHBox.addWidget(&deployButton);

    commandHBox.addWidget(HSpacer::create(2));

    destructButton.setSymbol(gfx->getUIGraphicSurface(UI_DestructIcon));
    destructButton.setTooltipText(_("Self-destruct this unit"));
    destructButton.setVisible((itemID == Unit_Devastator));
    destructButton.setOnClick([this] { onDestruct(); });
    commandHBox.addWidget(&destructButton);

    commandHBox.addWidget(HSpacer::create(2));

    sendToRepairButton.setSymbol(gfx->getUIGraphicSurface(UI_SendToRepairIcon));
    sendToRepairButton.setTooltipText(_("Repair this unit (Hotkey: R)"));
    sendToRepairButton.setOnClick([this] { OnSendToRepair(); });
    commandHBox.addWidget(&sendToRepairButton);

    buttonVBox.addWidget(&commandHBox, 26);

    buttonVBox.addWidget(VSpacer::create(6));

    guardButton.setText(_("Guard"));
    guardButton.setTextColor(color);
    guardButton.setTooltipText(_("Unit will not move from location"));
    guardButton.setToggleButton(true);
    guardButton.setOnClick([this] { onGuard(); });
    buttonVBox.addWidget(&guardButton, 26);

    buttonVBox.addWidget(VSpacer::create(6));

    areaGuardButton.setText(_("Area Guard"));
    areaGuardButton.setTextColor(color);
    areaGuardButton.setTooltipText(_("Unit will engage any unit within guard range"));
    areaGuardButton.setToggleButton(true);
    areaGuardButton.setOnClick([this] { onAreaGuard(); });
    buttonVBox.addWidget(&areaGuardButton, 26);

    buttonVBox.addWidget(VSpacer::create(6));

    stopButton.setText(_("Stop"));
    stopButton.setTextColor(color);
    stopButton.setTooltipText(_("Unit will not move, nor attack"));
    stopButton.setToggleButton(true);
    stopButton.setOnClick([this] { onStop(); });
    buttonVBox.addWidget(&stopButton, 26);

    buttonVBox.addWidget(VSpacer::create(6));

    ambushButton.setText(_("Ambush"));
    ambushButton.setTextColor(color);
    ambushButton.setTooltipText(_("Unit will not move until enemy unit spotted"));
    ambushButton.setToggleButton(true);
    ambushButton.setOnClick([this] { onAmbush(); });
    buttonVBox.addWidget(&ambushButton, 26);

    buttonVBox.addWidget(VSpacer::create(6));

    huntButton.setText(_("Hunt"));
    huntButton.setTextColor(color);
    huntButton.setTooltipText(_("Unit will immediately start to engage an enemy unit"));
    huntButton.setToggleButton(true);
    huntButton.setOnClick([this] { onHunt(); });
    buttonVBox.addWidget(&huntButton, 26);

    buttonVBox.addWidget(VSpacer::create(6));

    retreatButton.setText(_("Retreat"));
    retreatButton.setTextColor(color);
    retreatButton.setTooltipText(_("Unit will retreat back to base"));
    retreatButton.setToggleButton(true);
    retreatButton.setOnClick([this] { onRetreat(); });
    buttonVBox.addWidget(&retreatButton, 26);

    buttonVBox.addWidget(VSpacer::create(6));
    buttonVBox.addWidget(Spacer::create());
    buttonVBox.addWidget(VSpacer::create(6));

    mainHBox.addWidget(&buttonVBox);
    mainHBox.addWidget(HSpacer::create(5));

    update();
}

void UnitInterface::OnSendToRepair() {
    auto* const pGroundUnit = context_.objectManager.getObject<GroundUnit>(objectID);
    if ((pGroundUnit != nullptr) && (pGroundUnit->getHealth() < pGroundUnit->getMaxHealth())) {
        pGroundUnit->handleSendToRepairClick();
    }
}

void UnitInterface::onReturn() {
    auto* const pHarvester = context_.objectManager.getObject<Harvester>(objectID);
    if (pHarvester != nullptr) {
        pHarvester->handleReturnClick(context_);
    }
}

void UnitInterface::onDeploy() {
    auto* const pMCV = context_.objectManager.getObject<MCV>(objectID);
    if (pMCV != nullptr) {
        pMCV->handleDeployClick();
    }
}

void UnitInterface::onDestruct() {
    auto* const pDevastator = context_.objectManager.getObject<Devastator>(objectID);
    if (pDevastator != nullptr) {
        pDevastator->handleStartDevastateClick();
    }
}

void UnitInterface::setAttackMode(ATTACKMODE newAttackMode) {
    auto* pUnit = context_.objectManager.getObject<UnitBase>(objectID);

    if (pUnit != nullptr) {
        pUnit->handleSetAttackModeClick(context_, newAttackMode);
        pUnit->playConfirmSound();

        update();
    }
}

bool UnitInterface::update() {
    auto* pObject = context_.objectManager.getObject(objectID);
    if (pObject == nullptr)
        return false;

    const auto& game = context_.game;

    moveButton.setToggleState(game.currentCursorMode == Game::CursorMode_Move);
    attackButton.setToggleState(game.currentCursorMode == Game::CursorMode_Attack);
    attackButton.setVisible(pObject->canAttack());
    captureButton.setToggleState(game.currentCursorMode == Game::CursorMode_Capture);
    carryallDropButton.setToggleState(game.currentCursorMode == Game::CursorMode_CarryallDrop);
    carryallDropButton.setVisible(game.getGameInitSettings().getGameOptions().manualCarryallDrops
                                  && pObject->getOwner()->hasCarryalls());
    sendToRepairButton.setVisible(pObject->getHealth() < pObject->getMaxHealth());

    if (const auto* pUnit = dune_cast<UnitBase>(pObject)) {
        const auto AttackMode = pUnit->getAttackMode();

        guardButton.setToggleState(AttackMode == GUARD);
        areaGuardButton.setToggleState(AttackMode == AREAGUARD);
        stopButton.setToggleState(AttackMode == STOP);
        ambushButton.setToggleState(AttackMode == AMBUSH);
        huntButton.setToggleState(AttackMode == HUNT);
        retreatButton.setToggleState(AttackMode == RETREAT);
    }

    return true;
}
