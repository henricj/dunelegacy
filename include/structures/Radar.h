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

#ifndef RADAR_H
#define RADAR_H

#include <structures/StructureBase.h>

class Radar final : public StructureBase
{
public:
    inline static constexpr ItemID_enum item_id = Structure_Radar;
    using parent = StructureBase;

    Radar(ItemID_enum itemID, Uint32 objectID, const ObjectInitializer& initializer);
    Radar(ItemID_enum itemID, Uint32 objectID, const ObjectStreamInitializer& initializer);
    ~Radar() override;

    std::unique_ptr<ObjectInterface> getInterfaceContainer(const GameContext& context) override;

    /**
        Can this structure be captured by infantry units?
        \return true, if this structure can be captured, false otherwise
    */
    bool canBeCaptured() const noexcept override { return false; }

    void destroy(const GameContext& context) override;

private:
    void init();
};

#endif //RADAR_H
