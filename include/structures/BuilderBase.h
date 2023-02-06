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

#ifndef BUILDERBASE_H
#define BUILDERBASE_H

#include <structures/StructureBase.h>

#include <data.h>

#include <algorithm>
#include <deque>
#include <memory>

class BuildItem final {
public:
    BuildItem();

    BuildItem(ItemID_enum itemID, int price);

    BuildItem(InputStream& stream);

    void save(OutputStream& stream) const;

    void load(InputStream& stream);

    ItemID_enum itemID_ = ItemID_Invalid;
    uint32_t price_     = 0;
    uint32_t num_       = 0;
};

class ProductionQueueItem final {
public:
    ProductionQueueItem();

    ProductionQueueItem(ItemID_enum ItemID, uint32_t price);

    ProductionQueueItem(InputStream& stream);

    void save(OutputStream& stream) const;

    void load(InputStream& stream);

    ItemID_enum itemID_ = ItemID_Invalid;
    ;
    uint32_t price_ = 0;
};

class BuilderBaseConstants : public StructureBaseConstants {
public:
    constexpr explicit BuilderBaseConstants(ItemID_enum itemID, Coord structureSize)
        : StructureBaseConstants{itemID, structureSize} {
        aBuilder_ = true;
    }
};

class BuilderBase : public StructureBase {
protected:
    BuilderBase(const BuilderBaseConstants& constants, uint32_t objectID, const ObjectInitializer& initializer);
    BuilderBase(const BuilderBaseConstants& constants, uint32_t objectID, const ObjectStreamInitializer& initializer);

public:
    using parent                = StructureBase;
    using build_list_type       = std::vector<BuildItem>;
    using production_queue_type = std::deque<ProductionQueueItem>;

    ~BuilderBase() override = 0;

    BuilderBase(const BuilderBase&)            = delete;
    BuilderBase(BuilderBase&&)                 = delete;
    BuilderBase& operator=(const BuilderBase&) = delete;
    BuilderBase& operator=(BuilderBase&&)      = delete;

    void save(OutputStream& stream) const override;

    std::unique_ptr<ObjectInterface> getInterfaceContainer(const GameContext& context) override;

    void setOriginalHouseID(HOUSETYPE i) override {
        StructureBase::setOriginalHouseID(i);
        updateBuildList();
    }

    /**
        This method returns the maximum number of upgrades available in this building.
        \return the maximum number of upgrades available (0 if none)
    */
    int getMaxUpgradeLevel() const;

    /**
        This method checks what is available for purchase in this builder. It shall
        modify buildList appropriately.
    */
    virtual void updateBuildList();

    void setWaitingToPlace();
    void unSetWaitingToPlace();

    int getBuildListSize() const { return buildList_.size(); }

    int getProductionQueueSize() const { return currentProductionQueue_.size(); }

    /**
        Updates this builder.
        \return true if this object still exists, false if it was destroyed
    */
    bool update(const GameContext& context) override;

    virtual void handleUpgradeClick();
    virtual void handleProduceItemClick(ItemID_enum itemID, bool multipleMode = false);
    virtual void handleCancelItemClick(ItemID_enum itemID, bool multipleMode = false);
    virtual void handleSetOnHoldClick(bool OnHold);

    /**
        Start upgrading this builder if possible.
        \return true if upgrading was started, false if not possible or already upgrading
    */
    virtual bool doUpgrade(const GameContext& context);

    /**
        Start production of the specified item.
        \param  itemID          the item to produce
        \param  multipleMode    false = 1 item, true = 5 items
    */
    virtual void doProduceItem(ItemID_enum itemID, bool multipleMode = false);

    /**
        Cancel production of the specified item.
        \param  itemID          the item to cancel
        \param  multipleMode    false = 1 item, true = 5 items
    */
    virtual void doCancelItem(ItemID_enum itemID, bool multipleMode = false);

    /**
        Sets the currently produced item on hold or continues production.
        \param  bOnHold         true = hold production; false = resume production
    */
    void doSetOnHold(bool bOnHold) { bCurrentItemOnHold_ = bOnHold; }

    /**
        Start building a random item in this builder.
    */
    virtual void doBuildRandom(const GameContext& context);

    /**
        Limit the build speed of this builder to the given argument. The build speed can be limited by the AI
        to make it weaker, e.g. the campaign AI adjusts the build speed to the mission number
        \param newBuildSpeedLimit   the new limit; must be between 0 and 1
    */
    void doSetBuildSpeedLimit(FixPoint newBuildSpeedLimit) {
        buildSpeedLimit_ = std::max(0.0_fix, std::min(1.0_fix, newBuildSpeedLimit));
    }

    bool isUpgrading() const noexcept { return upgrading_; }
    bool isAllowedToUpgrade() const { return (curUpgradeLev_ < getMaxUpgradeLevel()); }
    int getCurrentUpgradeLevel() const noexcept { return curUpgradeLev_; }
    int getUpgradeCost(const GameContext& context) const;
    void produce_item(const GameContext& context);
    FixPoint getUpgradeProgress() const noexcept { return upgradeProgress_; }

    ItemID_enum getCurrentProducedItem() const noexcept { return currentProducedItem_; }
    bool isOnHold() const noexcept { return bCurrentItemOnHold_; }
    bool isWaitingToPlace() const;
    bool isUnitLimitReached(ItemID_enum itemID) const;
    FixPoint getProductionProgress() const noexcept { return productionProgress_; }
    const auto& getBuildList() const noexcept { return buildList_; }

    bool isAvailableToBuild(ItemID_enum itemID) const { return (getBuildItem(itemID) != nullptr); }

    /**
        Get the current build speed limit.
        \return the current build speed limit
    */
    FixPoint getBuildSpeedLimit() const { return buildSpeedLimit_; }

protected:
    void updateProductionProgress();

    void removeBuiltItemFromProductionQueue();

    void insertItem(build_list_type::iterator& iter, ItemID_enum item_id, int price = -1);

    void removeItem(build_list_type::iterator& iter, ItemID_enum item_id);

    BuildItem* getBuildItem(ItemID_enum itemID);

    const BuildItem* getBuildItem(ItemID_enum itemID) const;

    void produceNextAvailableItem();

    auto& getBuildList() noexcept { return buildList_; }

protected:
    static const ItemID_enum itemOrder[]; ///< the order in which items are in the build list

    // structure state
    bool upgrading_{};           ///< Currently upgrading?
    FixPoint upgradeProgress_{}; ///< The current state of the upgrade progress (measured in money spent)
    uint8_t curUpgradeLev_{};    ///< Current upgrade level

    bool bCurrentItemOnHold_{};                        ///< Is the currently produced item on hold?
    ItemID_enum currentProducedItem_ = ItemID_Invalid; ///< The ItemID of the currently produced item
    FixPoint productionProgress_{}; ///< The current state of the production progress (measured in money spent)
    uint32_t deployTimer_{};        ///< Timer for deploying a unit

    FixPoint buildSpeedLimit_; ///< Limit the build speed to that percentage [0;1]. This may be used by the AI to make
                               ///< it weaker.

    production_queue_type currentProductionQueue_; ///< This list is the production queue (It contains the item
                                                   ///< IDs of the units/structures to produce)
private:
    void init();

    build_list_type buildList_; ///< This list contains all the things that can be produced by this builder
};

template<>
inline BuilderBase* dune_cast(ObjectBase* base) {
    if (base && base->isABuilder())
        return static_cast<BuilderBase*>(base);

    return nullptr;
}

template<>
inline const BuilderBase* dune_cast(const ObjectBase* base) {
    if (base && base->isABuilder())
        return static_cast<const BuilderBase*>(base);

    return nullptr;
}

#endif // BUILDERBASE_H
