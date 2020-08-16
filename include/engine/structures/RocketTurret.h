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

#ifndef ENGINE_ROCKETTURRET_H
#define ENGINE_ROCKETTURRET_H

#include <structures/TurretBase.h>

namespace Dune::Engine {

class RocketTurret final : public TurretBase {
public:
    inline static constexpr ItemID_enum item_id = Structure_RocketTurret;
    using parent                                = TurretBase;

    RocketTurret(uint32_t objectID, const ObjectInitializer& initializer);
    RocketTurret(uint32_t objectID, const ObjectStreamInitializer& initializer);
    ~RocketTurret() override;

    bool canAttack(const GameContext& context, const ObjectBase* object) const override;

    void attack(const GameContext& context) override;

protected:
    /**
        Used for updating things that are specific to that particular structure. Is called from
        StructureBase::update() before the check if this structure is still alive.
    */
    void updateStructureSpecificStuff(const GameContext& context) override;

private:
    void init();
};

} // namespace Dune::Engine

#endif // ENGINE_ROCKETTURRET_H
