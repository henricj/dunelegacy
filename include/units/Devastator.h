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
#include <FileClasses/GFXManager.h>

class Devastator final : public TrackedUnit
{
public:
    inline static constexpr ItemID_enum item_id = ItemID_enum::Unit_Devastator;
    using parent                                = TrackedUnit;

    Devastator(Uint32 objectID, const ObjectInitializer& initializer);
    Devastator(Uint32 objectID, const ObjectStreamInitializer& initializer);
    ~Devastator() override;

    void save(OutputStream& stream) const override;

    void blitToScreen() override;

    void handleStartDevastateClick();

    void doStartDevastate();

    void destroy(const GameContext& context) override;

    /**
        Updates this devastator.
        \return true if this object still exists, false if it was destroyed
    */
    bool update(const GameContext& context) override;

    void playAttackSound() override;

private:
    void init();

    // devastator state
    Sint32      devastateTimer;       ///< When will this devastator devastate

    // drawing information
    zoomable_texture turretGraphic{}; ///< The graphic of the turret
    int              gunGraphicID;    ///< The id of the turret graphic (needed if we want to reload the graphic)
};

#endif // DEVASTATOR_H
