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

#ifndef ENGINE_MCV_H
#define ENGINE_MCV_H

#include <units/GroundUnit.h>

namespace Dune::Engine {

class MCV final : public GroundUnit {
public:
    inline static constexpr ItemID_enum item_id = Unit_MCV;
    using parent                                = GroundUnit;

    MCV(uint32_t objectID, const ObjectInitializer& initializer);
    MCV(uint32_t objectID, const ObjectStreamInitializer& initializer);
    ~MCV() override;

    /**
        Deploy this MCV. If deploying was successful this unit does not exist anymore.
        \return true, if deploying was successful, false otherwise.
    */
    bool doDeploy(const GameContext& context);

    bool canAttack(const GameContext& context, const ObjectBase* object) const override;

    void destroy(const GameContext& context) override;

    bool canDeploy(const GameContext& context) const { return canDeploy(context, getLocation().x, getLocation().y); }
private:
    void init();

    bool canDeploy(const GameContext& context, int x, int y) const;
};

} // namespace Dune::Engine

#endif // ENGINE_MCV_H
