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

#ifndef MULTIUNITINTERFACE_H
#define MULTIUNITINTERFACE_H

#include "ObjectInterface.h"

#include <globals.h>

#include <GUI/StaticContainer.h>
#include <GUI/HBox.h>
#include <GUI/VBox.h>

#include <FileClasses/GFXManager.h>
#include <FileClasses/TextManager.h>

#include <GUI/TextButton.h>
#include <GUI/SymbolButton.h>

#include <units/UnitBase.h>
#include <units/MCV.h>
#include <units/Harvester.h>
#include <units/Devastator.h>

class MultiUnitInterface : public ObjectInterface {
public:
    static MultiUnitInterface* create() {
        MultiUnitInterface* tmp = new MultiUnitInterface();
        tmp->pAllocated = true;
        return tmp;
    }

protected:
    MultiUnitInterface() : ObjectInterface() {
        Uint32 color = SDL2RGB(palette[houseToPaletteIndex[pLocalHouse->getHouseID()]+3]);

        addWidget(&topBox,Point(0,0),Point(SIDEBARWIDTH - 25,80));

        addWidget(&mainHBox,Point(0,80),Point(SIDEBARWIDTH - 25,getRendererHeight() - 80 - 148));

        topBox.addWidget(&topBoxHBox,Point(0,22),Point(SIDEBARWIDTH - 25,58));

        topBoxHBox.addWidget(Spacer::create());
        topBoxHBox.addWidget(VSpacer::create(56));
        topBoxHBox.addWidget(Spacer::create());


        mainHBox.addWidget(HSpacer::create(4));

        buttonVBox.addWidget(VSpacer::create(6));

        moveButton.setSymbol(pGFXManager->getUIGraphicSurface(UI_CursorMove_Zoomlevel0));
        moveButton.setTooltipText(_("Move to a position (Hotkey: M)"));
        moveButton.setToggleButton(true);
        moveButton.setOnClick(std::bind(&MultiUnitInterface::onMove, this));
        actionHBox.addWidget(&moveButton);

        actionHBox.addWidget(HSpacer::create(2));

        attackButton.setSymbol(pGFXManager->getUIGraphicSurface(UI_CursorAttack_Zoomlevel0));
        attackButton.setTooltipText(_("Attack a unit, structure or position (Hotkey: A)"));
        attackButton.setToggleButton(true);
        attackButton.setOnClick(std::bind(&MultiUnitInterface::onAttack, this));
        actionHBox.addWidget(&attackButton);

        actionHBox.addWidget(HSpacer::create(2));

        carryallDropButton.setSymbol(pGFXManager->getUIGraphicSurface(UI_CursorCarryallDrop_Zoomlevel0));
        carryallDropButton.setTooltipText(_("Request Carryall drop to a position (Hotkey: D)"));
        carryallDropButton.setToggleButton(true);
        carryallDropButton.setOnClick(std::bind(&MultiUnitInterface::onCarryallDrop, this));
        actionHBox.addWidget(&carryallDropButton);

        actionHBox.addWidget(HSpacer::create(2));

        captureButton.setSymbol(pGFXManager->getUIGraphicSurface(UI_CursorCapture_Zoomlevel0));
        captureButton.setTooltipText(_("Capture a building (Hotkey: C)"));
        captureButton.setToggleButton(true);
        captureButton.setOnClick(std::bind(&MultiUnitInterface::onCapture, this));
        actionHBox.addWidget(&captureButton);

        buttonVBox.addWidget(&actionHBox, 26);

        buttonVBox.addWidget(VSpacer::create(2));

        returnButton.setSymbol(pGFXManager->getUIGraphicSurface(UI_ReturnIcon));
        returnButton.setTooltipText(_("Return harvester to refinery (Hotkey: H)"));
        returnButton.setOnClick(std::bind(&MultiUnitInterface::onReturn, this));
        commandHBox.addWidget(&returnButton);

        commandHBox.addWidget(HSpacer::create(2));

        deployButton.setSymbol(pGFXManager->getUIGraphicSurface(UI_DeployIcon));
        deployButton.setTooltipText(_("Build a new construction yard"));
        deployButton.setOnClick(std::bind(&MultiUnitInterface::onDeploy, this));
        commandHBox.addWidget(&deployButton);

        commandHBox.addWidget(HSpacer::create(2));

        destructButton.setSymbol(pGFXManager->getUIGraphicSurface(UI_DestructIcon));
        destructButton.setTooltipText(_("Self-destruct this unit"));
        destructButton.setOnClick(std::bind(&MultiUnitInterface::onDestruct, this));
        commandHBox.addWidget(&destructButton);

        commandHBox.addWidget(HSpacer::create(2));

        sendToRepairButton.setSymbol(pGFXManager->getUIGraphicSurface(UI_SendToRepairIcon));
        sendToRepairButton.setTooltipText(_("Repair this unit (Hotkey: R)"));
        sendToRepairButton.setOnClick(std::bind(&MultiUnitInterface::OnSendToRepair, this));
        commandHBox.addWidget(&sendToRepairButton);

        buttonVBox.addWidget(&commandHBox, 26);

        buttonVBox.addWidget(VSpacer::create(6));

        guardButton.setText(_("Guard"));
        guardButton.setTextColor(color);
        guardButton.setTooltipText(_("Unit will not move from location"));
        guardButton.setToggleButton(true);
        guardButton.setOnClick(std::bind(&MultiUnitInterface::onGuard, this));
        buttonVBox.addWidget(&guardButton, 26);

        buttonVBox.addWidget(VSpacer::create(6));

        areaGuardButton.setText(_("Area Guard"));
        areaGuardButton.setTextColor(color);
        areaGuardButton.setTooltipText(_("Unit will engage any unit within guard range"));
        areaGuardButton.setToggleButton(true);
        areaGuardButton.setOnClick(std::bind(&MultiUnitInterface::onAreaGuard, this));
        buttonVBox.addWidget(&areaGuardButton, 26);

        buttonVBox.addWidget(VSpacer::create(6));

        stopButton.setText(_("Stop"));
        stopButton.setTextColor(color);
        stopButton.setTooltipText(_("Unit will not move, nor attack"));
        stopButton.setToggleButton(true);
        stopButton.setOnClick(std::bind(&MultiUnitInterface::onStop, this));
        buttonVBox.addWidget(&stopButton, 26);

        buttonVBox.addWidget(VSpacer::create(6));

        ambushButton.setText(_("Ambush"));
        ambushButton.setTextColor(color);
        ambushButton.setTooltipText(_("Unit will not move until enemy unit spotted"));
        ambushButton.setToggleButton(true);
        ambushButton.setOnClick(std::bind(&MultiUnitInterface::onAmbush, this));
        buttonVBox.addWidget(&ambushButton, 26);

        buttonVBox.addWidget(VSpacer::create(6));

        huntButton.setText(_("Hunt"));
        huntButton.setTextColor(color);
        huntButton.setTooltipText(_("Unit will immediately start to engage an enemy unit"));
        huntButton.setToggleButton(true);
        huntButton.setOnClick(std::bind(&MultiUnitInterface::onHunt, this));
        buttonVBox.addWidget(&huntButton, 26);

        buttonVBox.addWidget(VSpacer::create(6));

        retreatButton.setText(_("Retreat"));
        retreatButton.setTextColor(color);
        retreatButton.setTooltipText(_("Unit will retreat back to base"));
        retreatButton.setToggleButton(true);
        retreatButton.setOnClick(std::bind(&MultiUnitInterface::onRetreat, this));
        buttonVBox.addWidget(&retreatButton, 26);

        buttonVBox.addWidget(VSpacer::create(6));
        buttonVBox.addWidget(Spacer::create());
        buttonVBox.addWidget(VSpacer::create(6));

        mainHBox.addWidget(&buttonVBox);
        mainHBox.addWidget(HSpacer::create(5));

        update();
    }

    void onMove() {
        currentGame->currentCursorMode = Game::CursorMode_Move;
    }

    void onAttack() {
        currentGame->currentCursorMode = Game::CursorMode_Attack;
    }

    void onCapture() {
        currentGame->currentCursorMode = Game::CursorMode_Capture;
    }

    void onCarryallDrop() {
        currentGame->currentCursorMode = Game::CursorMode_CarryallDrop;
    }

    void onReturn() {
        for(const Uint32 selectedUnitID : currentGame->getSelectedList()) {
            ObjectBase* pObject = currentGame->getObjectManager().getObject(selectedUnitID);
            Harvester* pHarvester = dynamic_cast<Harvester*>(pObject);
            if(pHarvester != nullptr) {
                pHarvester->handleReturnClick();
            }
        }
    }

    void OnSendToRepair() {
        for(const Uint32 selectedUnitID : currentGame->getSelectedList()) {
            ObjectBase* pObject = currentGame->getObjectManager().getObject(selectedUnitID);
            GroundUnit* pGroundUnit = dynamic_cast<GroundUnit*>(pObject);
            if((pGroundUnit != nullptr) && (pGroundUnit->getHealth() < pGroundUnit->getMaxHealth())) {
                pGroundUnit->handleSendToRepairClick();
            }
        }
    }

    void onDeploy() {
        for(const Uint32 selectedUnitID : currentGame->getSelectedList()) {
            ObjectBase* pObject = currentGame->getObjectManager().getObject(selectedUnitID);
            MCV* pMCV = dynamic_cast<MCV*>(pObject);
            if(pMCV != nullptr) {
                pMCV->handleDeployClick();
            }
        }
    }

    void onDestruct() {
        for(const Uint32 selectedUnitID : currentGame->getSelectedList()) {
            ObjectBase* pObject = currentGame->getObjectManager().getObject(selectedUnitID);
            Devastator* pDevastator = dynamic_cast<Devastator*>(pObject);
            if(pDevastator != nullptr) {
                pDevastator->handleStartDevastateClick();
            }
        }
    }

    void onGuard() {
        setAttackMode(GUARD);
    }

    void onAreaGuard() {
        setAttackMode(AREAGUARD);
    }

    void onStop() {
        setAttackMode(STOP);
    }

    void onAmbush() {
        setAttackMode(AMBUSH);
    }

    void onHunt() {
        setAttackMode(HUNT);
    }

    void onRetreat(){
        setAttackMode(RETREAT);
    }

    void setAttackMode(ATTACKMODE newAttackMode) {

        UnitBase* pLastUnit = nullptr;
        for(const Uint32 selectedUnitID : currentGame->getSelectedList()) {
            ObjectBase* pObject = currentGame->getObjectManager().getObject(selectedUnitID);
            UnitBase* pUnit = dynamic_cast<UnitBase*>(pObject);
            if(pUnit != nullptr) {
                pLastUnit = pUnit;
                pUnit->handleSetAttackModeClick(newAttackMode);
            }
        }

        if(pLastUnit != nullptr) {
            pLastUnit->playConfirmSound();
        }

        update();
    }

    /**
        This method updates the object interface.
        If the object doesn't exists anymore then update returns false.
        \return true = everything ok, false = the object container should be removed
    */
    bool update() override
    {
        if(currentGame->getSelectedList().empty() == true) {
            return false;
        }

        moveButton.setToggleState(currentGame->currentCursorMode == Game::CursorMode_Move);
        attackButton.setToggleState(currentGame->currentCursorMode == Game::CursorMode_Attack);
        captureButton.setToggleState(currentGame->currentCursorMode == Game::CursorMode_Capture);

        bool bGuard = true;
        bool bAreaGuard = true;
        bool bStop = true;
        bool bAmbush = true;
        bool bHunt = true;
        bool bRetreat = true;

        bool bShowAttack = false;
        bool bShowCapture = false;
        bool bShowReturn = false;
        bool bShowDeploy = false;
        bool bShowDevastate = false;
        bool bShowRepair = false;
        bool bShowCarryallDrop = false;

        for(const Uint32 selectedUnitID : currentGame->getSelectedList()) {
            ObjectBase* pObject = currentGame->getObjectManager().getObject(selectedUnitID);
            UnitBase* pUnit = dynamic_cast<UnitBase*>(pObject);
            if(pUnit != nullptr) {
                ATTACKMODE attackMode = pUnit->getAttackMode();
                bGuard = bGuard && (attackMode == GUARD);
                bAreaGuard = bAreaGuard && (attackMode == AREAGUARD);
                bStop = bStop && (attackMode == STOP);
                bAmbush = bAmbush && (attackMode == AMBUSH);
                bHunt = bHunt && (attackMode == HUNT);
                bRetreat = bRetreat && (attackMode == RETREAT);

                if(pUnit->canAttack()) {
                    bShowAttack = true;
                }

                if(pUnit->getOwner()->hasCarryalls()){
                    bShowCarryallDrop = true;
                }

                if(pUnit->getHealth() < pUnit->getMaxHealth()){
                    bShowRepair = true;
                }

                switch(pUnit->getItemID()) {
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
        }

        attackButton.setVisible(bShowAttack);
        captureButton.setVisible(bShowCapture);
        returnButton.setVisible(bShowReturn);
        deployButton.setVisible(bShowDeploy);
        destructButton.setVisible(bShowDevastate);
        sendToRepairButton.setVisible(bShowRepair);
        carryallDropButton.setVisible(bShowCarryallDrop && currentGame->getGameInitSettings().getGameOptions().manualCarryallDrops);

        guardButton.setToggleState( bGuard );
        areaGuardButton.setToggleState( bAreaGuard );
        stopButton.setToggleState( bStop );
        ambushButton.setToggleState( bAmbush );
        huntButton.setToggleState( bHunt );
        retreatButton.setToggleState( bRetreat );

        return true;
    }

    StaticContainer topBox;
    HBox            topBoxHBox;
    HBox            mainHBox;

    HBox            buttonHBox;
    VBox            buttonVBox;
    HBox            actionHBox;
    HBox            commandHBox;

    SymbolButton    moveButton;
    SymbolButton    attackButton;
    SymbolButton    captureButton;
    SymbolButton    returnButton;
    SymbolButton    deployButton;
    SymbolButton    destructButton;
    SymbolButton    sendToRepairButton;
    SymbolButton    carryallDropButton;

    TextButton      guardButton;
    TextButton      areaGuardButton;
    TextButton      stopButton;
    TextButton      ambushButton;
    TextButton      huntButton;
    TextButton      retreatButton;
};

#endif //MULTIUNITINTERFACE_H
