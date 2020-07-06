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

#include <list>
#include <string>

class BuildItem final {
public:
    BuildItem() {
        itemID = ItemID_Invalid;
        price = 0;
        num = 0;
    }

    BuildItem(ItemID_enum itemID, int price) {
        this->itemID = itemID;
        this->price = price;
        num = 0;
    }

    BuildItem(InputStream& stream) {
        load(stream);
    }

    void save(OutputStream& stream) const {
        stream.writeUint32(itemID);
        stream.writeUint32(price);
        stream.writeUint32(num);
    }

    void load(InputStream& stream) {
        itemID = static_cast<ItemID_enum>(stream.readUint32());
        price = stream.readUint32();
        num = stream.readUint32();
    }

    ItemID_enum itemID;
    Uint32 price;
    Uint32 num;
};

class ProductionQueueItem final {
public:
    ProductionQueueItem()
        : itemID(ItemID_enum::ItemID_Invalid), price(0) {

    }

    ProductionQueueItem(ItemID_enum _ItemID, Uint32 _price)
        : itemID(_ItemID), price(_price) {

    }

    ProductionQueueItem(InputStream& stream) {
        load(stream);
    }

    void save(OutputStream& stream) const {
        stream.writeUint32(itemID);
        stream.writeUint32(price);
    }

    void load(InputStream& stream) {
        itemID = static_cast<ItemID_enum>(stream.readUint32());
        price = stream.readUint32();
    }

    ItemID_enum itemID;
    Uint32 price;
};


class BuilderBase : public StructureBase
{
public:
    BuilderBase(ItemID_enum itemID, Uint32 objectID, const ObjectInitializer& initializer);
    BuilderBase(ItemID_enum itemID, Uint32 objectID, const ObjectStreamInitializer& initializer);
    virtual ~BuilderBase() = 0;

    BuilderBase(const BuilderBase &) = delete;
    BuilderBase(BuilderBase &&) = delete;
    BuilderBase& operator=(const BuilderBase &) = delete;
    BuilderBase& operator=(BuilderBase &&) = delete;

    void save(OutputStream& stream) const override;

    ObjectInterface* getInterfaceContainer() override;

    void setOwner(House *no);

    void setOriginalHouseID(HOUSETYPE i) override
    {
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

    int getBuildListSize() const { return buildList.size(); }

    int getProductionQueueSize() const { return currentProductionQueue.size(); }

    /**
        Updates this builder.
        \return true if this object still exists, false if it was destroyed
    */
    bool update() override;

    virtual void handleUpgradeClick();
    virtual void handleProduceItemClick(ItemID_enum itemID, bool multipleMode = false);
    virtual void handleCancelItemClick(ItemID_enum itemID, bool multipleMode = false);
    virtual void handleSetOnHoldClick(bool OnHold);



    /**
        Start upgrading this builder if possible.
        \return true if upgrading was started, false if not possible or already upgrading
    */
    virtual bool doUpgrade();

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
    inline void doSetOnHold(bool bOnHold) { bCurrentItemOnHold = bOnHold; }

    /**
        Start building a random item in this builder.
    */
    virtual void doBuildRandom();

    /**
        Limit the build speed of this builder to the given argument. The build speed can be limited by the AI
        to make it weaker, e.g. the campaign AI adjusts the build speed to the mission number
        \param newBuildSpeedLimit   the new limit; must be between 0 and 1
    */
    void doSetBuildSpeedLimit(FixPoint newBuildSpeedLimit) { buildSpeedLimit = std::max(0.0_fix, std::min(1.0_fix, newBuildSpeedLimit)); }



    bool isUpgrading() const noexcept { return upgrading; }
    bool isAllowedToUpgrade() const { return (curUpgradeLev < getMaxUpgradeLevel()); }
    int getCurrentUpgradeLevel() const noexcept { return curUpgradeLev; }
    int getUpgradeCost() const;
    void produce_item();
    FixPoint getUpgradeProgress() const noexcept { return upgradeProgress; }

    ItemID_enum getCurrentProducedItem() const noexcept { return currentProducedItem; }
    bool isOnHold() const noexcept { return bCurrentItemOnHold; }
    bool isWaitingToPlace() const;
    bool isUnitLimitReached(ItemID_enum itemID) const;
    FixPoint getProductionProgress() const noexcept { return productionProgress; }
    const std::list<BuildItem>& getBuildList() const noexcept { return buildList; }

    virtual bool isAvailableToBuild(ItemID_enum itemID) const {
        return (getBuildItem(itemID) != nullptr);
    }

    /**
        Get the current build speed limit.
        \return the current build speed limit
    */
    FixPoint getBuildSpeedLimit() const { return buildSpeedLimit; }

protected:
    virtual void updateProductionProgress();

    void removeBuiltItemFromProductionQueue();

    virtual void insertItem(std::list<BuildItem>& buildItemList, std::list<BuildItem>::iterator& iter, ItemID_enum itemID, int price=-1);

    void removeItem(std::list<BuildItem>& buildItemList, std::list<BuildItem>::iterator& iter, ItemID_enum itemID);

    BuildItem* getBuildItem(ItemID_enum itemID) {
        for(auto& buildItem : buildList) {
            if(buildItem.itemID == itemID) {
                return &buildItem;
            }
        }
        return nullptr;
    }

    const BuildItem* getBuildItem(ItemID_enum itemID) const {
        for(const auto& buildItem : buildList) {
            if(buildItem.itemID == itemID) {
                return &buildItem;
            }
        }
        return nullptr;
    }

    void produceNextAvailableItem();

protected:
    static const ItemID_enum itemOrder[]; ///< the order in which items are in the build list

    // structure state
    bool     upgrading{};              ///< Currently upgrading?
    FixPoint upgradeProgress{};        ///< The current state of the upgrade progress (measured in money spent)
    Uint8    curUpgradeLev{};          ///< Current upgrade level

    bool     bCurrentItemOnHold{};     ///< Is the currently produced item on hold?
    ItemID_enum   currentProducedItem = ItemID_Invalid;    ///< The ItemID of the currently produced item
    FixPoint productionProgress{};     ///< The current state of the production progress (measured in money spent)
    Uint32   deployTimer{};            ///< Timer for deploying a unit

    FixPoint buildSpeedLimit;        ///< Limit the build speed to that percentage [0;1]. This may be used by the AI to make it weaker.

    std::list<ProductionQueueItem>  currentProductionQueue;     ///< This list is the production queue (It contains the item IDs of the units/structures to produce)
    std::list<BuildItem>            buildList;                  ///< This list contains all the things that can be produced by this builder

private:
    void init();
};


template<>
inline BuilderBase* dune_cast(ObjectBase* base) {
    if(base && base->isABuilder()) return static_cast<BuilderBase*>(base);

    return nullptr;
}

template<>
inline const BuilderBase* dune_cast(const ObjectBase* base) {
    if(base && base->isABuilder()) return static_cast<const BuilderBase*>(base);

    return nullptr;
}

#endif //BUILDERBASE_H
