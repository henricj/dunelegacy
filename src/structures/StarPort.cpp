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

#include <structures/StarPort.h>

#include <globals.h>

#include "mmath.h"
#include <Choam.h>
#include <FileClasses/GFXManager.h>
#include <FileClasses/TextManager.h>
#include <Game.h>
#include <House.h>
#include <Map.h>
#include <SoundPlayer.h>

#include <players/HumanPlayer.h>

#include <units/Frigate.h>

namespace {
// Starport is counting in 30s from 10 to 0
constexpr auto STARPORT_ARRIVETIME = (MILLI2CYCLES(30 * 1000));

constexpr auto STARPORT_NO_ARRIVAL_AWAITED = -1;

constexpr BuilderBaseConstants star_port_constants{StarPort::item_id, Coord{3, 3}};
} // namespace

StarPort::StarPort(uint32_t objectID, const ObjectInitializer& initializer)
    : BuilderBase(star_port_constants, objectID, initializer), arrivalTimer(STARPORT_NO_ARRIVAL_AWAITED) {
    StarPort::init();

    ObjectBase::setHealth(getMaxHealth());
}

StarPort::StarPort(uint32_t objectID, const ObjectStreamInitializer& initializer)
    : BuilderBase(star_port_constants, objectID, initializer) {
    StarPort::init();

    auto& stream = initializer.stream();

    arrivalTimer = stream.readSint32();
    if (stream.readBool()) {
        startDeploying();
    } else {
        deploying = false;
    }
}

void StarPort::init() {
    assert(itemID == Structure_StarPort);
    owner->incrementStructures(itemID);

    graphicID      = ObjPic_Starport;
    graphic        = dune::globals::pGFXManager->getObjPic(graphicID, getOwner()->getHouseID());
    numImagesX     = 10;
    numImagesY     = 1;
    firstAnimFrame = 2;
    lastAnimFrame  = 3;
}

StarPort::~StarPort() = default;

void StarPort::save(OutputStream& stream) const {
    BuilderBase::save(stream);
    stream.writeSint32(arrivalTimer);
    stream.writeBool(deploying);
}

void StarPort::doBuildRandom(const GameContext& context) {
    auto& buildList = getBuildList();

    if (!buildList.empty()) {
        auto item2Produce = ItemID_Invalid;

        do {
            item2Produce =
                std::next(buildList.begin(), context.game.randomGen.rand(0, static_cast<int32_t>(buildList.size()) - 1))
                    ->itemID;
        } while ((item2Produce == Unit_Harvester) || (item2Produce == Unit_MCV) || (item2Produce == Unit_Carryall));

        doProduceItem(item2Produce);
    }
}

void StarPort::handleProduceItemClick(ItemID_enum itemID, bool multipleMode) {
    const auto& choam       = owner->getChoam();
    const auto numAvailable = choam.getNumAvailable(itemID);

    using dune::globals::soundPlayer;
    const auto* const currentGame = dune::globals::currentGame.get();

    if (numAvailable <= 0) {
        soundPlayer->playSound(Sound_enum::Sound_InvalidAction);
        currentGame->addToNewsTicker(_("This unit is sold out"));
        return;
    }

    for (const auto& buildItem : getBuildList()) {
        if (buildItem.itemID == itemID) {
            if ((owner->getCredits() < static_cast<int>(buildItem.price))) {
                soundPlayer->playSound(Sound_enum::Sound_InvalidAction);
                currentGame->addToNewsTicker(_("Not enough money"));
                return;
            }
        }
    }

    BuilderBase::handleProduceItemClick(itemID, multipleMode);
}

void StarPort::handlePlaceOrderClick() {
    dune::globals::currentGame->getCommandManager().addCommand(
        Command(dune::globals::pLocalPlayer->getPlayerID(), CMDTYPE::CMD_STARPORT_PLACEORDER, objectID));
}

void StarPort::handleCancelOrderClick() {
    dune::globals::currentGame->getCommandManager().addCommand(
        Command(dune::globals::pLocalPlayer->getPlayerID(), CMDTYPE::CMD_STARPORT_CANCELORDER, objectID));
}

void StarPort::doProduceItem(ItemID_enum itemID, bool multipleMode) {
    auto& choam = owner->getChoam();

    for (auto& buildItem : getBuildList()) {
        if (buildItem.itemID != itemID)
            continue;

        for (auto i = 0; i < (multipleMode ? 5 : 1); i++) {
            const auto numAvailable = choam.getNumAvailable(itemID);

            if (numAvailable <= 0) {
                break;
            }

            if ((owner->getCredits() >= static_cast<int>(buildItem.price))) {
                buildItem.num++;
                currentProductionQueue.emplace_back(itemID, buildItem.price);
                owner->takeCredits(buildItem.price);

                if (!choam.setNumAvailable(itemID, numAvailable - 1)) {
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

    for (auto& buildItem : getBuildList()) {
        if (buildItem.itemID != itemID)
            continue;

        for (int i = 0; i < (multipleMode ? 5 : 1); i++) {
            if (buildItem.num > 0) {
                buildItem.num--;
                choam.setNumAvailable(itemID, choam.getNumAvailable(itemID) + 1);

                // find the most expensive item to cancel
                auto iterMostExpensiveItem      = currentProductionQueue.end();
                uint32_t mostExpensiveItemPrice = 0;
                for (auto iter = currentProductionQueue.begin(); iter != currentProductionQueue.end(); ++iter) {
                    if (iter->itemID == itemID) {
                        if (iter->price > mostExpensiveItemPrice) {
                            iterMostExpensiveItem  = iter;
                            mostExpensiveItemPrice = iter->price;
                        }
                    }
                }

                // Cancel the best found item if any was found
                if (iterMostExpensiveItem != currentProductionQueue.end()) {
                    owner->returnCredits(iterMostExpensiveItem->price);
                    currentProductionQueue.erase(iterMostExpensiveItem);
                }
            }
        }

        break;
    }
}

void StarPort::doPlaceOrder() {

    if (!currentProductionQueue.empty()) {

        if (dune::globals::currentGame->getGameInitSettings().getGameOptions().instantBuild) {
            arrivalTimer = 1;
        } else {
            arrivalTimer = STARPORT_ARRIVETIME;
        }

        firstAnimFrame = 2;
        lastAnimFrame  = 7;
    }
}

void StarPort::doCancelOrder() {
    if (arrivalTimer == STARPORT_NO_ARRIVAL_AWAITED) {
        while (!currentProductionQueue.empty()) {
            doCancelItem(currentProductionQueue.back().itemID, false);
        }

        currentProducedItem = ItemID_Invalid;
    }
}

void StarPort::updateBuildList() {
    auto& buildList = getBuildList();

    auto iter = buildList.begin();

    const auto& choam       = owner->getChoam();
    const auto& object_data = dune::globals::currentGame->objectData;

    for (auto i = 0; itemOrder[i] != ItemID_Invalid; ++i) {

        const auto& objData = object_data.data[itemOrder[i]][static_cast<int>(originalHouseID)];

        if (objData.enabled && (choam.getNumAvailable(itemOrder[i]) != INVALID)) {
            insertItem(iter, itemOrder[i], choam.getPrice(itemOrder[i]));
        } else {
            removeItem(iter, itemOrder[i]);
        }
    }
}

void StarPort::updateStructureSpecificStuff(const GameContext& context) {
    updateBuildList();

    if (arrivalTimer > 0) {
        if (--arrivalTimer == 0) {
            // make a frigate with all the cargo
            if (auto* const frigate = dune_cast<Frigate>(owner->createUnit(Unit_Frigate))) {
                const auto pos = context.map.findClosestEdgePoint(getLocation() + Coord(1, 1), Coord(1, 1));
                frigate->deploy(context, pos);
                frigate->setTarget(this);
                const auto closestPoint = getClosestPoint(frigate->getLocation());
                frigate->setDestination(closestPoint);

                if (pos.x == 0)
                    frigate->setAngle(ANGLETYPE::RIGHT);
                else if (pos.x == dune::globals::currentGameMap->getSizeX() - 1)
                    frigate->setAngle(ANGLETYPE::LEFT);
                else if (pos.y == 0)
                    frigate->setAngle(ANGLETYPE::DOWN);
                else if (pos.y == dune::globals::currentGameMap->getSizeY() - 1)
                    frigate->setAngle(ANGLETYPE::UP);

                deployTimer = MILLI2CYCLES(2000);

                currentProducedItem = ItemID_Invalid;

                if (getOwner() == dune::globals::pLocalHouse) {
                    dune::globals::soundPlayer->playVoice(Voice_enum::FrigateHasArrived, getOwner()->getHouseID());
                    context.game.addToNewsTicker(_("@DUNE.ENG|80#Frigate has arrived"));
                }
            } else
                sdl2::log_warn("Unable to create Frigate!");
        }
    } else if (deploying) {
        deployTimer--;
        if (deployTimer == 0) {

            if (!currentProductionQueue.empty()) {
                auto newUnitItemID = currentProductionQueue.front().itemID;

                auto num2Place = 1;

                if (newUnitItemID == Unit_Infantry) {
                    // make three
                    newUnitItemID = Unit_Soldier;
                    num2Place     = 3;
                } else if (newUnitItemID == Unit_Troopers) {
                    // make three
                    newUnitItemID = Unit_Trooper;
                    num2Place     = 3;
                }

                for (auto i = 0; i < num2Place; i++) {
                    auto* newUnit = getOwner()->createUnit(newUnitItemID);
                    if (newUnit != nullptr) {
                        Coord unitDestination;
                        if (getOwner()->isAI()
                            && ((newUnit->getItemID() == Unit_Carryall) || (newUnit->getItemID() == Unit_Harvester)
                                || (newUnit->getItemID() == Unit_MCV))) {
                            // Don't want harvesters going to the rally point
                            unitDestination = location;
                        } else {
                            unitDestination = destination;
                        }

                        const auto spot =
                            context.map.findDeploySpot(newUnit, location, unitDestination, getStructureSize());
                        newUnit->deploy(context, spot);

                        if (unitDestination.isValid()) {
                            newUnit->setGuardPoint(unitDestination);
                            newUnit->setDestination(unitDestination);
                            newUnit->setAngle(destinationDrawnAngle(newUnit->getLocation(), newUnit->getDestination()));
                        }

                        if (getOwner() == dune::globals::pLocalHouse) {
                            if (isFlyingUnit(newUnitItemID)) {
                                dune::globals::soundPlayer->playVoice(Voice_enum::UnitLaunched,
                                                                      getOwner()->getHouseID());
                            } else if (newUnitItemID == Unit_Harvester) {
                                dune::globals::soundPlayer->playVoice(Voice_enum::HarvesterDeployed,
                                                                      getOwner()->getHouseID());
                            } else {
                                dune::globals::soundPlayer->playVoice(Voice_enum::UnitDeployed,
                                                                      getOwner()->getHouseID());
                            }
                        }

                        // inform owner of its new unit
                        newUnit->getOwner()->informWasBuilt(newUnit);
                    }
                }

                auto& buildList = getBuildList();

                const auto currentProducedBuildItem = std::ranges::find_if(buildList, [&](BuildItem& buildItem) {
                    return (buildItem.itemID == currentProductionQueue.front().itemID);
                });
                if (currentProducedBuildItem != buildList.end()) {
                    currentProducedBuildItem->num--;
                }

                currentProductionQueue.pop_front();

                if (currentProductionQueue.empty()) {
                    arrivalTimer = STARPORT_NO_ARRIVAL_AWAITED;
                    deploying    = false;
                    // Remove box from starport
                    firstAnimFrame = 2;
                    lastAnimFrame  = 3;
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
    // stop blinking
    firstAnimFrame = 2;
    lastAnimFrame  = 3;
}
