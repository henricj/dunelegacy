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

#ifndef AIRUNIT_H
#define AIRUNIT_H

#include <units/UnitBase.h>

class AirUnit : public UnitBase
{
public:
    explicit AirUnit(House* newOwner);
    explicit AirUnit(InputStream& stream);
    void init();
    virtual ~AirUnit();

    void save(OutputStream& stream) const override;

    void blitToScreen() override;

    void playConfirmSound() override { }
    void playSelectSound() override { }

    void destroy() override;

    void assignToMap(const Coord& pos) override;
    void checkPos() override;
    bool canPass(int xPos, int yPos) const override;

    virtual FixPoint getMaxSpeed() const override {
        return currentMaxSpeed;
    }

protected:
    virtual FixPoint getDestinationAngle() const;

    virtual void navigate() override;
    virtual void move() override;
    virtual void turn() override;

    FixPoint currentMaxSpeed;               ///< The current maximum allowed speed

    zoomable_texture shadowGraphic{};       ///< The graphic for the shadow of this air unit
};

#endif // AIRUNIT_H
