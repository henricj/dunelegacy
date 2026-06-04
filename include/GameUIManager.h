/*
 *  This file is part of Dune Legacy.
 *
 *  Dune Legacy is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License, either version 2 of the License, or
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

#ifndef GAME_UI_MANAGER_H
#define GAME_UI_MANAGER_H

#include <DataTypes.h>
#include <memory>
#include <string_view>

union SDL_Event;
class Game;
class GameContext;
class GameInterface;
class MenuBase;
class InGameMenu;
class MentatHelp;
class WaitingForOtherPlayers;

/**
 * Manages UI state and lifecycle for a running Game.
 * Separates UI concerns from core simulation logic.
 * The UI layer (dune library) uses this to coordinate Game + UI rendering.
 */
class GameUIManager {
public:
    explicit GameUIManager(Game& game);
    ~GameUIManager();

    GameUIManager(const GameUIManager&) = delete;
    GameUIManager(GameUIManager&&)      = delete;
    GameUIManager& operator=(const GameUIManager&) = delete;
    GameUIManager& operator=(GameUIManager&&)      = delete;

    /**
     * Called when game is resized
     */
    void onGameResize(uint32_t width, uint32_t height);

    /**
     * Update UI state (menus, dialogs, etc.)
     */
    void updateUI();

    /**
     * Draw UI overlay
     */
    void drawUI();

    /**
     * Handle input through UI (menus, interface, etc.)
     * Returns true if input was consumed by UI
     */
    bool handleUIInput(const SDL_Event& event);

    /**
     * Show briefing menu before mission starts
     */
    void showBriefingMenu(HOUSETYPE houseID, int mission, int missionNumber);

    /**
     * Show map choice menu (for campaign)
     */
    void showMapChoiceMenu(HOUSETYPE houseID, uint32_t missionNumber, uint32_t alreadyPlayedRegions);

    /**
     * Create the in-game interface (radar, status bar, unit controls)
     */
    void createGameInterface(const GameContext& context);

    /**
     * Show in-game menu (pause/options)
     */
    void showInGameMenu(bool isMultiplayer, uint8_t color);

    /**
     * Show mentat help dialog
     */
    void showMentatHelp(HOUSETYPE houseID, int mission, int missionNumber);

    /**
     * Show waiting for other players dialog
     */
    void showWaitingDialog();

    /**
     * Hide waiting for other players dialog
     */
    void hideWaitingDialog();

    /**
     * Get the game interface (or nullptr if not created)
     */
    [[nodiscard]] GameInterface* getGameInterface() noexcept;

private:
    Game& game_;

    std::unique_ptr<GameInterface> gameInterface_;
    std::unique_ptr<InGameMenu> inGameMenu_;
    std::unique_ptr<MentatHelp> mentatHelp_;
    std::unique_ptr<WaitingForOtherPlayers> waitingDialog_;
};

#endif // GAME_UI_MANAGER_H
