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

class TrackedUnit : public GroundUnit {
protected:
    TrackedUnit(ItemID_enum itemID, Uint32 objectID, const ObjectInitializer& initializer);
    TrackedUnit(ItemID_enum itemID, Uint32 objectID, const ObjectStreamInitializer& initializer);

public:
    using parent = GroundUnit;

    ~TrackedUnit() override = 0;

    TrackedUnit(const TrackedUnit&) = delete;
    TrackedUnit(TrackedUnit&&)      = delete;
    TrackedUnit& operator=(const TrackedUnit&) = delete;
    TrackedUnit& operator=(TrackedUnit&&) = delete;

    void save(OutputStream& stream) const override;

    void checkPos() override;
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
    void init();

    static const FixPoint terrain_difficulty[Terrain_SpecialBloom + 1]; // TODO:: get better constant...
};

#endif // TRACKEDUNIT_H
