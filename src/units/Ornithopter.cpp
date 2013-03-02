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

#include <units/Ornithopter.h>

#include <globals.h>

#include <FileClasses/GFXManager.h>
#include <Map.h>
#include <House.h>
#include <SoundPlayer.h>

Ornithopter::Ornithopter(House* newOwner) : AirUnit(newOwner) {

    Ornithopter::init();

    setHealth(getMaxHealth());
}

Ornithopter::Ornithopter(InputStream& stream) : AirUnit(stream) {
    Ornithopter::init();
}

void Ornithopter::init() {
	itemID = Unit_Ornithopter;
	owner->incrementUnits(itemID);

	graphicID = ObjPic_Ornithopter;
	graphic = pGFXManager->getObjPic(graphicID,getOwner()->getHouseID());
    shadowGraphic = pGFXManager->getObjPic(ObjPic_OrnithopterShadow,getOwner()->getHouseID());

	numImagesX = NUM_ANGLES;
	numImagesY = 3;

	numWeapons = 1;
	bulletType = Bullet_SmallRocket;
}

Ornithopter::~Ornithopter() {
}

void Ornithopter::checkPos() {
	AirUnit::checkPos();

	++drawnFrame;
	if(drawnFrame >= 3) {
		drawnFrame = 0;
	}
}

bool Ornithopter::canAttack(const ObjectBase* object) const {
	if ((object != NULL)
		&& ((object->getOwner()->getTeam() != owner->getTeam()) || object->getItemID() == Unit_Sandworm)
		&& object->isVisible(getOwner()->getTeam()))
		return true;
	else
		return false;
}

void Ornithopter::destroy() {
	// place wreck
    if(currentGameMap->tileExists(location)) {
        Tile* pTile = currentGameMap->getTile(location);
        pTile->assignDeadUnit(DeadUnit_Ornithopter, owner->getHouseID(), Coord(lround(realX), lround(realY)));
    }

	AirUnit::destroy();
}

void Ornithopter::playAttackSound() {
	soundPlayer->playSoundAt(Sound_Rocket,location);
}

bool Ornithopter::canPass(int xPos, int yPos) const {
	return (currentGameMap->tileExists(xPos, yPos) && (!currentGameMap->getTile(xPos, yPos)->hasAnAirUnit()));
}

