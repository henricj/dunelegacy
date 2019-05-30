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

#include <units/Trooper.h>

#include <globals.h>

#include <FileClasses/GFXManager.h>
#include <House.h>
#include <SoundPlayer.h>

Trooper::Trooper(House* newOwner) : InfantryBase(newOwner) {
    Trooper::init();

    setHealth(getMaxHealth());
}

Trooper::Trooper(InputStream& stream) : InfantryBase(stream) {
    Trooper::init();
}

void Trooper::init() {
    itemID = Unit_Trooper;
    owner->incrementUnits(itemID);

    numWeapons = 1;
    bulletType = Bullet_SmallRocket;

    graphicID = ObjPic_Trooper;
    graphic = pGFXManager->getObjPic(graphicID,getOwner()->getHouseID());

    numImagesX = 4;
    numImagesY = 3;
}

Trooper::~Trooper() = default;

bool Trooper::canAttack(const ObjectBase* object) const {
    if ((object != nullptr)
        && ((object->getOwner()->getTeamID() != owner->getTeamID()) || (object->getItemID() == Unit_Sandworm))
        && object->isVisible(getOwner()->getTeamID()))
    {
        return true;
    }
    else
        return false;
}


void Trooper::playAttackSound() {
    soundPlayer->playSoundAt(Sound_RocketSmall,location);
}
