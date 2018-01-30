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

class Carryall final : public AirUnit
{
public:
    explicit Carryall(House* newOwner);
    explicit Carryall(InputStream& stream);
    void init();
    virtual ~Carryall();

    Carryall(const Carryall &) = delete;
    Carryall(Carryall &&) = delete;
    Carryall& operator=(const Carryall &) = delete;
    Carryall& operator=(Carryall &&) = delete;

    void checkPos() override;

    /**
        Updates this carryall.
        \return true if this object still exists, false if it was destroyed
    */
    bool update() override;

    void deploy(const Coord& newLocation) override;

    void destroy() override;

    void deployUnit(Uint32 unitID);

    void giveCargo(UnitBase* newUnit);

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
    void releaseTarget() override;
    void engageTarget() override;
    void pickupTarget();
    void targeting() override;
    virtual void turn() override;

    // unit state/properties
    std::vector<Uint32>   pickedUpUnitList;   ///< What units does this carryall carry?

    bool     owned;              ///< Is this carryall owned or is it just here to drop something off

    bool     aDropOfferer;       ///< This carryall just drops some units and vanishes afterwards
    bool     droppedOffCargo;    ///< Is the cargo already dropped off?


    template<typename F>
    void removeUnits(F&& predicate) {
        auto& units = pickedUpUnitList;

        units.erase(std::remove_if(units.begin(), units.end(),
            [](Uint32 unit_id) {
                const auto unit = static_cast<UnitBase*>(currentGame->getObjectManager().getObject(unit_id));

                if (!unit)
                    return true;

                return F(unit);
            }),
            units.end());
    }

    void pre_deployUnits();
    void deployUnit(Tile* tile, UnitBase* pUnit);
    void post_deployUnits();

    template<typename F>
    void deployUnits(F&& predicate) {
        pre_deployUnits();

        const auto tile = currentGameMap->getTile(location);

        removeUnits([=](UnitBase* unit)
                    {
                        if (!F(unit))
                            return false;

                        deployUnit(tile, unit);

                        return true;
                    });

        post_deployUnits();
    }
};

#endif // CARRYALL_H
