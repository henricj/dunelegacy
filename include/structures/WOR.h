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

#ifndef WOR_H
#define WOR_H

#include <structures/BuilderBase.h>

class WOR final : public BuilderBase {
public:
    inline static constexpr ItemID_enum item_id = Structure_WOR;
    using parent                                = BuilderBase;

    WOR(uint32_t objectID, const ObjectInitializer& initializer);
    WOR(uint32_t objectID, const ObjectStreamInitializer& initializer);
    ~WOR() override;

    /**
        Can this structure be captured by infantry units?
        \return true, if this structure can be captured, false otherwise
    */
    bool canBeCaptured() const noexcept override { return false; }

private:
    void init();
};

#endif // WOR_H
