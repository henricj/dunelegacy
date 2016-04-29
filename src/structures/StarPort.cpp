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

StarPort::~StarPort() {
}

void StarPort::save(OutputStream& stream) const {
    BuilderBase::save(stream);
    stream.writeSint32(arrivalTimer);
    stream.writeBool(deploying);
}

void StarPort::doBuildRandom() {
    int Item2Produce = ItemID_Invalid;

    do {
        int randNum = currentGame->randomGen.rand(0, getBuildListSize()-1);
        int i = 0;
        std::list<BuildItem>::iterator iter;
        for(iter = buildList.begin(); iter != buildList.end(); ++iter,i++) {
            if(i == randNum) {
                Item2Produce = iter->itemID;
                break;
            }
        }
    } while((Item2Produce == Unit_Harvester) || (Item2Produce == Unit_MCV) || (Item2Produce == Unit_Carryall));

    doProduceItem(Item2Produce);
}

void StarPort::handleProduceItemClick(Uint32 itemID, bool multipleMode) {
    Choam& choam = owner->getChoam();
    int numAvailable = choam.getNumAvailable(itemID);

    if(numAvailable <= 0) {
        soundPlayer->playSound(Sound_InvalidAction);
        currentGame->addToNewsTicker(_("This unit is sold out"));
        return;
    }

    std::list<BuildItem>::iterator iter;
    for(iter = buildList.begin(); iter != buildList.end(); ++iter) {
        if(iter->itemID == itemID) {
            if((owner->getCredits() < (int) iter->price)) {
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

    std::list<BuildItem>::iterator iter;
    for(iter = buildList.begin(); iter != buildList.end(); ++iter) {
        if(iter->itemID == itemID) {
            for(int i = 0; i < (multipleMode ? 5 : 1); i++) {
                int numAvailable = choam.getNumAvailable(itemID);

                if(numAvailable <= 0) {
                    break;
                }

                if((owner->getCredits() >= (int) iter->price)) {
                    iter->num++;
                    currentProductionQueue.push_back( ProductionQueueItem(itemID,iter->price) );
                    owner->takeCredits(iter->price);

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

    std::list<BuildItem>::iterator iter;
    for(iter = buildList.begin(); iter != buildList.end(); ++iter) {
        if(iter->itemID == itemID) {
            for(int i = 0; i < (multipleMode ? 5 : 1); i++) {
                if(iter->num > 0) {
                    iter->num--;
                    choam.setNumAvailable(iter->itemID, choam.getNumAvailable(iter->itemID) + 1);

                    // find the most expensive item to cancel
                    std::list<ProductionQueueItem>::iterator iterMostExpensiveItem = currentProductionQueue.end();
                    std::list<ProductionQueueItem>::iterator iter2;
                    for(iter2 = currentProductionQueue.begin(); iter2 != currentProductionQueue.end(); ++iter2) {
                        if(iter2->itemID == itemID) {

                            // have we found a better item to cancel?
                            if(iterMostExpensiveItem == currentProductionQueue.end() || iter2->price > iterMostExpensiveItem->price) {
                                iterMostExpensiveItem = iter2;
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

    if (currentProductionQueue.size() > 0) {

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
        if(choam.getNumAvailable(itemOrder[i]) != INVALID) {
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
                        Coord spot = newUnit->isAFlyingUnit() ? location + Coord(1,1) : currentGameMap->findDeploySpot(newUnit, location, destination, structureSize);
                        newUnit->deploy(spot);

                        if (getOwner()->isAI()
                            && (newUnit->getItemID() != Unit_Carryall)
                            && (newUnit->getItemID() != Unit_Harvester)
                            && (newUnit->getItemID() != Unit_MCV)) {
                            newUnit->doSetAttackMode(AREAGUARD);
                        } else{
                            // Don't want harvesters going to the rally point
                            destination = location;
                        }

                        if (destination.isValid()) {
                            newUnit->setGuardPoint(destination);
                            newUnit->setDestination(destination);
                            newUnit->setAngle(destinationDrawnAngle(newUnit->getLocation(), newUnit->getDestination()));
                        }

                        // inform owner of its new unit
                        newUnit->getOwner()->informWasBuilt(newUnitItemID);
                    }
                }

                std::list<BuildItem>::iterator iter2;
                for(iter2 = buildList.begin(); iter2 != buildList.end(); ++iter2) {
                    if(iter2->itemID == currentProductionQueue.front().itemID) {
                        iter2->num--;
                        break;
                    }
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
