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

#ifndef REFINERY_H
#define REFINERY_H

#include <ObjectPointer.h>
#include <structures/StructureBase.h>

// forward declarations
class Harvester;
class Carryall;

class Refinery final : public StructureBase {
public:
    inline static constexpr ItemID_enum item_id = Structure_Refinery;
    using parent                                = StructureBase;

    Refinery(uint32_t objectID, const ObjectInitializer& initializer);
    Refinery(uint32_t objectID, const ObjectStreamInitializer& initializer);
    ~Refinery() override;

    void cleanup(const GameContext& context, HumanPlayer* humanPlayer) override;
    void save(OutputStream& stream) const override;

    std::unique_ptr<ObjectInterface> getInterfaceContainer(const GameContext& context) override;

    void assignHarvester(Harvester* newHarvester);
    void deployHarvester(const GameContext& context, Carryall* pCarryall = nullptr);
    void startAnimate();
    void stopAnimate();

    void book() {
        bookings++;
        startAnimate();
    }

    void unBook() {
        bookings--;
        if (bookings == 0) {
            stopAnimate();
        }
    }

    bool isFree() const noexcept { return !extractingSpice; }
    int getNumBookings() const noexcept { return bookings; } // number of units goings there
    const Harvester* getHarvester() const { return reinterpret_cast<Harvester*>(harvester.getObjPointer()); }
    Harvester* getHarvester() { return reinterpret_cast<Harvester*>(harvester.getObjPointer()); }

protected:
    /**
        Used for updating things that are specific to that particular structure. Is called from
        StructureBase::update() before the check if this structure is still alive.
    */
    void updateStructureSpecificStuff(const GameContext& context) override;

private:
    void init();

    bool extractingSpice;    ///< Currently extracting spice?
    ObjectPointer harvester; ///< The harvester currently in the refinery
    uint32_t bookings;       ///< How many bookings?

    bool firstRun; ///< On first deploy of a harvester we tell it to the user
};

#endif // REFINERY_H
