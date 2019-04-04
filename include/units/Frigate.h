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

#ifndef FRIGATE_H
#define FRIGATE_H

#include <units/AirUnit.h>

class Frigate final : public AirUnit
{
public:
    explicit Frigate(House* newOwner);
    explicit Frigate(InputStream& stream);
    void init();
    virtual ~Frigate();

    void save(OutputStream& stream) const override;

    void checkPos() override;

    bool canPass(int xPos, int yPos) const override;

    /**
        Updates this frigate.
        \return true if this object still exists, false if it was destroyed
    */
    bool update() override;

    void deploy(const Coord& newLocation) override;

protected:
    virtual void turn() override;

private:
    bool    droppedOffCargo;    ///< Is the cargo already dropped off?
};

#endif // FRIGATE_H
