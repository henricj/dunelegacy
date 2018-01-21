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

#ifndef HARVESTER_H
#define HARVESTER_H

#include <units/TrackedUnit.h>

class Harvester final : public TrackedUnit
{
public:

    explicit Harvester(House* newOwner);
    explicit Harvester(InputStream& stream);
    void init();
    virtual ~Harvester();

    void save(OutputStream& stream) const override;

    void blitToScreen() override;

    void checkPos() override;
    void deploy(const Coord& newLocation) override;
    void destroy() override;
    void drawSelectionBox() override;
    void handleDamage(int damage, Uint32 damagerID, House* damagerOwner) override;

    void handleReturnClick();

    /**
        Order this harvester to return to a refinery.
    */
    void doReturn();

    void move() override;
    void setAmountOfSpice(FixPoint newSpice);
    void setReturned();

    void setDestination(int newX, int newY) override;
    inline void setDestination(const Coord& location) { setDestination(location.x, location.y); }

    void setTarget(const ObjectBase* newTarget) override;

    bool canAttack(const ObjectBase* object) const override;

    FixPoint extractSpice(FixPoint extractionSpeed);

    inline FixPoint getAmountOfSpice() const { return spice; }
    inline bool isReturning() const { return returningToRefinery; }
    bool isHarvesting() const;

private:

    void setSpeeds() override;

    // harvester state
    bool     harvestingMode;         ///< currently harvesting
    bool     returningToRefinery;    ///< currently on the way back to the refinery
    FixPoint spice;                  ///< loaded spice
    Uint32   spiceCheckCounter;      ///< Check for available spice on map to harvest
};

#endif // HARVESTER_H
