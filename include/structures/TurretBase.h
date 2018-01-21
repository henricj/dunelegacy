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

class TurretBase : public StructureBase
{
public:
    explicit TurretBase(House* newOwner);
    explicit TurretBase(InputStream& stream);
    void init();
    virtual ~TurretBase();

    TurretBase(const TurretBase &) = delete;
    TurretBase(TurretBase &&) = delete;
    TurretBase& operator=(const TurretBase &) = delete;
    TurretBase& operator=(TurretBase &&) = delete;

    void save(OutputStream& stream) const override;

    virtual void handleActionCommand(int xPos, int yPos);

    /**
        Set targetObjectID as the attack target for this turret.
        \param  targetObjectID  the object to attack
    */
    virtual void doAttackObject(Uint32 targetObjectID);

    /**
        Set pObject as the attack target for this turret.
        \param  pObject  the object to attack
    */
    virtual void doAttackObject(const ObjectBase* pObject);

    void turnLeft();
    void turnRight();
    virtual void attack();

    inline int getTurretAngle() const { return lround(angle); }

protected:
    /**
        Used for updating things that are specific to that particular structure. Is called from
        StructureBase::update() before the check if this structure is still alive.
    */
    void updateStructureSpecificStuff() override;

    // constant for all turrets of the same type
    int bulletType;             ///< The type of bullet used
    Sound_enum attackSound;     ///< The id of the sound to play when attack

    // turret state
    Sint32  findTargetTimer;    ///< Timer used for finding a new target
    Sint32  weaponTimer;        ///< Time until we can shot again
};

#endif //TURRETBASE_H
