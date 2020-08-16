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

#ifndef HUMANPLAYER_H
#define HUMANPLAYER_H


#include "Player.h"

namespace Dune::Engine {
// forward declarations
class UnitBase;
class StructureBase;
class ObjectBase;

class HumanPlayer final : public Player {
public:
    HumanPlayer(const GameContext& context, House* associatedHouse, std::string_view playername,
                const Random& random);
    HumanPlayer(const GameContext& context, InputStream& stream, House* associatedHouse);
    ~HumanPlayer() override;

    void save(OutputStream& stream) const override;

    void update() override;

public:
    uint32_t nextExpectedCommandsCycle{}; ///< The next cycle we expect commands for (using for network games)
};

} // namespace Dune::Engine

#endif // HUMANPLAYER_H
