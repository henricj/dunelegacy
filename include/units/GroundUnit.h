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
 *InputStream
 *  You should have received a copy of the GNU General Public License
 *  along with Dune Legacy.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef GROUNDUNIT_H
#define GROUNDUNIT_H

#include <units/UnitBase.h>


class GroundUnit : public UnitBase
{

public:

	GroundUnit(House* newOwner);
	GroundUnit(InputStream& stream);
	void init();
	virtual ~GroundUnit();

	virtual void save(OutputStream& stream) const;

	virtual void assignToMap(const Coord& pos);

	void playConfirmSound();
	void playSelectSound();

	void bookCarrier(UnitBase* newCarrier);
	virtual void checkPos();

    void doRequestCarryallDrop(int x, int y);
	bool requestCarryall();
	void setPickedUp(UnitBase* newCarrier);



	virtual void doRepair();

	inline void setawaitingPickup(bool status) { awaitingPickup = status; }
	inline bool isawaitingPickup() const { return awaitingPickup; }
	bool hasBookedCarrier() const;
	const UnitBase* getCarrier() const;

	/**
        Returns how fast a unit can move over the specified terrain type.
        \param  terrainType the type to consider
        \return Returns a speed factor. Higher values mean slower.
	*/
	virtual float getTerrainDifficulty(TERRAINTYPE terrainType) const {
	    switch(terrainType) {
            case Terrain_Slab:          return 1.0f;
            case Terrain_Sand:          return 1.375f;
            case Terrain_Rock:          return 1.5625f;
            case Terrain_Dunes:         return 1.375f;
            case Terrain_Mountain:      return 1.0f;
            case Terrain_Spice:         return 1.375f;
            case Terrain_ThickSpice:    return 1.375f;
            case Terrain_SpiceBloom:    return 1.375f;
            case Terrain_SpecialBloom:  return 1.375f;
            default:                    return 1.0f;
	    }
    }

protected:

	void	navigate();

	bool	awaitingPickup;     ///< Is this unit waiting for pickup?
	Uint32  bookedCarrier;      ///< What is the carrier if waiting for pickup?
};

#endif // GROUNDUNIT_H
