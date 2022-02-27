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

#ifndef WINDTRAP_H
#define WINDTRAP_H

#include <structures/StructureBase.h>

class WindTrap final : public StructureBase
{
public:
    inline static constexpr ItemID_enum item_id = Structure_WindTrap;
    using parent = StructureBase;

    WindTrap(uint32_t objectID, const ObjectInitializer& initializer);
    WindTrap(uint32_t objectID, const ObjectStreamInitializer& initializer);
    ~WindTrap() override;

    std::unique_ptr<ObjectInterface> getInterfaceContainer(const GameContext& context) override;

    /**
        Updates this object.
        \return true if this object still exists, false if it was destroyed
    */
    bool update(const GameContext& context) override;

    void setHealth(FixPoint newHealth) override;

protected:
    int getProducedPower() const;

private:
    void init();
};

#endif //WINDTRAP_H
