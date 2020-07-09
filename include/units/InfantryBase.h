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

class InfantryBase : public GroundUnit {
protected:
    InfantryBase(ItemID_enum itemID, Uint32 objectID, const ObjectInitializer& initializer);
    InfantryBase(ItemID_enum itemID, Uint32 objectID, const ObjectStreamInitializer& initializer);

public:
    using parent = GroundUnit;

    ~InfantryBase() override = 0;

    InfantryBase(const InfantryBase&) = delete;
    InfantryBase(InfantryBase&&)      = delete;
    InfantryBase& operator=(const InfantryBase&) = delete;
    InfantryBase& operator=(InfantryBase&&) = delete;

    void save(OutputStream& stream) const override;

    /**
        This method is called when an unit is ordered to capture
        \param  xPos    the x position on the map
        \param  yPos    the y position on the map
    */
    virtual void handleCaptureClick(const GameContext& context, int xPos, int yPos);

    /**
        This method is called when an unit should capture a structure
        \param  targetStructureID   the ID of the structure to capture
    */
    virtual void doCaptureStructure(const GameContext& context, Uint32 targetStructureID);

    /**
        This method is called when an unit should capture a structure
        \param  pStructure  the structure to capture
    */
    virtual void doCaptureStructure(const GameContext& context, const StructureBase* pStructure);

    void assignToMap(const GameContext& context, const Coord& pos) override;
    void blitToScreen() override;
    void checkPos(const GameContext& context) override;
    void destroy(const GameContext& context) override;
    void move(const GameContext& context) override;

    void setLocation(const GameContext& context, int xPos, int yPos) override;

    void squash(const GameContext& context);

    void playConfirmSound() override;
    void playSelectSound() override;

    bool canPassTile(const Tile* pTile) const override;

    int getTilePosition() const { return tilePosition; }

protected:
    void setSpeeds(const GameContext& context) override;

    // infantry state
    Sint8 tilePosition;    ///< The position in the current tile (0 to 4)
    Sint8 oldTilePosition; ///< The previous tile position (0 to 4)

    // drawing information
    int walkFrame{}; ///< What frame to draw

private:
    void init();
};


template<>
inline InfantryBase* dune_cast(ObjectBase* base) {
    if(base && base->isInfantry()) return static_cast<InfantryBase*>(base);

    return nullptr;
}

template<>
inline const InfantryBase* dune_cast(const ObjectBase* base) {
    if(base && base->isInfantry()) return static_cast<const InfantryBase*>(base);

    return nullptr;
}

#endif // INFANTRYBASE_H
