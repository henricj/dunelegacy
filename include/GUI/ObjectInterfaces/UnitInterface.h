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

#ifndef DEFAULTUNITINTERFACE_H
#define DEFAULTUNITINTERFACE_H

#include "DefaultObjectInterface.h"

#include <globals.h>

#include <GUI/HBox.h>
#include <GUI/SymbolButton.h>
#include <GUI/TextButton.h>
#include <GUI/VBox.h>

class UnitInterface final : public DefaultObjectInterface {
public:
    static std::unique_ptr<UnitInterface> create(const GameContext& context, int objectID);

protected:
    explicit UnitInterface(const GameContext& context, int objectID);

    static void onMove() { dune::globals::currentGame->currentCursorMode = Game::CursorMode_Move; }

    static void onAttack() { dune::globals::currentGame->currentCursorMode = Game::CursorMode_Attack; }

    static void onCapture() { dune::globals::currentGame->currentCursorMode = Game::CursorMode_Capture; }

    static void onCarryallDrop() { dune::globals::currentGame->currentCursorMode = Game::CursorMode_CarryallDrop; }

    void OnSendToRepair();

    void onReturn();

    void onDeploy();

    void onDestruct();

    void onGuard() { setAttackMode(GUARD); }

    void onAreaGuard() { setAttackMode(AREAGUARD); }

    void onStop() { setAttackMode(STOP); }

    void onAmbush() { setAttackMode(AMBUSH); }

    void onHunt() { setAttackMode(HUNT); }

    void onRetreat() { setAttackMode(RETREAT); }

    void setAttackMode(ATTACKMODE newAttackMode);

    /**
        This method updates the object interface.
        If the object doesn't exists anymore then update returns false.
        \return true = everything ok, false = the object container should be removed
    */
    bool update() override;

    HBox buttonHBox;
    VBox buttonVBox;
    HBox actionHBox;
    HBox commandHBox;

    SymbolButton moveButton;
    SymbolButton attackButton;
    SymbolButton captureButton;
    SymbolButton returnButton;
    SymbolButton deployButton;
    SymbolButton destructButton;
    SymbolButton sendToRepairButton;
    SymbolButton carryallDropButton;

    TextButton guardButton;
    TextButton areaGuardButton;
    TextButton stopButton;
    TextButton ambushButton;
    TextButton huntButton;
    TextButton retreatButton;
};

#endif // DEFAULTUNITINTERFACE_H
