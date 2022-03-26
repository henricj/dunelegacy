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

namespace {
constexpr TurretBaseConstants gun_turret_constants {GunTurret::item_id, Bullet_ShellTurret};
}

GunTurret::GunTurret(uint32_t objectID, const ObjectInitializer& initializer)
    : TurretBase(gun_turret_constants, objectID, initializer) {
    GunTurret::init();

    GunTurret::setHealth(getMaxHealth());
}

GunTurret::GunTurret(uint32_t objectID, const ObjectStreamInitializer& initializer)
    : TurretBase(gun_turret_constants, objectID, initializer) {
    GunTurret::init();
}

void GunTurret::init() {
    assert(itemID == Structure_GunTurret);
    owner->incrementStructures(itemID);

    attackSound = Sound_enum::Sound_ExplosionSmall;

    graphicID    = ObjPic_GunTurret;
    graphic      = pGFXManager->getObjPic(ObjPic_GunTurret, getOwner()->getHouseID());
    numImagesX   = 10;
    numImagesY   = 1;
    curAnimFrame = firstAnimFrame = lastAnimFrame = ((10 - static_cast<int>(drawnAngle)) % 8) + 2;
}

GunTurret::~GunTurret() = default;
