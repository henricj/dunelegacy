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

#ifndef TROOPER_H
#define TROOPER_H

#include <units/InfantryBase.h>

class Trooper : public InfantryBase
{
public:
	Trooper(House* newOwner);
	Trooper(InputStream& stream);
	void init();
	virtual ~Trooper();

	bool canAttack(const ObjectBase* object) const;

	virtual bool hasBumpyMovementOnRock() const { return true; }

	void playAttackSound();
};

#endif //TROOPER_H
