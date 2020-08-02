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

#ifndef PALACE_H
#define PALACE_H

#include <structures/StructureBase.h>

class Palace final : public StructureBase
{
public:
    inline static constexpr ItemID_enum item_id = Structure_Palace;
    using parent = StructureBase;

    Palace(Uint32 objectID, const ObjectInitializer& initializer);
    Palace(Uint32 objectID, const ObjectStreamInitializer& initializer);
    ~Palace() override;

    void save(OutputStream& stream) const override;

    std::unique_ptr<ObjectInterface> getInterfaceContainer(const GameContext& context) override;

    void handleSpecialClick(const GameContext& context);

    void handleDeathhandClick(const GameContext& context, int xPos, int yPos);

    /**
        Activate the special palace weapon Fremen or Saboteur. For the Deathhand see doLaunchDeathhand.
    */
    void doSpecialWeapon(const GameContext& context);

    /**
        Launch the deathhand missile an target position x,y.
        \param  x   x coordinate (in tile coordinates)
        \param  y   y coordinate (in tile coordinates)
    */
    void doLaunchDeathhand(const GameContext& context, int x, int y);


    /**
        Can this structure be captured by infantry units?
        \return true, if this structure can be captured, false otherwise
    */
    bool canBeCaptured() const noexcept override { return false; }

    int getPercentComplete() const {
        return specialWeaponTimer*100/getMaxSpecialWeaponTimer();
    }

    bool isSpecialWeaponReady() const { return (specialWeaponTimer == 0); }
    int getSpecialWeaponTimer() const { return specialWeaponTimer; }

    int getMaxSpecialWeaponTimer() const {
        if(originalHouseID == HOUSETYPE::HOUSE_HARKONNEN || originalHouseID == HOUSETYPE::HOUSE_SARDAUKAR) {
            // 10 min
            return MILLI2CYCLES(10*60*1000);
        }             // 5 min

            return MILLI2CYCLES(5*60*1000);

       
    }

protected:
    bool callFremen(const GameContext& context);
    bool spawnSaboteur(const GameContext& context);

    /**
        Used for updating things that are specific to that particular structure. Is called from
        StructureBase::update() before the check if this structure is still alive.
    */
    void updateStructureSpecificStuff(const GameContext& context) override;

private:
    void   init();

    Sint32 specialWeaponTimer; ///< When is the special weapon ready?
};

#endif // PALACE_H
