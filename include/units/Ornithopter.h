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

#ifndef ORNITHOPTER_H
#define ORNITHOPTER_H

#include <units/AirUnit.h>

class Ornithopter final : public AirUnit
{
public:
    explicit Ornithopter(House* newOwner);
    explicit Ornithopter(InputStream& stream);
    void init();
    virtual ~Ornithopter();

    void save(OutputStream& stream) const override;

    void checkPos() override;
    bool canAttack(const ObjectBase* object) const override;

    bool canPass(int xPos, int yPos) const override;

    void destroy() override;

    void playAttackSound() override;

protected:
    virtual FixPoint getDestinationAngle() const override;

    virtual bool attack() override;

private:
    Uint32 timeLastShot;
};

#endif //ORNITHOPTER_H
