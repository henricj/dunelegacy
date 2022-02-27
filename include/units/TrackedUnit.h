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

#ifndef TRACKEDUNIT_H
#define TRACKEDUNIT_H

#include <units/GroundUnit.h>

class TrackedUnitConstants : public GroundUnitConstants {
public:
    constexpr explicit TrackedUnitConstants(ItemID_enum itemID, int num_weapons = 0,
                                            BulletID_enum bullet_id = BulletID_enum::Bullet_Rocket)
        : GroundUnitConstants{itemID, num_weapons, bullet_id} {
        tracked_ = true;
    }
};

class TrackedUnit : public GroundUnit {
protected:
    TrackedUnit(const TrackedUnitConstants& constants, uint32_t objectID, const ObjectInitializer& initializer);
    TrackedUnit(const TrackedUnitConstants& constants, uint32_t objectID, const ObjectStreamInitializer& initializer);

public:
    using parent = GroundUnit;

    ~TrackedUnit() override = 0;

    TrackedUnit(const TrackedUnit&) = delete;
    TrackedUnit(TrackedUnit&&)      = delete;
    TrackedUnit& operator=(const TrackedUnit&) = delete;
    TrackedUnit& operator=(TrackedUnit&&) = delete;

    void save(OutputStream& stream) const override;

    void checkPos(const GameContext& context) override;
    bool canPassTile(const Tile* pTile) const override;

    /**
        Returns how fast a unit can move over the specified terrain type.
        \param  terrainType the type to consider
        \return Returns a speed factor. Higher values mean slower.
    */
    FixPoint getTerrainDifficulty(TERRAINTYPE terrainType) const override {
        if(terrainType < 0 || terrainType > Terrain_SpecialBloom) return 1_fix;

        return terrain_difficulty[terrainType];
    }

private:
    static const std::array<FixPoint, Terrain_SpecialBloom + 1> terrain_difficulty; // TODO:: get better constant...
};

#endif // TRACKEDUNIT_H
