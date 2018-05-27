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

#include <units/Frigate.h>

#include <globals.h>

#include <FileClasses/GFXManager.h>
#include <FileClasses/SFXManager.h>
#include <House.h>
#include <Map.h>
#include <Game.h>
#include <SoundPlayer.h>

#include <structures/StarPort.h>

Frigate::Frigate(House* newOwner) : AirUnit(newOwner)
{
    Frigate::init();

    setHealth(getMaxHealth());

    attackMode = GUARD;

    respondable = false;
    droppedOffCargo = false;
}

Frigate::Frigate(InputStream& stream) : AirUnit(stream)
{
    Frigate::init();

    droppedOffCargo = stream.readBool();
}

void Frigate::init()
{
    itemID = Unit_Frigate;
    owner->incrementUnits(itemID);

    canAttackStuff = false;

    graphicID = ObjPic_Frigate;
    graphic = pGFXManager->getObjPic(graphicID,getOwner()->getHouseID());
    shadowGraphic = pGFXManager->getObjPic(ObjPic_FrigateShadow,getOwner()->getHouseID());

    numImagesX = NUM_ANGLES;
    numImagesY = 1;
}

Frigate::~Frigate()
{
    StarPort* pStarPort = dynamic_cast<StarPort*>(target.getObjPointer());
    if(pStarPort) {
        pStarPort->informFrigateDestroyed();
    }
}

void Frigate::save(OutputStream& stream) const
{
    AirUnit::save(stream);

    stream.writeBool(droppedOffCargo);
}

bool Frigate::canPass(int xPos, int yPos) const
{
    // frigate can always pass other air units
    return currentGameMap->tileExists(xPos, yPos);
}

void Frigate::checkPos()
{
    AirUnit::checkPos();

    if ((location == destination) && (distanceFrom(realX, realY, destination.x * TILESIZE + (TILESIZE/2), destination.y * TILESIZE + (TILESIZE/2)) < TILESIZE/8) ) {
        StarPort* pStarport = dynamic_cast<StarPort*>(target.getStructurePointer());

        if(pStarport != nullptr) {
            pStarport->startDeploying();
            setTarget(nullptr);
            setDestination(guardPoint);
            droppedOffCargo = true;
            soundPlayer->playSoundAt(Sound_Drop, location);
        }
    }
}

bool Frigate::update() {
    const FixPoint& maxSpeed = currentGame->objectData.data[itemID][originalHouseID].maxspeed;

    FixPoint dist = -1;
    ObjectBase* pTarget = target.getObjPointer();
    if(pTarget != nullptr && pTarget->isAUnit()) {
        dist = distanceFrom(realX, realY, pTarget->getRealX(), pTarget->getRealY());
    } else if((pTarget != nullptr) || !droppedOffCargo) {
        dist = distanceFrom(realX, realY, destination.x*TILESIZE + TILESIZE/2, destination.y*TILESIZE + TILESIZE/2);
    }

    if(dist >= 0) {
        static const FixPoint minSpeed = FixPoint32(TILESIZE/32);
        if(dist < TILESIZE/2) {
            currentMaxSpeed = std::min(dist, minSpeed);
        } else if(dist >= 10*TILESIZE) {
            currentMaxSpeed = maxSpeed;
        } else {
            FixPoint m = (maxSpeed-minSpeed) / ((10*TILESIZE)-(TILESIZE/2));
            FixPoint t = minSpeed-(TILESIZE/2)*m;
            currentMaxSpeed = dist*m+t;
        }
    } else {
        currentMaxSpeed = std::min(currentMaxSpeed + 0.2_fix, maxSpeed);
    }

    if(AirUnit::update() == false) {
        return false;
    }

    // check if target is destroyed
    if((droppedOffCargo == false) && target.getStructurePointer() == nullptr) {
        setDestination(guardPoint);
        droppedOffCargo = true;
    }

    // check if this frigate has to be removed because it has just brought all units to the Starport
    if (active) {
        if(droppedOffCargo
            && ((getRealX() < -TILESIZE) || (getRealX() > (currentGameMap->getSizeX()+1)*TILESIZE)
                || (getRealY() < -TILESIZE) || (getRealY() > (currentGameMap->getSizeY()+1)*TILESIZE))) {

            setVisible(VIS_ALL, false);
            destroy();
            return false;
        }
    }
    return true;
}

void Frigate::deploy(const Coord& newLocation) {
    AirUnit::deploy(newLocation);

    respondable = false;
}

void Frigate::turn() {
    if (active && droppedOffCargo
        && ((getRealX() < TILESIZE/2) || (getRealX() > currentGameMap->getSizeX()*TILESIZE - TILESIZE/2)
            || (getRealY() < TILESIZE/2) || (getRealY() > currentGameMap->getSizeY()*TILESIZE - TILESIZE/2))) {
        // already partially outside the map => do not turn
        return;
    }

    AirUnit::turn();
}
