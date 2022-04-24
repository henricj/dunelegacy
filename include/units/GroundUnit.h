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

class GroundUnitConstants : public UnitBaseConstants {
public:
    constexpr explicit GroundUnitConstants(ItemID_enum itemID, int num_weapons = 0,
                                           BulletID_enum bullet_id = BulletID_enum::Bullet_Rocket)
        : UnitBaseConstants{itemID, num_weapons, bullet_id} {
        aGroundUnit_ = true;
    }
};

class GroundUnit : public UnitBase {
protected:
    GroundUnit(const GroundUnitConstants& constants, uint32_t objectID, const ObjectInitializer& initializer);
    GroundUnit(const GroundUnitConstants& constants, uint32_t objectID, const ObjectStreamInitializer& initializer);

public:
    using parent = UnitBase;

    ~GroundUnit() override = 0;

    GroundUnit(const GroundUnit&)            = delete;
    GroundUnit(GroundUnit&&)                 = delete;
    GroundUnit& operator=(const GroundUnit&) = delete;
    GroundUnit& operator=(GroundUnit&&)      = delete;

    void save(OutputStream& stream) const override;

    void assignToMap(const GameContext& context, const Coord& pos) override;

    void playConfirmSound() override;
    void playSelectSound() override;

    void bookCarrier(UnitBase* newCarrier);
    void checkPos(const GameContext& context) override;

    void doRequestCarryallDrop(const GameContext& context, int x, int y);
    bool requestCarryall(const GameContext& context);
    void setPickedUp(const GameContext& context, UnitBase* newCarrier) override;
    bool isPickedUp() const { return pickedUp; }

    /**
        This method is called when the user clicks on the repair button for this unit
    */
    virtual void handleSendToRepairClick();

    void doRepair(const GameContext& context) noexcept override;

    void setAwaitingPickup(bool status) { awaitingPickup = status; }
    bool isAwaitingPickup() const noexcept { return awaitingPickup; }
    bool hasBookedCarrier();
    const UnitBase* getCarrier() const;

    /**
        Returns how fast a unit can move over the specified terrain type.
        \param  terrainType the type to consider
        \return Returns a speed factor. Higher values mean slower.
    */
    FixPoint getTerrainDifficulty(TERRAINTYPE terrainType) const override;

protected:
    void move(const GameContext& context) override;
    void navigate(const GameContext& context) override;

    bool awaitingPickup    = false;   ///< Is this unit waiting for pickup?
    uint32_t bookedCarrier = NONE_ID; ///< What is the carrier if waiting for pickup?
};

template<>
inline GroundUnit* dune_cast(ObjectBase* base) {
    if (base && base->isAGroundUnit())
        return static_cast<GroundUnit*>(base);

    return nullptr;
}

template<>
inline const GroundUnit* dune_cast(const ObjectBase* base) {
    if (base && base->isAGroundUnit())
        return static_cast<const GroundUnit*>(base);

    return nullptr;
}

#endif // GROUNDUNIT_H
