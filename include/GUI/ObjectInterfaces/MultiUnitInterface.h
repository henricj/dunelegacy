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
	    int color = houseColor[pLocalHouse->getHouseID()];

        addWidget(&topBox,Point(0,0),Point(SIDEBARWIDTH - 25,80));

		addWidget(&mainHBox,Point(0,80),Point(SIDEBARWIDTH - 25,screen->h - 80 - 148));

		topBox.addWidget(&topBoxHBox,Point(0,22),Point(SIDEBARWIDTH - 25,58));

		topBoxHBox.addWidget(Spacer::create());
		topBoxHBox.addWidget(VSpacer::create(56));
		topBoxHBox.addWidget(Spacer::create());


		mainHBox.addWidget(HSpacer::create(5));

		buttonVBox.addWidget(VSpacer::create(6));

		moveButton.setSymbol(pGFXManager->getUIGraphic(UI_CursorMove_Zoomlevel0), false);
		moveButton.setTooltipText(_("Move to a position (Hotkey: M)"));
		moveButton.setToggleButton(true);
		moveButton.setOnClick(std::bind(&MultiUnitInterface::onMove, this));
		actionHBox.addWidget(&moveButton);

		actionHBox.addWidget(HSpacer::create(3));

        attackButton.setSymbol(pGFXManager->getUIGraphic(UI_CursorAttack_Zoomlevel0), false);
		attackButton.setTooltipText(_("Attack a unit, structure or position (Hotkey: A)"));
		attackButton.setToggleButton(true);
		attackButton.setOnClick(std::bind(&MultiUnitInterface::onAttack, this));
		actionHBox.addWidget(&attackButton);

        actionHBox.addWidget(HSpacer::create(3));

        requestCarryallButton.setSymbol(pGFXManager->getUIGraphic(UI_CursorCapture_Zoomlevel0), false);
		requestCarryallButton.setTooltipText(_("Request Carryall drop to a position (Hotkey: C)"));
		requestCarryallButton.setToggleButton(true);
		requestCarryallButton.setOnClick(std::bind(&MultiUnitInterface::onRequestCarryall, this));
		actionHBox.addWidget(&requestCarryallButton);

        actionHBox.addWidget(HSpacer::create(3));

        /* Use the capture button for requesting a carryall

        captureButton.setSymbol(pGFXManager->getUIGraphic(UI_CursorCapture_Zoomlevel0), false);
        captureButton.setTooltipText(_("Capture a building (Hotkey: C)"));
		captureButton.setToggleButton(true);
		captureButton.setOnClick(std::bind(&MultiUnitInterface::onCapture, this));
		actionHBox.addWidget(&captureButton);
        */

		buttonVBox.addWidget(&actionHBox, 28);


		buttonVBox.addWidget(VSpacer::create(3));

        returnButton.setSymbol(pGFXManager->getUIGraphic(UI_ReturnIcon), false);
        returnButton.setTooltipText(_("Return harvester to refinery (Hotkey: R)"));
		returnButton.setOnClick(std::bind(&MultiUnitInterface::onReturn, this));
		commandHBox.addWidget(&returnButton);

		commandHBox.addWidget(HSpacer::create(3));

		deployButton.setSymbol(pGFXManager->getUIGraphic(UI_DeployIcon), false);
		deployButton.setTooltipText(_("Build a new construction yard"));
		deployButton.setOnClick(std::bind(&MultiUnitInterface::onDeploy, this));
		commandHBox.addWidget(&deployButton);

        commandHBox.addWidget(HSpacer::create(3));

        destructButton.setSymbol(pGFXManager->getUIGraphic(UI_DestructIcon), false);
        destructButton.setTooltipText(_("Self-destruct this unit"));
		destructButton.setOnClick(std::bind(&MultiUnitInterface::onDestruct, this));
		commandHBox.addWidget(&destructButton);

		buttonVBox.addWidget(&commandHBox, 28);

		buttonVBox.addWidget(VSpacer::create(6));

        guardButton.setText(_("Guard"));
        guardButton.setTextColor(color+3);
		guardButton.setTooltipText(_("Unit will not move from location"));
		guardButton.setToggleButton(true);
		guardButton.setOnClick(std::bind(&MultiUnitInterface::onGuard, this));
		buttonVBox.addWidget(&guardButton, 28);

		buttonVBox.addWidget(VSpacer::create(6));

		areaGuardButton.setText(_("Area Guard"));
        areaGuardButton.setTextColor(color+3);
		areaGuardButton.setTooltipText(_("Unit will engage any unit within guard range"));
		areaGuardButton.setToggleButton(true);
		areaGuardButton.setOnClick(std::bind(&MultiUnitInterface::onAreaGuard, this));
		buttonVBox.addWidget(&areaGuardButton, 28);

		buttonVBox.addWidget(VSpacer::create(6));

		stopButton.setText(_("Stop"));
        stopButton.setTextColor(color+3);
		stopButton.setTooltipText(_("Unit will not move, nor attack"));
		stopButton.setToggleButton(true);
		stopButton.setOnClick(std::bind(&MultiUnitInterface::onStop, this));
		buttonVBox.addWidget(&stopButton, 28);

		buttonVBox.addWidget(VSpacer::create(6));

        ambushButton.setText(_("Ambush"));
        ambushButton.setTextColor(color+3);
		ambushButton.setTooltipText(_("Unit will not move until enemy unit spotted"));
		ambushButton.setToggleButton(true);
		ambushButton.setOnClick(std::bind(&MultiUnitInterface::onAmbush, this));
		buttonVBox.addWidget(&ambushButton, 28);

		buttonVBox.addWidget(VSpacer::create(6));

        huntButton.setText(_("Hunt"));
        huntButton.setTextColor(color+3);
		huntButton.setTooltipText(_("Unit will immediately start to engage an enemy unit"));
		huntButton.setToggleButton(true);
		huntButton.setOnClick(std::bind(&MultiUnitInterface::onHunt, this));
		buttonVBox.addWidget(&huntButton, 28);

        buttonVBox.addWidget(VSpacer::create(6));

		retreatButton.setText(_("Retreat"));
        retreatButton.setTextColor(color+3);
		retreatButton.setTooltipText(_("Unit will retreat back to base"));
		retreatButton.setToggleButton(true);
		retreatButton.setOnClick(std::bind(&MultiUnitInterface::onRetreat, this));
		buttonVBox.addWidget(&retreatButton, 28);

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

    void onRequestCarryall() {
        currentGame->currentCursorMode = Game::CursorMode_RequestCarryall;
	}

	void onReturn() {
        std::set<Uint32>::const_iterator iter;
		for(iter = currentGame->getSelectedList().begin(); iter != currentGame->getSelectedList().end(); ++iter) {
            ObjectBase* pObject = currentGame->getObjectManager().getObject(*iter);
            Harvester* pHarvester = dynamic_cast<Harvester*>(pObject);
            if(pHarvester != NULL) {
                pHarvester->handleReturnClick();
            }
		}
	}

	void onDeploy() {
        std::set<Uint32>::const_iterator iter;
		for(iter = currentGame->getSelectedList().begin(); iter != currentGame->getSelectedList().end(); ++iter) {
            ObjectBase* pObject = currentGame->getObjectManager().getObject(*iter);
            MCV* pMCV = dynamic_cast<MCV*>(pObject);
            if(pMCV != NULL) {
                pMCV->handleDeployClick();
            }
		}
	}

	void onDestruct() {
        std::set<Uint32>::const_iterator iter;
		for(iter = currentGame->getSelectedList().begin(); iter != currentGame->getSelectedList().end(); ++iter) {
            ObjectBase* pObject = currentGame->getObjectManager().getObject(*iter);
            Devastator* pDevastator = dynamic_cast<Devastator*>(pObject);
            if(pDevastator != NULL) {
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

	    UnitBase* pLastUnit = NULL;
        std::set<Uint32>::const_iterator iter;
		for(iter = currentGame->getSelectedList().begin(); iter != currentGame->getSelectedList().end(); ++iter) {
            ObjectBase* pObject = currentGame->getObjectManager().getObject(*iter);
            UnitBase* pUnit = dynamic_cast<UnitBase*>(pObject);
            if(pUnit != NULL) {
                pLastUnit = pUnit;
                pUnit->handleSetAttackModeClick(newAttackMode);
            }
		}

		if(pLastUnit != NULL) {
		    pLastUnit->playConfirmSound();
		}

		update();
	}

	/**
		This method updates the object interface.
		If the object doesn't exists anymore then update returns false.
		\return true = everything ok, false = the object container should be removed
	*/
	virtual bool update() {
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

        bool bShowAttack = false;
        bool bShowCapture = false;
        bool bShowReturn = false;
		bool bShowDeploy = false;
		bool bShowDevastate = false;

		std::set<Uint32>::const_iterator iter;
		for(iter = currentGame->getSelectedList().begin(); iter != currentGame->getSelectedList().end(); ++iter) {
            ObjectBase* pObject = currentGame->getObjectManager().getObject(*iter);
            UnitBase* pUnit = dynamic_cast<UnitBase*>(pObject);
            if(pUnit != NULL) {
                ATTACKMODE attackMode = pUnit->getAttackMode();
                bGuard = bGuard && (attackMode == GUARD);
                bAreaGuard = bAreaGuard && (attackMode == AREAGUARD);
                bStop = bStop && (attackMode == STOP);
                bAmbush = bAmbush && (attackMode == AMBUSH);
                bHunt = bHunt && (attackMode == HUNT);

                if(pUnit->canAttack()) {
                    bShowAttack = true;
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

        guardButton.setToggleState( bGuard );
        areaGuardButton.setToggleState( bAreaGuard );
        stopButton.setToggleState( bStop );
        ambushButton.setToggleState( bAmbush );
        huntButton.setToggleState( bHunt );

		return true;
	}

	StaticContainer	topBox;
	HBox			topBoxHBox;
	HBox			mainHBox;

	HBox		    buttonHBox;
	VBox		    buttonVBox;
	HBox            actionHBox;
	HBox            commandHBox;

	SymbolButton    moveButton;
	SymbolButton    attackButton;
    SymbolButton    captureButton;
    SymbolButton    returnButton;
    SymbolButton    deployButton;
    SymbolButton    destructButton;
    SymbolButton    repairButton;
    SymbolButton    requestCarryallButton;

    TextButton	    guardButton;
	TextButton	    areaGuardButton;
	TextButton      stopButton;
	TextButton	    ambushButton;
	TextButton      huntButton;
    TextButton      retreatButton;
};

#endif //MULTIUNITINTERFACE_H
