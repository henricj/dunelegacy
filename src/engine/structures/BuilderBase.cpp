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

#include "engine_mmath.h"

#include <structures/BuilderBase.h>

#include <Game.h>
#include <Map.h>
#include <House.h>
#include <units/UnitBase.h>

#include <players/HumanPlayer.h>

namespace Dune::Engine {

const ItemID_enum BuilderBase::itemOrder[] = {Structure_Slab4,
                                              Structure_Slab1,
                                              Structure_IX,
                                              Structure_StarPort,
                                              Structure_HighTechFactory,
                                              Structure_HeavyFactory,
                                              Structure_RocketTurret,
                                              Structure_RepairYard,
                                              Structure_GunTurret,
                                              Structure_WOR,
                                              Structure_Barracks,
                                              Structure_Wall,
                                              Structure_LightFactory,
                                              Structure_Silo,
                                              Structure_Radar,
                                              Structure_Refinery,
                                              Structure_WindTrap,
                                              Structure_Palace,
                                              Unit_SonicTank,
                                              Unit_Devastator,
                                              Unit_Deviator,
                                              Unit_Special,
                                              Unit_Launcher,
                                              Unit_SiegeTank,
                                              Unit_Tank,
                                              Unit_MCV,
                                              Unit_Harvester,
                                              Unit_Ornithopter,
                                              Unit_Carryall,
                                              Unit_Quad,
                                              Unit_RaiderTrike,
                                              Unit_Trike,
                                              Unit_Troopers,
                                              Unit_Trooper,
                                              Unit_Infantry,
                                              Unit_Soldier,
                                              Unit_Frigate,
                                              Unit_Sandworm,
                                              Unit_Saboteur,
                                              ItemID_Invalid};

BuilderBase::BuilderBase(const BuilderBaseConstants& constants, uint32_t objectID, const ObjectInitializer& initializer)
    : StructureBase(constants, objectID, initializer) {
    BuilderBase::init();

    buildSpeedLimit = 1.0_fix;
}

BuilderBase::BuilderBase(const BuilderBaseConstants& constants, uint32_t objectID,
                         const ObjectStreamInitializer& initializer)
    : StructureBase(constants, objectID, initializer) {
    BuilderBase::init();

    auto& stream = initializer.stream();

    upgrading       = stream.readBool();
    upgradeProgress = stream.readFixPoint();
    curUpgradeLev   = stream.readUint8();

    bCurrentItemOnHold  = stream.readBool();
    currentProducedItem = static_cast<ItemID_enum>(stream.readUint32());
    productionProgress  = stream.readFixPoint();
    deployTimer         = stream.readUint32();

    buildSpeedLimit = stream.readFixPoint();

    const int numProductionQueueItem = stream.readUint32();
    for(auto i = 0; i < numProductionQueueItem; i++) {
        currentProductionQueue.emplace_back(stream);
    }

    const int numBuildItem = stream.readUint32();
    for(auto i = 0; i < numBuildItem; i++) {
        buildList.emplace_back(stream);
    }
}

void BuilderBase::init() { }

BuilderBase::~BuilderBase() = default;

void BuilderBase::save(const Game& game, OutputStream& stream) const {
    StructureBase::save(game, stream);

    stream.writeBool(upgrading);
    stream.writeFixPoint(upgradeProgress);
    stream.writeUint8(curUpgradeLev);

    stream.writeBool(bCurrentItemOnHold);
    stream.writeUint32(currentProducedItem);
    stream.writeFixPoint(productionProgress);
    stream.writeUint32(deployTimer);

    stream.writeFixPoint(buildSpeedLimit);

    stream.writeUint32(currentProductionQueue.size());
    for(const auto& queueItem : currentProductionQueue) {
        queueItem.save(stream);
    }

    stream.writeUint32(buildList.size());
    for(const auto& buildItem : buildList) {
        buildItem.save(stream);
    }
}

void BuilderBase::insertItem(const Game& game, std::list<BuildItem>& buildItemList, std::list<BuildItem>::iterator& iter,
                             ItemID_enum itemID, int price) {
    if(iter != buildItemList.end()) {
        if(iter->itemID == itemID) {
            if(price != -1) { iter->price = price; }
            ++iter;
            return;
        }
    }

    if(price == -1) { price = game.getObjectData(itemID, originalHouseID).price; }

    buildItemList.insert(iter, BuildItem(itemID, price));
}

void BuilderBase::removeItem(std::list<BuildItem>& buildItemList, std::list<BuildItem>::iterator& iter,
                             ItemID_enum itemID) {
    if(iter == buildItemList.end() || iter->itemID != itemID) return;

    const auto iter2 = iter;
    ++iter;
    buildItemList.erase(iter2);

    // is this item currently produced?
    if(currentProducedItem == itemID) {
        owner->returnCredits(productionProgress);
        productionProgress  = 0;
        currentProducedItem = ItemID_Invalid;
    }

    // remove from production list
    auto iter3 = currentProductionQueue.begin();
    while(iter3 != currentProductionQueue.end()) {
        if(iter3->itemID == itemID) {
            const auto iter4 = iter3;
            ++iter3;
            currentProductionQueue.erase(iter4);
        } else {
            ++iter3;
        }
    }

    produceNextAvailableItem();
}

void BuilderBase::setOwner(House* no) { this->owner = no; }

bool BuilderBase::isWaitingToPlace() const {
    if((currentProducedItem == ItemID_Invalid) || isUnit(currentProducedItem)) { return false; }

    const auto* const tmp = getBuildItem(currentProducedItem);
    if(tmp == nullptr) { return false; }
    return (productionProgress >= tmp->price);
}

bool BuilderBase::isUnitLimitReached(ItemID_enum itemID) const {
    if((currentProducedItem == ItemID_Invalid) || isStructure(currentProducedItem)) { return false; }

    if(isInfantryUnit(itemID)) { return getOwner()->isInfantryUnitLimitReached(); }
    if(isFlyingUnit(itemID)) {

        return getOwner()->isAirUnitLimitReached();

    } else {

        return getOwner()->isGroundUnitLimitReached();
    }
}

void BuilderBase::updateProductionProgress(const Game& game) {
    if(currentProducedItem == ItemID_Invalid) return;

    auto* const tmp = getBuildItem(currentProducedItem);

    if((productionProgress < tmp->price) && (!isOnHold()) && (!isUnitLimitReached(currentProducedItem)) &&
       (owner->getCredits() > 0)) {

        const FixPoint oldProgress = productionProgress;

        const auto& item_data = game.getObjectData(currentProducedItem, originalHouseID);

        if(game.getGameInitSettings().getGameOptions().instantBuild) {
            const FixPoint totalBuildCosts = item_data.price;
            const auto buildCosts = totalBuildCosts - productionProgress;

            productionProgress += owner->takeCredits(buildCosts);
        } else {

            const auto     buildSpeed = std::min(getHealth() / getMaxHealth(game), buildSpeedLimit);
            const FixPoint totalBuildCosts = item_data.price;
            const auto totalBuildGameTicks = item_data.buildtime * 15;
            const auto buildCosts = totalBuildCosts / totalBuildGameTicks;

            productionProgress += owner->takeCredits(buildCosts * buildSpeed);

            /* That was wrong. Build speed does not depend on power production
                if (getOwner()->hasPower() || (((currentGame->gameType == GameType::Campaign) || (currentGame->gameType
               == GameType::Skirmish)) && getOwner()->isAI())) {
                    //if not enough power, production is halved
                    ProductionProgress += owner->takeCredits(0.25_fix);
                } else {
                    ProductionProgress += owner->takeCredits(0.125_fix);
                }*/
        }

        if(productionProgress >= tmp->price) { setWaitingToPlace(); }
    }
}

void BuilderBase::doBuildRandom(const GameContext& context) {
    if(buildList.empty()) return;

    const auto item2Produce =
        std::next(buildList.begin(), context.game.randomGen.rand(0, static_cast<int32_t>(buildList.size()) - 1))
            ->itemID;
    doProduceItem(context, item2Produce);
}

void BuilderBase::produceNextAvailableItem() {
    if(currentProductionQueue.empty()) {
        currentProducedItem = ItemID_Invalid;
    } else {
        currentProducedItem = currentProductionQueue.front().itemID;
    }

    productionProgress = 0;
    bCurrentItemOnHold = false;
}

int BuilderBase::getMaxUpgradeLevel(const Game& game) const {
    auto upgradeLevel = 0;

    for(int i = ItemID_FirstID; i <= ItemID_LastID; ++i) {
        const auto& objData = game.getObjectData(static_cast<ItemID_enum>(i), originalHouseID);

        if(objData.enabled && (objData.builder == static_cast<int>(itemID)) &&
           (objData.techLevel <= game.techLevel)) {
            upgradeLevel = std::max(upgradeLevel, static_cast<int>(objData.upgradeLevel));
        }
    }

    return upgradeLevel;
}

void BuilderBase::updateBuildList(const Game& game) {
    auto iter = buildList.begin();

    for(int i = 0; itemOrder[i] != ItemID_Invalid; i++) {

        const auto itemID2Add = itemOrder[i];

        const auto& objData = game.getObjectData(itemID2Add, originalHouseID);

        if(!objData.enabled || (objData.builder != static_cast<int>(itemID)) ||
           (objData.upgradeLevel > curUpgradeLev) || (objData.techLevel > game.techLevel)) {
            // first simple checks have rejected this item as being available for built in this builder
            removeItem(buildList, iter, itemID2Add);
        } else {

            // check if prerequisites are met
            auto bPrerequisitesMet = true;
            for(int itemID2Test = Structure_FirstID; itemID2Test <= Structure_LastID; itemID2Test++) {
                if(objData.prerequisiteStructuresSet[itemID2Test] &&
                   (owner->getNumItems(static_cast<ItemID_enum>(itemID2Test)) <= 0)) {
                    bPrerequisitesMet = false;
                    break;
                }
            }

            if(bPrerequisitesMet) {
                insertItem(game, buildList, iter, itemID2Add);
            } else {
                removeItem(buildList, iter, itemID2Add);
            }
        }
    }
}

void BuilderBase::setWaitingToPlace() {
    if(currentProducedItem == ItemID_Invalid) return;

    if(isUnit(currentProducedItem)) {
        // if its a unit
        deployTimer = MILLI2CYCLES(750);
    } else {
    }
}

void BuilderBase::unSetWaitingToPlace() { removeBuiltItemFromProductionQueue(); }

int BuilderBase::getUpgradeCost(const Game& game) const {
    return game.getObjectData(itemID, originalHouseID).price / 2;
}

void BuilderBase::produce_item(const GameContext& context) {
    auto finishedItemID = currentProducedItem;
    removeBuiltItemFromProductionQueue();

    auto num2Place = 1;

    if(finishedItemID == Unit_Infantry) {
        // make three
        finishedItemID = Unit_Soldier;
        num2Place      = 3;
    } else if(finishedItemID == Unit_Troopers) {
        // make three
        finishedItemID = Unit_Trooper;
        num2Place      = 3;
    }

    for(auto i = 0; i < num2Place; i++) {
        auto* newUnit = getOwner()->createUnit(finishedItemID);

        if(newUnit != nullptr) {
            Coord unitDestination;
            if(getOwner()->isAI() && ((newUnit->getItemID() == Unit_Carryall) ||
                                      (newUnit->getItemID() == Unit_Harvester) || (newUnit->getItemID() == Unit_MCV))) {
                // Don't want harvesters going to the rally point
                unitDestination = location;
            } else {
                unitDestination = destination;
            }

            const auto spot = context.map.findDeploySpot(newUnit, location, unitDestination, getStructureSize());

            newUnit->deploy(context, spot);

            if(unitDestination.isValid()) {
                newUnit->setGuardPoint(context, unitDestination);
                newUnit->ObjectBase::setDestination(context, unitDestination);
                newUnit->setAngle(destinationDrawnAngle(newUnit->getLocation(), newUnit->getDestination()));
            }

            // inform owner of its new unit
            newUnit->getOwner()->informWasBuilt(newUnit);
        }
    }
}

bool BuilderBase::update(const GameContext& context) {
    if(!StructureBase::update(context)) { return false; }

    if(isUnit(currentProducedItem) && (productionProgress >= getBuildItem(currentProducedItem)->price)) {
        deployTimer--;
        if(deployTimer == 0) { produce_item(context); }
    }

    if(upgrading) {
        const FixPoint totalUpgradePrice = getUpgradeCost(context.game);

        if(context.game.getGameInitSettings().getGameOptions().instantBuild) {
            const FixPoint upgradePriceLeft = totalUpgradePrice - upgradeProgress;
            upgradeProgress += owner->takeCredits(upgradePriceLeft);
        } else {
            const FixPoint totalUpgradeGameTicks = 30 * 100 / 5;
            upgradeProgress += owner->takeCredits(totalUpgradePrice / totalUpgradeGameTicks);
        }

        if(upgradeProgress >= totalUpgradePrice) {
            upgrading = false;
            curUpgradeLev++;
            updateBuildList(context.game);

            upgradeProgress = 0;
        }
    } else {
        updateProductionProgress(context.game);
    }

    return true;
}

void BuilderBase::removeBuiltItemFromProductionQueue() {
    productionProgress = 0;

    const auto currentBuildItemIter = std::find_if(buildList.begin(), buildList.end(), [&](BuildItem& buildItem) {
        return ((buildItem.itemID == currentProducedItem) && (buildItem.num > 0));
    });

    if(currentBuildItemIter != buildList.end()) { currentBuildItemIter->num--; }

    deployTimer = 0;
    currentProductionQueue.pop_front();
    produceNextAvailableItem();
}

bool BuilderBase::doUpgrade(const GameContext& context) {
    if(upgrading) { return false; }
    if(isAllowedToUpgrade(context.game) && (owner->getCredits() >= getUpgradeCost(context.game))) {

        upgrading = true;

        upgradeProgress = 0;

        return true;

    } else {

        return false;
    }
}

void BuilderBase::doProduceItem(const GameContext& context, ItemID_enum itemID, bool multipleMode) {
    for(BuildItem& buildItem : buildList) {
        if(buildItem.itemID == itemID) {
            for(int i = 0; i < (multipleMode ? 5 : 1); i++) {
                if(context.game.getGameInitSettings().getGameOptions().onlyOnePalace && (itemID == Structure_Palace) &&
                   ((buildItem.num > 0) || (owner->getNumItems(Structure_Palace) > 0))) {
                    // only one palace allowed
                    return;
                }

                buildItem.num++;
                currentProductionQueue.emplace_back(itemID, buildItem.price);
                if(currentProducedItem == ItemID_Invalid) {
                    productionProgress  = 0;
                    currentProducedItem = itemID;
                }
            }
            break;
        }
    }
}

void BuilderBase::doCancelItem(ItemID_enum itemID, bool multipleMode) {
    for(auto& buildItem : buildList) {
        if(buildItem.itemID == itemID) {
            for(auto i = 0; i < (multipleMode ? 5 : 1); i++) {
                if(buildItem.num > 0) {
                    buildItem.num--;

                    bool bCancelCurrentItem = (itemID == currentProducedItem);

                    const auto queueItemIter =
                        std::find_if(currentProductionQueue.rbegin(), currentProductionQueue.rend(),
                                     [&](ProductionQueueItem& queueItem) { return (queueItem.itemID == itemID); });

                    if(queueItemIter != currentProductionQueue.rend()) {
                        if(buildItem.num == 0 && bCancelCurrentItem) {
                            owner->returnCredits(productionProgress);
                        } else {
                            bCancelCurrentItem = false;
                        }
                        currentProductionQueue.erase(std::next(queueItemIter).base());
                    }

                    if(bCancelCurrentItem) {
                        deployTimer = 0;
                        produceNextAvailableItem();
                    }
                }
            }
            break;
        }
    }
}

} // namespace Dune::Engine
