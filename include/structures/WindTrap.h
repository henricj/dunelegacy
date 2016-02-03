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

class WindTrap : public StructureBase
{
public:
	WindTrap(House* newOwner);
	WindTrap(InputStream& stream);
	void init();
	virtual ~WindTrap();

    virtual ObjectInterface* getInterfaceContainer();

    /**
        Updates this object.
        \return true if this object still exists, false if it was destroyed
	*/
	virtual bool update();

    virtual void setHealth(FixPoint newHealth);

protected:
	int getProducedPower() const;
};

#endif //WINDTRAP_H
