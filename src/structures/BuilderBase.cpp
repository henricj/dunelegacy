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

#include <structures/BuilderBase.h>

#include <FileClasses/TextManager.h>

#include <globals.h>

#include "mmath.h"
#include <House.h>
#include <Map.h>
#include <SoundPlayer.h>

#include <units/UnitBase.h>

#include <players/HumanPlayer.h>

#include <GUI/ObjectInterfaces/BuilderInterface.h>

constexpr ItemID_enum BuilderBase::itemOrder[] = {Structure_Slab4,
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
    for (auto i = 0; i < numProductionQueueItem; i++) {
        currentProductionQueue.emplace_back(stream);
    }

    const int numBuildItem = stream.readUint32();
    for (auto i = 0; i < numBuildItem; i++) {
        buildList.emplace_back(stream);
    }
}

void BuilderBase::init() { }

BuilderBase::~BuilderBase() = default;

void BuilderBase::save(OutputStream& stream) const {
    StructureBase::save(stream);

    stream.writeBool(upgrading);
    stream.writeFixPoint(upgradeProgress);
    stream.writeUint8(curUpgradeLev);

    stream.writeBool(bCurrentItemOnHold);
    stream.writeUint32(currentProducedItem);
    stream.writeFixPoint(productionProgress);
    stream.writeUint32(deployTimer);

    stream.writeFixPoint(buildSpeedLimit);

    stream.writeUint32(currentProductionQueue.size());
    for (const auto& queueItem : currentProductionQueue) {
        queueItem.save(stream);
    }

    stream.writeUint32(buildList.size());
    for (const auto& buildItem : buildList) {
        buildItem.save(stream);
    }
}

std::unique_ptr<ObjectInterface> BuilderBase::getInterfaceContainer(const GameContext& context) {
    if ((dune::globals::pLocalHouse == owner_) || (dune::globals::debug)) {
        return BuilderInterface::create(context, objectID_);
    }
    return DefaultObjectInterface::create(context, objectID_);
}

void BuilderBase::insertItem(build_list_type::iterator& iter, ItemID_enum item_id, int price) {

    auto& buildItemList = getBuildList();

    if (iter != std::end(buildItemList)) {
        if (iter->itemID == item_id) {
            if (price != -1) {
                iter->price = price;
            }
            ++iter;
            return;
        }
    }

    if (price == -1) {
        price = dune::globals::currentGame->objectData.data[item_id][static_cast<int>(originalHouseID_)].price;
    }

    iter = buildItemList.emplace(iter, item_id, price);

    ++iter;
}

void BuilderBase::removeItem(build_list_type::iterator& iter, ItemID_enum item_id) {
    auto& buildItemList = getBuildList();

    if (iter == std::end(buildItemList) || iter->itemID != item_id)
        return;

    iter = buildItemList.erase(iter);

    // is this item currently produced?
    if (currentProducedItem == item_id) {
        owner_->returnCredits(productionProgress);
        productionProgress  = 0;
        currentProducedItem = ItemID_Invalid;
    }

    // remove from production list
    std::erase_if(currentProductionQueue, [item_id](auto& it) { return it.itemID == item_id; });

    produceNextAvailableItem();
}

void BuilderBase::setOwner(House* no) {
    this->owner_ = no;
}

bool BuilderBase::isWaitingToPlace() const {
    if ((currentProducedItem == ItemID_Invalid) || isUnit(currentProducedItem)) {
        return false;
    }

    const auto* const tmp = getBuildItem(currentProducedItem);
    if (tmp == nullptr) {
        return false;
    }
    return (productionProgress >= tmp->price);
}

bool BuilderBase::isUnitLimitReached(ItemID_enum itemID) const {
    if ((currentProducedItem == ItemID_Invalid) || isStructure(currentProducedItem)) {
        return false;
    }

    if (isInfantryUnit(itemID)) {
        return getOwner()->isInfantryUnitLimitReached();
    }
    if (isFlyingUnit(itemID)) {

        return getOwner()->isAirUnitLimitReached();
    }
    return getOwner()->isGroundUnitLimitReached();
}

void BuilderBase::updateProductionProgress() {
    if (currentProducedItem == ItemID_Invalid)
        return;

    const auto* const tmp = getBuildItem(currentProducedItem);

    if ((productionProgress < tmp->price) && (!isOnHold()) && (!isUnitLimitReached(currentProducedItem))
        && (owner_->getCredits() > 0)) {

        const FixPoint oldProgress = productionProgress;

        const auto* const game = dune::globals::currentGame.get();

        if (game->getGameInitSettings().getGameOptions().instantBuild) {
            const FixPoint totalBuildCosts =
                game->objectData.data[currentProducedItem][static_cast<int>(originalHouseID_)].price;
            const auto buildCosts = totalBuildCosts - productionProgress;

            productionProgress += owner_->takeCredits(buildCosts);
        } else {

            const auto buildSpeed = std::min(getHealth() / getMaxHealth(), buildSpeedLimit);
            const FixPoint totalBuildCosts =
                game->objectData.data[currentProducedItem][static_cast<int>(originalHouseID_)].price;
            const auto totalBuildGameTicks =
                game->objectData.data[currentProducedItem][static_cast<int>(originalHouseID_)].buildtime * 15;
            const auto buildCosts = totalBuildCosts / totalBuildGameTicks;

            productionProgress += owner_->takeCredits(buildCosts * buildSpeed);

            /* That was wrong. Build speed does not depend on power production
                if (getOwner()->hasPower() || (((currentGame->gameType == GameType::Campaign) || (currentGame->gameType
               == GameType::Skirmish)) && getOwner()->isAI())) {
                    //if not enough power, production is halved
                    ProductionProgress += owner->takeCredits(0.25_fix);
                } else {
                    ProductionProgress += owner->takeCredits(0.125_fix);
                }*/
        }

        if ((oldProgress == productionProgress) && (owner_ == dune::globals::pLocalHouse)) {
            game->addToNewsTicker(_("Not enough money"));
        }

        if (productionProgress >= tmp->price) {
            setWaitingToPlace();
        }
    }
}

void BuilderBase::doBuildRandom(const GameContext& context) {
    if (buildList.empty())
        return;

    const auto item2Produce =
        std::next(buildList.begin(), context.game.randomGen.rand(0, static_cast<int32_t>(buildList.size()) - 1))->itemID;
    doProduceItem(item2Produce);
}

void BuilderBase::produceNextAvailableItem() {
    if (currentProductionQueue.empty()) {
        currentProducedItem = ItemID_Invalid;
    } else {
        currentProducedItem = currentProductionQueue.front().itemID;
    }

    productionProgress = 0;
    bCurrentItemOnHold = false;
}

int BuilderBase::getMaxUpgradeLevel() const {
    auto upgradeLevel = 0;

    for (int i = ItemID_FirstID; i <= ItemID_LastID; ++i) {
        const auto* const game = dune::globals::currentGame.get();

        const auto& objData = game->objectData.data[i][static_cast<int>(originalHouseID_)];

        if (objData.enabled && (objData.builder == itemID_) && (objData.techLevel <= game->techLevel)) {
            upgradeLevel = std::max(upgradeLevel, static_cast<int>(objData.upgradeLevel));
        }
    }

    return upgradeLevel;
}

void BuilderBase::updateBuildList() {
    auto iter = buildList.begin();

    for (int i = 0; itemOrder[i] != ItemID_Invalid; i++) {

        const auto itemID2Add = itemOrder[i];

        const auto* const game = dune::globals::currentGame.get();

        const auto& objData = game->objectData.data[itemID2Add][static_cast<int>(originalHouseID_)];

        if (!objData.enabled || (objData.builder != itemID_) || (objData.upgradeLevel > curUpgradeLev)
            || (objData.techLevel > game->techLevel)) {
            // first simple checks have rejected this item as being available for built in this builder
            removeItem(iter, itemID2Add);
        } else {

            // check if prerequisites are met
            auto bPrerequisitesMet = true;
            for (int itemID2Test = Structure_FirstID; itemID2Test <= Structure_LastID; itemID2Test++) {
                if (objData.prerequisiteStructuresSet[itemID2Test]
                    && (owner_->getNumItems(static_cast<ItemID_enum>(itemID2Test)) <= 0)) {
                    bPrerequisitesMet = false;
                    break;
                }
            }

            if (bPrerequisitesMet) {
                insertItem(iter, itemID2Add);
            } else {
                removeItem(iter, itemID2Add);
            }
        }
    }
}

void BuilderBase::setWaitingToPlace() {
    if (currentProducedItem == ItemID_Invalid)
        return;

    if (owner_ == dune::globals::pLocalHouse) {
        using dune::globals::soundPlayer;

        if (isStructure(currentProducedItem)) {
            soundPlayer->playVoice(Voice_enum::ConstructionComplete, getOwner()->getHouseID());
        } else if (isFlyingUnit(currentProducedItem)) {
            soundPlayer->playVoice(Voice_enum::UnitLaunched, getOwner()->getHouseID());
        } else if (currentProducedItem == Unit_Harvester) {
            soundPlayer->playVoice(Voice_enum::HarvesterDeployed, getOwner()->getHouseID());
        } else {
            soundPlayer->playVoice(Voice_enum::UnitDeployed, getOwner()->getHouseID());
        }
    }

    if (isUnit(currentProducedItem)) {
        // if its a unit
        deployTimer = MILLI2CYCLES(750);
    } else {
        // its a structure
        if (owner_ == dune::globals::pLocalHouse) {
            dune::globals::currentGame->addToNewsTicker(_("@DUNE.ENG|51#Construction is complete"));
        }
    }
}

void BuilderBase::unSetWaitingToPlace() {
    removeBuiltItemFromProductionQueue();
}

int BuilderBase::getUpgradeCost(const GameContext& context) const {
    return context.game.objectData.data[itemID_][static_cast<int>(originalHouseID_)].price / 2;
}

void BuilderBase::produce_item(const GameContext& context) {
    auto finishedItemID = currentProducedItem;
    removeBuiltItemFromProductionQueue();

    auto num2Place = 1;

    if (finishedItemID == Unit_Infantry) {
        // make three
        finishedItemID = Unit_Soldier;
        num2Place      = 3;
    } else if (finishedItemID == Unit_Troopers) {
        // make three
        finishedItemID = Unit_Trooper;
        num2Place      = 3;
    }

    for (auto i = 0; i < num2Place; i++) {
        auto* newUnit = getOwner()->createUnit(finishedItemID);

        if (newUnit != nullptr) {
            Coord unitDestination;
            if (getOwner()->isAI()
                && ((newUnit->getItemID() == Unit_Carryall) || (newUnit->getItemID() == Unit_Harvester)
                    || (newUnit->getItemID() == Unit_MCV))) {
                // Don't want harvesters going to the rally point
                unitDestination = location_;
            } else {
                unitDestination = destination_;
            }

            const auto spot = context.map.findDeploySpot(newUnit, location_, unitDestination, getStructureSize());

            newUnit->deploy(context, spot);

            if (unitDestination.isValid()) {
                newUnit->setGuardPoint(unitDestination);
                newUnit->ObjectBase::setDestination(unitDestination);
                newUnit->setAngle(destinationDrawnAngle(newUnit->getLocation(), newUnit->getDestination()));
            }

            // inform owner of its new unit
            newUnit->getOwner()->informWasBuilt(newUnit);
        }
    }
}

bool BuilderBase::update(const GameContext& context) {
    if (!StructureBase::update(context)) {
        return false;
    }

    if (isUnit(currentProducedItem) && (productionProgress >= getBuildItem(currentProducedItem)->price)) {
        deployTimer--;
        if (deployTimer == 0) {
            produce_item(context);
        }
    }

    if (upgrading) {
        const FixPoint totalUpgradePrice = getUpgradeCost(context);

        if (context.game.getGameInitSettings().getGameOptions().instantBuild) {
            const FixPoint upgradePriceLeft = totalUpgradePrice - upgradeProgress;
            upgradeProgress += owner_->takeCredits(upgradePriceLeft);
        } else {
            const FixPoint totalUpgradeGameTicks = 30 * 100 / 5;
            upgradeProgress += owner_->takeCredits(totalUpgradePrice / totalUpgradeGameTicks);
        }

        if (upgradeProgress >= totalUpgradePrice) {
            upgrading = false;
            curUpgradeLev++;
            updateBuildList();

            upgradeProgress = 0;
        }
    } else {
        updateProductionProgress();
    }

    return true;
}

void BuilderBase::removeBuiltItemFromProductionQueue() {
    productionProgress = 0;

    const auto currentBuildItemIter = std::ranges::find_if(buildList, [&](BuildItem& buildItem) {
        return ((buildItem.itemID == currentProducedItem) && (buildItem.num > 0));
    });

    if (currentBuildItemIter != buildList.end()) {
        currentBuildItemIter->num--;
    }

    deployTimer = 0;
    currentProductionQueue.pop_front();
    produceNextAvailableItem();
}

void BuilderBase::handleUpgradeClick() {
    dune::globals::currentGame->getCommandManager().addCommand(
        Command(dune::globals::pLocalPlayer->getPlayerID(), CMDTYPE::CMD_BUILDER_UPGRADE, objectID_));
}

void BuilderBase::handleProduceItemClick(ItemID_enum itemID, bool multipleMode) {
    for (const auto& buildItem : buildList) {
        if (buildItem.itemID == itemID) {
            if (dune::globals::currentGame->getGameInitSettings().getGameOptions().onlyOnePalace
                && (itemID == Structure_Palace)
                && ((buildItem.num > 0) || (owner_->getNumItems(Structure_Palace) > 0))) {
                // only one palace allowed
                dune::globals::soundPlayer->playSound(Sound_enum::Sound_InvalidAction);
                return;
            }
        }
    }

    dune::globals::currentGame->getCommandManager().addCommand(Command(dune::globals::pLocalPlayer->getPlayerID(),
                                                                       CMDTYPE::CMD_BUILDER_PRODUCEITEM, objectID_,
                                                                       itemID, static_cast<uint32_t>(multipleMode)));
}

void BuilderBase::handleCancelItemClick(ItemID_enum itemID, bool multipleMode) {
    dune::globals::currentGame->getCommandManager().addCommand(Command(dune::globals::pLocalPlayer->getPlayerID(),
                                                                       CMDTYPE::CMD_BUILDER_CANCELITEM, objectID_,
                                                                       itemID, static_cast<uint32_t>(multipleMode)));
}

void BuilderBase::handleSetOnHoldClick(bool OnHold) {
    dune::globals::currentGame->getCommandManager().addCommand(Command(dune::globals::pLocalPlayer->getPlayerID(),
                                                                       CMDTYPE::CMD_BUILDER_SETONHOLD, objectID_,
                                                                       static_cast<uint32_t>(OnHold)));
}

bool BuilderBase::doUpgrade(const GameContext& context) {
    if (upgrading) {
        return false;
    }
    if (isAllowedToUpgrade() && (owner_->getCredits() >= getUpgradeCost(context))) {

        upgrading = true;

        upgradeProgress = 0;

        return true;
    }
    return false;
}

void BuilderBase::doProduceItem(ItemID_enum itemID, bool multipleMode) {
    for (BuildItem& buildItem : buildList) {
        if (buildItem.itemID == itemID) {
            for (int i = 0; i < (multipleMode ? 5 : 1); i++) {
                if (dune::globals::currentGame->getGameInitSettings().getGameOptions().onlyOnePalace
                    && (itemID == Structure_Palace)
                    && ((buildItem.num > 0) || (owner_->getNumItems(Structure_Palace) > 0))) {
                    // only one palace allowed
                    return;
                }

                buildItem.num++;
                currentProductionQueue.emplace_back(itemID, buildItem.price);
                if (currentProducedItem == ItemID_Invalid) {
                    productionProgress  = 0;
                    currentProducedItem = itemID;
                }

                if (dune::globals::pLocalHouse == getOwner()) {
                    dune::globals::pLocalPlayer->onProduceItem(itemID);
                }
            }
            break;
        }
    }
}

void BuilderBase::doCancelItem(ItemID_enum itemID, bool multipleMode) {
    for (auto& buildItem : buildList) {
        if (buildItem.itemID == itemID) {
            for (auto i = 0; i < (multipleMode ? 5 : 1); i++) {
                if (buildItem.num > 0) {
                    buildItem.num--;

                    bool bCancelCurrentItem = (itemID == currentProducedItem);

                    const auto queueItemIter =
                        std::find_if(currentProductionQueue.rbegin(), currentProductionQueue.rend(),
                                     [&](ProductionQueueItem& queueItem) { return (queueItem.itemID == itemID); });

                    if (queueItemIter != currentProductionQueue.rend()) {
                        if (buildItem.num == 0 && bCancelCurrentItem) {
                            owner_->returnCredits(productionProgress);
                        } else {
                            bCancelCurrentItem = false;
                        }
                        currentProductionQueue.erase(std::next(queueItemIter).base());
                    }

                    if (bCancelCurrentItem) {
                        deployTimer = 0;
                        produceNextAvailableItem();
                    }
                }
            }
            break;
        }
    }
}
