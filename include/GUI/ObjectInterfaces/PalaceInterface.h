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

#ifndef PALACEINTERFACE_H
#define PALACEINTERFACE_H

#include "DefaultStructureInterface.h"

#include <GUI/ProgressBar.h>

class PalaceInterface final : public DefaultStructureInterface {
public:
    static std::unique_ptr<PalaceInterface> create(const GameContext& context, int objectID);

    static sdl2::surface_ptr createSurface(SurfaceLoader* surfaceLoader, GeneratedPicture id);

protected:
    PalaceInterface(const GameContext& context, int objectID);

    /**
        This method updates the object interface.
        If the object doesn't exists anymore then update returns false.
        \return true = everything ok, false = the object container should be removed
    */
    bool update() override;

private:
    void onSpecial(const GameContext& context) const;

    StaticContainer weaponBox;
    PictureProgressBar weaponProgressBar;
    PictureButton weaponSelectButton;
};

#endif // PALACEINTERFACE_H
