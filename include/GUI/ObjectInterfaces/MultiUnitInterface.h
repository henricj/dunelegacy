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

#include <GUI/HBox.h>
#include <GUI/StaticContainer.h>
#include <GUI/VBox.h>

#include <GUI/SymbolButton.h>
#include <GUI/TextButton.h>

class MultiUnitInterface final : public ObjectInterface {
public:
    static std::unique_ptr<MultiUnitInterface> create(const GameContext& context);

protected:
    MultiUnitInterface(const GameContext& context);

    static void onMove() { dune::globals::currentGame->currentCursorMode = Game::CursorMode_Move; }

    static void onAttack() { dune::globals::currentGame->currentCursorMode = Game::CursorMode_Attack; }

    static void onCapture() { dune::globals::currentGame->currentCursorMode = Game::CursorMode_Capture; }

    static void onCarryallDrop() { dune::globals::currentGame->currentCursorMode = Game::CursorMode_CarryallDrop; }

    void onReturn();

    void OnSendToRepair();

    void onDeploy();

    void onDestruct() const;

    void onGuard(const GameContext& context) { setAttackMode(context, GUARD); }

    void onAreaGuard(const GameContext& context) { setAttackMode(context, AREAGUARD); }

    void onStop(const GameContext& context) { setAttackMode(context, STOP); }

    void onAmbush(const GameContext& context) { setAttackMode(context, AMBUSH); }

    void onHunt(const GameContext& context) { setAttackMode(context, HUNT); }

    void onRetreat(const GameContext& context) { setAttackMode(context, RETREAT); }

    void setAttackMode(const GameContext& context, ATTACKMODE newAttackMode);

    /**
        This method updates the object interface.
        If the object doesn't exists anymore then update returns false.
        \return true = everything ok, false = the object container should be removed
    */
    bool update() override;

    StaticContainer topBox;
    HBox topBoxHBox;
    HBox mainHBox;

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

    const GameContext context_;
};

#endif // MULTIUNITINTERFACE_H
