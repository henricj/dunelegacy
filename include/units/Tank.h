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

#ifndef TANK_H
#define TANK_H

#include <units/TankBase.h>

class Tank final : public TankBase {
public:
    inline static constexpr ItemID_enum item_id = Unit_Tank;
    using parent                                = TankBase;

    Tank(uint32_t objectID, const ObjectInitializer& initializer);
    Tank(uint32_t objectID, const ObjectStreamInitializer& initializer);
    ~Tank() override;

    void blitToScreen() override;

    void destroy(const GameContext& context) override;

    void playAttackSound() override;

private:
    void init();
};

#endif // TANK_H
