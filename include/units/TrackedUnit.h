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

class TrackedUnit : public GroundUnit
{
public:

    explicit TrackedUnit(House* newOwner);
    explicit TrackedUnit(InputStream& stream);
    void init();
    virtual ~TrackedUnit();

    TrackedUnit(const TrackedUnit &) = delete;
    TrackedUnit(TrackedUnit &&) = delete;
    TrackedUnit& operator=(const TrackedUnit &) = delete;
    TrackedUnit& operator=(TrackedUnit &&) = delete;

    void save(OutputStream& stream) const override;

    void checkPos() override;
    bool canPass(int xPos, int yPos) const override;

    /**
        Returns how fast a unit can move over the specified terrain type.
        \param  terrainType the type to consider
        \return Returns a speed factor. Higher values mean slower.
    */
    FixPoint getTerrainDifficulty(TERRAINTYPE terrainType) const override
    {
        switch(terrainType) {
            case Terrain_Slab:          return 1.0_fix;
            case Terrain_Sand:          return 1.5625_fix;
            case Terrain_Rock:          return 1.375_fix;
            case Terrain_Dunes:         return 1.375_fix;
            case Terrain_Mountain:      return 1.0_fix;
            case Terrain_Spice:         return 1.375_fix;
            case Terrain_ThickSpice:    return 1.375_fix;
            case Terrain_SpiceBloom:    return 1.5625_fix;
            case Terrain_SpecialBloom:  return 1.5625_fix;
            default:                    return 1.0_fix;
        }
    }
};

#endif // TRACKEDUNIT_H
