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

#ifndef FRIGATE_H
#define FRIGATE_H

#include <units/AirUnit.h>

class Frigate final : public AirUnit
{
public:
    static const ItemID_enum item_id = ItemID_enum::Unit_Frigate;
    using parent = AirUnit;

    Frigate(ItemID_enum itemID, Uint32 objectID, const ObjectInitializer& initializer);
    Frigate(ItemID_enum itemID, Uint32 objectID, const ObjectStreamInitializer& initializer);
    ~Frigate() override;

    void save(OutputStream& stream) const override;

    void checkPos() override;

    /**
        Updates this frigate.
        \return true if this object still exists, false if it was destroyed
    */
    bool update() override;

    void deploy(const Coord& newLocation) override;

protected:
    void turn() override;

private:
    void init();

    bool droppedOffCargo; ///< Is the cargo already dropped off?
};

#endif // FRIGATE_H
