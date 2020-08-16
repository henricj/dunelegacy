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

#ifndef ENGINE_REPAIRYARD_H
#define ENGINE_REPAIRYARD_H

#include <structures/StructureBase.h>
#include <ObjectPointer.h>

namespace Dune::Engine {

class Carryall;

class RepairYard final : public StructureBase {
public:
    inline static constexpr ItemID_enum item_id = Structure_RepairYard;
    using parent                                = StructureBase;

    RepairYard(uint32_t objectID, const ObjectInitializer& initializer);
    RepairYard(uint32_t objectID, const ObjectStreamInitializer& initializer);
    ~RepairYard() override;

    void cleanup(const GameContext& context, HumanPlayer* humanPlayer) override;
    void save(const Game& game, OutputStream& stream) const override;

    void deployRepairUnit(const GameContext& context, Carryall* pCarryall = nullptr);

    void book() { bookings++; }
    void unBook() { bookings--; }
    void assignUnit(ObjectPointer newUnit) {
        repairUnit     = newUnit;
        repairingAUnit = true;
    }
    bool            isFree() const noexcept { return !repairingAUnit; }
    int             getNumBookings() const noexcept { return bookings; } // number of harvesters goings there
    const UnitBase* getRepairUnit(const ObjectManager& objectManager) const { return repairUnit.getUnitPointer(objectManager); }
    UnitBase* getRepairUnit(const ObjectManager& objectManager) { return repairUnit.getUnitPointer(objectManager); }

protected:
    /**
        Used for updating things that are specific to that particular structure. Is called from
        StructureBase::update() before the check if this structure is still alive.
    */
    void updateStructureSpecificStuff(const GameContext& context) override;

private:
    void init();

    bool          repairingAUnit; ///< Currently repairing?
    ObjectPointer repairUnit;     ///< The unit to repair
    uint32_t      bookings;       ///< Number of bookings for this repair yard
};

} // namespace Dune::Engine

#endif // ENGINE_REPAIRYARD_H
