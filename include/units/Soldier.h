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

#ifndef SOLDIER_H
#define SOLDIER_H

#include <units/InfantryBase.h>

class Soldier final : public InfantryBase
{

public:
    explicit Soldier(House* newOwner);
    explicit Soldier(InputStream& stream);
    void init();
    virtual ~Soldier();

    bool canAttack(const ObjectBase* object) const override;

    bool hasBumpyMovementOnRock() const override { return true; }

    void playAttackSound() override;
};

#endif // SOLDIER_H
