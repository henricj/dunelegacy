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

#ifndef BARRACKS_H
#define BARRACKS_H

#include <structures/BuilderBase.h>

class Barracks final : public BuilderBase {
public:
    static const ItemID_enum item_id = ItemID_enum::Structure_Barracks;
    using parent = BuilderBase;

    Barracks(ItemID_enum itemID, Uint32 objectID, const ObjectInitializer& initializer);
    Barracks(ItemID_enum itemID, Uint32 objectID, const ObjectStreamInitializer& initializer);
    ~Barracks() override;

    /**
        Can this structure be captured by infantry units?
        \return true, if this structure can be captured, false otherwise
    */
    bool canBeCaptured() const noexcept override { return false; }

private:
    void init();
};

#endif // BARRACKS_H
