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
    inline static constexpr ItemID_enum item_id = Unit_Sandworm;
    using parent = GroundUnit;

    Sandworm(ItemID_enum itemID, Uint32 objectID, const ObjectInitializer& initializer);
    Sandworm(ItemID_enum itemID, Uint32 objectID, const ObjectStreamInitializer& initializer);
    ~Sandworm() override;

    void save(OutputStream& stream) const override;

    void deploy(const GameContext& context, const Coord& newLocation) override;

    void assignToMap(const GameContext& context, const Coord& pos) override;
    bool attack(const GameContext& context) override;
    void blitToScreen() override;
    void checkPos(const GameContext& context) override;

    void setLocation(const GameContext& context, int xPos, int yPos) override;

    void setTarget(const ObjectBase* newTarget) override;

    void handleDamage(const GameContext& context, int damage, Uint32 damagerID, House* damagerOwner) override;

    /**
        Updates this sandworm.
        \return true if this object still exists, false if it was destroyed
    */
    bool update(const GameContext& context) override;

    bool canAttack(const ObjectBase* object) const override;
    bool canPassTile(const Tile* pTile) const override;
    int getSleepTimer() const noexcept { return sleepTimer; }

    ANGLETYPE getCurrentAttackAngle() const override;

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

    bool isEating() const noexcept { return (drawnFrame != INVALID); }

protected:
    const ObjectBase* findTarget() const override;
    void engageTarget(const GameContext& context) override;
    void sleep(const GameContext& context);
    bool sleepOrDie(const GameContext& context);

private:
    void init();

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
