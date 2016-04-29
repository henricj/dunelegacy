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

    virtual void save(OutputStream& stream) const;

    void checkPos();
    bool canPass(int xPos, int yPos) const;

    /**
        Returns how fast a unit can move over the specified terrain type.
        \param  terrainType the type to consider
        \return Returns a speed factor. Higher values mean slower.
    */
    virtual FixPoint getTerrainDifficulty(TERRAINTYPE terrainType) const {
        switch(terrainType) {
            case Terrain_Slab:          return FixPt(1,0);
            case Terrain_Sand:          return FixPt(1,5625);
            case Terrain_Rock:          return FixPt(1,375);
            case Terrain_Dunes:         return FixPt(1,375);
            case Terrain_Mountain:      return FixPt(1,0);
            case Terrain_Spice:         return FixPt(1,375);
            case Terrain_ThickSpice:    return FixPt(1,375);
            case Terrain_SpiceBloom:    return FixPt(1,5625);
            case Terrain_SpecialBloom:  return FixPt(1,5625);
            default:                    return FixPt(1,0);
        }
    }
};

#endif // TRACKEDUNIT_H
