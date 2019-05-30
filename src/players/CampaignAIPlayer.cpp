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
#include <House.h>
#include <sand.h>
#include <Map.h>
#include <misc/Random.h>

#include <structures/ConstructionYard.h>
#include <structures/Palace.h>
#include <units/UnitBase.h>

#define AIUPDATEINTERVAL 50

static std::map<Uint32, int> buildPriorityMap = {
    { Unit_Carryall, 2 },
    { Unit_Ornithopter, 6 },
    { Unit_Infantry, 2 },
    { Unit_Troopers, 3 },
    { Unit_Soldier, 1 },
    { Unit_Trooper, 2 },
    { Unit_Saboteur, 0 },
    { Unit_Launcher, 8 },
    { Unit_Deviator, 3 },
    { Unit_Tank, 7 },
    { Unit_SiegeTank, 9 },
    { Unit_Devastator, 10 },
    { Unit_SonicTank, 7 },
    { Unit_Trike, 3 },
    { Unit_RaiderTrike, 4 },
    { Unit_Quad, 5 },
    { Unit_Harvester, 1 },
    { Unit_MCV, 1 }
};

static std::map<Uint32, int> targetPriorityMap = {
    { Unit_Carryall, 36 },
    { Unit_Ornithopter, 105 },
    { Unit_Infantry, 40 },
    { Unit_Troopers, 100 },
    { Unit_Soldier, 20 },
    { Unit_Trooper, 50 },
    { Unit_Saboteur, 700 },
    { Unit_Launcher, 250 },
    { Unit_Deviator, 225 },
    { Unit_Tank, 180 },
    { Unit_SiegeTank, 280 },
    { Unit_Devastator, 355 },
    { Unit_SonicTank, 190 },
    { Unit_Trike, 100 },
    { Unit_RaiderTrike, 115 },
    { Unit_Quad, 120 },
    { Unit_Harvester, 160 },
    { Unit_MCV, 160 },
    { Unit_Frigate, 0 },
    { Unit_Sandworm, 0 },
    { Structure_Slab1, 5 },
    { Structure_Slab4, 10 },
    { Structure_Palace, 400 },
    { Structure_LightFactory, 200 },
    { Structure_HeavyFactory, 600 },
    { Structure_HighTechFactory, 200 },
    { Structure_IX, 100 },
    { Structure_WOR, 175 },
    { Structure_ConstructionYard, 300 },
    { Structure_WindTrap, 300 },
    { Structure_Barracks, 100 },
    { Structure_StarPort, 250 },
    { Structure_Refinery, 300 },
    { Structure_RepairYard, 600 },
    { Structure_Wall, 30 },
    { Structure_GunTurret, 225 },
    { Structure_RocketTurret, 175 },
    { Structure_Silo, 150 },
    { Structure_Radar, 275 }
};


CampaignAIPlayer::CampaignAIPlayer(House* associatedHouse, const std::string& playername)
 : Player(associatedHouse, playername) {
}

CampaignAIPlayer::CampaignAIPlayer(InputStream& stream, House* associatedHouse) : Player(stream, associatedHouse) {
    CampaignAIPlayer::init();

    Uint32 numStructureInfo = stream.readUint32();
    for(Uint32 i = 0; i < numStructureInfo; i++) {
        structureQueue.emplace_back(stream);
    }
}

void CampaignAIPlayer::init() {
}


CampaignAIPlayer::~CampaignAIPlayer() = default;

void CampaignAIPlayer::save(OutputStream& stream) const {
    Player::save(stream);

    stream.writeUint32(structureQueue.size());
    for(const auto& structureInfo : structureQueue) {
        structureInfo.save(stream);
    }
}



void CampaignAIPlayer::update() {
    if( (getGameCycleCount() + getHouse()->getHouseID()) % AIUPDATEINTERVAL != 0) {
        // we are not updating this AI player this cycle
        return;
    }

    if(!getHouse()->hadDirectContactWithEnemy()) {
        // we are not doing anything until we had contact with the enemy
        return;
    }

    updateStructures();
    updateUnits();
}

void CampaignAIPlayer::onObjectWasBuilt(const ObjectBase* pObject) {
}

void CampaignAIPlayer::onDecrementStructures(int itemID, const Coord& location) {
    if(structureQueue.size() < 5) {
        structureQueue.emplace_back(itemID, location);
    }
}

void CampaignAIPlayer::onDamage(const ObjectBase* pObject, int damage, Uint32 damagerID) {
    if(!pObject->isAUnit() || !pObject->isRespondable()) {
        return;
    }
    const UnitBase* pUnit = static_cast<const UnitBase*>(pObject);

    const ObjectBase* pDamager = getObject(damagerID);
    if(!pDamager) {
        return;
    }

    if(pDamager->getOwner()->getTeamID() == pUnit->getOwner()->getTeamID()) {
        // do not respond to friendly fire
        return;
    }

    if(!pUnit->canAttack(pDamager)) {
        return;
    }

    if(!pUnit->hasATarget() || pUnit->getTarget()->getTarget() != pUnit) {
        // the unit has no target or the target is not targeting the unit
        doAttackObject(pUnit, pDamager, true);
    }
}

void CampaignAIPlayer::updateStructures() {
    for(const StructureBase* pStructure : getStructureList()) {
        if(pStructure->getOwner() != getHouse()) {
            continue;
        }

        if( pStructure->getItemID() == Structure_Palace) {
            const Palace* pPalace = static_cast<const Palace*>(pStructure);
            if(pPalace->isSpecialWeaponReady()){

                if(getHouse()->getHouseID() != HOUSE_HARKONNEN && getHouse()->getHouseID() != HOUSE_SARDAUKAR) {
                    doSpecialWeapon(pPalace);
                } else {
                    const House* pBestHouse = nullptr;

                    for(int i = 0; i < NUM_HOUSES; i++) {
                        const House* pHouse = getHouse(i);
                        if(!pHouse || pHouse->getTeamID() == getHouse()->getTeamID()) {
                            continue;
                        }

                        if(!pBestHouse) {
                            pBestHouse = pHouse;
                        } else if(pHouse->getNumStructures() > pBestHouse->getNumStructures()) {
                            pBestHouse = pHouse;
                        } else if(pBestHouse->getNumStructures() == 0 && (pHouse->getNumUnits() > pBestHouse->getNumUnits())) {
                            pBestHouse = pHouse;
                        }
                    }

                    if(pBestHouse) {
                        Coord target = pBestHouse->getNumStructures() > 0 ? pBestHouse->getCenterOfMainBase() : pBestHouse->getStrongestUnitPosition();
                        doLaunchDeathhand(pPalace, target.x, target.y);
                    }
                }
            }
        }

        if( pStructure->getHealth() < pStructure->getMaxHealth()/2 ) {
            if(!pStructure->isRepairing()) {
                doRepair(pStructure);
            }
            continue;
        }

        if(pStructure->isABuilder()) {
            const BuilderBase* pBuilder = static_cast<const BuilderBase*>(pStructure);
            if( pBuilder->getCurrentUpgradeLevel() < pBuilder->getMaxUpgradeLevel()) {
                if(!pStructure->isRepairing() && !pBuilder->isUpgrading()) {
                    doUpgrade(pBuilder);
                }
                continue;
            }

            if(pBuilder->getItemID() == Structure_ConstructionYard) {
                // rebuild the last five destroyed buildings
                if(pBuilder->isWaitingToPlace()) {
                    int itemID = pBuilder->getCurrentProducedItem();
                    for(auto iter = structureQueue.begin(); iter != structureQueue.end(); ++iter) {
                        if(iter->itemID == itemID) {
                            const auto location = iter->location;
                            Coord itemsize = getStructureSize(itemID);
                            const auto* pConstYard = static_cast<const ConstructionYard*>(pBuilder);
                            if(getMap().okayToPlaceStructure(location.x, location.y, itemsize.x, itemsize.y, false, pConstYard->getOwner())) {
                                doPlaceStructure(pConstYard, location.x, location.y);
                            } else if(itemID == Structure_Slab1) {
                                //forget about concrete
                                doCancelItem(pConstYard, Structure_Slab1);
                            } else if(itemID == Structure_Slab4) {
                                //forget about concrete
                                doCancelItem(pConstYard, Structure_Slab4);
                            } else {
                                //cancel item
                                doCancelItem(pConstYard, itemID);
                            }
                            structureQueue.erase(iter);
                            break;
                        }
                    }
                } else if(!structureQueue.empty() && (pBuilder->getProductionQueueSize() <= 0)) {
                    const StructureInfo& structureInfo = structureQueue.front();
                    if(pBuilder->isAvailableToBuild(structureInfo.itemID)) {
                        doSetBuildSpeedLimit(pBuilder, std::min(1.0_fix, ((getTechLevel()-1) * 20 + 95)/255.0_fix) );
                        doProduceItem(pBuilder, structureInfo.itemID);
                    } else {
                        // dequeue unavailable structures
                        structureQueue.erase(structureQueue.begin());
                    }
                }

            } else if(pBuilder->getItemID() != Structure_StarPort) {
                // build units

                if(pBuilder->getProductionQueueSize() >= 1) {
                    // already busy building something
                    continue;
                }

                Uint32 bestItemID = ItemID_Invalid;
                int bestItemPriority = 0;
                for(Uint32 currentItemID = ItemID_FirstID; currentItemID < ItemID_LastID; currentItemID++) {
                    if(!pBuilder->isAvailableToBuild(currentItemID)) {
                        continue;
                    }

                    if((currentItemID == Unit_Carryall) && getHouse()->hasCarryalls()) {
                        // build only one carryall
                        continue;
                    }

                    if((currentItemID == Unit_Harvester) || (currentItemID == Unit_MCV)) {
                        // never build harvesters or MCVs
                        continue;
                    }

                    auto buildPriorityIter = buildPriorityMap.find(currentItemID);
                    if(buildPriorityIter == buildPriorityMap.end()) {
                        continue;
                    }

                    if((getRandomGen().rand() % 4 == 0) || (buildPriorityIter->second > bestItemPriority)) {
                        // build with 25% chance or if higher priority
                        bestItemID = currentItemID;
                        bestItemPriority = buildPriorityIter->second;
                    }
                }

                if(bestItemID != ItemID_Invalid) {
                    doSetBuildSpeedLimit(pBuilder, std::min(1.0_fix, ((getTechLevel()-1) * 20 + 95)/255.0_fix) );
                    doProduceItem(pBuilder, bestItemID);
                }
            }
        }
    }
}

void CampaignAIPlayer::updateUnits() {
    for(const UnitBase* pUnit : getUnitList()) {
        if(pUnit->getOwner() != getHouse() || pUnit->wasForced() || !pUnit->isRespondable() || pUnit->isByScenario() || pUnit->hasATarget()) {
            continue;
        }

        if((pUnit->getItemID() == Unit_Harvester) || (pUnit->getItemID() == Unit_MCV) || (pUnit->getItemID() == Unit_Carryall) || (pUnit->getItemID() == Unit_Frigate)) {
            continue;
        }

        const ObjectBase* pBestCandidate = nullptr;
        int bestCandidatePriority = -1;
        for(const StructureBase* pCandidate : getStructureList()) {
            if(!pUnit->canAttack(pCandidate)) {
                continue;
            }

            int priority = calculateTargetPriority(pUnit, pCandidate);
            if(priority > bestCandidatePriority) {
                bestCandidatePriority = priority;
                pBestCandidate = pCandidate;
            }
        }

        for(const UnitBase* pCandidate : getUnitList()) {
            if(!pUnit->canAttack(pCandidate)) {
                continue;
            }

            int priority = calculateTargetPriority(pUnit, pCandidate);
            if(priority > bestCandidatePriority) {
                bestCandidatePriority = priority;
                pBestCandidate = pCandidate;
            }
        }

        if(pBestCandidate) {
            doAttackObject(pUnit, pBestCandidate, true);
        }
    }
}

int CampaignAIPlayer::calculateTargetPriority(const UnitBase* pUnit, const ObjectBase* pObject) {
    if (pUnit->getLocation().isInvalid() || pObject->getLocation().isInvalid()) {
        return 0;
    }

    int priority = targetPriorityMap[pObject->getItemID()];
    int distance = blockDistanceApprox(pUnit->getLocation(), pObject->getLocation());

    return (distance > 0) ? ( (priority / distance) + 1 ) : priority;
}
