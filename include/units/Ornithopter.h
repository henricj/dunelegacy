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

#ifndef ORNITHOPTER_H
#define ORNITHOPTER_H

#include <units/AirUnit.h>

class Ornithopter final : public AirUnit {
public:
    inline static constexpr ItemID_enum item_id = Unit_Ornithopter;
    using parent                                = AirUnit;

    Ornithopter(uint32_t objectID, const ObjectInitializer& initializer);
    Ornithopter(uint32_t objectID, const ObjectStreamInitializer& initializer);
    ~Ornithopter() override;

    void save(OutputStream& stream) const override;

    void checkPos(const GameContext& context) override;
    bool canAttack(const ObjectBase* object) const override;

    bool canPassTile(const Tile* pTile) const override;

    void destroy(const GameContext& context) override;

    void playAttackSound() override;

protected:
    FixPoint getDestinationAngle() const override;

    bool attack(const GameContext& context) override;

private:
    void init();

    uint32_t timeLastShot = 0;
};

#endif // ORNITHOPTER_H
