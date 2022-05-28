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

#include "GUI/ObjectInterfaces/MultiUnitInterface.h"

#include "GUI/Spacer.h"

#include "FileClasses/GFXManager.h"
#include "Game.h"
#include <FileClasses/TextManager.h>

#include <units/Devastator.h>
#include <units/Harvester.h>
#include <units/MCV.h>
#include <units/UnitBase.h>

MultiUnitInterface::MultiUnitInterface(const GameContext& context) : context_{context} {
    const auto house_id = static_cast<int>(dune::globals::pLocalHouse->getHouseID());
    const Uint32 color  = SDL2RGB(dune::globals::palette[dune::globals::houseToPaletteIndex[house_id] + 3]);

    MultiUnitInterface::addWidget(&topBox, Point(0, 0), Point(SIDEBARWIDTH - 25, 80));

    MultiUnitInterface::addWidget(&mainHBox, Point(0, 80), Point(SIDEBARWIDTH - 25, getRendererHeight() - 80 - 148));

    topBox.addWidget(&topBoxHBox, Point(0, 22), Point(SIDEBARWIDTH - 25, 58));

    topBoxHBox.addWidget(Widget::create<Spacer>().release());
    topBoxHBox.addWidget(Widget::create<VSpacer>(56).release());
    topBoxHBox.addWidget(Widget::create<Spacer>().release());

    mainHBox.addWidget(Widget::create<HSpacer>(4).release());

    buttonVBox.addWidget(Widget::create<VSpacer>(6).release());

    auto* const gfx = dune::globals::pGFXManager.get();

    moveButton.setSymbol(gfx->getUIGraphicSurface(UI_CursorMove_Zoomlevel0));
    moveButton.setTooltipText(_("Move to a position (Hotkey: M)"));
    moveButton.setToggleButton(true);
    moveButton.setOnClick([&game = context_.game] { game.currentCursorMode = Game::CursorMode_Move; });
    actionHBox.addWidget(&moveButton);

    actionHBox.addWidget(Widget::create<HSpacer>(2).release());

    attackButton.setSymbol(gfx->getUIGraphicSurface(UI_CursorAttack_Zoomlevel0));
    attackButton.setTooltipText(_("Attack a unit, structure or position (Hotkey: A)"));
    attackButton.setToggleButton(true);
    attackButton.setOnClick([&game = context_.game] { game.currentCursorMode = Game::CursorMode_Attack; });
    actionHBox.addWidget(&attackButton);

    actionHBox.addWidget(Widget::create<HSpacer>(2).release());

    carryallDropButton.setSymbol(gfx->getUIGraphicSurface(UI_CursorCarryallDrop_Zoomlevel0));
    carryallDropButton.setTooltipText(_("Request Carryall drop to a position (Hotkey: D)"));
    carryallDropButton.setToggleButton(true);
    carryallDropButton.setOnClick([&game = context_.game] { game.currentCursorMode = Game::CursorMode_CarryallDrop; });
    actionHBox.addWidget(&carryallDropButton);

    actionHBox.addWidget(Widget::create<HSpacer>(2).release());

    captureButton.setSymbol(gfx->getUIGraphicSurface(UI_CursorCapture_Zoomlevel0));
    captureButton.setTooltipText(_("Capture a building (Hotkey: C)"));
    captureButton.setToggleButton(true);
    captureButton.setOnClick([&game = context_.game] { game.currentCursorMode = Game::CursorMode_Capture; });
    actionHBox.addWidget(&captureButton);

    buttonVBox.addWidget(&actionHBox, 26);

    buttonVBox.addWidget(Widget::create<VSpacer>(2).release());

    returnButton.setSymbol(gfx->getUIGraphicSurface(UI_ReturnIcon));
    returnButton.setTooltipText(_("Return harvester to refinery (Hotkey: H)"));
    returnButton.setOnClick([this] { onReturn(); });
    commandHBox.addWidget(&returnButton);

    commandHBox.addWidget(Widget::create<HSpacer>(2).release());

    deployButton.setSymbol(gfx->getUIGraphicSurface(UI_DeployIcon));
    deployButton.setTooltipText(_("Build a new construction yard"));
    deployButton.setOnClick([this] { onDeploy(); });
    commandHBox.addWidget(&deployButton);

    commandHBox.addWidget(Widget::create<HSpacer>(2).release());

    destructButton.setSymbol(gfx->getUIGraphicSurface(UI_DestructIcon));
    destructButton.setTooltipText(_("Self-destruct this unit"));
    destructButton.setOnClick([this] { onDestruct(); });
    commandHBox.addWidget(&destructButton);

    commandHBox.addWidget(Widget::create<HSpacer>(2).release());

    sendToRepairButton.setSymbol(gfx->getUIGraphicSurface(UI_SendToRepairIcon));
    sendToRepairButton.setTooltipText(_("Repair this unit (Hotkey: R)"));
    sendToRepairButton.setOnClick([this] { OnSendToRepair(); });
    commandHBox.addWidget(&sendToRepairButton);

    buttonVBox.addWidget(&commandHBox, 26);

    buttonVBox.addWidget(Widget::create<VSpacer>(6).release());

    guardButton.setText(_("Guard"));
    guardButton.setTextColor(color);
    guardButton.setTooltipText(_("Unit will not move from location"));
    guardButton.setToggleButton(true);
    guardButton.setOnClick([this] { setAttackMode(GUARD); });
    buttonVBox.addWidget(&guardButton, 26);

    buttonVBox.addWidget(Widget::create<VSpacer>(6).release());

    areaGuardButton.setText(_("Area Guard"));
    areaGuardButton.setTextColor(color);
    areaGuardButton.setTooltipText(_("Unit will engage any unit within guard range"));
    areaGuardButton.setToggleButton(true);
    areaGuardButton.setOnClick([this] { setAttackMode(AREAGUARD); });
    buttonVBox.addWidget(&areaGuardButton, 26);

    buttonVBox.addWidget(Widget::create<VSpacer>(6).release());

    stopButton.setText(_("Stop"));
    stopButton.setTextColor(color);
    stopButton.setTooltipText(_("Unit will not move, nor attack"));
    stopButton.setToggleButton(true);
    stopButton.setOnClick([this] { setAttackMode(STOP); });
    buttonVBox.addWidget(&stopButton, 26);

    buttonVBox.addWidget(Widget::create<VSpacer>(6).release());

    ambushButton.setText(_("Ambush"));
    ambushButton.setTextColor(color);
    ambushButton.setTooltipText(_("Unit will not move until enemy unit spotted"));
    ambushButton.setToggleButton(true);
    ambushButton.setOnClick([this] { setAttackMode(AMBUSH); });
    buttonVBox.addWidget(&ambushButton, 26);

    buttonVBox.addWidget(Widget::create<VSpacer>(6).release());

    huntButton.setText(_("Hunt"));
    huntButton.setTextColor(color);
    huntButton.setTooltipText(_("Unit will immediately start to engage an enemy unit"));
    huntButton.setToggleButton(true);
    huntButton.setOnClick([this] { setAttackMode(HUNT); });
    buttonVBox.addWidget(&huntButton, 26);

    buttonVBox.addWidget(Widget::create<VSpacer>(6).release());

    retreatButton.setText(_("Retreat"));
    retreatButton.setTextColor(color);
    retreatButton.setTooltipText(_("Unit will retreat back to base"));
    retreatButton.setToggleButton(true);
    retreatButton.setOnClick([this] { setAttackMode(RETREAT); });
    buttonVBox.addWidget(&retreatButton, 26);

    buttonVBox.addWidget(Widget::create<VSpacer>(6).release());
    buttonVBox.addWidget(Widget::create<Spacer>().release());
    buttonVBox.addWidget(Widget::create<VSpacer>(6).release());

    mainHBox.addWidget(&buttonVBox);
    mainHBox.addWidget(Widget::create<HSpacer>(5).release());

    update();
}

MultiUnitInterface::~MultiUnitInterface() = default;

void MultiUnitInterface::onReturn() {
    const auto& [game, map, object_manager] = context_;

    for (const auto selectedUnitID : game.getSelectedList()) {
        auto* const pHarvester = object_manager.getObject<Harvester>(selectedUnitID);

        if (nullptr == pHarvester)
            continue;

        pHarvester->handleReturnClick(context_);
    }
}

void MultiUnitInterface::OnSendToRepair() {
    const auto& [game, map, object_manager] = context_;

    for (const auto selectedUnitID : game.getSelectedList()) {
        auto* const pGroundUnit = object_manager.getObject<GroundUnit>(selectedUnitID);

        if (nullptr == pGroundUnit)
            continue;

        if (pGroundUnit->getHealth() < pGroundUnit->getMaxHealth())
            pGroundUnit->handleSendToRepairClick();
    }
}

void MultiUnitInterface::onDeploy() {
    const auto& [game, map, object_manager] = context_;

    for (const auto selectedUnitID : game.getSelectedList()) {
        auto* const pMCV = object_manager.getObject<MCV>(selectedUnitID);

        if (nullptr == pMCV)
            continue;

        pMCV->handleDeployClick();
    }
}

void MultiUnitInterface::onDestruct() const {
    const auto& [game, map, object_manager] = context_;

    for (const auto selectedUnitID : game.getSelectedList()) {
        auto* const pDevastator = object_manager.getObject<Devastator>(selectedUnitID);

        if (nullptr == pDevastator)
            continue;

        pDevastator->handleStartDevastateClick();
    }
}

void MultiUnitInterface::setAttackMode(ATTACKMODE newAttackMode) {
    auto& [game, map, objectManager] = context_;

    UnitBase* pLastUnit = nullptr;
    for (const auto selectedUnitID : game.getSelectedList()) {
        auto* const pUnit = objectManager.getObject<UnitBase>(selectedUnitID);

        if (pUnit == nullptr)
            continue;

        pLastUnit = pUnit;
        pUnit->handleSetAttackModeClick(context_, newAttackMode);
    }

    if (pLastUnit != nullptr) {
        pLastUnit->playConfirmSound();
    }

    update();
}

bool MultiUnitInterface::update() {
    const auto& [game, map, object_manager] = context_;

    if (game.getSelectedList().empty())
        return false;

    moveButton.setToggleState(game.currentCursorMode == Game::CursorMode_Move);
    attackButton.setToggleState(game.currentCursorMode == Game::CursorMode_Attack);
    captureButton.setToggleState(game.currentCursorMode == Game::CursorMode_Capture);

    bool bGuard     = true;
    bool bAreaGuard = true;
    bool bStop      = true;
    bool bAmbush    = true;
    bool bHunt      = true;
    bool bRetreat   = true;

    bool bShowAttack       = false;
    bool bShowCapture      = false;
    bool bShowReturn       = false;
    bool bShowDeploy       = false;
    bool bShowDevastate    = false;
    bool bShowRepair       = false;
    bool bShowCarryallDrop = false;

    for (const auto selectedUnitID : game.getSelectedList()) {
        const auto* const pUnit = object_manager.getObject<UnitBase>(selectedUnitID);

        if (nullptr == pUnit)
            continue;

        const ATTACKMODE attackMode = pUnit->getAttackMode();
        bGuard                      = bGuard && (attackMode == GUARD);
        bAreaGuard                  = bAreaGuard && (attackMode == AREAGUARD);
        bStop                       = bStop && (attackMode == STOP);
        bAmbush                     = bAmbush && (attackMode == AMBUSH);
        bHunt                       = bHunt && (attackMode == HUNT);
        bRetreat                    = bRetreat && (attackMode == RETREAT);

        if (pUnit->canAttack())
            bShowAttack = true;

        if (pUnit->getOwner()->hasCarryalls())
            bShowCarryallDrop = true;

        if (pUnit->getHealth() < pUnit->getMaxHealth())
            bShowRepair = true;

        switch (pUnit->getItemID()) {
            case Unit_Soldier:
            case Unit_Trooper: {
                bShowCapture = true;
            } break;

            case Unit_Harvester: {
                bShowReturn = true;
            } break;

            case Unit_MCV: {
                bShowDeploy = true;
            } break;

            case Unit_Devastator: {
                bShowDevastate = true;
            } break;

            default: {
            } break;
        }
    }

    attackButton.setVisible(bShowAttack);
    captureButton.setVisible(bShowCapture);
    returnButton.setVisible(bShowReturn);
    deployButton.setVisible(bShowDeploy);
    destructButton.setVisible(bShowDevastate);
    sendToRepairButton.setVisible(bShowRepair);
    carryallDropButton.setVisible(bShowCarryallDrop && game.getGameInitSettings().getGameOptions().manualCarryallDrops);

    guardButton.setToggleState(bGuard);
    areaGuardButton.setToggleState(bAreaGuard);
    stopButton.setToggleState(bStop);
    ambushButton.setToggleState(bAmbush);
    huntButton.setToggleState(bHunt);
    retreatButton.setToggleState(bRetreat);

    return true;
}
