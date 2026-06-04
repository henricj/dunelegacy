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

#include <GameUIController.h>

#include <config.h>
#include <globals.h>

#include <GUI/dune/InGameMenu.h>
#include <GUI/dune/WaitingForOtherPlayers.h>
#include <Game.h>
#include <GameInterface.h>
#include <Menu/BriefingMenu.h>
#include <Menu/MapChoice.h>
#include <Menu/MentatHelp.h>

#include <utility>

GameUIController::GameUIController(Game& game) : game_(game) { }

GameUIController::~GameUIController() {
    resetGameInterface();
    resetInGameMenu();
    resetInGameMentat();
    resetWaitingForOtherPlayers();
}

void GameUIController::resize(uint32_t width, uint32_t height) {
    if (gameInterface_) {
        gameInterface_->resize(width, height);
    }
}

void GameUIController::createGameInterface(const GameContext& context) {
    if (gameInterface_ != nullptr)
        return;

    setGameInterface(std::make_unique<GameInterface>(context));

    const auto hasRadarOn = dune::globals::pLocalHouse->hasRadarOn();

    if (game_.gameState == GameState::Loading) {
        gameInterface_->getRadarView().setRadarMode(hasRadarOn);
    } else if (hasRadarOn) {
        gameInterface_->getRadarView().switchRadarMode(true);
    }
}

GameInterface* GameUIController::getGameInterface() noexcept {
    return gameInterface_.get();
}

GameInterface* GameUIController::getGameInterface() const noexcept {
    return gameInterface_.get();
}

void GameUIController::setGameInterface(std::unique_ptr<GameInterface> gameInterface) {
    gameInterface_ = std::move(gameInterface);
}

void GameUIController::resetGameInterface() {
    gameInterface_.reset();
}

void GameUIController::publishUIEvent(GameUIEvent event) {
    uiEvents_.push(std::move(event));
}

bool GameUIController::pollUIEvent(GameUIEvent& event) {
    if (uiEvents_.empty())
        return false;

    event = std::move(uiEvents_.front());
    uiEvents_.pop();
    return true;
}

void GameUIController::addToNewsTicker(std::string text) const {
    if (gameInterface_ != nullptr) {
        gameInterface_->addToNewsTicker(std::move(text));
    }
}

void GameUIController::addUrgentMessageToNewsTicker(std::string text) const {
    if (gameInterface_ != nullptr) {
        gameInterface_->addUrgentMessageToNewsTicker(std::move(text));
    }
}

void GameUIController::updateObjectInterface() {
    if (gameInterface_ != nullptr) {
        gameInterface_->updateObjectInterface();
    }
}

void GameUIController::showBriefingMenu(HOUSETYPE houseID, int mission, int type,
                                        MenuBase::event_handler_type handler) {
    BriefingMenu(houseID, mission, type).showMenu(handler);
}

std::pair<int, uint32_t>
GameUIController::showMapChoiceMenu(HOUSETYPE houseID, int mission, uint32_t alreadyPlayedRegions,
                                    MenuBase::event_handler_type handler) {
    MapChoice mapChoice(houseID, mission, alreadyPlayedRegions);
    mapChoice.showMenu(handler);
    return {mapChoice.getSelectedMission(), mapChoice.getAlreadyPlayedRegions()};
}

void GameUIController::showInGameMenu(bool bMultiplayer, int color) {
    setInGameMenu(std::make_unique<InGameMenu>(bMultiplayer, color));
}

void GameUIController::showMentatHelp(HOUSETYPE houseID, int techLevel, int mission) {
    setInGameMentat(std::make_unique<MentatHelp>(houseID, techLevel, mission));
}

void GameUIController::showWaitingDialog() {
    setWaitingForOtherPlayers(std::make_unique<WaitingForOtherPlayers>());
}

void GameUIController::hideWaitingDialog() {
    resetWaitingForOtherPlayers();
}

void GameUIController::updateDialogs() {
    if (mentatHelp_ != nullptr) {
        mentatHelp_->update();
    }

    if (waitingForOtherPlayers_ != nullptr) {
        waitingForOtherPlayers_->update();
    }
}

InGameMenu* GameUIController::getInGameMenu() noexcept {
    return inGameMenu_.get();
}

InGameMenu* GameUIController::getInGameMenu() const noexcept {
    return inGameMenu_.get();
}

void GameUIController::setInGameMenu(std::unique_ptr<InGameMenu> inGameMenu) {
    inGameMenu_ = std::move(inGameMenu);
}

void GameUIController::resetInGameMenu() {
    inGameMenu_.reset();
}

MentatHelp* GameUIController::getInGameMentat() noexcept {
    return mentatHelp_.get();
}

MentatHelp* GameUIController::getInGameMentat() const noexcept {
    return mentatHelp_.get();
}

void GameUIController::setInGameMentat(std::unique_ptr<MentatHelp> mentatHelp) {
    mentatHelp_ = std::move(mentatHelp);
}

void GameUIController::resetInGameMentat() {
    mentatHelp_.reset();
}

WaitingForOtherPlayers* GameUIController::getWaitingForOtherPlayers() noexcept {
    return waitingForOtherPlayers_.get();
}

WaitingForOtherPlayers* GameUIController::getWaitingForOtherPlayers() const noexcept {
    return waitingForOtherPlayers_.get();
}

void GameUIController::setWaitingForOtherPlayers(std::unique_ptr<WaitingForOtherPlayers> waitingForOtherPlayers) {
    waitingForOtherPlayers_ = std::move(waitingForOtherPlayers);
}

void GameUIController::resetWaitingForOtherPlayers() {
    waitingForOtherPlayers_.reset();
}
