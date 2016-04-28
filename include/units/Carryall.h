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
    Carryall(House* newOwner);
    Carryall(InputStream& stream);
    void init();
    virtual ~Carryall();

    void checkPos();

    /**
        Updates this carryall.
        \return true if this object still exists, false if it was destroyed
    */
    bool update();

    virtual FixPoint getMaxSpeed() const;

    virtual void deploy(const Coord& newLocation);

    void destroy();

    void deployUnit(Uint32 unitID);

    void giveCargo(UnitBase* newUnit);

    void save(OutputStream& stream) const;

    void setTarget(const ObjectBase* newTarget);

    bool hasCargo() const {
        return !pickedUpUnitList.empty();
    }

    inline void book() { booked = true; }

    inline void setOwned(bool b) { owned = b; }

    inline void setDropOfferer(bool status) {
        aDropOfferer = status;
        if(aDropOfferer) {
            booked = true;
        }
    }

    inline bool isBooked() const { return (booked || hasCargo()); }

protected:
    void findConstYard();
    void releaseTarget();
    void engageTarget();
    void pickupTarget();
    void targeting();

    // unit state/properties
    std::list<Uint32>   pickedUpUnitList;   ///< What units does this carryall carry?

    bool     booked;             ///< Is this carryall currently booked?
    bool     idle;               ///< Is this carryall currently idle?
    bool     firstRun;           ///< Is this carryall new?
    bool     owned;              ///< Is this carryall owned or is it just here to drop something off

    bool     aDropOfferer;       ///< This carryall just drops some units and vanishes afterwards
    bool     droppedOffCargo;    ///< Is the cargo already dropped off?

    FixPoint currentMaxSpeed;    ///< The current maximum allowed speed

    Uint8    curFlyPoint;        ///< The current flyPoint
    Coord    flyPoints[8];       ///< Array of flight points
    Coord    constYardPoint;     ///< The position of the construction yard to fly around
};

#endif // CARRYALL_H
