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

class TankBase : public TrackedUnit
{
public:
    explicit TankBase(House* newOwner);
    explicit TankBase(InputStream& stream);
    void init();
    virtual ~TankBase();

    TankBase(const TankBase &) = delete;
    TankBase(TankBase &&) = delete;
    TankBase& operator=(const TankBase &) = delete;
    TankBase& operator=(TankBase &&) = delete;

    void save(OutputStream& stream) const override;

    void navigate() override;

    inline int getTurretAngle() const { return lround(turretAngle); }

    void setTurretAngle(int newAngle);

    int getCurrentAttackAngle() const override;

protected:
    void engageTarget() override;
    void targeting() override;

    /**
        When the unit is currently idling this method is called about every 5 seconds.
    */
    void idleAction() override;

    void turn() override;
    void turnTurretLeft();
    void turnTurretRight();

    // constant for all tanks of the same type
    FixPoint turretTurnSpeed = 0.0625_fix;        ///< How fast can we turn the turret

    // tank state
    FixPoint turretAngle;            ///< The angle of the turret
    Sint8    drawnTurretAngle;       ///< The drawn angle of the turret

    ObjectPointer   closeTarget;     ///< a enemy target that can be shot at while moving

    // drawing information
    zoomable_texture turretGraphic{};   ///< The turret graphic
    int              gunGraphicID = -1; ///< The id of the turret graphic (needed if we want to reload the graphic)
};

#endif // TANKBASE_H
