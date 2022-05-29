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

BuildItem::BuildItem() = default;

BuildItem::BuildItem(ItemID_enum itemID, int price) {
    itemID_ = itemID;
    price_  = price;
}

BuildItem::BuildItem(InputStream& stream) {
    load(stream);
}

void BuildItem::save(OutputStream& stream) const {
    stream.writeUint32(itemID_);
    stream.writeUint32(price_);
    stream.writeUint32(num_);
}

void BuildItem::load(InputStream& stream) {
    itemID_ = static_cast<ItemID_enum>(stream.readUint32());
    price_  = stream.readUint32();
    num_    = stream.readUint32();
}

ProductionQueueItem::ProductionQueueItem() = default;

ProductionQueueItem::ProductionQueueItem(ItemID_enum ItemID, uint32_t price) : itemID_(ItemID), price_(price) { }

ProductionQueueItem::ProductionQueueItem(InputStream& stream) {
    load(stream);
}

void ProductionQueueItem::save(OutputStream& stream) const {
    stream.writeUint32(itemID_);
    stream.writeUint32(price_);
}

void ProductionQueueItem::load(InputStream& stream) {
    itemID_ = static_cast<ItemID_enum>(stream.readUint32());
    price_  = stream.readUint32();
}

BuilderBase::BuilderBase(const BuilderBaseConstants& constants, uint32_t objectID, const ObjectInitializer& initializer)
    : StructureBase(constants, objectID, initializer) {
    BuilderBase::init();

    buildSpeedLimit_ = 1.0_fix;
}

BuilderBase::BuilderBase(const BuilderBaseConstants& constants, uint32_t objectID,
                         const ObjectStreamInitializer& initializer)
    : StructureBase(constants, objectID, initializer) {
    BuilderBase::init();

    auto& stream = initializer.stream();

    upgrading_       = stream.readBool();
    upgradeProgress_ = stream.readFixPoint();
    curUpgradeLev_   = stream.readUint8();

    bCurrentItemOnHold_  = stream.readBool();
    currentProducedItem_ = static_cast<ItemID_enum>(stream.readUint32());
    productionProgress_  = stream.readFixPoint();
    deployTimer_         = stream.readUint32();

    buildSpeedLimit_ = stream.readFixPoint();

    const int numProductionQueueItem = stream.readUint32();
    for (auto i = 0; i < numProductionQueueItem; i++) {
        currentProductionQueue_.emplace_back(stream);
    }

    const int numBuildItem = stream.readUint32();
    for (auto i = 0; i < numBuildItem; i++) {
        buildList_.emplace_back(stream);
    }
}

void BuilderBase::init() { }

BuilderBase::~BuilderBase() = default;

void BuilderBase::save(OutputStream& stream) const {
    StructureBase::save(stream);

    stream.writeBool(upgrading_);
    stream.writeFixPoint(upgradeProgress_);
    stream.writeUint8(curUpgradeLev_);

    stream.writeBool(bCurrentItemOnHold_);
    stream.writeUint32(currentProducedItem_);
    stream.writeFixPoint(productionProgress_);
    stream.writeUint32(deployTimer_);

    stream.writeFixPoint(buildSpeedLimit_);

    stream.writeUint32(currentProductionQueue_.size());
    for (const auto& queueItem : currentProductionQueue_) {
        queueItem.save(stream);
    }

    stream.writeUint32(buildList_.size());
    for (const auto& buildItem : buildList_) {
        buildItem.save(stream);
    }
}

std::unique_ptr<ObjectInterface> BuilderBase::getInterfaceContainer(const GameContext& context) {
    if ((dune::globals::pLocalHouse == owner_) || (dune::globals::debug)) {
        return Widget::create<BuilderInterface>(context, objectID_);
    }
    return Widget::create<DefaultObjectInterface>(context, objectID_);
}

void BuilderBase::insertItem(build_list_type::iterator& iter, ItemID_enum item_id, int price) {

    auto& buildItemList = getBuildList();

    if (iter != std::end(buildItemList)) {
        if (iter->itemID_ == item_id) {
            if (price != -1) {
                iter->price_ = price;
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

    if (iter == std::end(buildItemList) || iter->itemID_ != item_id)
        return;

    iter = buildItemList.erase(iter);

    // is this item currently produced?
    if (currentProducedItem_ == item_id) {
        owner_->returnCredits(productionProgress_);
        productionProgress_  = 0;
        currentProducedItem_ = ItemID_Invalid;
    }

    // remove from production list
    std::erase_if(currentProductionQueue_, [item_id](auto& it) { return it.itemID_ == item_id; });

    produceNextAvailableItem();
}

BuildItem* BuilderBase::getBuildItem(ItemID_enum itemID) {
    for (auto& buildItem : buildList_) {
        if (buildItem.itemID_ == itemID) {
            return &buildItem;
        }
    }
    return nullptr;
}

const BuildItem* BuilderBase::getBuildItem(ItemID_enum itemID) const {
    for (const auto& buildItem : buildList_) {
        if (buildItem.itemID_ == itemID) {
            return &buildItem;
        }
    }
    return nullptr;
}

bool BuilderBase::isWaitingToPlace() const {
    if ((currentProducedItem_ == ItemID_Invalid) || isUnit(currentProducedItem_)) {
        return false;
    }

    const auto* const tmp = getBuildItem(currentProducedItem_);
    if (tmp == nullptr) {
        return false;
    }
    return (productionProgress_ >= tmp->price_);
}

bool BuilderBase::isUnitLimitReached(ItemID_enum itemID) const {
    if ((currentProducedItem_ == ItemID_Invalid) || isStructure(currentProducedItem_)) {
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
    if (currentProducedItem_ == ItemID_Invalid)
        return;

    const auto* const tmp = getBuildItem(currentProducedItem_);

    if ((productionProgress_ < tmp->price_) && (!isOnHold()) && (!isUnitLimitReached(currentProducedItem_))
        && (owner_->getCredits() > 0)) {

        const FixPoint oldProgress = productionProgress_;

        const auto* const game = dune::globals::currentGame.get();

        if (game->getGameInitSettings().getGameOptions().instantBuild) {
            const FixPoint totalBuildCosts =
                game->objectData.data[currentProducedItem_][static_cast<int>(originalHouseID_)].price;
            const auto buildCosts = totalBuildCosts - productionProgress_;

            productionProgress_ += owner_->takeCredits(buildCosts);
        } else {

            const auto buildSpeed = std::min(getHealth() / getMaxHealth(), buildSpeedLimit_);
            const FixPoint totalBuildCosts =
                game->objectData.data[currentProducedItem_][static_cast<int>(originalHouseID_)].price;
            const auto totalBuildGameTicks =
                game->objectData.data[currentProducedItem_][static_cast<int>(originalHouseID_)].buildtime * 15;
            const auto buildCosts = totalBuildCosts / totalBuildGameTicks;

            productionProgress_ += owner_->takeCredits(buildCosts * buildSpeed);

            /* That was wrong. Build speed does not depend on power production
                if (getOwner()->hasPower() || (((currentGame->gameType == GameType::Campaign) || (currentGame->gameType
               == GameType::Skirmish)) && getOwner()->isAI())) {
                    //if not enough power, production is halved
                    ProductionProgress += owner->takeCredits(0.25_fix);
                } else {
                    ProductionProgress += owner->takeCredits(0.125_fix);
                }*/
        }

        if ((oldProgress == productionProgress_) && (owner_ == dune::globals::pLocalHouse)) {
            game->addToNewsTicker(_("Not enough money"));
        }

        if (productionProgress_ >= tmp->price_) {
            setWaitingToPlace();
        }
    }
}

void BuilderBase::doBuildRandom(const GameContext& context) {
    if (buildList_.empty())
        return;

    const auto item2Produce =
        std::next(buildList_.begin(), context.game.randomGen.rand(0, static_cast<int32_t>(buildList_.size()) - 1))
            ->itemID_;
    doProduceItem(item2Produce);
}

void BuilderBase::produceNextAvailableItem() {
    if (currentProductionQueue_.empty()) {
        currentProducedItem_ = ItemID_Invalid;
    } else {
        currentProducedItem_ = currentProductionQueue_.front().itemID_;
    }

    productionProgress_ = 0;
    bCurrentItemOnHold_ = false;
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
    auto iter = buildList_.begin();

    for (int i = 0; itemOrder[i] != ItemID_Invalid; i++) {

        const auto itemID2Add = itemOrder[i];

        const auto* const game = dune::globals::currentGame.get();

        const auto& objData = game->objectData.data[itemID2Add][static_cast<int>(originalHouseID_)];

        if (!objData.enabled || (objData.builder != itemID_) || (objData.upgradeLevel > curUpgradeLev_)
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
    if (currentProducedItem_ == ItemID_Invalid)
        return;

    if (owner_ == dune::globals::pLocalHouse) {
        using dune::globals::soundPlayer;

        if (isStructure(currentProducedItem_)) {
            soundPlayer->playVoice(Voice_enum::ConstructionComplete, getOwner()->getHouseID());
        } else if (isFlyingUnit(currentProducedItem_)) {
            soundPlayer->playVoice(Voice_enum::UnitLaunched, getOwner()->getHouseID());
        } else if (currentProducedItem_ == Unit_Harvester) {
            soundPlayer->playVoice(Voice_enum::HarvesterDeployed, getOwner()->getHouseID());
        } else {
            soundPlayer->playVoice(Voice_enum::UnitDeployed, getOwner()->getHouseID());
        }
    }

    if (isUnit(currentProducedItem_)) {
        // if its a unit
        deployTimer_ = MILLI2CYCLES(750);
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
    auto finishedItemID = currentProducedItem_;
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

    if (isUnit(currentProducedItem_) && (productionProgress_ >= getBuildItem(currentProducedItem_)->price_)) {
        deployTimer_--;
        if (deployTimer_ == 0) {
            produce_item(context);
        }
    }

    if (upgrading_) {
        const FixPoint totalUpgradePrice = getUpgradeCost(context);

        if (context.game.getGameInitSettings().getGameOptions().instantBuild) {
            const FixPoint upgradePriceLeft = totalUpgradePrice - upgradeProgress_;
            upgradeProgress_ += owner_->takeCredits(upgradePriceLeft);
        } else {
            const FixPoint totalUpgradeGameTicks = 30 * 100 / 5;
            upgradeProgress_ += owner_->takeCredits(totalUpgradePrice / totalUpgradeGameTicks);
        }

        if (upgradeProgress_ >= totalUpgradePrice) {
            upgrading_ = false;
            curUpgradeLev_++;
            updateBuildList();

            upgradeProgress_ = 0;
        }
    } else {
        updateProductionProgress();
    }

    return true;
}

void BuilderBase::removeBuiltItemFromProductionQueue() {
    productionProgress_ = 0;

    const auto currentBuildItemIter = std::ranges::find_if(buildList_, [&](BuildItem& buildItem) {
        return ((buildItem.itemID_ == currentProducedItem_) && (buildItem.num_ > 0));
    });

    if (currentBuildItemIter != buildList_.end()) {
        currentBuildItemIter->num_--;
    }

    deployTimer_ = 0;
    currentProductionQueue_.pop_front();
    produceNextAvailableItem();
}

void BuilderBase::handleUpgradeClick() {
    dune::globals::currentGame->getCommandManager().addCommand(
        Command(dune::globals::pLocalPlayer->getPlayerID(), CMDTYPE::CMD_BUILDER_UPGRADE, objectID_));
}

void BuilderBase::handleProduceItemClick(ItemID_enum itemID, bool multipleMode) {
    for (const auto& buildItem : buildList_) {
        if (buildItem.itemID_ == itemID) {
            if (dune::globals::currentGame->getGameInitSettings().getGameOptions().onlyOnePalace
                && (itemID == Structure_Palace)
                && ((buildItem.num_ > 0) || (owner_->getNumItems(Structure_Palace) > 0))) {
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
    if (upgrading_) {
        return false;
    }
    if (isAllowedToUpgrade() && (owner_->getCredits() >= getUpgradeCost(context))) {

        upgrading_ = true;

        upgradeProgress_ = 0;

        return true;
    }
    return false;
}

void BuilderBase::doProduceItem(ItemID_enum itemID, bool multipleMode) {
    for (BuildItem& buildItem : buildList_) {
        if (buildItem.itemID_ == itemID) {
            for (int i = 0; i < (multipleMode ? 5 : 1); i++) {
                if (dune::globals::currentGame->getGameInitSettings().getGameOptions().onlyOnePalace
                    && (itemID == Structure_Palace)
                    && ((buildItem.num_ > 0) || (owner_->getNumItems(Structure_Palace) > 0))) {
                    // only one palace allowed
                    return;
                }

                buildItem.num_++;
                currentProductionQueue_.emplace_back(itemID, buildItem.price_);
                if (currentProducedItem_ == ItemID_Invalid) {
                    productionProgress_  = 0;
                    currentProducedItem_ = itemID;
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
    for (auto& buildItem : buildList_) {
        if (buildItem.itemID_ == itemID) {
            for (auto i = 0; i < (multipleMode ? 5 : 1); i++) {
                if (buildItem.num_ > 0) {
                    buildItem.num_--;

                    bool bCancelCurrentItem = (itemID == currentProducedItem_);

                    const auto queueItemIter =
                        std::find_if(currentProductionQueue_.rbegin(), currentProductionQueue_.rend(),
                                     [&](ProductionQueueItem& queueItem) { return (queueItem.itemID_ == itemID); });

                    if (queueItemIter != currentProductionQueue_.rend()) {
                        if (buildItem.num_ == 0 && bCancelCurrentItem) {
                            owner_->returnCredits(productionProgress_);
                        } else {
                            bCancelCurrentItem = false;
                        }
                        currentProductionQueue_.erase(std::next(queueItemIter).base());
                    }

                    if (bCancelCurrentItem) {
                        deployTimer_ = 0;
                        produceNextAvailableItem();
                    }
                }
            }
            break;
        }
    }
}
