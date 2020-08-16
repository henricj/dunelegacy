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

#include <players/HumanPlayer.h>

#include <Game.h>
#include <Map.h>

#include <units/UnitBase.h>

namespace Dune::Engine {
HumanPlayer::HumanPlayer(const GameContext& context, House* associatedHouse, std::string_view playername,
                         const Random& random)
    : Player(context, associatedHouse, playername, random) {
}

HumanPlayer::HumanPlayer(const GameContext& context, InputStream& stream, House* associatedHouse)
    : Player(context, stream, associatedHouse) {
}

HumanPlayer::~HumanPlayer() = default;

void HumanPlayer::save(OutputStream& stream) const {
    Player::save(stream);

}

void HumanPlayer::update() { }


} // namespace Dune::Engine
