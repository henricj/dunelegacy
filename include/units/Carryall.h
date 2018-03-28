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

#ifndef CARRYALL_H
#define CARRYALL_H

#include <units/AirUnit.h>

#include <list>

class Carryall : public AirUnit
{
public:
    explicit Carryall(House* newOwner);
    explicit Carryall(InputStream& stream);
    void init();
    virtual ~Carryall();

    void checkPos();

    /**
        Updates this carryall.
        \return true if this object still exists, false if it was destroyed
    */
    bool update();

    virtual void deploy(const Coord& newLocation);

    void destroy();

    void deployUnit(Uint32 unitID);

    void giveCargo(UnitBase* newUnit);

    void save(OutputStream& stream) const;

    void setTarget(const ObjectBase* newTarget);

    bool hasCargo() const {
        return !pickedUpUnitList.empty();
    }

    inline void setOwned(bool b) { owned = b; }

    inline void setDropOfferer(bool status) {
        aDropOfferer = status;
    }

    inline bool isBooked() const { return (target || hasCargo()); }

protected:
    void releaseTarget();
    void engageTarget();
    void pickupTarget();
    void targeting();
    virtual void turn();

    // unit state/properties
    std::list<Uint32>   pickedUpUnitList;   ///< What units does this carryall carry?

    bool     owned;              ///< Is this carryall owned or is it just here to drop something off

    bool     aDropOfferer;       ///< This carryall just drops some units and vanishes afterwards
    bool     droppedOffCargo;    ///< Is the cargo already dropped off?
};

#endif // CARRYALL_H
