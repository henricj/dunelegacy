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

#include <players/Player.h>

#include <Map.h>
#include <Game.h>
#include <GameInitSettings.h>

#include <structures/StructureBase.h>
#include <structures/BuilderBase.h>
#include <structures/StarPort.h>
#include <structures/ConstructionYard.h>
#include <structures/Palace.h>
#include <structures/TurretBase.h>
#include <units/UnitBase.h>
#include <units/GroundUnit.h>
#include <units/Devastator.h>
#include <units/Harvester.h>
#include <units/InfantryBase.h>
#include <units/MCV.h>

#include <misc/Random.h>

#include <globals.h>


Player::Player(House* associatedHouse, std::string playername) : pHouse(associatedHouse), playerID(0), playername(playername) {
}

Player::Player(InputStream& stream, House* associatedHouse) : pHouse(associatedHouse) {
    playerID = stream.readUint8();
    playername = stream.readString();
}

Player::~Player() {
    currentGame->unregisterPlayer(this);
}

void Player::save(OutputStream& stream) const {
    stream.writeUint8(playerID);
    stream.writeString(playername);
}

Random& Player::getRandomGen() const {
    return currentGame->randomGen;
}

const GameInitSettings& Player::getGameInitSettings() const {
    return currentGame->getGameInitSettings();
}

Uint32 Player::getGameCylceCount() const {
    return currentGame->getGameCycleCount();
}

const Map& Player::getMap() const {
    return *currentGameMap;
}

const ObjectBase* Player::getObject(Uint32 objectID) const {
    return currentGame->getObjectManager().getObject(objectID);
}

const RobustList<const StructureBase*>& Player::getStructureList() const {
    return reinterpret_cast<const RobustList<const StructureBase*>&>(structureList);
}

const RobustList<const UnitBase*>& Player::getUnitList() const {
    return reinterpret_cast<const RobustList<const UnitBase*>&>(unitList);
}

const House* Player::getHouse(int houseID) const {
    if(houseID < 0 || houseID >= NUM_HOUSES) {
        return nullptr;
    }
    return currentGame->getHouse(houseID);
}

void Player::doRepair(const ObjectBase* pObject) {
    if(pObject->getOwner() == getHouse()) {
        const_cast<ObjectBase*>(pObject)->doRepair();
    } else {
        fprintf(stderr,"Player %s tries to repair a structure or unit he doesn't own!\n", playername.c_str());
    }
}

void Player::doSetDeployPosition(const StructureBase* pStructure, int x, int y) {
    if(pStructure->getOwner() == getHouse()) {
        const_cast<StructureBase*>(pStructure)->doSetDeployPosition(x, y);
    } else {
        fprintf(stderr,"Player %s tries to set the deploy position of a structure he doesn't own!\n", playername.c_str());
    }
}

bool Player::doUpgrade(const BuilderBase* pBuilder) {
    if(pBuilder->getOwner() == getHouse()) {
        return const_cast<BuilderBase*>(pBuilder)->doUpgrade();
    } else {
        fprintf(stderr,"Player %s tries to upgrade a structure he doesn't own!\n", playername.c_str());
        return false;
    }
}

void Player::doProduceItem(const BuilderBase* pBuilder, Uint32 itemID) {
    if(pBuilder->getOwner() == getHouse()) {
        const_cast<BuilderBase*>(pBuilder)->doProduceItem(itemID);
    } else {
        fprintf(stderr,"Player %s tries to build some item in a structure he doesn't own!\n", playername.c_str());
    }
}

void Player::doCancelItem(const BuilderBase* pBuilder, Uint32 itemID) {
    if(pBuilder->getOwner() == getHouse()) {
        const_cast<BuilderBase*>(pBuilder)->doCancelItem(itemID);
    } else {
        fprintf(stderr,"Player %s tries to cancel production of some item in a structure he doesn't own!\n", playername.c_str());
    }
}

void Player::doSetOnHold(const BuilderBase* pBuilder, bool bOnHold) {
    if(pBuilder->getOwner() == getHouse()) {
        const_cast<BuilderBase*>(pBuilder)->doSetOnHold(bOnHold);
    } else {
        fprintf(stderr,"Player %s tries to hold/resume production in a structure he doesn't own!\n", playername.c_str());
    }
}

void Player::doBuildRandom(const BuilderBase* pBuilder) {
    if(pBuilder->getOwner() == getHouse()) {
        const_cast<BuilderBase*>(pBuilder)->doBuildRandom();
    } else {
        fprintf(stderr,"Player %s tries to randomly build some item in a structure he doesn't own!\n", playername.c_str());
    }
}

void Player::doPlaceOrder(const StarPort* pStarport) {
    if(pStarport->getOwner() == getHouse()) {
        const_cast<StarPort*>(pStarport)->doPlaceOrder();
    } else {
        fprintf(stderr,"Player %s tries to order something from a starport he doesn't own!\n", playername.c_str());
    }
}

bool Player::doPlaceStructure(const ConstructionYard* pConstYard, int x, int y) {
    if(pConstYard->getOwner() == getHouse()) {
        return const_cast<ConstructionYard*>(pConstYard)->doPlaceStructure(x, y);
    } else {
        fprintf(stderr,"Player %s tries to place a structure he hasn't produced!\n", playername.c_str());
        return false;
    }
}

void Player::doSpecialWeapon(const Palace* pPalace) {
    if(pPalace->getOwner() == getHouse()) {
        const_cast<Palace*>(pPalace)->doSpecialWeapon();
    } else {
        fprintf(stderr,"Player %s tries to activate a special weapon from a palace he doesn't own!\n", playername.c_str());
    }
}

void Player::doLaunchDeathhand(const Palace* pPalace, int x, int y) {
    if(pPalace->getOwner() == getHouse()) {
        const_cast<Palace*>(pPalace)->doLaunchDeathhand(x, y);
    } else {
        fprintf(stderr,"Player %s tries to launch a deathhand from a palace he doesn't own!\n", playername.c_str());
    }
}

void Player::doAttackObject(const TurretBase* pTurret, const ObjectBase* pTargetObject) {
    if(pTurret->getOwner() == getHouse()) {
        const_cast<TurretBase*>(pTurret)->doAttackObject(pTargetObject);
    } else {
        fprintf(stderr,"Player %s tries to attack with a turret he doesn't own!\n", playername.c_str());
        return;
    }
}



void Player::doMove2Pos(const UnitBase* pUnit, int x, int y, bool bForced) {
    if(pUnit->getOwner() == getHouse()) {
        const_cast<UnitBase*>(pUnit)->doMove2Pos(x, y, bForced);
    } else {
        fprintf(stderr,"Player %s tries to move a unit he doesn't own!\n", playername.c_str());
        return;
    }
}

void Player::doMove2Object(const UnitBase* pUnit, const ObjectBase* pTargetObject) {
    if(pUnit->getOwner() == getHouse()) {
        const_cast<UnitBase*>(pUnit)->doMove2Object(pTargetObject);
    } else {
        fprintf(stderr,"Player %s tries to move a unit (to an object) he doesn't own!\n", playername.c_str());
        return;
    }
}

void Player::doAttackPos(const UnitBase* pUnit, int x, int y, bool bForced) {
    if(pUnit->getOwner() == getHouse()) {
        const_cast<UnitBase*>(pUnit)->doAttackPos(x, y, bForced);
    } else {
        fprintf(stderr,"Player %s tries to order a unit (to attack a position) he doesn't own!\n", playername.c_str());
        return;
    }
}

void Player::doAttackObject(const UnitBase* pUnit, const ObjectBase* pTargetObject, bool bForced) {
    if(pUnit->getOwner() == getHouse()) {
        const_cast<UnitBase*>(pUnit)->doAttackObject(pTargetObject, bForced);
    } else {
        fprintf(stderr,"Player %s tries to attack with a unit he doesn't own!\n", playername.c_str());
        return;
    }
}

void Player::doSetAttackMode(const UnitBase* pUnit, ATTACKMODE attackMode) {
    if(pUnit->getOwner() == getHouse()) {
        const_cast<UnitBase*>(pUnit)->doSetAttackMode(attackMode);
    } else {
        fprintf(stderr,"Player %s tries to change the attack mode of a unit he doesn't own!\n", playername.c_str());
        return;
    }
}

void Player::doStartDevastate(const Devastator* pDevastator) {
    if(pDevastator->getOwner() == getHouse()) {
        const_cast<Devastator*>(pDevastator)->doStartDevastate();
    } else {
        fprintf(stderr,"Player %s tries to devastate a devastator he doesn't own!\n", playername.c_str());
        return;
    }
}

void Player::doReturn(const Harvester* pHarvester) {
    if(pHarvester->getOwner() == getHouse()) {
        const_cast<Harvester*>(pHarvester)->doReturn();
    } else {
        fprintf(stderr,"Player %s tries to return a harvester he doesn't own!\n", playername.c_str());
        return;
    }
}

void Player::doCaptureStructure(const InfantryBase* pInfantry, const StructureBase* pTargetStructure) {
    if(pInfantry->getOwner() == getHouse()) {
        const_cast<InfantryBase*>(pInfantry)->doCaptureStructure(pTargetStructure);
    } else {
        fprintf(stderr,"Player %s tries to capture with a unit he doesn't own!\n", playername.c_str());
        return;
    }
}

bool Player::doDeploy(const MCV* pMCV) {
    if(pMCV->getOwner() == getHouse()) {
        return const_cast<MCV*>(pMCV)->doDeploy();
    } else {
        fprintf(stderr,"Player %s tries to deploy a MCV he doesn't own!\n", playername.c_str());
        return false;
    }
}


bool Player::doRequestCarryallDrop(const GroundUnit* pGroundUnit) {
    if(pGroundUnit->getOwner() == getHouse()
       ) {
        return const_cast<GroundUnit*>(pGroundUnit)->requestCarryall();
    } else {
        fprintf(stderr,"Player %s tries request a carryall for a ground unit he doesn't own!\n", playername.c_str());
        return false;
    }
}

