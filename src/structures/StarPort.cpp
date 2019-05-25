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

#include <FileClasses/GFXManager.h>
#include <FileClasses/TextManager.h>
#include <House.h>
#include <Game.h>
#include <Choam.h>
#include <Map.h>
#include <SoundPlayer.h>

#include <players/HumanPlayer.h>

#include <units/Frigate.h>

// Starport is counting in 30s from 10 to 0
#define STARPORT_ARRIVETIME         (MILLI2CYCLES(30*1000))

#define STARPORT_NO_ARRIVAL_AWAITED -1



StarPort::StarPort(House* newOwner) : BuilderBase(newOwner) {
    StarPort::init();

    setHealth(getMaxHealth());

    arrivalTimer = STARPORT_NO_ARRIVAL_AWAITED;
    deploying = false;
}

StarPort::StarPort(InputStream& stream) : BuilderBase(stream) {
    StarPort::init();

    arrivalTimer = stream.readSint32();
    if(stream.readBool() == true) {
        startDeploying();
    } else {
        deploying = false;
    }
}
void StarPort::init() {
    itemID = Structure_StarPort;
    owner->incrementStructures(itemID);

    structureSize.x = 3;
    structureSize.y = 3;

    graphicID = ObjPic_Starport;
    graphic = pGFXManager->getObjPic(graphicID,getOwner()->getHouseID());
    numImagesX = 10;
    numImagesY = 1;
    firstAnimFrame = 2;
    lastAnimFrame = 3;
}

StarPort::~StarPort() = default;

void StarPort::save(OutputStream& stream) const {
    BuilderBase::save(stream);
    stream.writeSint32(arrivalTimer);
    stream.writeBool(deploying);
}

void StarPort::doBuildRandom() {
    if(!buildList.empty()) {
        int item2Produce = ItemID_Invalid;

        do {
            item2Produce = std::next(buildList.begin(), currentGame->randomGen.rand(0, static_cast<Sint32>(buildList.size())-1))->itemID;
        } while((item2Produce == Unit_Harvester) || (item2Produce == Unit_MCV) || (item2Produce == Unit_Carryall));

        doProduceItem(item2Produce);
    }
}

void StarPort::handleProduceItemClick(Uint32 itemID, bool multipleMode) {
    Choam& choam = owner->getChoam();
    int numAvailable = choam.getNumAvailable(itemID);

    if(numAvailable <= 0) {
        soundPlayer->playSound(Sound_InvalidAction);
        currentGame->addToNewsTicker(_("This unit is sold out"));
        return;
    }

    for(const BuildItem& buildItem : buildList) {
        if(buildItem.itemID == itemID) {
            if((owner->getCredits() < (int) buildItem.price)) {
                soundPlayer->playSound(Sound_InvalidAction);
                currentGame->addToNewsTicker(_("Not enough money"));
                return;
            }
        }
    }

    BuilderBase::handleProduceItemClick(itemID, multipleMode);
}

void StarPort::handlePlaceOrderClick() {
    currentGame->getCommandManager().addCommand(Command(pLocalPlayer->getPlayerID(), CMD_STARPORT_PLACEORDER, objectID));
}

void StarPort::handleCancelOrderClick() {
    currentGame->getCommandManager().addCommand(Command(pLocalPlayer->getPlayerID(), CMD_STARPORT_CANCELORDER, objectID));
}

void StarPort::doProduceItem(Uint32 itemID, bool multipleMode) {
    Choam& choam = owner->getChoam();

    for(BuildItem& buildItem : buildList) {
        if(buildItem.itemID == itemID) {
            for(int i = 0; i < (multipleMode ? 5 : 1); i++) {
                int numAvailable = choam.getNumAvailable(itemID);

                if(numAvailable <= 0) {
                    break;
                }

                if((owner->getCredits() >= (int) buildItem.price)) {
                    buildItem.num++;
                    currentProductionQueue.emplace_back(itemID,buildItem.price );
                    owner->takeCredits(buildItem.price);

                    if(choam.setNumAvailable(itemID, numAvailable - 1) == false) {
                        // sold out
                        break;
                    }
                }
            }
            break;
        }
    }
}

void StarPort::doCancelItem(Uint32 itemID, bool multipleMode) {
    Choam& choam = owner->getChoam();

    for(BuildItem& buildItem : buildList) {
        if(buildItem.itemID == itemID) {
            for(int i = 0; i < (multipleMode ? 5 : 1); i++) {
                if(buildItem.num > 0) {
                    buildItem.num--;
                    choam.setNumAvailable(itemID, choam.getNumAvailable(itemID) + 1);

                    // find the most expensive item to cancel
                    auto iterMostExpensiveItem = currentProductionQueue.end();
                    Uint32 mostExpensiveItemPrice = 0;
                    for(auto iter = currentProductionQueue.begin(); iter != currentProductionQueue.end(); ++iter) {
                        if(iter->itemID == itemID) {
                            if(iter->price > mostExpensiveItemPrice) {
                                iterMostExpensiveItem = iter;
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
}

void StarPort::doPlaceOrder() {

    if (!currentProductionQueue.empty()) {

        if(currentGame->getGameInitSettings().getGameOptions().instantBuild == true) {
            arrivalTimer = 1;
        } else {
            arrivalTimer = STARPORT_ARRIVETIME;
        }

        firstAnimFrame = 2;
        lastAnimFrame = 7;
    }
}

void StarPort::doCancelOrder() {
    if (arrivalTimer == STARPORT_NO_ARRIVAL_AWAITED) {
        while(currentProductionQueue.empty() == false) {
            doCancelItem(currentProductionQueue.back().itemID, false);
        }

        currentProducedItem = ItemID_Invalid;
    }
}


void StarPort::updateBuildList() {
    std::list<BuildItem>::iterator iter = buildList.begin();

    Choam& choam = owner->getChoam();

    for(int i = 0; itemOrder[i] != ItemID_Invalid; ++i) {

        const ObjectData::ObjectDataStruct& objData = currentGame->objectData.data[itemOrder[i]][originalHouseID];

        if(objData.enabled && (choam.getNumAvailable(itemOrder[i]) != INVALID)) {
            insertItem(buildList, iter, itemOrder[i], choam.getPrice(itemOrder[i]));
        } else {
            removeItem(buildList, iter, itemOrder[i]);
        }
    }
}

void StarPort::updateStructureSpecificStuff() {
    updateBuildList();

    if (arrivalTimer > 0) {
        if (--arrivalTimer == 0) {

            Frigate*        frigate;
            Coord       pos;

            //make a frigate with all the cargo
            frigate = static_cast<Frigate*>(owner->createUnit(Unit_Frigate));
            pos = currentGameMap->findClosestEdgePoint(getLocation() + Coord(1,1), Coord(1,1));
            frigate->deploy(pos);
            frigate->setTarget(this);
            Coord closestPoint = getClosestPoint(frigate->getLocation());
            frigate->setDestination(closestPoint);

            if (pos.x == 0)
                frigate->setAngle(RIGHT);
            else if (pos.x == currentGameMap->getSizeX()-1)
                frigate->setAngle(LEFT);
            else if (pos.y == 0)
                frigate->setAngle(DOWN);
            else if (pos.y == currentGameMap->getSizeY()-1)
                frigate->setAngle(UP);

            deployTimer = MILLI2CYCLES(2000);

            currentProducedItem = ItemID_Invalid;

            if(getOwner() == pLocalHouse) {
                soundPlayer->playVoice(FrigateHasArrived,getOwner()->getHouseID());
                currentGame->addToNewsTicker(_("@DUNE.ENG|80#Frigate has arrived"));
            }

        }
    } else if(deploying == true) {
        deployTimer--;
        if(deployTimer == 0) {

            if(currentProductionQueue.empty() == false) {
                Uint32 newUnitItemID = currentProductionQueue.front().itemID;

                int num2Place = 1;

                if(newUnitItemID == Unit_Infantry) {
                    // make three
                    newUnitItemID = Unit_Soldier;
                    num2Place = 3;
                } else if(newUnitItemID == Unit_Troopers) {
                    // make three
                    newUnitItemID = Unit_Trooper;
                    num2Place = 3;
                }

                for(int i = 0; i < num2Place; i++) {
                    UnitBase* newUnit = getOwner()->createUnit(newUnitItemID);
                    if (newUnit != nullptr) {
                        Coord unitDestination;
                        if( getOwner()->isAI()
                            && ((newUnit->getItemID() == Unit_Carryall)
                                || (newUnit->getItemID() == Unit_Harvester)
                                || (newUnit->getItemID() == Unit_MCV))) {
                            // Don't want harvesters going to the rally point
                            unitDestination = location;
                        } else {
                            unitDestination = destination;
                        }

                        Coord spot = newUnit->isAFlyingUnit() ? location + Coord(1,1) : currentGameMap->findDeploySpot(newUnit, location, currentGame->randomGen, unitDestination, structureSize);
                        newUnit->deploy(spot);

                        if(unitDestination.isValid()) {
                            newUnit->setGuardPoint(unitDestination);
                            newUnit->setDestination(unitDestination);
                            newUnit->setAngle(destinationDrawnAngle(newUnit->getLocation(), newUnit->getDestination()));
                        }

                        if(getOwner() == pLocalHouse) {
                            if(isFlyingUnit(newUnitItemID)) {
                                soundPlayer->playVoice(UnitLaunched, getOwner()->getHouseID());
                            } else if(newUnitItemID == Unit_Harvester) {
                                soundPlayer->playVoice(HarvesterDeployed, getOwner()->getHouseID());
                            } else {
                                soundPlayer->playVoice(UnitDeployed, getOwner()->getHouseID());
                            }
                        }

                        // inform owner of its new unit
                        newUnit->getOwner()->informWasBuilt(newUnit);
                    }
                }

                auto currentProducedBuildItem = std::find_if(   buildList.begin(),
                                                                buildList.end(),
                                                                [&](BuildItem& buildItem) {
                                                                    return (buildItem.itemID == currentProductionQueue.front().itemID);
                                                                });
                if(currentProducedBuildItem != buildList.end()) {
                    currentProducedBuildItem->num--;
                }

                currentProductionQueue.pop_front();

                if(currentProductionQueue.empty() == true) {
                    arrivalTimer = STARPORT_NO_ARRIVAL_AWAITED;
                    deploying = false;
                    // Remove box from starport
                    firstAnimFrame = 2;
                    lastAnimFrame = 3;
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
    deployTimer = 0;
    deploying = false;
    // stop blinking
    firstAnimFrame = 2;
    lastAnimFrame = 3;
}
