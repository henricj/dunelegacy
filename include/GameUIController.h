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

#ifndef GAMEUICONTROLLER_H
#define GAMEUICONTROLLER_H

#include <GameEvents.h>

#include <DataTypes.h>

#include <Menu/MenuBase.h>

#include <cstdint>
#include <memory>
#include <queue>
#include <string>
#include <utility>

class Game;
class GameContext;
class GameInterface;
class InGameMenu;
class MentatHelp;
class WaitingForOtherPlayers;

class GameUIController final {
public:
    explicit GameUIController(Game& game);
    ~GameUIController();

    GameUIController(const GameUIController&)            = delete;
    GameUIController(GameUIController&&)                 = delete;
    GameUIController& operator=(const GameUIController&) = delete;
    GameUIController& operator=(GameUIController&&)      = delete;

    void resize(uint32_t width, uint32_t height);

    void createGameInterface(const GameContext& context);

    [[nodiscard]] GameInterface* getGameInterface() noexcept;
    [[nodiscard]] GameInterface* getGameInterface() const noexcept;
    void setGameInterface(std::unique_ptr<GameInterface> gameInterface);
    void resetGameInterface();

    void publishUIEvent(GameUIEvent event);
    [[nodiscard]] bool pollUIEvent(GameUIEvent& event);

    void addToNewsTicker(std::string text) const;
    void addUrgentMessageToNewsTicker(std::string text) const;
    void updateObjectInterface();

    void showBriefingMenu(HOUSETYPE houseID, int mission, int type, MenuBase::event_handler_type handler);
    [[nodiscard]] std::pair<int, uint32_t>
    showMapChoiceMenu(HOUSETYPE houseID, int mission, uint32_t alreadyPlayedRegions,
                      MenuBase::event_handler_type handler);

    void showInGameMenu(bool bMultiplayer, int color);
    void showMentatHelp(HOUSETYPE houseID, int techLevel, int mission);
    void showWaitingDialog();
    void hideWaitingDialog();
    void updateDialogs();

    [[nodiscard]] InGameMenu* getInGameMenu() noexcept;
    [[nodiscard]] InGameMenu* getInGameMenu() const noexcept;
    void setInGameMenu(std::unique_ptr<InGameMenu> inGameMenu);
    void resetInGameMenu();

    [[nodiscard]] MentatHelp* getInGameMentat() noexcept;
    [[nodiscard]] MentatHelp* getInGameMentat() const noexcept;
    void setInGameMentat(std::unique_ptr<MentatHelp> mentatHelp);
    void resetInGameMentat();

    [[nodiscard]] WaitingForOtherPlayers* getWaitingForOtherPlayers() noexcept;
    [[nodiscard]] WaitingForOtherPlayers* getWaitingForOtherPlayers() const noexcept;
    void setWaitingForOtherPlayers(std::unique_ptr<WaitingForOtherPlayers> waitingForOtherPlayers);
    void resetWaitingForOtherPlayers();

private:
    Game& game_;

    std::unique_ptr<GameInterface> gameInterface_;
    std::unique_ptr<InGameMenu> inGameMenu_;
    std::unique_ptr<MentatHelp> mentatHelp_;
    std::unique_ptr<WaitingForOtherPlayers> waitingForOtherPlayers_;

    std::queue<GameUIEvent> uiEvents_;
};

#endif // GAMEUICONTROLLER_H
