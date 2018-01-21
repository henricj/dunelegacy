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

#ifndef INFANTRYBASE_H
#define INFANTRYBASE_H

#include <units/GroundUnit.h>

class InfantryBase : public GroundUnit
{

public:
    explicit InfantryBase(House* newOwner);
    explicit InfantryBase(InputStream& stream);
    void init();
    virtual ~InfantryBase();

    InfantryBase(const InfantryBase &) = delete;
    InfantryBase(InfantryBase &&) = delete;
    InfantryBase& operator=(const InfantryBase &) = delete;
    InfantryBase& operator=(InfantryBase &&) = delete;

    void save(OutputStream& stream) const override;

    /**
        This method is called when an unit is ordered to capture
        \param  xPos    the x position on the map
        \param  yPos    the y position on the map
    */
    virtual void handleCaptureClick(int xPos, int yPos);

    /**
        This method is called when an unit should capture a structure
        \param  targetStructureID   the ID of the structure to capture
    */
    virtual void doCaptureStructure(Uint32 targetStructureID);

    /**
        This method is called when an unit should capture a structure
        \param  pStructure  the structure to capture
    */
    virtual void doCaptureStructure(const StructureBase* pStructure);

    void assignToMap(const Coord& pos) override;
    void blitToScreen() override;
    void checkPos() override;
    void destroy() override;
    void move() override;

    inline void setLocation(const Coord& location) { setLocation(location.x, location.y); }
    void setLocation(int xPos, int yPos) override;

    void squash();

    void playConfirmSound() override;
    void playSelectSound() override;

    bool canPass(int xPos, int yPos) const override;

    inline int getTilePosition() const { return tilePosition; }

protected:
    void setSpeeds() override;

    // infantry state
    Sint8   tilePosition;       ///< The position in the current tile (0 to 4)
    Sint8   oldTilePosition;    ///< The previous tile position (0 to 4)

    // drawing information
    int     walkFrame;          ///< What frame to draw
};

#endif // INFANTRYBASE_H
