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

#include <sand.h>
#include <globals.h>


Player::Player(House* associatedHouse, const std::string& playername) : pHouse(associatedHouse), playerID(0), playername(playername) {
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

void Player::logDebug(const char* fmt, ...) const {
#ifdef DEBUG_AI
    std::string house = getHouseNameByNumber((HOUSETYPE) pHouse->getHouseID());
    fprintf(stderr, "%s (%s):   ", playername.c_str(), house.c_str());
    va_list arg;
    va_start(arg, fmt);
    vfprintf(stderr, fmt, arg);
    va_end(arg);
    fprintf(stderr, "\n");
#endif
}

void Player::logWarn(const char* fmt, ...) const {
    std::string house = getHouseNameByNumber((HOUSETYPE) pHouse->getHouseID());
    fprintf(stderr, "%s (%s):   ", playername.c_str(), house.c_str());
    va_list arg;
    va_start(arg, fmt);
    vfprintf(stderr, fmt, arg);
    va_end(arg);
    fprintf(stderr, "\n");
}

Random& Player::getRandomGen() const {
    return currentGame->randomGen;
}

const GameInitSettings& Player::getGameInitSettings() const {
    return currentGame->getGameInitSettings();
}

Uint32 Player::getGameCycleCount() const {
    return currentGame->getGameCycleCount();
}

int Player::getTechLevel() const {
    return currentGame->techLevel;
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

void Player::doRepair(const ObjectBase* pObject) const {
    if(pObject->getOwner() == getHouse() && pObject->isActive()) {
        const_cast<ObjectBase*>(pObject)->doRepair();
    } else {
        logWarn("The player '%s' tries to repair a structure or unit he doesn't own or that is inactive!\n", playername.c_str());
    }
}

void Player::doSetDeployPosition(const StructureBase* pStructure, int x, int y) const {
    if(pStructure->getOwner() == getHouse() && pStructure->isActive()) {
        const_cast<StructureBase*>(pStructure)->doSetDeployPosition(x, y);
    } else {
        logWarn("The player '%s' tries to set the deploy position of a structure he doesn't own or that is inactive!\n", playername.c_str());
    }
}

bool Player::doUpgrade(const BuilderBase* pBuilder) const {
    if(pBuilder->getOwner() == getHouse() && pBuilder->isActive()) {
        return const_cast<BuilderBase*>(pBuilder)->doUpgrade();
    } else {
        logWarn("The player '%s' tries to upgrade a structure he doesn't own or that is inactive!\n", playername.c_str());
        return false;
    }
}

void Player::doProduceItem(const BuilderBase* pBuilder, Uint32 itemID) const {
    if(pBuilder->getOwner() == getHouse() && pBuilder->isActive()) {
        const_cast<BuilderBase*>(pBuilder)->doProduceItem(itemID);
    } else {
        logWarn("The player '%s' tries to build some item in a structure he doesn't own or that is inactive!\n", playername.c_str());
    }
}

void Player::doCancelItem(const BuilderBase* pBuilder, Uint32 itemID) const {
    if(pBuilder->getOwner() == getHouse() && pBuilder->isActive()) {
        const_cast<BuilderBase*>(pBuilder)->doCancelItem(itemID);
    } else {
        logWarn("The player '%s' tries to cancel production of some item in a structure he doesn't own or that is inactive!\n", playername.c_str());
    }
}

void Player::doSetOnHold(const BuilderBase* pBuilder, bool bOnHold) const {
    if(pBuilder->getOwner() == getHouse() && pBuilder->isActive()) {
        const_cast<BuilderBase*>(pBuilder)->doSetOnHold(bOnHold);
    } else {
        logWarn("The player '%s' tries to hold/resume production in a structure he doesn't own or that is inactive!\n", playername.c_str());
    }
}

void Player::doSetBuildSpeedLimit(const BuilderBase* pBuilder, FixPoint buildSpeedLimit) const {
    if(pBuilder->getOwner() == getHouse() && pBuilder->isActive()) {
        const_cast<BuilderBase*>(pBuilder)->doSetBuildSpeedLimit(buildSpeedLimit);
    } else {
        logWarn("The player '%s' tries to limit the build speed of a structure he doesn't own or that is inactive!\n", playername.c_str());
    }
}

void Player::doBuildRandom(const BuilderBase* pBuilder) const {
    if(pBuilder->getOwner() == getHouse() && pBuilder->isActive()) {
        const_cast<BuilderBase*>(pBuilder)->doBuildRandom();
    } else {
        logWarn("The player '%s' tries to randomly build some item in a structure he doesn't own or that is inactive!\n", playername.c_str());
    }
}

void Player::doPlaceOrder(const StarPort* pStarport) const {
    if(pStarport->getOwner() == getHouse() && pStarport->isActive()) {
        const_cast<StarPort*>(pStarport)->doPlaceOrder();
    } else {
        logWarn("The player '%s' tries to order something from a starport he doesn't own or that is inactive!\n", playername.c_str());
    }
}

bool Player::doPlaceStructure(const ConstructionYard* pConstYard, int x, int y) const {
    if(pConstYard->getOwner() == getHouse() && pConstYard->isActive()) {
        return const_cast<ConstructionYard*>(pConstYard)->doPlaceStructure(x, y);
    } else {
        logWarn("The player '%s' tries to place a structure he hasn't produced (or the construction yard is inactive)!\n", playername.c_str());
        return false;
    }
}

void Player::doSpecialWeapon(const Palace* pPalace) const {
    if(pPalace->getOwner() == getHouse() && pPalace->isActive()) {
        const_cast<Palace*>(pPalace)->doSpecialWeapon();
    } else {
        logWarn("The player '%s' tries to activate a special weapon from a palace he doesn't own or that is inactive!\n", playername.c_str());
    }
}

void Player::doLaunchDeathhand(const Palace* pPalace, int x, int y) const {
    if(pPalace->getOwner() == getHouse() && pPalace->isActive()) {
        const_cast<Palace*>(pPalace)->doLaunchDeathhand(x, y);
    } else {
        logWarn("The player '%s' tries to launch a deathhand from a palace he doesn't own or that is inactive!\n", playername.c_str());
    }
}

void Player::doAttackObject(const TurretBase* pTurret, const ObjectBase* pTargetObject) const {
    if(pTurret->getOwner() == getHouse() && pTurret->isActive()) {
        const_cast<TurretBase*>(pTurret)->doAttackObject(pTargetObject);
    } else {
        logWarn("The player '%s' tries to attack with a turret he doesn't own or that is inactive!\n", playername.c_str());
        return;
    }
}



void Player::doMove2Pos(const UnitBase* pUnit, int x, int y, bool bForced) const {
    if(pUnit->getOwner() == getHouse() && pUnit->isActive()) {
        const_cast<UnitBase*>(pUnit)->doMove2Pos(x, y, bForced);
    } else {
        logWarn("The player '%s' tries to move a unit he doesn't own or that is inactive!\n", playername.c_str());
        return;
    }
}

void Player::doMove2Object(const UnitBase* pUnit, const ObjectBase* pTargetObject) const {
    if(pUnit->getOwner() == getHouse() && pUnit->isActive()) {
        const_cast<UnitBase*>(pUnit)->doMove2Object(pTargetObject);
    } else {
        logWarn("The player '%s' tries to move a unit (to an object) he doesn't own or that is inactive!\n", playername.c_str());
        return;
    }
}

void Player::doAttackPos(const UnitBase* pUnit, int x, int y, bool bForced) const {
    if(pUnit->getOwner() == getHouse() && pUnit->isActive()) {
        const_cast<UnitBase*>(pUnit)->doAttackPos(x, y, bForced);
    } else {
        logWarn("The player '%s' tries to order a unit (to attack a position) he doesn't own or that is inactive!\n", playername.c_str());
        return;
    }
}

void Player::doAttackObject(const UnitBase* pUnit, const ObjectBase* pTargetObject, bool bForced) const {
    if(pUnit->getOwner() == getHouse() && pUnit->isActive()) {
        const_cast<UnitBase*>(pUnit)->doAttackObject(pTargetObject, bForced);
    } else {
        logWarn("The player '%s' tries to attack with a unit he doesn't own or that is inactive!\n", playername.c_str());
        return;
    }
}

void Player::doSetAttackMode(const UnitBase* pUnit, ATTACKMODE attackMode) const {
    if(pUnit->getOwner() == getHouse() && pUnit->isActive()) {
        const_cast<UnitBase*>(pUnit)->doSetAttackMode(attackMode);
    } else {
        logWarn("The player '%s' tries to change the attack mode of a unit he doesn't own or that is inactive!\n", playername.c_str());
        return;
    }
}

void Player::doStartDevastate(const Devastator* pDevastator) const {
    if(pDevastator->getOwner() == getHouse() && pDevastator->isActive()) {
        const_cast<Devastator*>(pDevastator)->doStartDevastate();
    } else {
        logWarn("The player '%s' tries to devastate a devastator he doesn't own or that is inactive!\n", playername.c_str());
        return;
    }
}

void Player::doReturn(const Harvester* pHarvester) const {
    if(pHarvester->getOwner() == getHouse() && pHarvester->isActive()) {
        const_cast<Harvester*>(pHarvester)->doReturn();
    } else {
        logWarn("The player '%s' tries to return a harvester he doesn't own or that is inactive!\n", playername.c_str());
        return;
    }
}

void Player::doCaptureStructure(const InfantryBase* pInfantry, const StructureBase* pTargetStructure) const {
    if(pInfantry->getOwner() == getHouse() && pInfantry->isActive()) {
        const_cast<InfantryBase*>(pInfantry)->doCaptureStructure(pTargetStructure);
    } else {
        logWarn("The player '%s' tries to capture with a unit he doesn't own or that is inactive!\n", playername.c_str());
        return;
    }
}

bool Player::doDeploy(const MCV* pMCV) const {
    if(pMCV->getOwner() == getHouse() && pMCV->isActive()) {
        return const_cast<MCV*>(pMCV)->doDeploy();
    } else {
        logWarn("The player '%s' tries to deploy a MCV he doesn't own or that is inactive!\n", playername.c_str());
        return false;
    }
}


bool Player::doRequestCarryallDrop(const GroundUnit* pGroundUnit) const {
    if(pGroundUnit->getOwner() == getHouse() && pGroundUnit->isActive()) {
        return const_cast<GroundUnit*>(pGroundUnit)->requestCarryall();
    } else {
        logWarn("The player '%s' tries request a carryall for a ground unit he doesn't own or that is inactive!\n", playername.c_str());
        return false;
    }
}

