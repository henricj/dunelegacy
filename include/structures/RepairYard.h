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
    static const ItemID_enum item_id = Structure_RepairYard;

    RepairYard(ItemID_enum itemID, Uint32 objectID, const ObjectInitializer& initializer);
    RepairYard(ItemID_enum itemID, Uint32 objectID, const ObjectStreamInitializer& initializer);
    ~RepairYard() override;

    void save(OutputStream& stream) const override;

    ObjectInterface* getInterfaceContainer() override;

    void deployRepairUnit(Carryall* pCarryall = nullptr);

    void book() { bookings++; }
    void unBook() { bookings--; }
    void assignUnit(ObjectPointer newUnit) { repairUnit = newUnit; repairingAUnit = true; }
    bool isFree() const noexcept { return !repairingAUnit; }
    int getNumBookings() const noexcept { return bookings; }  //number of harvesters goings there
    const UnitBase* getRepairUnit() const { return repairUnit.getUnitPointer(); }
    UnitBase* getRepairUnit() { return repairUnit.getUnitPointer(); }

protected:
    /**
        Used for updating things that are specific to that particular structure. Is called from
        StructureBase::update() before the check if this structure is still alive.
    */
    void updateStructureSpecificStuff() override;

private:
    void            init();

    bool            repairingAUnit; ///< Currently repairing?
    ObjectPointer   repairUnit;     ///< The unit to repair
    Uint32          bookings;       ///< Number of bookings for this repair yard
};

#endif // REPAIRYARD_H
