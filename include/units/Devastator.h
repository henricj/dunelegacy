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

#ifndef DEVASTATOR_H
#define DEVASTATOR_H

#include <units/TrackedUnit.h>

class Devastator final : public TrackedUnit
{
public:
    explicit Devastator(House* newOwner);
    explicit Devastator(InputStream& stream);
    void init();
    virtual ~Devastator();

    void save(OutputStream& stream) const override;

    void blitToScreen() override;

    void handleStartDevastateClick();

    void doStartDevastate();

    void destroy() override;

    /**
        Updates this devastator.
        \return true if this object still exists, false if it was destroyed
    */
    bool update() override;

    void playAttackSound() override;

private:
    // devastator state
    Sint32      devastateTimer;       ///< When will this devastator devastate

    // drawing information
    zoomable_texture turretGraphic{}; ///< The graphic of the turret
    int              gunGraphicID;    ///< The id of the turret graphic (needed if we want to reload the graphic)
};

#endif // DEVASTATOR_H
