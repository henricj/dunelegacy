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

#include <Game.h>

class Carryall final : public AirUnit {
public:
    inline static constexpr ItemID_enum item_id = ItemID_enum::Unit_Carryall;
    using parent                                = AirUnit;

    Carryall(uint32_t objectID, const ObjectInitializer& initializer);
    Carryall(uint32_t objectID, const ObjectStreamInitializer& initializer);
    ~Carryall() override;

    Carryall(const Carryall&) = delete;
    Carryall(Carryall&&)      = delete;
    Carryall& operator=(const Carryall&) = delete;
    Carryall& operator=(Carryall&&) = delete;

    void checkPos(const GameContext& context) override;

    /**
        Updates this carryall.
        \return true if this object still exists, false if it was destroyed
    */
    bool update(const GameContext& context) override;

    void deploy(const GameContext& context, const Coord& newLocation) override;

    void destroy(const GameContext& context) override;

    void deployUnit(const GameContext& context, uint32_t unitID);

    void giveCargo(const GameContext& context, UnitBase* newUnit);

    void save(OutputStream& stream) const override;

    void setTarget(const ObjectBase* newTarget) override;

    bool hasCargo() const noexcept {
        return !pickedUpUnitList.empty();
    }

    void setOwned(bool b) noexcept { owned = b; }

    void setDropOfferer(bool status) {
        aDropOfferer = status;
    }

    bool isBooked() const noexcept { return (target || hasCargo()); }

private:
    void init();

    void releaseTarget() override;
    void engageTarget(const GameContext& context) override;
    void pickupTarget(const GameContext& context);
    void targeting(const GameContext& context) override;
    void turn(const GameContext& context) override;

    // unit state/properties
    std::vector<uint32_t> pickedUpUnitList; ///< What units does this carryall carry?

    bool owned = true; ///< Is this carryall owned or is it just here to drop something off

    bool aDropOfferer    = false; ///< This carryall just drops some units and vanishes afterwards
    bool droppedOffCargo = false; ///< Is the cargo already dropped off?

    template<typename F>
    void removeUnits(const GameContext& context, F&& predicate) {
        auto& units               = pickedUpUnitList;
        const auto& objectManager = context.objectManager;

        units.erase(std::remove_if(units.begin(), units.end(),
                                   [&](uint32_t unit_id) {
                                       auto* const unit = static_cast<UnitBase*>(objectManager.getObject(unit_id));

                                       return unit ? F(unit) : true;
                                   }),
                    units.end());
    }

    void pre_deployUnits(const GameContext& context);
    void deployUnit(const GameContext& context, Tile* tile, UnitBase* pUnit);
    void post_deployUnits();
};

#endif // CARRYALL_H
