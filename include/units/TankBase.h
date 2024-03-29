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

#ifndef TANKBASE_H
#define TANKBASE_H

#include <units/TrackedUnit.h>

class TankBaseConstants : public TrackedUnitConstants {
public:
    constexpr explicit TankBaseConstants(ItemID_enum itemID, int num_weapons = 0,
                                         BulletID_enum bullet_id = BulletID_enum::Bullet_Rocket)
        : TrackedUnitConstants{itemID, num_weapons, bullet_id} {
        turreted_ = true;
    }
};

class TankBase : public TrackedUnit {
protected:
    TankBase(const TankBaseConstants& constants, uint32_t objectID, const ObjectInitializer& initializer);
    TankBase(const TankBaseConstants& constants, uint32_t objectID, const ObjectStreamInitializer& initializer);

public:
    using parent = TrackedUnit;

    ~TankBase() override = 0;

    TankBase(const TankBase&)            = delete;
    TankBase(TankBase&&)                 = delete;
    TankBase& operator=(const TankBase&) = delete;
    TankBase& operator=(TankBase&&)      = delete;

    void save(OutputStream& stream) const override;

    void navigate(const GameContext& context) override;

    ANGLETYPE getTurretAngle() const { return static_cast<ANGLETYPE>(lround(turretAngle)); }

    void setTurretAngle(ANGLETYPE newAngle);

    ANGLETYPE getCurrentAttackAngle() const override;

protected:
    void engageTarget(const GameContext& context) override;
    void targeting(const GameContext& context) override;

    /**
        When the unit is currently idling this method is called about every 5 seconds.
    */
    void idleAction(const GameContext& context) override;

    void turn(const GameContext& context) override;
    void turnTurretLeft();
    void turnTurretRight();

    // constant for all tanks of the same type
    FixPoint turretTurnSpeed = 0.0625_fix; ///< How fast can we turn the turret

    // tank state
    FixPoint turretAngle;       ///< The angle of the turret
    ANGLETYPE drawnTurretAngle; ///< The drawn angle of the turret

    ObjectPointer closeTarget; ///< a enemy target that can be shot at while moving

    // drawing information
    zoomable_texture turretGraphic{}; ///< The turret graphic
    ObjPic_enum gunGraphicID =
        ObjPic_enum(-1); ///< The id of the turret graphic (needed if we want to reload the graphic)
};

template<>
inline TankBase* dune_cast(ObjectBase* base) {
    auto* unit = dune_cast<UnitBase>(base);

    if (unit && unit->isTurreted())
        return static_cast<TankBase*>(unit);

    return nullptr;
}

template<>
inline const TankBase* dune_cast(const ObjectBase* base) {
    const auto* unit = dune_cast<UnitBase>(base);

    if (unit && unit->isTurreted())
        return static_cast<const TankBase*>(unit);

    return nullptr;
}

#endif // TANKBASE_H
