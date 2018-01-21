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

#ifndef REPAIRYARD_H
#define REPAIRYARD_H

#include <structures/StructureBase.h>
#include <ObjectPointer.h>

class Carryall;

class RepairYard final : public StructureBase
{
public:
    explicit RepairYard(House* newOwner);
    explicit RepairYard(InputStream& stream);
    void init();
    virtual ~RepairYard();

    void save(OutputStream& stream) const override;

    ObjectInterface* getInterfaceContainer() override;

    void deployRepairUnit(Carryall* pCarryall = nullptr);

    inline void book() { bookings++; }
    inline void unBook() { bookings--; }
    inline void assignUnit(ObjectPointer newUnit) { repairUnit = newUnit; repairingAUnit = true; }
    inline bool isFree() const { return !repairingAUnit; }
    inline int getNumBookings() const { return bookings; }  //number of harvesters goings there
    inline const UnitBase* getRepairUnit() const { return repairUnit.getUnitPointer(); }
    inline UnitBase* getRepairUnit() { return repairUnit.getUnitPointer(); }

protected:
    /**
        Used for updating things that are specific to that particular structure. Is called from
        StructureBase::update() before the check if this structure is still alive.
    */
    void updateStructureSpecificStuff() override;

private:
    bool            repairingAUnit; ///< Currently repairing?
    ObjectPointer   repairUnit;     ///< The unit to repair
    Uint32          bookings;       ///< Number of bookings for this repair yard
};

#endif // REPAIRYARD_H
