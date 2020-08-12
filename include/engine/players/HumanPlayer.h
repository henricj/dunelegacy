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


inline constexpr int xyz10 = 123;
namespace {
inline constexpr int abc10 = ::xyz10;
}

#include "Player.h"

inline constexpr int xyz10b = 123;
namespace {
inline constexpr int abc10b = ::xyz10b;
}

#include <EngineDefinitions.h>

inline constexpr int xyz10c = 123;
namespace {
inline constexpr int abc10c = ::xyz10c;
}

namespace Dune::Engine {
// forward declarations
class UnitBase;
class StructureBase;
class ObjectBase;

class HumanPlayer : public Player {
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
    virtual void onDamage(const ::Dune::Engine::ObjectBase* pObject, int damage, uint32_t damagerID) override;

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

public:
    uint32_t nextExpectedCommandsCycle{}; ///< The next cycle we expect commands for (using for network games)

private:
    void triggerStructureTutorialHint(ItemID_enum itemID);

    [[nodiscard]] bool hasConcreteOfSize(const Coord& concreteSize) const;
    [[nodiscard]] bool hasConcreteAtPositionOfSize(const Coord& pos, const Coord& concreteSize) const;
};

} // namespace Dune::Engine

#endif // HUMANPLAYER_H
