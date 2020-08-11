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

#ifndef TURRETBASE_H
#define TURRETBASE_H

#include <structures/StructureBase.h>

#include <FileClasses/SFXManager.h>

class TurretBaseConstants : public StructureBaseConstants {
public:
    constexpr explicit TurretBaseConstants(ItemID_enum itemID, BulletID_enum bullet_type)
        : StructureBaseConstants{itemID, Coord{1, 1}}, bulletType_{bullet_type} {
        canAttackStuff_ = true;
    }

    BulletID_enum bulletType() const noexcept { return bulletType_; }

private:
    // constant for all turrets of the same type
    BulletID_enum bulletType_; ///< The type of bullet used
};

class TurretBase : public StructureBase
{
    void init();

protected:
    explicit TurretBase(const TurretBaseConstants& constants, uint32_t objectID, const ObjectInitializer& initializer);
    explicit TurretBase(const TurretBaseConstants& constants, uint32_t objectID,
                        const ObjectStreamInitializer& initializer);

public:
    using parent = StructureBase;

    ~TurretBase() override;

    TurretBase(const TurretBase &) = delete;
    TurretBase(TurretBase &&) = delete;
    TurretBase& operator=(const TurretBase &) = delete;
    TurretBase& operator=(TurretBase &&) = delete;

    void save(OutputStream& stream) const override;

    virtual void handleActionCommand(const GameContext& context, int xPos, int yPos);

    /**
        Set targetObjectID as the attack target for this turret.
        \param  targetObjectID  the object to attack
    */
    virtual void doAttackObject(const GameContext& context, uint32_t targetObjectID);

    /**
        Set pObject as the attack target for this turret.
        \param  pObject  the object to attack
    */
    virtual void doAttackObject(const ObjectBase* pObject);


    void turnLeft(const GameContext& context);
    void turnRight(const GameContext& context);

    virtual void attack(const GameContext& context);

    int getTurretAngle() const { return lround(angle); }

protected:
    const TurretBaseConstants& turret_constants() const noexcept {
        return *static_cast<const TurretBaseConstants*>(&constants_);
    }

    /**
        Used for updating things that are specific to that particular structure. Is called from
        StructureBase::update() before the check if this structure is still alive.
    */
    void updateStructureSpecificStuff(const GameContext& context) override;

    // constant for all turrets of the same type
    Sound_enum attackSound;     ///< The id of the sound to play when attack

    // turret state
    int32_t  findTargetTimer;    ///< Timer used for finding a new target
    int32_t  weaponTimer;        ///< Time until we can shot again
};


template<>
inline TurretBase* dune_cast(ObjectBase* base) {
    return dynamic_cast<TurretBase*>(base);
}

template<>
inline const TurretBase* dune_cast(const ObjectBase* base) {
    return dynamic_cast<const TurretBase*>(base);
}

#endif //TURRETBASE_H
