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

#ifndef ENGINE_FRIGATE_H
#define ENGINE_FRIGATE_H

#include <units/AirUnit.h>

namespace Dune::Engine {

class Frigate final : public AirUnit {
public:
    inline static constexpr ItemID_enum item_id = ItemID_enum::Unit_Frigate;
    using parent                                = AirUnit;

    Frigate(uint32_t objectID, const ObjectInitializer& initializer);
    Frigate(uint32_t objectID, const ObjectStreamInitializer& initializer);
    ~Frigate() override;

    void save(const Game& game, OutputStream& stream) const override;

    void checkPos(const GameContext& context) override;

    /**
        Updates this frigate.
        \return true if this object still exists, false if it was destroyed
    */
    bool update(const GameContext& context) override;

    void deploy(const GameContext& context, const Coord& newLocation) override;

protected:
    void turn(const GameContext& context) override;

private:
    void init();

    bool droppedOffCargo; ///< Is the cargo already dropped off?
};

} // namespace Dune::Engine

#endif // ENGINE_FRIGATE_H
