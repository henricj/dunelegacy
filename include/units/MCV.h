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

#ifndef MCV_H
#define MCV_H

#include <units/GroundUnit.h>

class MCV final : public GroundUnit
{
public:
    inline static constexpr ItemID_enum item_id = Unit_MCV;
    using parent = GroundUnit;

    MCV(Uint32 objectID, const ObjectInitializer& initializer);
    MCV(Uint32 objectID, const ObjectStreamInitializer& initializer);
    ~MCV() override;

    void handleDeployClick();

    /**
        Deploy this MCV. If deploying was successful this unit does not exist anymore.
        \return true, if deploying was successful, false otherwise.
    */
    bool doDeploy();

    bool canAttack(const ObjectBase* object) const override;

    void destroy(const GameContext& context) override;

    bool canDeploy() const {
        return canDeploy(getLocation().x, getLocation().y);
    }

private:
    void init();

    static bool canDeploy(int x, int y);
};

#endif // MCV_H
