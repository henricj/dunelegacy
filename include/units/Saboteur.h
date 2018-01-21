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

#ifndef SABOTEUR_H
#define SABOTEUR_H

#include <units/InfantryBase.h>

class Saboteur final : public InfantryBase
{
public:
    explicit Saboteur(House* newOwner);
    explicit Saboteur(InputStream& stream);
    void init();
    virtual ~Saboteur();

    void checkPos() override;

    /**
        Updates this saboteur.
        \return true if this object still exists, false if it was destroyed
    */
    bool update() override;

    void deploy(const Coord& newLocation) override;
    bool canAttack(const ObjectBase* object) const override;

    void destroy() override;
};

#endif // SABOTEUR_H
