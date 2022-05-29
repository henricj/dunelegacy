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

#include <players/Player.h>

#include <Definitions.h>

#include <misc/SDL2pp.h>

// forward declarations
class UnitBase;
class StructureBase;
class ObjectBase;

class HumanPlayer final : public Player {
public:
    /// This enum has the same int values as ItemID_enum for structures (exceptions are the first and the last entry)
    enum class TutorialHint {
        HarvestSpice            = 0,
        ThisIsABarrack          = 1,
        ThisIsAConstructionYard = 2,
        ThisIsAGunTurret        = 3,
        ThisIsAHeavyFactory     = 4,
        ThisIsAHighTechFactory  = 5,
        ThisIsHouseIX           = 6,
        ThisIsALightFactory     = 7,
        ThisIsAPalace           = 8,
        ThisIsARadar            = 9,
        ThisIsARefinery         = 10,
        ThisIsARepairYard       = 11,
        ThisIsARocketTurret     = 12,
        ThisIsASilo             = 13,
        ThisIsASlab1            = 14,
        ThisIsASlab4            = 15,
        ThisIsAStarPort         = 16,
        ThisIsAWall             = 17,
        ThisIsAWindTrap         = 18,
        ThisIsAWOR              = 19,
        NotEnoughConrete        = 20,
    };

    HumanPlayer(const GameContext& context, House* associatedHouse, const std::string& playername,
                const Random& random);
    HumanPlayer(const GameContext& context, InputStream& stream, House* associatedHouse);
    ~HumanPlayer() override;
    void save(OutputStream& stream) const override;

    void update() override;

    /**
        An object was hit by something or damaged somehow else.
        \param  pObject     the object that was damaged
        \param  damage      the damage taken
        \param  damagerID   the shooter of the bullet, rocket, etc. if known; NONE_ID otherwise
    */
    void onDamage(const ObjectBase* pObject, int damage, uint32_t damagerID) override;

    [[nodiscard]] uint32_t getAlreadyShownTutorialHints() const { return alreadyShownTutorialHints; }

    /**
        The player just started to produce an item.
        \param  itemID  the item that is produced
    */
    virtual void onProduceItem(ItemID_enum itemID);

    /**
        The player just placed a structure.
        \param  pStructure  the structure that was placed (nullptr for Slab)
    */
    virtual void onPlaceStructure(const StructureBase* pStructure);

    /**
        A unit of this player was deployed.
        \param  pUnit  the unit that was deployed
    */
    virtual void onUnitDeployed(const UnitBase* pUnit);

    /**
        The set of selected units or structures has changed.
        \param  selectedObjectIDs   the new set of selected objects
    */
    virtual void onSelectionChanged(const Dune::selected_set_type& selectedObjectIDs);

    /**
        Returns one of the 9 saved units lists
        \param  groupListIndex   which list should be returned
        \return the n-th list.
    */
    [[nodiscard]] Dune::selected_set_type& getGroupList(int groupListIndex) { return selectedLists[groupListIndex]; }

    /**
        Sets one of the 9 saved units lists
        \param  groupListIndex     which list should be set
        \param  newGroupList        the new list to set
    */
    void setGroupList(int groupListIndex, const Dune::selected_set_type& newGroupList);

    uint32_t nextExpectedCommandsCycle{}; ///< The next cycle we expect commands for (using for network games)

    Dune::selected_set_type selectedLists[NUMSELECTEDLISTS]; ///< Sets of all the different groups on key 1 to 9

    uint32_t alreadyShownTutorialHints; ///< Contains flags for each tutorial hint (see enum TutorialHint)

private:
    void triggerStructureTutorialHint(ItemID_enum itemID);

    [[nodiscard]] bool hasConcreteOfSize(const Coord& concreteSize) const;
    [[nodiscard]] bool hasConcreteAtPositionOfSize(const Coord& pos, const Coord& concreteSize) const;

    uint32_t lastAttackNotificationCycle; ///< When was the last time that the player was informed about an attack
};

#endif // HUMANPLAYER_H
