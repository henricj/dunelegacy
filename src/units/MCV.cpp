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

#include <units/MCV.h>

#include <globals.h>

#include <FileClasses/GFXManager.h>
#include <FileClasses/TextManager.h>
#include <House.h>
#include <Explosion.h>
#include <Game.h>
#include <SoundPlayer.h>
#include <Map.h>
#include <sand.h>

#include <players/HumanPlayer.h>

MCV::MCV(House* newOwner) : GroundUnit(newOwner)
{
    MCV::init();

    setHealth(getMaxHealth());
    attackMode = GUARD;
}

MCV::MCV(InputStream& stream) : GroundUnit(stream)
{
    MCV::init();
}

void MCV::init() {
    itemID = Unit_MCV;
    owner->incrementUnits(itemID);

    canAttackStuff = false;

    graphicID = ObjPic_MCV;
    graphic = pGFXManager->getObjPic(graphicID,getOwner()->getHouseID());

    numImagesX = NUM_ANGLES;
    numImagesY = 1;
}

MCV::~MCV() = default;

void MCV::handleDeployClick() {
    currentGame->getCommandManager().addCommand(Command(pLocalPlayer->getPlayerID(), CMD_MCV_DEPLOY,objectID));
}

bool MCV::doDeploy() {
    // check if there is enough room for construction yard
    if(canDeploy()) {
        // save needed values
        House* pOwner = getOwner();
        Coord newLocation = getLocation();

        // first place construction yard and then destroy MCV, otherwise a player with only MCV left will lose

        // place construction yard (force placing to place on still existing MCV)
        if(pOwner->placeStructure(NONE_ID, Structure_ConstructionYard, newLocation.x, newLocation.y, false, true) != nullptr) {
            // we hide the MVC so we don't get a soldier on destroy
            setVisible(VIS_ALL, false);

            // destroy MCV but with base class method since we want no explosion
            GroundUnit::destroy();

            return true;
        }
    }

    if(getOwner() == pLocalHouse) {
        currentGame->addToNewsTicker(_("You cannot deploy here."));
    }

    return false;
}

bool MCV::canAttack(const ObjectBase* object) const {
    return((object != nullptr)
            && object->isInfantry()
            && (object->getOwner()->getTeamID() != owner->getTeamID())
            && object->isVisible(getOwner()->getTeamID()));
}

void MCV::destroy() {
    if(currentGameMap->tileExists(location) && isVisible()) {
        Coord realPos(lround(realX), lround(realY));
        currentGame->getExplosionList().push_back(new Explosion(Explosion_SmallUnit, realPos, owner->getHouseID()));

        if(isVisible(getOwner()->getTeamID()))
            soundPlayer->playSoundAt(Sound_ExplosionSmall,location);
    }

    GroundUnit::destroy();
}

bool MCV::canDeploy(int x, int y) const {
    for(int i = 0; i < getStructureSize(Structure_ConstructionYard).x; i++) {
        for(int j = 0; j < getStructureSize(Structure_ConstructionYard).y; j++) {
            if(!currentGameMap->tileExists(x+i, y+j)) {
                return false;
            }
            const Tile* pTile = currentGameMap->getTile(x+i, y+j);
            if(!pTile->isBlocked() || ((i == 0) && (j == 0))) {
                // tile is not blocked or we're checking the tile with the MCV on
                if(!pTile->isRock()) {
                    return false;
                }
            } else {
                return false;
            }
        }
    }

    return true;
}
