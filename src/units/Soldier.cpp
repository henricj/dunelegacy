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

#include <units/Soldier.h>

#include <globals.h>

#include <FileClasses/GFXManager.h>
#include <House.h>
#include <SoundPlayer.h>

Soldier::Soldier(House* newOwner) : InfantryBase(newOwner) {
    Soldier::init();

    setHealth(getMaxHealth());
}

Soldier::Soldier(InputStream& stream) : InfantryBase(stream) {
    Soldier::init();
}

void Soldier::init() {
    itemID = Unit_Soldier;
    owner->incrementUnits(itemID);

    numWeapons = 1;
    bulletType = Bullet_ShellSmall;

    graphicID = ObjPic_Soldier;
    graphic = pGFXManager->getObjPic(graphicID,getOwner()->getHouseID());

    numImagesX = 4;
    numImagesY = 3;
}

Soldier::~Soldier() = default;

bool Soldier::canAttack(const ObjectBase* object) const {
    if ((object != nullptr)
        && (object->isAStructure()
            || !object->isAFlyingUnit())
        && ((object->getOwner()->getTeamID() != owner->getTeamID())
            || object->getItemID() == Unit_Sandworm)
        && object->isVisible(getOwner()->getTeamID()))
    {
        return true;
    }
    else
        return false;
}


void Soldier::playAttackSound() {
    soundPlayer->playSoundAt(Sound_Gun,location);
}
