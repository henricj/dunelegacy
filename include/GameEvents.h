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

#ifndef GAMEEVENTS_H
#define GAMEEVENTS_H

#include <cstdint>
#include <string>

enum class GameUIEventType : uint8_t {
    None,
    ShowNewsTickerMessage,
    ShowUrgentNewsTickerMessage,
    OpenInGameMenu,
    OpenMentatHelp,
    ShowWaitingDialog,
    HideWaitingDialog,
    RefreshObjectInterface,
};

struct GameUIEvent final {
    GameUIEventType type{GameUIEventType::None};
    std::string text;
    uint32_t value{0};
    uint32_t secondaryValue{0};
    bool flag{false};
};

#endif // GAMEEVENTS_H
