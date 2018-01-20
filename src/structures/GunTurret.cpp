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

#include <structures/GunTurret.h>

#include <globals.h>

#include <FileClasses/GFXManager.h>
#include <FileClasses/SFXManager.h>
#include <House.h>

GunTurret::GunTurret(House* newOwner) : TurretBase(newOwner) {
    GunTurret::init();

    setHealth(getMaxHealth());
}

GunTurret::GunTurret(InputStream& stream) : TurretBase(stream) {
    GunTurret::init();
}

void GunTurret::init() {
    itemID = Structure_GunTurret;
    owner->incrementStructures(itemID);

    attackSound = Sound_ExplosionSmall;
    bulletType = Bullet_ShellTurret;

    graphicID = ObjPic_GunTurret;
    graphic = pGFXManager->getObjPic(ObjPic_GunTurret,getOwner()->getHouseID());
    numImagesX = 10;
    numImagesY = 1;
    curAnimFrame = firstAnimFrame = lastAnimFrame = ((10-drawnAngle) % 8) + 2;
}

GunTurret::~GunTurret() = default;
