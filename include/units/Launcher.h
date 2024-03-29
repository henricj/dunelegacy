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

#ifndef LAUNCHER_H
#define LAUNCHER_H

#include <units/TrackedUnit.h>

class Launcher final : public TrackedUnit {
public:
    inline static constexpr ItemID_enum item_id = ItemID_enum::Unit_Launcher;
    using parent                                = TrackedUnit;

    Launcher(uint32_t objectID, const ObjectInitializer& initializer);
    Launcher(uint32_t objectID, const ObjectStreamInitializer& initializer);
    ~Launcher() override;

    void blitToScreen() override;
    void destroy(const GameContext& context) override;
    bool canAttack(const ObjectBase* object) const override;

    void playAttackSound() override;

private:
    void init();

    // drawing information
    zoomable_texture turretGraphic{}; ///< The turret graphic
    ObjPic_enum gunGraphicID;         ///< The id of the turret graphic (needed if we want to reload the graphic)
};

#endif // LAUNCHER_H
