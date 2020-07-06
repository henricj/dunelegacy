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
 *InputStream
 *  You should have received a copy of the GNU General Public License
 *  along with Dune Legacy.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef GROUNDUNIT_H
#define GROUNDUNIT_H

#include <units/UnitBase.h>


class GroundUnit : public UnitBase
{
protected:
    GroundUnit(ItemID_enum itemID, Uint32 objectID, const ObjectInitializer& initializer);
    GroundUnit(ItemID_enum itemID, Uint32 objectID, const ObjectStreamInitializer& initializer);

public:
    using parent = UnitBase;

    ~GroundUnit() override = 0;

    GroundUnit(const GroundUnit &) = delete;
    GroundUnit(GroundUnit &&) = delete;
    GroundUnit& operator=(const GroundUnit &) = delete;
    GroundUnit& operator=(GroundUnit &&) = delete;

    void save(OutputStream& stream) const override;

    void assignToMap(const Coord& pos) override;

    void playConfirmSound() override;
    void playSelectSound() override;

    void bookCarrier(UnitBase* newCarrier);
    void checkPos() override;

    void doRequestCarryallDrop(int x, int y);
    bool requestCarryall();
    void setPickedUp(UnitBase* newCarrier) override;

    /**
        This method is called when the user clicks on the repair button for this unit
    */
    virtual void handleSendToRepairClick();

    void doRepair() noexcept override;

    void setAwaitingPickup(bool status) { awaitingPickup = status; }
    bool isAwaitingPickup() const noexcept { return awaitingPickup; }
    bool hasBookedCarrier() const;
    const UnitBase* getCarrier() const;

    /**
        Returns how fast a unit can move over the specified terrain type.
        \param  terrainType the type to consider
        \return Returns a speed factor. Higher values mean slower.
    */
    FixPoint getTerrainDifficulty(TERRAINTYPE terrainType) const override;

protected:
    void move() override;
    void navigate() override;

    bool    awaitingPickup;     ///< Is this unit waiting for pickup?
    Uint32  bookedCarrier;      ///< What is the carrier if waiting for pickup?
private:
    void init();
};

template<>
inline GroundUnit* dune_cast(ObjectBase* base) {
    if(base && base->isAGroundUnit()) return static_cast<GroundUnit*>(base);

    return nullptr;
}

template<>
inline const GroundUnit* dune_cast(const ObjectBase* base) {
    if(base && base->isAGroundUnit()) return static_cast<const GroundUnit*>(base);

    return nullptr;
}


#endif // GROUNDUNIT_H
