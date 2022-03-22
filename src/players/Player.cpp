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

#include <Game.h>
#include <GameInitSettings.h>
#include <Map.h>

#include <structures/BuilderBase.h>
#include <structures/ConstructionYard.h>
#include <structures/Palace.h>
#include <structures/StarPort.h>
#include <structures/StructureBase.h>
#include <structures/TurretBase.h>
#include <units/Devastator.h>
#include <units/GroundUnit.h>
#include <units/Harvester.h>
#include <units/InfantryBase.h>
#include <units/MCV.h>
#include <units/UnitBase.h>

#include <misc/Random.h>

#include <globals.h>
#include <sand.h>
#include <utility>

Player::Player(const GameContext& context, House* associatedHouse, std::string playername, const Random& random)
    : pHouse(associatedHouse), playerID(0), playername(std::move(playername)), random_ {random},
      context_ {context} { }

Player::Player(const GameContext& context, InputStream& stream, House* associatedHouse)
    : pHouse(associatedHouse), context_ {context} {
    playerID         = stream.readUint8();
    playername       = stream.readString();
    const auto state = stream.readUint8Vector();
    if (state.size() != decltype(random_)::state_bytes)
        THROW(std::runtime_error, "Random state size mismatch!");
    random_.setState(gsl::span<const uint8_t, Random::state_bytes> {state});
}

Player::~Player() {
    context_.game.unregisterPlayer(this);
}

void Player::save(OutputStream& stream) const {
    stream.writeUint8(playerID);
    stream.writeString(playername);
    stream.writeUint8Vector(random_.getState());
}

Random& Player::getRandomGen() {
    return random_;
}

const GameInitSettings& Player::getGameInitSettings() const {
    return context_.game.getGameInitSettings();
}

uint32_t Player::getGameCycleCount() const {
    return context_.game.getGameCycleCount();
}

int Player::getTechLevel() const {
    return context_.game.techLevel;
}

Map& Player::getMap() {
    return context_.map;
}

const Map& Player::getMap() const {
    return context_.map;
}

const ObjectBase* Player::getObject(uint32_t objectID) const {
    return context_.objectManager.getObject(objectID);
}

const RobustList<const StructureBase*>& Player::getStructureList() {
    return reinterpret_cast<const RobustList<const StructureBase*>&>(structureList);
}

const RobustList<const UnitBase*>& Player::getUnitList() const {
    return reinterpret_cast<const RobustList<const UnitBase*>&>(unitList);
}

const House* Player::getHouse(HOUSETYPE houseID) {
    if (static_cast<int>(houseID) < 0 || houseID >= HOUSETYPE::NUM_HOUSES) {
        return nullptr;
    }
    return context_.game.getHouse(houseID);
}

void Player::doRepair(const ObjectBase* pObject) const {
    if (pObject->getOwner() == getHouse() && pObject->isActive()) {
        const_cast<ObjectBase*>(pObject)->doRepair(context_);
    } else {
        logWarn("The player '%s' tries to repair a structure or unit he doesn't own or that is inactive!\n", playername.c_str());
    }
}

void Player::doSetDeployPosition(const StructureBase* pStructure, int x, int y) const {
    if (pStructure->getOwner() == getHouse() && pStructure->isActive()) {
        const_cast<StructureBase*>(pStructure)->doSetDeployPosition(x, y);
    } else {
        logWarn("The player '%s' tries to set the deploy position of a structure he doesn't own or that is inactive!\n", playername.c_str());
    }
}

bool Player::doUpgrade(const BuilderBase* pBuilder) const {
    if (pBuilder->getOwner() == getHouse() && pBuilder->isActive()) {
        return const_cast<BuilderBase*>(pBuilder)->doUpgrade(context_);
    }
    logWarn("The player '%s' tries to upgrade a structure he doesn't own or that is inactive!\n", playername.c_str());

    return false;
}

void Player::doProduceItem(const BuilderBase* pBuilder, ItemID_enum itemID) const {
    if (pBuilder->getOwner() == getHouse() && pBuilder->isActive()) {
        const_cast<BuilderBase*>(pBuilder)->doProduceItem(itemID);
    } else {
        logWarn("The player '%s' tries to build some item in a structure he doesn't own or that is inactive!\n", playername.c_str());
    }
}

void Player::doCancelItem(const BuilderBase* pBuilder, ItemID_enum itemID) const {
    if (pBuilder->getOwner() == getHouse() && pBuilder->isActive()) {
        const_cast<BuilderBase*>(pBuilder)->doCancelItem(itemID);
    } else {
        logWarn("The player '%s' tries to cancel production of some item in a structure he doesn't own or that is inactive!\n", playername.c_str());
    }
}

void Player::doSetOnHold(const BuilderBase* pBuilder, bool bOnHold) const {
    if (pBuilder->getOwner() == getHouse() && pBuilder->isActive()) {
        const_cast<BuilderBase*>(pBuilder)->doSetOnHold(bOnHold);
    } else {
        logWarn("The player '%s' tries to hold/resume production in a structure he doesn't own or that is inactive!\n", playername.c_str());
    }
}

void Player::doSetBuildSpeedLimit(const BuilderBase* pBuilder, FixPoint buildSpeedLimit) const {
    if (pBuilder->getOwner() == getHouse() && pBuilder->isActive()) {
        const_cast<BuilderBase*>(pBuilder)->doSetBuildSpeedLimit(buildSpeedLimit);
    } else {
        logWarn("The player '%s' tries to limit the build speed of a structure he doesn't own or that is inactive!\n", playername.c_str());
    }
}

void Player::doBuildRandom(const BuilderBase* pBuilder) const {
    if (pBuilder->getOwner() == getHouse() && pBuilder->isActive()) {
        const_cast<BuilderBase*>(pBuilder)->doBuildRandom(context_);
    } else {
        logWarn("The player '%s' tries to randomly build some item in a structure he doesn't own or that is inactive!\n", playername.c_str());
    }
}

void Player::doPlaceOrder(const StarPort* pStarport) const {
    if (pStarport->getOwner() == getHouse() && pStarport->isActive()) {
        const_cast<StarPort*>(pStarport)->doPlaceOrder();
    } else {
        logWarn("The player '%s' tries to order something from a starport he doesn't own or that is inactive!\n", playername.c_str());
    }
}

bool Player::doPlaceStructure(const ConstructionYard* pConstYard, int x, int y) const {
    if (pConstYard->getOwner() == getHouse() && pConstYard->isActive()) {
        return const_cast<ConstructionYard*>(pConstYard)->doPlaceStructure(x, y);
    }
    logWarn("The player '%s' tries to place a structure he hasn't produced (or the construction yard is inactive)!\n", playername.c_str());

    return false;
}

void Player::doSpecialWeapon(const Palace* pPalace) const {
    if (pPalace->getOwner() == getHouse() && pPalace->isActive()) {
        const_cast<Palace*>(pPalace)->doSpecialWeapon(context_);
    } else {
        logWarn("The player '%s' tries to activate a special weapon from a palace he doesn't own or that is inactive!\n", playername.c_str());
    }
}

void Player::doLaunchDeathhand(const Palace* pPalace, int x, int y) const {
    if (pPalace->getOwner() == getHouse() && pPalace->isActive()) {
        const_cast<Palace*>(pPalace)->doLaunchDeathhand(context_, x, y);
    } else {
        logWarn("The player '%s' tries to launch a deathhand from a palace he doesn't own or that is inactive!\n", playername.c_str());
    }
}

void Player::doAttackObject(const TurretBase* pTurret, const ObjectBase* pTargetObject) const {
    if (pTurret->getOwner() == getHouse() && pTurret->isActive()) {
        const_cast<TurretBase*>(pTurret)->doAttackObject(pTargetObject);
    } else {
        logWarn("The player '%s' tries to attack with a turret he doesn't own or that is inactive!\n", playername.c_str());
        return;
    }
}

void Player::doMove2Pos(const UnitBase* pUnit, int x, int y, bool bForced) const {
    if (pUnit->getOwner() == getHouse() && pUnit->isActive()) {
        const_cast<UnitBase*>(pUnit)->doMove2Pos(context_, x, y, bForced);
    } else {
        logWarn("The player '%s' tries to move a unit he doesn't own or that is inactive!\n", playername.c_str());
        return;
    }
}

void Player::doMove2Object(const UnitBase* pUnit, const ObjectBase* pTargetObject) const {
    if (pUnit->getOwner() == getHouse() && pUnit->isActive()) {
        const_cast<UnitBase*>(pUnit)->doMove2Object(context_, pTargetObject);
    } else {
        logWarn("The player '%s' tries to move a unit (to an object) he doesn't own or that is inactive!\n", playername.c_str());
        return;
    }
}

void Player::doAttackPos(const UnitBase* pUnit, int x, int y, bool bForced) const {
    if (pUnit->getOwner() == getHouse() && pUnit->isActive()) {
        const_cast<UnitBase*>(pUnit)->doAttackPos(context_, x, y, bForced);
    } else {
        logWarn("The player '%s' tries to order a unit (to attack a position) he doesn't own or that is inactive!\n", playername.c_str());
        return;
    }
}

void Player::doAttackObject(const UnitBase* pUnit, const ObjectBase* pTargetObject,
                            bool bForced) const {
    if (pUnit->getOwner() == getHouse() && pUnit->isActive()) {
        const_cast<UnitBase*>(pUnit)->doAttackObject(context_, pTargetObject, bForced);
    } else {
        logWarn("The player '%s' tries to attack with a unit he doesn't own or that is inactive!\n", playername.c_str());
        return;
    }
}

void Player::doSetAttackMode(const UnitBase* pUnit, ATTACKMODE attackMode) const {
    if (pUnit->getOwner() == getHouse() && pUnit->isActive()) {
        const_cast<UnitBase*>(pUnit)->doSetAttackMode(context_, attackMode);
    } else {
        logWarn("The player '%s' tries to change the attack mode of a unit he doesn't own or that is inactive!\n", playername.c_str());
        return;
    }
}

void Player::doStartDevastate(const Devastator* pDevastator) const {
    if (pDevastator->getOwner() == getHouse() && pDevastator->isActive()) {
        const_cast<Devastator*>(pDevastator)->doStartDevastate();
    } else {
        logWarn("The player '%s' tries to devastate a devastator he doesn't own or that is inactive!\n", playername.c_str());
        return;
    }
}

void Player::doReturn(const Harvester* pHarvester) const {
    if (pHarvester->getOwner() == getHouse() && pHarvester->isActive()) {
        const_cast<Harvester*>(pHarvester)->doReturn();
    } else {
        logWarn("The player '%s' tries to return a harvester he doesn't own or that is inactive!\n", playername.c_str());
        return;
    }
}

void Player::doCaptureStructure(const InfantryBase* pInfantry, const StructureBase* pTargetStructure) const {
    if (pInfantry->getOwner() == getHouse() && pInfantry->isActive()) {
        const_cast<InfantryBase*>(pInfantry)->doCaptureStructure(context_, pTargetStructure);
    } else {
        logWarn("The player '%s' tries to capture with a unit he doesn't own or that is inactive!\n", playername.c_str());
        return;
    }
}

bool Player::doDeploy(const MCV* pMCV) const {
    if (pMCV->getOwner() == getHouse() && pMCV->isActive()) {
        return const_cast<MCV*>(pMCV)->doDeploy();
    }
    logWarn("The player '%s' tries to deploy a MCV he doesn't own or that is inactive!\n", playername.c_str());

    return false;
}

bool Player::doRequestCarryallDrop(const GroundUnit* pGroundUnit) const {
    if (pGroundUnit->getOwner() == getHouse() && pGroundUnit->isActive()) {
        return const_cast<GroundUnit*>(pGroundUnit)->requestCarryall(context_);
    }
    logWarn("The player '%s' tries request a carryall for a ground unit he doesn't own or that is inactive!\n",
            playername.c_str());

    return false;
}
