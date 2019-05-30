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

#include <units/Saboteur.h>

#include <globals.h>

#include <FileClasses/GFXManager.h>
#include <House.h>
#include <Map.h>
#include <Game.h>
#include <ScreenBorder.h>
#include <Explosion.h>
#include <SoundPlayer.h>


Saboteur::Saboteur(House* newOwner) : InfantryBase(newOwner)
{
    Saboteur::init();

    setHealth(getMaxHealth());

    setVisible(VIS_ALL, false);
    setVisible(getOwner()->getTeamID(), true);
    attackMode = GUARD;
}

Saboteur::Saboteur(InputStream& stream) : InfantryBase(stream)
{
    Saboteur::init();
}

void Saboteur::init()
{
    itemID = Unit_Saboteur;
    owner->incrementUnits(itemID);

    graphicID = ObjPic_Saboteur;
    graphic = pGFXManager->getObjPic(graphicID,getOwner()->getHouseID());

    numImagesX = 4;
    numImagesY = 3;

    numWeapons = 0;
}

Saboteur::~Saboteur() = default;


void Saboteur::checkPos()
{
    InfantryBase::checkPos();

    if(active) {
        bool canBeSeen[NUM_TEAMS];
        for(int i = 0; i < NUM_TEAMS; i++) {
            canBeSeen[i] = false;
        }

        for(int x = location.x - 2; (x <= location.x + 2); x++) {
            for(int y = location.y - 2; (y <= location.y + 2); y++) {
                if(currentGameMap->tileExists(x, y) && currentGameMap->getTile(x, y)->hasAnObject()) {
                    canBeSeen[currentGameMap->getTile(x, y)->getObject()->getOwner()->getTeamID()] = true;
                }
            }
        }

        for(int i = 0; i < NUM_TEAMS; i++) {
            setVisible(i, canBeSeen[i]);
        }

        setVisible(getOwner()->getTeamID(), true);    //owner team can always see it
        //setVisible(pLocalHouse->getTeamID(), true);
    }
}

bool Saboteur::update() {
    if(active) {
        if(!moving) {
            //check to see if close enough to blow up target
            if(target.getObjPointer() != nullptr){ //&& target.getObjPointer()->isAStructure()
                if(getOwner()->getTeamID() != target.getObjPointer()->getOwner()->getTeamID())
                {
                    Coord   closestPoint;
                    closestPoint = target.getObjPointer()->getClosestPoint(location);


                    if(blockDistance(location, closestPoint) <= 1.5_fix) {
                        if(isVisible(getOwner()->getTeamID())) {
                            screenborder->shakeScreen(18);
                        }

                        ObjectBase* pObject = target.getObjPointer();
                        destroy();
                        pObject->setHealth(0);
                        pObject->destroy();
                        return false;
                    }
                }
            }
        }
    }

    return InfantryBase::update();
}

void Saboteur::deploy(const Coord& newLocation) {
    UnitBase::deploy(newLocation);

    setVisible(VIS_ALL, false);
    setVisible(getOwner()->getTeamID(), true);
}


bool Saboteur::canAttack(const ObjectBase* object) const {
    if(object != nullptr){
        if((object->isAStructure() || (object->isAGroundUnit() && !object->isInfantry() && object->getItemID() != Unit_Sandworm)) /* allow attack tanks*/
            && (object->getOwner()->getTeamID() != owner->getTeamID())
            && object->isVisible(getOwner()->getTeamID())){

            return true;
        }

    }

    return false;


}

void Saboteur::destroy()
{
    Coord realPos(lround(realX), lround(realY));
    Uint32 explosionID = currentGame->randomGen.getRandOf({Explosion_Medium1, Explosion_Medium2});
    currentGame->getExplosionList().push_back(new Explosion(explosionID, realPos, owner->getHouseID()));

    if(isVisible(getOwner()->getTeamID())) {
        soundPlayer->playSoundAt(Sound_ExplosionLarge,location);
    }

    InfantryBase::destroy();
}
