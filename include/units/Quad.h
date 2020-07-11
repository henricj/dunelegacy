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

#ifndef QUAD_H
#define QUAD_H

#include <units/GroundUnit.h>

class Quad final : public GroundUnit
{
public:
    inline static constexpr ItemID_enum item_id = Unit_Quad;
    using parent = GroundUnit;

    Quad(ItemID_enum itemID, Uint32 objectID, const ObjectInitializer& initializer);
    Quad(ItemID_enum itemID, Uint32 objectID, const ObjectStreamInitializer& initializer);
    ~Quad() override;

    void playAttackSound() override;

    void destroy(const GameContext& context) override;

    bool hasBumpyMovementOnRock() const override { return true; }

private:
    void init();
};

#endif //QUAD_H
