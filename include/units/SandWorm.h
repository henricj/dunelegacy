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

#ifndef SANDWORM_H
#define SANDWORM_H

#define SANDWORM_SEGMENTS 4 //how many shimmer things will be drawn per worm

#include <units/GroundUnit.h>

class Sandworm final : public GroundUnit
{
public:
    explicit Sandworm(House* newOwner);
    explicit Sandworm(InputStream& stream);
    void init();
    virtual ~Sandworm();

    void save(OutputStream& stream) const override;

    void deploy(const Coord& newLocation) override;

    void assignToMap(const Coord& pos) override;
    bool attack() override;
    void blitToScreen() override;
    void checkPos() override;

    inline void setLocation(const Coord& location) { setLocation(location.x, location.y); }
    void setLocation(int xPos, int yPos) override;

    void setTarget(const ObjectBase* newTarget) override;

    void handleDamage(int damage, Uint32 damagerID, House* damagerOwner) override;

    /**
        Updates this sandworm.
        \return true if this object still exists, false if it was destroyed
    */
    bool update() override;

    bool canAttack(const ObjectBase* object) const override;
    bool canPass(int xPos, int yPos) const override;
    inline int getSleepTimer() const { return sleepTimer; }

    int getCurrentAttackAngle() const override;

    void playAttackSound() override;

    FixPoint getTerrainDifficulty(TERRAINTYPE terrainType) const override
    {
        switch(terrainType) {
            case Terrain_Slab:          return 1.0_fix;
            case Terrain_Sand:          return 1.25_fix;
            case Terrain_Rock:          return 1.0_fix;
            case Terrain_Dunes:         return 1.25_fix;
            case Terrain_Mountain:      return 1.0_fix;
            case Terrain_Spice:         return 1.25_fix;
            case Terrain_ThickSpice:    return 1.25_fix;
            case Terrain_SpiceBloom:    return 1.25_fix;
            case Terrain_SpecialBloom:  return 1.25_fix;
            default:                    return 1.0_fix;
        }
    }

    bool isEating() const { return (drawnFrame != INVALID); }

protected:
    const ObjectBase* findTarget() const override;
    void engageTarget() override;
    void sleep();
    bool sleepOrDie();

private:
    // sandworm state
    Sint32      kills;                      ///< How many units does this sandworm alreay killed?
    Sint32      attackFrameTimer;           ///< When to show the next attack frame
    Sint32      sleepTimer;                 ///< How long has this sandworm slept?
    Uint8       warningWormSignPlayedFlags; ///< Was the "Worm Sign" warning played? If yes, the corresponding flag is the for the local house

    // drawing information
    Sint32 shimmerOffsetIndex;
    Coord lastLocs[SANDWORM_SEGMENTS];    ///< Last locations of the sandworm
};

#endif // SANDWORM_H
