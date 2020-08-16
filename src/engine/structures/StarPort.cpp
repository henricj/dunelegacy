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

#include <structures/StarPort.h>

#include <House.h>
#include <Game.h>
#include <Choam.h>
#include <Map.h>

#include <players/HumanPlayer.h>

#include <units/Frigate.h>

namespace {
using namespace Dune::Engine;

// Starport is counting in 30s from 10 to 0
constexpr int STARPORT_ARRIVETIME = MILLI2CYCLES(30 * 1000);

constexpr int STARPORT_NO_ARRIVAL_AWAITED = -1;

constexpr BuilderBaseConstants star_port_constants{StarPort::item_id, Coord{3, 3}};

}

namespace Dune::Engine {

StarPort::StarPort(uint32_t objectID, const ObjectInitializer& initializer)
    : BuilderBase(star_port_constants, objectID, initializer) {
    StarPort::init();

    parent::setHealth(initializer.game(), getMaxHealth(initializer.game()));

    arrivalTimer = STARPORT_NO_ARRIVAL_AWAITED;
    deploying    = false;
}

StarPort::StarPort(uint32_t objectID, const ObjectStreamInitializer& initializer)
    : BuilderBase(star_port_constants, objectID, initializer) {
    StarPort::init();

    auto& stream = initializer.stream();

    arrivalTimer = stream.readSint32();
    if(stream.readBool()) {
        startDeploying();
    } else {
        deploying = false;
    }
}

void StarPort::init() {
    assert(itemID == Structure_StarPort);
    owner->incrementStructures(itemID);
}

StarPort::~StarPort() = default;

void StarPort::save(const Game& game, OutputStream& stream) const {
    BuilderBase::save(game, stream);
    stream.writeSint32(arrivalTimer);
    stream.writeBool(deploying);
}

void StarPort::doBuildRandom(const GameContext& context) {
    if(!buildList.empty()) {
        auto item2Produce = ItemID_Invalid;

        do {
            item2Produce =
                std::next(buildList.begin(), context.game.randomGen.rand(0, static_cast<int32_t>(buildList.size()) - 1))
                    ->itemID;
        } while((item2Produce == Unit_Harvester) || (item2Produce == Unit_MCV) || (item2Produce == Unit_Carryall));

        doProduceItem(context, item2Produce);
    }
}

void StarPort::doProduceItem(const GameContext& context, ItemID_enum itemID, bool multipleMode) {
    auto& choam = owner->getChoam();

    for(auto& buildItem : buildList) {
        if(buildItem.itemID != itemID) continue;

        for(auto i = 0; i < (multipleMode ? 5 : 1); i++) {
            const auto numAvailable = choam.getNumAvailable(itemID);

            if(numAvailable <= 0) { break; }

            if((owner->getCredits() >= static_cast<int>(buildItem.price))) {
                buildItem.num++;
                currentProductionQueue.emplace_back(itemID, buildItem.price);
                owner->takeCredits(buildItem.price);

                if(!choam.setNumAvailable(itemID, numAvailable - 1)) {
                    // sold out
                    break;
                }
            }
        }
        break;
    }
}

void StarPort::doCancelItem(ItemID_enum itemID, bool multipleMode) {
    auto& choam = owner->getChoam();

    for(auto& buildItem : buildList) {
        if(buildItem.itemID != itemID) continue;

        for(int i = 0; i < (multipleMode ? 5 : 1); i++) {
            if(buildItem.num > 0) {
                buildItem.num--;
                choam.setNumAvailable(itemID, choam.getNumAvailable(itemID) + 1);

                // find the most expensive item to cancel
                auto     iterMostExpensiveItem  = currentProductionQueue.end();
                uint32_t mostExpensiveItemPrice = 0;
                for(auto iter = currentProductionQueue.begin(); iter != currentProductionQueue.end(); ++iter) {
                    if(iter->itemID == itemID) {
                        if(iter->price > mostExpensiveItemPrice) {
                            iterMostExpensiveItem  = iter;
                            mostExpensiveItemPrice = iter->price;
                        }
                    }
                }

                // Cancel the best found item if any was found
                if(iterMostExpensiveItem != currentProductionQueue.end()) {
                    owner->returnCredits(iterMostExpensiveItem->price);
                    currentProductionQueue.erase(iterMostExpensiveItem);
                }
            }
        }

        break;
    }
}

void StarPort::doPlaceOrder(const Game& game) {

    if(!currentProductionQueue.empty()) {

        if(game.getGameInitSettings().getGameOptions().instantBuild) {
            arrivalTimer = 1;
        } else {
            arrivalTimer = STARPORT_ARRIVETIME;
        }
    }
}

void StarPort::doCancelOrder() {
    if(arrivalTimer == STARPORT_NO_ARRIVAL_AWAITED) {
        while(!currentProductionQueue.empty()) {
            doCancelItem(currentProductionQueue.back().itemID, false);
        }

        currentProducedItem = ItemID_Invalid;
    }
}

void StarPort::updateBuildList(const Game& game) {
    auto iter = buildList.begin();

    auto& choam = owner->getChoam();

    for(auto i = 0; itemOrder[i] != ItemID_Invalid; ++i) {

        const auto& objData = game.getObjectData(itemOrder[i], originalHouseID);

        if(objData.enabled && (choam.getNumAvailable(itemOrder[i]) != INVALID)) {
            insertItem(game, buildList, iter, itemOrder[i], choam.getPrice(itemOrder[i]));
        } else {
            removeItem(buildList, iter, itemOrder[i]);
        }
    }
}

void StarPort::updateStructureSpecificStuff(const GameContext& context) {
    updateBuildList(context.game);

    if(arrivalTimer > 0) {
        if(--arrivalTimer == 0) {
            // make a frigate with all the cargo
            auto* const frigate = owner->createUnit<Frigate>();
            const auto  pos     = context.map.findClosestEdgePoint(getLocation() + Coord(1, 1), Coord(1, 1));
            frigate->deploy(context, pos);
            frigate->setTarget(context.objectManager,this);
            const auto closestPoint = getClosestPoint(frigate->getLocation());
            frigate->setDestination(context, closestPoint);

            if(pos.x == 0) frigate->setAngle(ANGLETYPE::RIGHT);
            else if(pos.x == context.map.getSizeX() - 1)
                frigate->setAngle(ANGLETYPE::LEFT);
            else if(pos.y == 0)
                frigate->setAngle(ANGLETYPE::DOWN);
            else if(pos.y == context.map.getSizeY() - 1)
                frigate->setAngle(ANGLETYPE::UP);

            deployTimer = MILLI2CYCLES(2000);

            currentProducedItem = ItemID_Invalid;
        }
    } else if(deploying) {
        deployTimer--;
        if(deployTimer == 0) {

            if(!currentProductionQueue.empty()) {
                auto newUnitItemID = currentProductionQueue.front().itemID;

                auto num2Place = 1;

                if(newUnitItemID == Unit_Infantry) {
                    // make three
                    newUnitItemID = Unit_Soldier;
                    num2Place     = 3;
                } else if(newUnitItemID == Unit_Troopers) {
                    // make three
                    newUnitItemID = Unit_Trooper;
                    num2Place     = 3;
                }

                for(auto i = 0; i < num2Place; i++) {
                    auto* newUnit = getOwner()->createUnit(newUnitItemID);
                    if(newUnit != nullptr) {
                        Coord unitDestination;
                        if(getOwner()->isAI() &&
                           ((newUnit->getItemID() == Unit_Carryall) || (newUnit->getItemID() == Unit_Harvester) ||
                            (newUnit->getItemID() == Unit_MCV))) {
                            // Don't want harvesters going to the rally point
                            unitDestination = location;
                        } else {
                            unitDestination = destination;
                        }

                        const auto spot =
                            context.map.findDeploySpot(newUnit, location, unitDestination, getStructureSize());
                        newUnit->deploy(context, spot);

                        if(unitDestination.isValid()) {
                            newUnit->setGuardPoint(context, unitDestination);
                            newUnit->setDestination(context, unitDestination);
                            newUnit->setAngle(destinationDrawnAngle(newUnit->getLocation(), newUnit->getDestination()));
                        }

                        // inform owner of its new unit
                        newUnit->getOwner()->informWasBuilt(newUnit);
                    }
                }

                const auto currentProducedBuildItem =
                    std::find_if(buildList.begin(), buildList.end(), [&](BuildItem& buildItem) {
                        return (buildItem.itemID == currentProductionQueue.front().itemID);
                    });
                if(currentProducedBuildItem != buildList.end()) { currentProducedBuildItem->num--; }

                currentProductionQueue.pop_front();

                if(currentProductionQueue.empty()) {
                    arrivalTimer = STARPORT_NO_ARRIVAL_AWAITED;
                    deploying    = false;
                } else {
                    deployTimer = MILLI2CYCLES(2000);
                }
            }
        }
    }
}

void StarPort::informFrigateDestroyed() {
    currentProductionQueue.clear();
    arrivalTimer = STARPORT_NO_ARRIVAL_AWAITED;
    deployTimer  = 0;
    deploying    = false;
}

} // namespace Dune::Engine
