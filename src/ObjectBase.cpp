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

#include <ObjectBase.h>

#include <globals.h>

#include <FileClasses/GFXManager.h>
#include <FileClasses/music/MusicPlayer.h>

#include <Game.h>
#include <House.h>
#include <SoundPlayer.h>
#include <Map.h>
#include <ScreenBorder.h>
#include <players/HumanPlayer.h>
#include <GUI/ObjectInterfaces/DefaultObjectInterface.h>

//structures
#include <structures/Barracks.h>
#include <structures/ConstructionYard.h>
#include <structures/GunTurret.h>
#include <structures/HeavyFactory.h>
#include <structures/HighTechFactory.h>
#include <structures/IX.h>
#include <structures/LightFactory.h>
#include <structures/Palace.h>
#include <structures/Radar.h>
#include <structures/Refinery.h>
#include <structures/RepairYard.h>
#include <structures/RocketTurret.h>
#include <structures/Silo.h>
#include <structures/StarPort.h>
#include <structures/Wall.h>
#include <structures/WindTrap.h>
#include <structures/WOR.h>

//units
#include <units/Carryall.h>
#include <units/Devastator.h>
#include <units/Deviator.h>
#include <units/Frigate.h>
#include <units/Harvester.h>
#include <units/Launcher.h>
#include <units/MCV.h>
#include <units/Ornithopter.h>
#include <units/Quad.h>
#include <units/RaiderTrike.h>
#include <units/Saboteur.h>
#include <units/SandWorm.h>
#include <units/SiegeTank.h>
#include <units/Soldier.h>
#include <units/SonicTank.h>
#include <units/Tank.h>
#include <units/Trike.h>
#include <units/Trooper.h>

ObjectBase::ObjectBase(House* newOwner) : originalHouseID(newOwner->getHouseID()), owner(newOwner) {
    ObjectBase::init();

    objectID = NONE;

    health = 0;
    badlyDamaged = false;

    location = Coord::Invalid();
    oldLocation = Coord::Invalid();
    destination = Coord::Invalid();
    realX = 0;
    realY = 0;

    drawnAngle = 0;
    angle = drawnAngle;

    active = false;
    respondable = true;
    selected = false;
    selectedByOtherPlayer = false;

    forced = false;
    setTarget(nullptr);
    targetFriendly = false;
    attackMode = GUARD;

    setVisible(VIS_ALL, false);
}

ObjectBase::ObjectBase(InputStream& stream) {
    originalHouseID = stream.readUint32();
    owner = currentGame->getHouse(stream.readUint32());

    ObjectBase::init();

    health = stream.readFixPoint();
    badlyDamaged = stream.readBool();

    location.x = stream.readSint32();
    location.y = stream.readSint32();
    oldLocation.x = stream.readSint32();
    oldLocation.y = stream.readSint32();
    destination.x = stream.readSint32();
    destination.y = stream.readSint32();
    realX = stream.readFixPoint();
    realY = stream.readFixPoint();

    angle = stream.readFixPoint();
    drawnAngle = stream.readSint8();

    active = stream.readBool();
    respondable = stream.readBool();

    if(currentGame->getGameInitSettings().getGameType() != GAMETYPE_CUSTOM_MULTIPLAYER) {
        selected = stream.readBool();
        selectedByOtherPlayer = stream.readBool();
    } else {
        selected = false;
        selectedByOtherPlayer = false;
    }

    forced = stream.readBool();
    target.load(stream);
    targetFriendly = stream.readBool();
    attackMode = (ATTACKMODE) stream.readUint32();

    stream.readBools(&visible[0], &visible[1], &visible[2], &visible[3], &visible[4], &visible[5]);
}

void ObjectBase::init() {
    itemID = ItemID_Invalid;

    aFlyingUnit = false;
    aGroundUnit = false;
    aStructure = false;
    aUnit = false;
    infantry = false;
    aBuilder = false;

    canAttackStuff = false;

    radius = TILESIZE/2;

    graphicID = -1;
    graphic = nullptr;
    numImagesX = 0;
    numImagesY = 0;

}

ObjectBase::~ObjectBase() {
}

void ObjectBase::save(OutputStream& stream) const {
    stream.writeUint32(originalHouseID);
    stream.writeUint32(owner->getHouseID());

    stream.writeFixPoint(health);
    stream.writeBool(badlyDamaged);

    stream.writeSint32(location.x);
    stream.writeSint32(location.y);
    stream.writeSint32(oldLocation.x);
    stream.writeSint32(oldLocation.y);
    stream.writeSint32(destination.x);
    stream.writeSint32(destination.y);
    stream.writeFixPoint(realX);
    stream.writeFixPoint(realY);

    stream.writeFixPoint(angle);
    stream.writeSint8(drawnAngle);

    stream.writeBool(active);
    stream.writeBool(respondable);

    if(currentGame->getGameInitSettings().getGameType() != GAMETYPE_CUSTOM_MULTIPLAYER) {
        stream.writeBool(selected);
        stream.writeBool(selectedByOtherPlayer);
    }

    stream.writeBool(forced);
    target.save(stream);
    stream.writeBool(targetFriendly);
    stream.writeUint32(attackMode);

    stream.writeBools(visible[0], visible[1], visible[2], visible[3], visible[4], visible[5]);
}


/**
    Returns the center point of this object
    \return the center point in world coordinates
*/
Coord ObjectBase::getCenterPoint() const {
    return Coord(lround(realX), lround(realY));
}

Coord ObjectBase::getClosestCenterPoint(const Coord& objectLocation) const {
    return getCenterPoint();
}


int ObjectBase::getMaxHealth() const {
    return currentGame->objectData.data[itemID][originalHouseID].hitpoints;
}

void ObjectBase::handleDamage(int damage, Uint32 damagerID, House* damagerOwner) {
    if(damage >= 0) {
        FixPoint newHealth = getHealth();

        newHealth -= damage;

        if(newHealth <= 0) {
            setHealth(0);

            if(damagerOwner != nullptr) {
                damagerOwner->informHasKilled(itemID);
            }
        } else {
            setHealth(newHealth);
        }
    }

    if(getOwner() == pLocalHouse) {
        musicPlayer->changeMusic(MUSIC_ATTACK);
    }

    getOwner()->noteDamageLocation(this, damage, damagerID);
}

void ObjectBase::handleInterfaceEvent(SDL_Event* event) {
    ;
}

ObjectInterface* ObjectBase::getInterfaceContainer() {
    return DefaultObjectInterface::create(objectID);
}

void ObjectBase::removeFromSelectionLists() {
    currentGame->getSelectedList().erase(getObjectID());
    currentGame->selectionChanged();
    currentGame->getSelectedByOtherPlayerList().erase(getObjectID());
    selected = false;

    for(int i=0; i < NUMSELECTEDLISTS; i++) {
        pLocalPlayer->getGroupList(i).erase(getObjectID());
    }
}

void ObjectBase::setDestination(int newX, int newY) {
    if(currentGameMap->tileExists(newX, newY) || ((newX == INVALID_POS) && (newY == INVALID_POS))) {
        destination.x = newX;
        destination.y = newY;
    }
}

void ObjectBase::setHealth(FixPoint newHealth) {
    if((newHealth >= 0) && (newHealth <= getMaxHealth())) {
        health = newHealth;
        badlyDamaged = (health/getMaxHealth() < BADLYDAMAGEDRATIO);
    }
}

void ObjectBase::setLocation(int xPos, int yPos) {
    if((xPos == INVALID_POS) && (yPos == INVALID_POS)) {
        location.invalidate();
    } else if (currentGameMap->tileExists(xPos, yPos))  {
        location.x = xPos;
        location.y = yPos;
        realX = location.x*TILESIZE;
        realY = location.y*TILESIZE;

        assignToMap(location);
    }
}

void ObjectBase::setObjectID(int newObjectID) {
    if(newObjectID >= 0) {
        objectID = newObjectID;
    }
}

void ObjectBase::setVisible(int team, bool status) {
    if(team == VIS_ALL) {
        for(int i = 0; i < NUM_HOUSES; i++) {
            visible[i] = status;
        }
    } else if ((team >= 0) && (team < NUM_HOUSES)) {
        visible[team] = status;
    }
}

void ObjectBase::setTarget(const ObjectBase* newTarget) {
    target.pointTo(const_cast<ObjectBase*>(newTarget));
    targetFriendly = (target && (target.getObjPointer()->getOwner()->getTeam() == owner->getTeam()) && (getItemID() != Unit_Sandworm) && (target.getObjPointer()->getItemID() != Unit_Sandworm));
}

void ObjectBase::unassignFromMap(const Coord& location) {
    if(currentGameMap->tileExists(location)) {
        currentGameMap->getTile(location)->unassignObject(getObjectID());
    }
}


bool ObjectBase::canAttack(const ObjectBase* object) const {

    if( canAttack()
        && (object != nullptr)
        && (object->isAStructure()
            || !object->isAFlyingUnit())
        && ((object->getOwner()->getTeam() != owner->getTeam())
            || object->getItemID() == Unit_Sandworm)
        && object->isVisible(getOwner()->getTeam()))
    {
        return true;
    } else {
        return false;
    }
    return false;
}

bool ObjectBase::isOnScreen() const {
    Coord position = Coord(lround(getRealX()), lround(getRealY()));
    Coord size = Coord(getWidth(graphic[currentZoomlevel])/numImagesX, getHeight(graphic[currentZoomlevel])/numImagesY);

    if(screenborder->isInsideScreen(position,size) == true){
        return true;
    } else {
        return false;
    }
}

bool ObjectBase::isVisible(int team) const {
    if((team >= 0) && (team < NUM_HOUSES)) {
        return visible[team];
    } else {
        return false;
    }
}

bool ObjectBase::isVisible() const {
    for(int i=0;i<NUM_HOUSES;i++) {
        if(visible[i]) {
            return true;
        }
    }

    return false;
}

Uint32 ObjectBase::getHealthColor() const {
    FixPoint healthPercent = health/getMaxHealth();

    if(healthPercent >= BADLYDAMAGEDRATIO) {
        return COLOR_LIGHTGREEN;
    } else if(healthPercent >= HEAVILYDAMAGEDRATIO) {
        return COLOR_YELLOW;
    } else {
        return COLOR_RED;
    }
}

Coord ObjectBase::getClosestPoint(const Coord& point) const {
    return location;
}

const StructureBase* ObjectBase::findClosestTargetStructure() const {

    StructureBase   *closestStructure = nullptr;
    FixPoint        closestDistance = FixPt_MAX;

    RobustList<StructureBase*>::const_iterator iter;
    for(iter = structureList.begin(); iter != structureList.end(); ++iter) {
        StructureBase* tempStructure = *iter;

        if(canAttack(tempStructure)) {
            Coord closestPoint = tempStructure->getClosestPoint(getLocation());
            FixPoint structureDistance = blockDistance(getLocation(), closestPoint);

            if(tempStructure->getItemID() == Structure_Wall) {
                    structureDistance += 20000000; //so that walls are targeted very last
            }

            if(structureDistance < closestDistance) {
                closestDistance = structureDistance;
                closestStructure = tempStructure;
            }
        }
    }

    return closestStructure;
}

const UnitBase* ObjectBase::findClosestTargetUnit() const {
    UnitBase    *closestUnit = nullptr;
    FixPoint    closestDistance = FixPt_MAX;

    RobustList<UnitBase*>::const_iterator iter;
    for(iter = unitList.begin(); iter != unitList.end(); ++iter) {
        UnitBase* tempUnit = *iter;

        if(canAttack(tempUnit)) {
            Coord closestPoint = tempUnit->getClosestPoint(getLocation());
            FixPoint unitDistance = blockDistance(getLocation(), closestPoint);

                if(unitDistance < closestDistance) {
                    closestDistance = unitDistance;
                    closestUnit = tempUnit;
                }

        }
    }

    return closestUnit;
}

const ObjectBase* ObjectBase::findClosestTarget() const {

    ObjectBase  *closestObject = nullptr;
    FixPoint    closestDistance = FixPt_MAX;

    RobustList<StructureBase*>::const_iterator iter1;
    for(iter1 = structureList.begin(); iter1 != structureList.end(); ++iter1) {
        StructureBase* tempStructure = *iter1;

        if(canAttack(tempStructure)) {
            Coord closestPoint = tempStructure->getClosestPoint(getLocation());
            FixPoint structureDistance = blockDistance(getLocation(), closestPoint);

            if(tempStructure->getItemID() == Structure_Wall) {
                    structureDistance += 20000000; //so that walls are targeted very last
            }

            if(structureDistance < closestDistance) {
                closestDistance = structureDistance;
                closestObject = tempStructure;
            }
        }
    }

    RobustList<UnitBase*>::const_iterator iter2;
    for(iter2 = unitList.begin(); iter2 != unitList.end(); ++iter2) {
        UnitBase* tempUnit = *iter2;

        if(canAttack(tempUnit)) {
            Coord closestPoint = tempUnit->getClosestPoint(getLocation());
            FixPoint unitDistance = blockDistance(getLocation(), closestPoint);

            if(unitDistance < closestDistance) {
                closestDistance = unitDistance;
                closestObject = tempUnit;
            }
        }
    }

    return closestObject;
}

const ObjectBase* ObjectBase::findTarget() const {
    ObjectBase  *tempTarget,
                *closestTarget = nullptr;

    int checkRange = 0;
    int xPos = location.x;
    int yPos = location.y;

    FixPoint closestDistance = FixPt_MAX;

//searches for a target in an area like as shown below
//                     *****
//                   *********
//                  *****T*****
//                   *********
//                     *****

    switch(attackMode) {
        case GUARD: {
            checkRange = getWeaponRange();
        } break;

        case AREAGUARD: {
            checkRange = getAreaGuardRange();
        } break;

        case AMBUSH: {
            checkRange = getViewRange();
        } break;

        case HUNT: {
            // check whole map
            return findClosestTarget();
        } break;

        case STOP:
        default: {
            return nullptr;
        } break;
    }

    if(getItemID() == Unit_Sandworm) {
        checkRange = getViewRange();
    }

    int xCheck = xPos - checkRange;

    if(xCheck < 0) {
        xCheck = 0;
    }

    while((xCheck < currentGameMap->getSizeX()) && ((xCheck - xPos) <= checkRange)) {
        int yCheck = (yPos - lookDist[abs(xCheck - xPos)]);

        if(yCheck < 0) {
            yCheck = 0;
        }

        while((yCheck < currentGameMap->getSizeY()) && ((yCheck - yPos) <=  lookDist[abs(xCheck - xPos)])) {
            if(currentGameMap->getTile(xCheck,yCheck)->hasAnObject()) {
                tempTarget = currentGameMap->getTile(xCheck,yCheck)->getObject();

                if((tempTarget->getItemID() != Structure_Wall
                    || tempTarget->getItemID() != Unit_Carryall
                    || closestTarget == nullptr)
                    && canAttack(tempTarget)) {
                    FixPoint targetDistance = blockDistance(location, tempTarget->getLocation());
                    if(targetDistance < closestDistance) {
                        closestTarget = tempTarget;
                        closestDistance = targetDistance;
                    }
                }
            }

            yCheck++;
        }

        xCheck++;
    }

    return closestTarget;
}

int ObjectBase::getViewRange() const {
    return currentGame->objectData.data[itemID][originalHouseID].viewrange;
}

int ObjectBase::getAreaGuardRange() const {
    return 2*getWeaponRange();
}

int ObjectBase::getWeaponRange() const {
    return currentGame->objectData.data[itemID][originalHouseID].weaponrange;
}

int ObjectBase::getWeaponReloadTime() const {
    return currentGame->objectData.data[itemID][originalHouseID].weaponreloadtime;
}

int ObjectBase::getInfSpawnProp() const {
    return currentGame->objectData.data[itemID][originalHouseID].infspawnprop;
}

ObjectBase* ObjectBase::createObject(int itemID, House* Owner, Uint32 objectID) {

    ObjectBase* newObject = nullptr;
    switch(itemID) {
        case Structure_Barracks:            newObject = new Barracks(Owner); break;
        case Structure_ConstructionYard:    newObject = new ConstructionYard(Owner); break;
        case Structure_GunTurret:           newObject = new GunTurret(Owner); break;
        case Structure_HeavyFactory:        newObject = new HeavyFactory(Owner); break;
        case Structure_HighTechFactory:     newObject = new HighTechFactory(Owner); break;
        case Structure_IX:                  newObject = new IX(Owner); break;
        case Structure_LightFactory:        newObject = new LightFactory(Owner); break;
        case Structure_Palace:              newObject = new Palace(Owner); break;
        case Structure_Radar:               newObject = new Radar(Owner); break;
        case Structure_Refinery:            newObject = new Refinery(Owner); break;
        case Structure_RepairYard:          newObject = new RepairYard(Owner); break;
        case Structure_RocketTurret:        newObject = new RocketTurret(Owner); break;
        case Structure_Silo:                newObject = new Silo(Owner); break;
        case Structure_StarPort:            newObject = new StarPort(Owner); break;
        case Structure_Wall:                newObject = new Wall(Owner); break;
        case Structure_WindTrap:            newObject = new WindTrap(Owner); break;
        case Structure_WOR:                 newObject = new WOR(Owner); break;

        case Unit_Carryall:                 newObject = new Carryall(Owner); break;
        case Unit_Devastator:               newObject = new Devastator(Owner); break;
        case Unit_Deviator:                 newObject = new Deviator(Owner); break;
        case Unit_Frigate:                  newObject = new Frigate(Owner); break;
        case Unit_Harvester:                newObject = new Harvester(Owner); break;
        case Unit_Soldier:                  newObject = new Soldier(Owner); break;
        case Unit_Launcher:                 newObject = new Launcher(Owner); break;
        case Unit_MCV:                      newObject = new MCV(Owner); break;
        case Unit_Ornithopter:              newObject = new Ornithopter(Owner); break;
        case Unit_Quad:                     newObject = new Quad(Owner); break;
        case Unit_Saboteur:                 newObject = new Saboteur(Owner); break;
        case Unit_Sandworm:                 newObject = new Sandworm(Owner); break;
        case Unit_SiegeTank:                newObject = new SiegeTank(Owner); break;
        case Unit_SonicTank:                newObject = new SonicTank(Owner); break;
        case Unit_Tank:                     newObject = new Tank(Owner); break;
        case Unit_Trike:                    newObject = new Trike(Owner); break;
        case Unit_RaiderTrike:              newObject = new RaiderTrike(Owner); break;
        case Unit_Trooper:                  newObject = new Trooper(Owner); break;
        case Unit_Special: {
            switch(Owner->getHouseID()) {
                case HOUSE_HARKONNEN:       newObject = new Devastator(Owner); break;
                case HOUSE_ATREIDES:        newObject = new SonicTank(Owner); break;
                case HOUSE_ORDOS:           newObject = new Deviator(Owner); break;
                case HOUSE_FREMEN:
                case HOUSE_SARDAUKAR:
                case HOUSE_MERCENARY: {
                    if(currentGame->randomGen.randBool()) {
                         newObject = new SonicTank(Owner);
                    } else {
                        newObject = new Devastator(Owner);
                    }
                } break;
                default:    /* should never be reached */  break;
            }
        } break;

        default:                            newObject = nullptr;
                                            fprintf(stderr,"ObjectBase::createObject(): %d is no valid ItemID!\n",itemID);
                                            break;
    }

    if(newObject == nullptr) {
        return nullptr;
    }

    if(objectID == NONE) {
        objectID = currentGame->getObjectManager().addObject(newObject);
        newObject->setObjectID(objectID);
    } else {
        newObject->setObjectID(objectID);
    }

    return newObject;
}

ObjectBase* ObjectBase::loadObject(InputStream& stream, int itemID, Uint32 objectID) {
    ObjectBase* newObject = nullptr;
    switch(itemID) {
        case Structure_Barracks:            newObject = new Barracks(stream); break;
        case Structure_ConstructionYard:    newObject = new ConstructionYard(stream); break;
        case Structure_GunTurret:           newObject = new GunTurret(stream); break;
        case Structure_HeavyFactory:        newObject = new HeavyFactory(stream); break;
        case Structure_HighTechFactory:     newObject = new HighTechFactory(stream); break;
        case Structure_IX:                  newObject = new IX(stream); break;
        case Structure_LightFactory:        newObject = new LightFactory(stream); break;
        case Structure_Palace:              newObject = new Palace(stream); break;
        case Structure_Radar:               newObject = new Radar(stream); break;
        case Structure_Refinery:            newObject = new Refinery(stream); break;
        case Structure_RepairYard:          newObject = new RepairYard(stream); break;
        case Structure_RocketTurret:        newObject = new RocketTurret(stream); break;
        case Structure_Silo:                newObject = new Silo(stream); break;
        case Structure_StarPort:            newObject = new StarPort(stream); break;
        case Structure_Wall:                newObject = new Wall(stream); break;
        case Structure_WindTrap:            newObject = new WindTrap(stream); break;
        case Structure_WOR:                 newObject = new WOR(stream); break;

        case Unit_Carryall:                 newObject = new Carryall(stream); break;
        case Unit_Devastator:               newObject = new Devastator(stream); break;
        case Unit_Deviator:                 newObject = new Deviator(stream); break;
        case Unit_Frigate:                  newObject = new Frigate(stream); break;
        case Unit_Harvester:                newObject = new Harvester(stream); break;
        case Unit_Soldier:                  newObject = new Soldier(stream); break;
        case Unit_Launcher:                 newObject = new Launcher(stream); break;
        case Unit_MCV:                      newObject = new MCV(stream); break;
        case Unit_Ornithopter:              newObject = new Ornithopter(stream); break;
        case Unit_Quad:                     newObject = new Quad(stream); break;
        case Unit_Saboteur:                 newObject = new Saboteur(stream); break;
        case Unit_Sandworm:                 newObject = new Sandworm(stream); break;
        case Unit_SiegeTank:                newObject = new SiegeTank(stream); break;
        case Unit_SonicTank:                newObject = new SonicTank(stream); break;
        case Unit_Tank:                     newObject = new Tank(stream); break;
        case Unit_Trike:                    newObject = new Trike(stream); break;
        case Unit_RaiderTrike:              newObject = new RaiderTrike(stream); break;
        case Unit_Trooper:                  newObject = new Trooper(stream); break;

        default:                            newObject = nullptr;
                                            fprintf(stderr,"ObjectBase::loadObject(): %d is no valid ItemID!\n",itemID);
                                            break;
    }

    if(newObject == nullptr) {
        return nullptr;
    }

    newObject->setObjectID(objectID);

    return newObject;
}

bool ObjectBase::targetInWeaponRange() const {
    Coord coord = (target.getObjPointer())->getClosestPoint(location);
    FixPoint dist = blockDistance(location,coord);

    return ( dist <= currentGame->objectData.data[itemID][originalHouseID].weaponrange);
}
