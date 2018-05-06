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

#ifndef REINFORCEMENTTRIGGER_H
#define REINFORCEMENTTRIGGER_H

#include <Trigger/Trigger.h>

#include <DataTypes.h>

#include <vector>

/**
    This trigger is used for delivering reinforcements at specific game cycles.
*/
class ReinforcementTrigger final : public Trigger {
public:

    /**
        Constructor
        \param  houseID             the owner of the new unit
        \param  itemID              the itemID of the new unit
        \param  location            the kind of drop
        \param  bRepeat             true = repeat the dropping (every triggerCycleNumber game cycles), false = do not repeat
        \param  triggerCycleNumber  the game cycle this trigger shall be triggered (must be >0 if bRepeat == true)
    */
    ReinforcementTrigger(int houseID, Uint32 itemID, DropLocation location, bool bRepeat, Uint32 triggerCycleNumber);
    ReinforcementTrigger(const ReinforcementTrigger &) = default;
    ReinforcementTrigger(ReinforcementTrigger &&) = default;

    ReinforcementTrigger& operator=(const ReinforcementTrigger &&);
    ReinforcementTrigger& operator=(ReinforcementTrigger &&);

    /**
        This constructor constructs the trigger from a stream.
        \param  stream  the stream to read from
    */
    explicit ReinforcementTrigger(InputStream& stream);

    /// destructor
    ~ReinforcementTrigger();

    /**
        This method saves this trigger to a stream.
        \param  stream  the stream to save to
    */
    void save(OutputStream& stream) const override;

    /**
        Get the house of the unit to be dropped.
        \return the house of the owner
    */
    int getHouseID() const { return houseID; }

    /**
        Get the type of drop.
        \return the type of the drop
    */
    DropLocation getDropLocation() const { return dropLocation; }

    /**
        Return if this Reinforcment is repeated.
        \return true = is repeated, false = not repeated
    */
    bool isRepeat() const { return (repeatCycle != 0); }

    /**
        Get a vector of all the itemIDs of the to be dropped units (that will be dropped by one carryall)
        \return a vector of the itemIDs of the to be dropped units.
    */
    const std::vector<Uint32>& getDroppedUnits() const { return droppedUnits; }

    /**
        Adds another unit to this delivery.
        \param  itemID  the itemID of the unit to add
    */
    void addUnit(Uint32 itemID) { droppedUnits.push_back(itemID); }

    /**
        Trigger this trigger. Shall only be called when getCycleNumber() is equal to the current game cycle
    */
    void trigger() override;

private:
    std::vector<Uint32> droppedUnits;   ///< a vector of the itemIDs of the to be dropped units
    DropLocation        dropLocation;   ///< the kind of drop
    int                 houseID;        ///< the owner of the new unit
    Uint32              repeatCycle;    ///< the interval in game cycles between two drops. Will be equal to triggerCycleNumber at the beginning of the game. repeatCycle = 0 if there is no repeat
};

#endif // REINFORCEMENTTRIGGER_H
