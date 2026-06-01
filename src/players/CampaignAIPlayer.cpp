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

#include <players/CampaignAIPlayer.h>

#include "Game.h"
#include "mmath.h"
#include <House.h>
#include <Map.h>
#include <misc/Random.h>
#include <sand.h>

#include <structures/ConstructionYard.h>
#include <structures/Palace.h>
#include <units/UnitBase.h>

#include <gsl/gsl>

#include <algorithm>
#include <limits>
#include <unordered_map>

namespace {
inline constexpr auto AIUPDATEINTERVAL = 50;

const std::unordered_map<uint32_t, int> buildPriorityMap = {{Unit_Carryall, 2},
                                                            {Unit_Ornithopter, 6},
                                                            {Unit_Infantry, 2},
                                                            {Unit_Troopers, 3},
                                                            {Unit_Soldier, 1},
                                                            {Unit_Trooper, 2},
                                                            {Unit_Saboteur, 0},
                                                            {Unit_Launcher, 8},
                                                            {Unit_Deviator, 3},
                                                            {Unit_Tank, 7},
                                                            {Unit_SiegeTank, 9},
                                                            {Unit_Devastator, 10},
                                                            {Unit_SonicTank, 7},
                                                            {Unit_Trike, 3},
                                                            {Unit_RaiderTrike, 4},
                                                            {Unit_Quad, 5},
                                                            {Unit_Harvester, 1},
                                                            {Unit_MCV, 1}};

const std::unordered_map<uint32_t, int> targetPriorityMap = {{Unit_Carryall, 36},
                                                             {Unit_Ornithopter, 105},
                                                             {Unit_Infantry, 40},
                                                             {Unit_Troopers, 100},
                                                             {Unit_Soldier, 20},
                                                             {Unit_Trooper, 50},
                                                             {Unit_Saboteur, 700},
                                                             {Unit_Launcher, 250},
                                                             {Unit_Deviator, 225},
                                                             {Unit_Tank, 180},
                                                             {Unit_SiegeTank, 280},
                                                             {Unit_Devastator, 355},
                                                             {Unit_SonicTank, 190},
                                                             {Unit_Trike, 100},
                                                             {Unit_RaiderTrike, 115},
                                                             {Unit_Quad, 120},
                                                             {Unit_Harvester, 160},
                                                             {Unit_MCV, 160},
                                                             {Unit_Frigate, 0},
                                                             {Unit_Sandworm, 0},
                                                             {Structure_Slab1, 5},
                                                             {Structure_Slab4, 10},
                                                             {Structure_Palace, 400},
                                                             {Structure_LightFactory, 200},
                                                             {Structure_HeavyFactory, 600},
                                                             {Structure_HighTechFactory, 200},
                                                             {Structure_IX, 100},
                                                             {Structure_WOR, 175},
                                                             {Structure_ConstructionYard, 300},
                                                             {Structure_WindTrap, 300},
                                                             {Structure_Barracks, 100},
                                                             {Structure_StarPort, 250},
                                                             {Structure_Refinery, 300},
                                                             {Structure_RepairYard, 600},
                                                             {Structure_Wall, 30},
                                                             {Structure_GunTurret, 225},
                                                             {Structure_RocketTurret, 175},
                                                             {Structure_Silo, 150},
                                                             {Structure_Radar, 275}};
} // namespace

CampaignAIPlayer::CampaignAIPlayer(const GameContext& context, House* associatedHouse, const std::string& playername,
                                   const Random& random)
    : Player(context, associatedHouse, playername, random) {
    initializeAttackTeamMinSizeFromScenario();
}

CampaignAIPlayer::CampaignAIPlayer(const GameContext& context, InputStream& stream, House* associatedHouse)
    : Player(context, stream, associatedHouse) {
    const auto numStructureInfo = stream.readUint32();
    for (uint32_t i = 0; i < numStructureInfo; i++) {
        structureQueue.emplace_back(stream);
    }

    attackTriggered              = stream.readBool();
    attackTeam.minSize           = stream.readUint32();
    attackTeam.cooldownCycles    = stream.readUint32();
    attackTeam.nextLaunchCycle   = stream.readUint32();
    attackTeamMinSizeInitialized = true;
}

CampaignAIPlayer::~CampaignAIPlayer() = default;

void CampaignAIPlayer::save(OutputStream& stream) const {
    Player::save(stream);

    stream.writeUint32(gsl::narrow<uint32_t>(structureQueue.size()));
    for (const auto& structureInfo : structureQueue) {
        structureInfo.save(stream);
    }

    stream.writeBool(attackTriggered);
    stream.writeUint32(attackTeam.minSize);
    stream.writeUint32(attackTeam.cooldownCycles);
    stream.writeUint32(attackTeam.nextLaunchCycle);
}

void CampaignAIPlayer::update() {
    if ((getGameCycleCount() + static_cast<int>(getHouse()->getHouseID())) % AIUPDATEINTERVAL != 0) {
        // we are not updating this AI player this cycle
        return;
    }

    if (!getHouse()->hadDirectContactWithEnemy()) {
        // we are not doing anything until we had contact with the enemy
        return;
    }

    updateStructures();
    updateUnits();
}

void CampaignAIPlayer::onObjectWasBuilt([[maybe_unused]] const ObjectBase* pObject) { }

void CampaignAIPlayer::onDecrementStructures(ItemID_enum itemID, const Coord& location) {
    if (structureQueue.size() < 5) {
        structureQueue.emplace_back(itemID, location);
    }
}

void CampaignAIPlayer::onDamage(const ObjectBase* pObject, [[maybe_unused]] int damage, uint32_t damagerID) {
    const auto* const pDamager = getObject(damagerID);
    if (!pDamager || !pDamager->getOwner()) {
        return;
    }

    if (pObject->isAStructure()) {
        if (pDamager->getOwner()->getTeamID() == pObject->getOwner()->getTeamID()) {
            return;
        }

        attackTriggered = true;
        scrambleUnitsAndDefend(pDamager);
        return;
    }

    if (!pObject->isAUnit() || !pObject->isRespondable())
        return;

    const auto* const pUnit = static_cast<const UnitBase*>(pObject);

    if (pDamager->getOwner()->getTeamID() == pUnit->getOwner()->getTeamID()) {
        // do not respond to friendly fire
        return;
    }

    attackTriggered = true;

    if (pUnit->getItemID() == Unit_Harvester) {
        scrambleUnitsAndDefend(pDamager);
    }

    if (!pUnit->canAttack(pDamager)) {
        return;
    }

    if (!pUnit->hasATarget() || pUnit->getTarget()->getTarget() != pUnit) {
        // the unit has no target or the target is not targeting the unit
        doAttackObject(pUnit, pDamager, true);
    }
}

void CampaignAIPlayer::updateStructures() {
    for (const auto* pStructure : getStructureList()) {
        if (pStructure->getOwner() != getHouse()) {
            continue;
        }

        if (pStructure->getItemID() == Structure_Palace) {
            const auto* pPalace = static_cast<const Palace*>(pStructure);
            if (pPalace->isSpecialWeaponReady()) {

                if (getHouse()->getHouseID() != HOUSETYPE::HOUSE_HARKONNEN
                    && getHouse()->getHouseID() != HOUSETYPE::HOUSE_SARDAUKAR) {
                    doSpecialWeapon(pPalace);
                } else {
                    const House* pBestHouse = nullptr;

                    context_.game.for_each_house([&](const auto& house) {
                        if (house.getTeamID() == getHouse()->getTeamID()) {
                            return;
                        }

                        if (!pBestHouse) {
                            pBestHouse = &house;
                        } else if (house.getNumStructures() > pBestHouse->getNumStructures()) {
                            pBestHouse = &house;
                        } else if (pBestHouse->getNumStructures() == 0
                                   && (house.getNumUnits() > pBestHouse->getNumUnits())) {
                            pBestHouse = &house;
                        }
                    });

                    if (pBestHouse) {
                        const Coord target = pBestHouse->getNumStructures() > 0
                                               ? pBestHouse->getCenterOfMainBase()
                                               : pBestHouse->getStrongestUnitPosition();
                        doLaunchDeathhand(pPalace, target.x, target.y);
                    }
                }
            }
        }

        if (pStructure->getHealth() < pStructure->getMaxHealth() / 2) {
            if (!pStructure->isRepairing()) {
                doRepair(pStructure);
            }
            continue;
        }

        if (pStructure->isABuilder()) {
            const auto* pBuilder = static_cast<const BuilderBase*>(pStructure);
            if (pBuilder->getCurrentUpgradeLevel() < pBuilder->getMaxUpgradeLevel()) {
                if (!pStructure->isRepairing() && !pBuilder->isUpgrading()) {
                    doUpgrade(pBuilder);
                }
                continue;
            }

            if (pBuilder->getItemID() == Structure_ConstructionYard) {
                // rebuild the last five destroyed buildings
                if (pBuilder->isWaitingToPlace()) {
                    const ItemID_enum itemID = pBuilder->getCurrentProducedItem();
                    for (auto iter = structureQueue.begin(); iter != structureQueue.end(); ++iter) {
                        if (iter->itemID == itemID) {
                            const auto location    = iter->location;
                            const Coord itemsize   = getStructureSize(itemID);
                            const auto* pConstYard = static_cast<const ConstructionYard*>(pBuilder);
                            if (getMap().okayToPlaceStructure(
                                    location.x, location.y, itemsize.x, itemsize.y, false, pConstYard->getOwner())) {
                                doPlaceStructure(pConstYard, location.x, location.y);
                            } else if (itemID == Structure_Slab1) {
                                // forget about concrete
                                doCancelItem(pConstYard, Structure_Slab1);
                            } else if (itemID == Structure_Slab4) {
                                // forget about concrete
                                doCancelItem(pConstYard, Structure_Slab4);
                            } else {
                                // cancel item
                                doCancelItem(pConstYard, itemID);
                            }
                            structureQueue.erase(iter);
                            break;
                        }
                    }
                } else if (!structureQueue.empty() && (pBuilder->getProductionQueueSize() <= 0)) {
                    const StructureInfo& structureInfo = structureQueue.front();
                    if (pBuilder->isAvailableToBuild(structureInfo.itemID)) {
                        doSetBuildSpeedLimit(pBuilder, std::min(1.0_fix, ((getTechLevel() - 1) * 20 + 95) / 255.0_fix));
                        doProduceItem(pBuilder, structureInfo.itemID);
                    } else {
                        // dequeue unavailable structures
                        structureQueue.erase(structureQueue.begin());
                    }
                }

            } else if (pBuilder->getItemID() != Structure_StarPort) {
                // build units

                if (pBuilder->getProductionQueueSize() >= 1) {
                    // already busy building something
                    continue;
                }

                auto bestItemID      = ItemID_Invalid;
                int bestItemPriority = 0;
                for (uint32_t currentItemID = ItemID_FirstID; currentItemID < ItemID_LastID; currentItemID++) {
                    if (!pBuilder->isAvailableToBuild(static_cast<ItemID_enum>(currentItemID))) {
                        continue;
                    }

                    if ((currentItemID == Unit_Carryall) && getHouse()->hasCarryalls()) {
                        // build only one carryall
                        continue;
                    }

                    if ((currentItemID == Unit_Harvester) || (currentItemID == Unit_MCV)) {
                        // never build harvesters or MCVs
                        continue;
                    }

                    auto buildPriorityIter = buildPriorityMap.find(currentItemID);
                    if (buildPriorityIter == buildPriorityMap.end()) {
                        continue;
                    }

                    if ((getRandomGen().rand() % 4 == 0) || (buildPriorityIter->second > bestItemPriority)) {
                        // build with 25% chance or if higher priority
                        bestItemID       = static_cast<ItemID_enum>(currentItemID);
                        bestItemPriority = buildPriorityIter->second;
                    }
                }

                if (bestItemID != ItemID_Invalid) {
                    doSetBuildSpeedLimit(pBuilder, std::min(1.0_fix, ((getTechLevel() - 1) * 20 + 95) / 255.0_fix));
                    doProduceItem(pBuilder, bestItemID);
                }
            }
        }
    }
}

void CampaignAIPlayer::scrambleUnitsAndDefend(const ObjectBase* pIntruder) {
    for (const UnitBase* pUnit : getUnitList()) {
        if (pUnit->getOwner() != getHouse() || pUnit->wasForced() || !pUnit->isRespondable()) {
            continue;
        }

        const auto itemID = pUnit->getItemID();
        if ((itemID == Unit_Harvester) || (itemID == Unit_MCV) || (itemID == Unit_Carryall) || (itemID == Unit_Frigate)
            || (itemID == Unit_Saboteur) || (itemID == Unit_Sandworm)) {
            continue;
        }

        const bool isArtillery = (itemID == Unit_Launcher) || (itemID == Unit_Deviator);
        if (pUnit->hasATarget() && !isArtillery) {
            continue;
        }

        doSetAttackMode(pUnit, HUNT);
        doAttackObject(pUnit, pIntruder, false);
    }
}

void CampaignAIPlayer::initializeAttackTeamMinSizeFromScenario() {
    if (attackTeamMinSizeInitialized) {
        return;
    }

    attackTeamMinSizeInitialized = true;

    uint32_t scenarioMinSize = std::numeric_limits<uint32_t>::max();
    for (const auto& aiTeam : getHouse()->getAITeams()) {
        if ((aiTeam.houseID != getHouse()->getHouseID()) || (aiTeam.minUnits <= 0)) {
            continue;
        }

        scenarioMinSize = std::min(scenarioMinSize, gsl::narrow<uint32_t>(aiTeam.minUnits));
    }

    if (scenarioMinSize != std::numeric_limits<uint32_t>::max()) {
        attackTeam.minSize = scenarioMinSize;
    }
}

void CampaignAIPlayer::updateUnits() {
    initializeAttackTeamMinSizeFromScenario();

    if (!attackTriggered) {
        return;
    }

    const auto now = getGameCycleCount();
    if (now < attackTeam.nextLaunchCycle) {
        return;
    }

    attackTeam.memberIds.clear();

    for (const UnitBase* pUnit : getUnitList()) {
        if (pUnit->getOwner() != getHouse() || pUnit->wasForced() || !pUnit->isRespondable() || pUnit->isByScenario()
            || pUnit->hasATarget()) {
            continue;
        }

        if ((pUnit->getItemID() == Unit_Harvester) || (pUnit->getItemID() == Unit_MCV)
            || (pUnit->getItemID() == Unit_Carryall) || (pUnit->getItemID() == Unit_Frigate)
            || (pUnit->getItemID() == Unit_Sandworm)) {
            continue;
        }

        attackTeam.memberIds.push_back(pUnit->getObjectID());
    }

    if (attackTeam.memberIds.size() < attackTeam.minSize) {
        return;
    }

    const ObjectBase* pBestCandidate = nullptr;
    int bestPriority                 = -1;

    auto considerTarget = [&](const ObjectBase* pCandidate) {
        int candidatePriority = -1;
        for (const auto unitID : attackTeam.memberIds) {
            const auto* pUnit = static_cast<const UnitBase*>(getObject(unitID));
            if (!pUnit || !pUnit->canAttack(pCandidate)) {
                continue;
            }

            candidatePriority = std::max(candidatePriority, calculateTargetPriority(pUnit, pCandidate));
        }

        if (candidatePriority > bestPriority) {
            bestPriority   = candidatePriority;
            pBestCandidate = pCandidate;
        }
    };

    for (const StructureBase* pCandidate : getStructureList()) {
        if (pCandidate->getOwner() == getHouse()) {
            continue;
        }
        considerTarget(pCandidate);
    }

    for (const UnitBase* pCandidate : getUnitList()) {
        if (pCandidate->getOwner() == getHouse()) {
            continue;
        }
        considerTarget(pCandidate);
    }

    if (!pBestCandidate) {
        return;
    }

    for (const auto unitID : attackTeam.memberIds) {
        const auto* pUnit = static_cast<const UnitBase*>(getObject(unitID));
        if (!pUnit) {
            continue;
        }
        doSetAttackMode(pUnit, HUNT);
        doAttackObject(pUnit, pBestCandidate, true);
    }

    attackTeam.nextLaunchCycle = now + attackTeam.cooldownCycles;
}

int CampaignAIPlayer::calculateTargetPriority(const UnitBase* pUnit, const ObjectBase* pObject) {
    if (pUnit->getLocation().isInvalid() || pObject->getLocation().isInvalid()) {
        return 0;
    }

    const auto it = targetPriorityMap.find(pObject->getItemID());

    const int priority = it == targetPriorityMap.end() ? 0 : it->second;
    const int distance = blockDistanceApprox(pUnit->getLocation(), pObject->getLocation());

    return (distance > 0) ? ((priority / distance) + 1) : priority;
}
