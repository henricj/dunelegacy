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


#include <players/SmartBot.h>

#include <Game.h>
#include <GameInitSettings.h>
#include <Map.h>
#include <sand.h>
#include <House.h>

#include <structures/StructureBase.h>
#include <structures/BuilderBase.h>
#include <structures/StarPort.h>
#include <structures/ConstructionYard.h>
#include <units/UnitBase.h>
#include <units/MCV.h>
#include <units/Harvester.h>

#include <algorithm>

#define AIUPDATEINTERVAL 50
#define REFINERYLIMIT 10


SmartBot::SmartBot(House* associatedHouse, const std::string& playername, Difficulty difficulty)
 : Player(associatedHouse, playername), difficulty(difficulty) {
    SmartBot::init();

    buildTimer = getRandomGen().rand(0,3) * 50;
    attackTimer = getRandomGen().rand(MILLI2CYCLES(6*60*1000), MILLI2CYCLES(11*60*1000));
}


SmartBot::SmartBot(InputStream& stream, House* associatedHouse) : Player(stream, associatedHouse) {
    SmartBot::init();

    difficulty = static_cast<Difficulty>(stream.readUint8());
    attackTimer = stream.readSint32();
    buildTimer = stream.readSint32();

    Uint32 NumPlaceLocations = stream.readUint32();
    for(Uint32 i = 0; i < NumPlaceLocations; i++) {
        Sint32 x = stream.readSint32();
        Sint32 y = stream.readSint32();

        placeLocations.emplace_back(x,y);
    }
}


void SmartBot::init() {
    harvesterLimit = (currentGameMap->getSizeX() * currentGameMap->getSizeY())/512;
}


SmartBot::~SmartBot() = default;


void SmartBot::save(OutputStream& stream) const {
    Player::save(stream);

    stream.writeUint8(static_cast<Uint8>(difficulty));
    stream.writeSint32(attackTimer);
    stream.writeSint32(buildTimer);

    stream.writeUint32(placeLocations.size());
    for(const Coord& placeLocation : placeLocations) {
        stream.writeSint32(placeLocation.x);
        stream.writeSint32(placeLocation.y);
    }
}


void SmartBot::update() {
    if( (getGameCycleCount() + getHouse()->getHouseID()) % AIUPDATEINTERVAL != 0) {
        // we are not updating this AI player this cycle
        return;
    }

    checkAllUnits();

    if(buildTimer <= 0) {
        build();
    } else {
        buildTimer -= AIUPDATEINTERVAL;
    }

    if(attackTimer <= 0) {
        attack();
    } else {
        attackTimer -= AIUPDATEINTERVAL;
    }
}


void SmartBot::onObjectWasBuilt(const ObjectBase* pObject) {
}


void SmartBot::onDecrementStructures(int itemID, const Coord& location) {
}


void SmartBot::onDamage(const ObjectBase* pObject, int damage, Uint32 damagerID) {
    const ObjectBase* pDamager = getObject(damagerID);

    if(pDamager == nullptr || pDamager->getOwner() == getHouse()) {
        return;
    }

    if(pObject->isAStructure()) {
        //scramble some free units to defend
        scrambleUnitsAndDefend(pDamager);
    } else if(pObject->getItemID() == Unit_Harvester) {
        //scramble some free units to defend
        scrambleUnitsAndDefend(pDamager);

        if((pDamager != nullptr) && pDamager->isInfantry()) {
            doAttackObject(static_cast<const Harvester*>(pObject), pDamager, false);
        }
    } else if(pObject->isAUnit() && pObject->canAttack(pDamager)) {
        const UnitBase* pUnit = static_cast<const UnitBase*>(pObject);

        // if it is a rocket launcher and the distance is under 5 then run away!!
        if(pUnit->getItemID() == Unit_Launcher){
            doRepair(pUnit);

        }

        else if(pUnit->getAttackMode() == GUARD || pUnit->getAttackMode() == AMBUSH) {
            doSetAttackMode(pUnit, AREAGUARD);
            doAttackObject(pUnit, pDamager, false);
        } else if(pUnit->getAttackMode() == AREAGUARD) {
            doAttackObject(pUnit, pDamager, false);
        }
    }
}


void SmartBot::scrambleUnitsAndDefend(const ObjectBase* pIntruder) {
    for(const UnitBase* pUnit : getUnitList()) {
        if(pUnit->isRespondable() && (pUnit->getOwner() == getHouse())) {

            if((pUnit->getAttackMode() != HUNT) && !pUnit->hasATarget()) {
                Uint32 itemID = pUnit->getItemID();
                if((itemID != Unit_Harvester) && (pUnit->getItemID() != Unit_MCV) && (pUnit->getItemID() != Unit_Carryall)
                    && (pUnit->getItemID() != Unit_Frigate) && (pUnit->getItemID() != Unit_Saboteur) && (pUnit->getItemID() != Unit_Sandworm)) {

                    if(pUnit->getItemID() == Unit_Launcher ) {
                        doAttackObject(pUnit, pIntruder, true);
                    } else {
                        doMove2Pos(pUnit, pIntruder->getLocation().x, pIntruder->getLocation().y, false);

                        if(getGameInitSettings().getGameOptions().manualCarryallDrops
                            && pUnit->isAGroundUnit()
                            && pUnit->isVisible()
                            && (blockDistance(pUnit->getLocation(), pUnit->getDestination()) >= 10)
                            && (pUnit->getHealth() / pUnit->getMaxHealth() > BADLYDAMAGEDRATIO)) {

                            doRequestCarryallDrop(static_cast<const GroundUnit*>(pUnit));
                        }
                    }
                }
            }
        }
    }
}


Coord SmartBot::findPlaceLocation(Uint32 itemID) {
    int structureSizeX = getStructureSize(itemID).x;
    int structureSizeY = getStructureSize(itemID).y;

    int minX = getMap().getSizeX();
    int maxX = -1;
    int minY = getMap().getSizeY();
    int maxY = -1;

    if(itemID == Structure_ConstructionYard || itemID == Structure_Slab1) {
        // construction yard can only be build with mcv and thus be build anywhere
        minX = 0;
        minY = 0;
        maxX = getMap().getSizeX() - 1;
        maxY = getMap().getSizeY() - 1;
    } else {
        for(const StructureBase* pStructure : getStructureList()) {
            if (pStructure->getOwner() == getHouse()) {
                if (pStructure->getX() < minX)
                    minX = pStructure->getX();
                if (pStructure->getX() > maxX)
                    maxX = pStructure->getX();
                if (pStructure->getY() < minY)
                    minY = pStructure->getY();
                if (pStructure->getY() > maxY)
                    maxY = pStructure->getY();
            }
        }
    }

    // make search rect a bit bigger to make it possible to build on places far off the main base and only connected through slab
    minX -= structureSizeX + 5;
    maxX += 5;
    minY -= structureSizeY + 5;
    maxY += 5;
    if (minX < 0) minX = 0;
    if (maxX >= getMap().getSizeX()) maxX = getMap().getSizeX() - structureSizeX;
    if (minY < 0) minY = 0;
    if (maxY >= getMap().getSizeY()) maxY = getMap().getSizeY() - structureSizeY;

    FixPoint bestrating = 0;
    Coord bestLocation = Coord::Invalid();
    int count = 0;
    do {
        int x = getRandomGen().rand(minX, maxX);
        int y = getRandomGen().rand(minY, maxY);

        Coord pos = Coord(x, y);

        count++;

        if(getMap().okayToPlaceStructure(pos.x, pos.y, structureSizeX, structureSizeY, false, (itemID == Structure_ConstructionYard) ? nullptr : getHouse())
            && getMap().isAStructureGap(pos.x, pos.y, structureSizeX, structureSizeY)) { // Code to make a path between buildings
            FixPoint rating;

            switch(itemID) {
                case Structure_Slab1: {
                    rating = 10000000;
                } break;

                case Structure_Refinery: {
                    // place near spice
                    Coord spicePos;
                    if(getMap().findSpice(spicePos, pos)) {
                        rating = 10000000 - blockDistance(pos, spicePos);
                    } else {
                        rating = 10000000;
                    }
                } break;


                case Structure_ConstructionYard: {
                    FixPoint nearestUnit = 10000000;

                    for(const UnitBase* pUnit : getUnitList()) {
                        if(pUnit->getOwner() == getHouse()) {
                            FixPoint tmp = blockDistance(pos, pUnit->getLocation());
                            if(tmp < nearestUnit) {
                                nearestUnit = tmp;
                            }
                        }
                    }

                    rating = 10000000 - nearestUnit;
                } break;

                case Structure_Barracks:
                case Structure_HeavyFactory:
                case Structure_LightFactory:
                case Structure_RepairYard:
                case Structure_StarPort:
                case Structure_WOR: {
                    // place near sand
                    FixPoint nearestSand = 10000000;
                    for(int y = 0 ; y < currentGameMap->getSizeY(); y++) {
                        for(int x = 0; x < currentGameMap->getSizeX(); x++) {
                            if(currentGameMap->getTile(x,y)->isRock() == false) {
                                FixPoint tmp = blockDistance(pos, Coord(x,y));
                                if(tmp < nearestSand) {
                                    nearestSand = tmp;
                                }
                            }
                        }
                    }

                    rating = 10000000 - nearestSand;
                    rating *= (1+getNumAdjacentStructureTiles(pos, structureSizeX, structureSizeY));
                } break;

                case Structure_Wall:
                case Structure_GunTurret:
                case Structure_RocketTurret: {
                    // place towards enemy
                    FixPoint nearestEnemy = 10000000;

                    for(const StructureBase* pStructure : getStructureList()) {
                        if(pStructure->getOwner()->getTeamID() != getHouse()->getTeamID()) {
                            FixPoint dist = blockDistance(pos, pStructure->getLocation());
                            if(dist < nearestEnemy) {
                                nearestEnemy = dist;
                            }

                        }
                    }

                    rating = 10000000 - nearestEnemy;
                } break;

                case Structure_HighTechFactory:
                case Structure_IX:
                case Structure_Palace:
                case Structure_Radar:
                case Structure_Silo:
                case Structure_WindTrap:
                default: {
                    // place at a save place
                    FixPoint nearestEnemy = 10000000;

                    for(const StructureBase* pStructure : getStructureList()) {
                        if(pStructure->getOwner()->getTeamID() != getHouse()->getTeamID()) {
                            FixPoint dist = blockDistance(pos, pStructure->getLocation());
                            if(dist < nearestEnemy) {
                                nearestEnemy = dist;
                            }
                        }
                    }

                    rating = nearestEnemy;
                    rating *= (1+getNumAdjacentStructureTiles(pos, structureSizeX, structureSizeY));
                } break;
            }

            if(rating > bestrating) {
                bestLocation = pos;
                bestrating = rating;
            }
        }

    } while(count <= ((itemID == Structure_ConstructionYard) ? 10000 : 100));

    return bestLocation;
}


int SmartBot::getNumAdjacentStructureTiles(Coord pos, int structureSizeX, int structureSizeY) {
    int numAdjacentStructureTiles = 0;

    for(int y = pos.y; y < pos.y + structureSizeY; y++) {
        if(getMap().tileExists(pos.x-1, y) && getMap().getTile(pos.x-1, y)->hasAStructure()) {
            numAdjacentStructureTiles++;
        }
        if(getMap().tileExists(pos.x+structureSizeX, y) && getMap().getTile(pos.x+structureSizeX, y)->hasAStructure()) {
            numAdjacentStructureTiles++;
        }
    }

    for(int x = pos.x; x < pos.x + structureSizeX; x++) {
        if(getMap().tileExists(x, pos.y-1) && getMap().getTile(x, pos.y-1)->hasAStructure()) {
            numAdjacentStructureTiles++;
        }
        if(getMap().tileExists(x, pos.y+structureSizeY) && getMap().getTile(x, pos.y+structureSizeY)->hasAStructure()) {
            numAdjacentStructureTiles++;
        }
    }

    return numAdjacentStructureTiles;
}


void SmartBot::build() {
    // Lets count what we are building
    int buildQueue[ItemID_LastID] = {};

    for(const StructureBase* pStructure : getStructureList()) {
        if(pStructure->getOwner() == getHouse() && pStructure->isABuilder()) {
            const BuilderBase* pBuilder = static_cast<const BuilderBase*>(pStructure);
            if(pBuilder->getBuildListSize() > 0){
                buildQueue[pBuilder->getCurrentProducedItem()]++;
            }
        }
    }

    for(const StructureBase* pStructure : getStructureList()) {
        if(pStructure->getOwner() == getHouse()) {

            if((pStructure->isRepairing() == false)
               && (pStructure->getHealth() < pStructure->getMaxHealth()))
            {
                doRepair(pStructure);
            }

            if(pStructure->isABuilder()) {
                const BuilderBase* pBuilder = static_cast<const BuilderBase*>(pStructure);

                switch (pBuilder->getItemID()) {

                    case Structure_HeavyFactory: {

                        // only if the factory isn't busy
                        if((pBuilder->isUpgrading() == false) && (pBuilder->getProductionQueueSize() < 1)){

                            // we need a construction yard. Build an MCV if we don't have a starport
                            // or if we are really rich
                            if(getHouse()->getNumItems(Structure_ConstructionYard) < 1
                                && getHouse()->getNumItems(Unit_MCV) < 1
                                && getHouse()->getNumItems(Structure_StarPort) < 1
                                && pBuilder->isAvailableToBuild(Unit_MCV)){

                               doProduceItem(pBuilder, Unit_MCV);
                            }

                            // If we are really rich, like in all against atriedes
                            if((getHouse()->getNumItems(Structure_ConstructionYard) + getHouse()->getNumItems(Unit_MCV))*25000 < getHouse()->getCredits()
                                && pBuilder->isAvailableToBuild(Unit_MCV)){

                               doProduceItem(pBuilder, Unit_MCV);
                            }

                            // In case we get given lots of money, it will eventually run out so we need to be prepared
                            if((getHouse()->getNumItems(Unit_Harvester) < (getHouse()->getNumItems(Unit_SiegeTank)
                                                                          + getHouse()->getNumItems(Unit_Launcher)
                                                                          + getHouse()->getNumItems(Unit_Tank)
                                                                          + getHouse()->getNumItems(Unit_Ornithopter))/2.5_fix)
                                    && (getHouse()->getNumItems(Unit_Harvester) < harvesterLimit )) {
                                    doProduceItem(pBuilder, Unit_Harvester);
                            }


                            if(focusMilitary()
                               && (pBuilder->getHealth() >= pBuilder->getMaxHealth())
                               && (pBuilder->isUpgrading() == false)
                               && (pBuilder->getCurrentUpgradeLevel() < pBuilder->getMaxUpgradeLevel()))
                            {
                                doUpgrade(pBuilder);
                                continue;
                            }

                            else if((pBuilder->getProductionQueueSize() < 1)
                                    && (pBuilder->getBuildListSize() > 0)) {

                                if(getHouse()->getNumItems(Unit_Harvester) < harvesterLimit
                                   && focusEconomy()) {
                                    doProduceItem(pBuilder, Unit_Harvester);
                                } else if(focusMilitary()) {


                                    if(pBuilder->isAvailableToBuild(Unit_Launcher)
                                      &&((getHouse()->getNumItems(Unit_Tank) / 1.5_fix)
                                         + getHouse()->getNumItems(Unit_SiegeTank)
                                         + getHouse()->getNumItems(Unit_Devastator)
                                         > getHouse()->getNumItems(Unit_Launcher) * 2)){

                                        doProduceItem(pBuilder, Unit_Launcher);


                                    }
                                    else if(pBuilder->isAvailableToBuild(Unit_SiegeTank)) {
                                        doProduceItem(pBuilder, Unit_SiegeTank);

                                    } else if(pBuilder->isAvailableToBuild(Unit_Tank)) {
                                        doProduceItem(pBuilder, Unit_Tank);
                                    }
                                }
                            }
                        }
                    } break;

                    case Structure_HighTechFactory: {
                        if(focusMilitary() && pBuilder->getProductionQueueSize() < 1) {

                            if(getHouse()->getNumItems(Unit_Carryall) < 1
                               || getHouse()->getNumItems(Unit_Carryall) <
                                  (getHouse()->getNumItems(Structure_Refinery)
                                   + getHouse()->getNumItems(Structure_RepairYard))) {
                                doProduceItem(pBuilder, Unit_Carryall);
                            }
                        }


                    } break;

                   case Structure_StarPort: {
                        const StarPort* pStarPort = static_cast<const StarPort*>(pBuilder);
                        if(pStarPort->okToOrder())  {
                            const Choam& choam = getHouse()->getChoam();

                            // What's our spending money
                            int money = getHouse()->getCredits();

                            // We need a construction yard!!
                            if(pStarPort->isAvailableToBuild(Unit_MCV)
                               && choam.getNumAvailable(Unit_MCV) > 0
                               && getHouse()->getNumItems(Structure_ConstructionYard) < 1
                               && getHouse()->getNumItems(Unit_MCV)
                               + buildQueue[Unit_MCV] < 1) {

                                doProduceItem(pBuilder, Unit_MCV);
                                money = money - choam.getPrice(Unit_MCV);
                            }

                            while (money > choam.getPrice(Unit_Harvester)
                                   && choam.getNumAvailable(Unit_Harvester) > 0
                                   && getHouse()->getNumItems(Unit_Harvester) + buildQueue[Unit_Harvester] < harvesterLimit) {
                                doProduceItem(pBuilder, Unit_Harvester);
                                buildQueue[Unit_Harvester]++;
                                money = money - choam.getPrice(Unit_Harvester);
                            }

                            while (money > choam.getPrice(Unit_Carryall) && choam.getNumAvailable(Unit_Carryall) > 0
                                   && (getHouse()->getNumItems(Unit_Carryall) + buildQueue[Unit_Carryall] < 2)) {
                                doProduceItem(pBuilder, Unit_Carryall);
                                buildQueue[Unit_Carryall]++;
                                money = money - choam.getPrice(Unit_Carryall);
                            }

                            if(focusMilitary()) {
                                /*
                                while (money > choam.getPrice(Unit_Ornithopter) && choam.getNumAvailable(Unit_Ornithopter) > 0)
                                {
                                    doProduceItem(pBuilder, Unit_Ornithopter);
                                    money = money - choam.getPrice(Unit_Ornithopter);
                                }
                                */

                                while (money > choam.getPrice(Unit_SiegeTank) && choam.getNumAvailable(Unit_SiegeTank) > 0 && choam.isCheap(Unit_SiegeTank)) {
                                    doProduceItem(pBuilder, Unit_SiegeTank);
                                    money = money - choam.getPrice(Unit_SiegeTank);
                                }

                                while (money > choam.getPrice(Unit_Launcher) && choam.getNumAvailable(Unit_Launcher) > 0 && choam.isCheap(Unit_Launcher)) {
                                    doProduceItem(pBuilder, Unit_Launcher);
                                    money = money - choam.getPrice(Unit_Launcher);
                                }

                                while (money > choam.getPrice(Unit_Tank) && choam.getNumAvailable(Unit_Tank) > 0 && choam.isCheap(Unit_Tank)) {
                                    doProduceItem(pBuilder, Unit_Tank);
                                    money = money - choam.getPrice(Unit_Tank);
                                }

                                while (money > choam.getPrice(Unit_Quad) && choam.getNumAvailable(Unit_Quad) > 0 && choam.isCheap(Unit_Quad)) {
                                    doProduceItem(pBuilder, Unit_Quad);
                                    money = money - choam.getPrice(Unit_Quad);
                                }

                                while (money > choam.getPrice(Unit_Trike) && choam.getNumAvailable(Unit_Trike) > 0 && choam.isCheap(Unit_Trike)) {
                                    doProduceItem(pBuilder, Unit_Trike);
                                    money = money - choam.getPrice(Unit_Trike);
                                }
                            }

                            if(pStarPort->isAvailableToBuild(Unit_MCV) && choam.getNumAvailable(Unit_MCV) > 0 && focusBase()) {
                                doProduceItem(pBuilder, Unit_MCV);
                            }

                            doPlaceOrder(pStarPort);
                        }

                    } break;

                    case Structure_ConstructionYard: {
                        // For maps where concrete is required you want to be able to place 4 squares
                        if(getGameInitSettings().getGameOptions().concreteRequired) {
                            doUpgrade(pBuilder);
                        }

                        if(!pBuilder->isUpgrading()
                           && getHouse()->getCredits() > 0
                           && pBuilder->getProductionQueueSize() < 1
                           && pBuilder->getBuildListSize() > 0){

                            Uint32 itemID = NONE_ID;

                            if(getHouse()->getNumItems(Structure_WindTrap) + buildQueue[Structure_WindTrap] < 1
                               && pBuilder->isAvailableToBuild(Structure_WindTrap)) {
                                // We need one wind trap
                                itemID = Structure_WindTrap;
                                buildQueue[Structure_WindTrap]++;
                            } else if(getHouse()->getNumItems(Structure_Refinery) + buildQueue[Structure_Refinery]  < 1
                                     && pBuilder->isAvailableToBuild(Structure_Refinery)) {
                                // We need one refinery
                                itemID = Structure_Refinery;
                                buildQueue[Structure_Refinery]++;
                            } else if(pBuilder->isAvailableToBuild(Structure_Refinery)
                                    && ((focusEconomy()
                                     || (getHouse()->getNumItems(Structure_Refinery) +  buildQueue[Structure_Refinery])  * 3 < getHouse()->getNumItems(Unit_Harvester)
                                     || getHouse()->getNumItems(Structure_Refinery) +  buildQueue[Structure_Refinery] <
                                        getHouse()->getNumItems(Structure_HeavyFactory) + buildQueue[Structure_HeavyFactory]
                                        + getHouse()->getNumItems(Structure_HighTechFactory) + buildQueue[Structure_HighTechFactory])
                                      && (getHouse()->getNumItems(Structure_Refinery) +  buildQueue[Structure_Refinery] < REFINERYLIMIT
                                      && getHouse()->getNumItems(Unit_Harvester) < harvesterLimit
                                         ))){
                                // The most important element of success in dune is having a strong economy
                                // Due to the exponential growth available, focussing heavily on refinerys
                                // at the beginning of the game will give you a strong advantage in the mid game
                                // provided you are not suprised by your enemy early.
                                // => Build if we haven't exceeded the refinery limit
                                itemID = Structure_Refinery;
                                buildQueue[Structure_Refinery]++;
                            } else if(getHouse()->getNumItems(Structure_StarPort) + buildQueue[Structure_StarPort] < 1
                                    && pBuilder->isAvailableToBuild(Structure_StarPort)) {
                                // We need one starport, light factory, radar and heavy factory, high tech factory
                                // TODO: build in some logic to ensure you don't build the same unit in multiple construction yards...
                                itemID = Structure_StarPort;
                                buildQueue[Structure_StarPort]++;
                            } else if((getHouse()->getNumItems(Structure_LightFactory) + buildQueue[Structure_LightFactory] < 1)
                                    && pBuilder->isAvailableToBuild(Structure_LightFactory)) {
                                itemID = Structure_LightFactory;
                                buildQueue[Structure_LightFactory]++;
                            } else if(getHouse()->getNumItems(Structure_Radar) + buildQueue[Structure_Radar] < 1
                                    && pBuilder->isAvailableToBuild(Structure_Radar)) {
                                itemID = Structure_Radar;
                                buildQueue[Structure_Radar]++;
                            } else if(getHouse()->getNumItems(Structure_HeavyFactory) + buildQueue[Structure_HeavyFactory]  < 1
                                     && pBuilder->isAvailableToBuild(Structure_HeavyFactory)) {
                                itemID = Structure_HeavyFactory;
                                buildQueue[Structure_HeavyFactory] ++;
                            } else if(getHouse()->getNumItems(Structure_RepairYard) + buildQueue[Structure_RepairYard]  < 1
                                     && pBuilder->isAvailableToBuild(Structure_RepairYard)) {
                                itemID = Structure_RepairYard;
                                buildQueue[Structure_RepairYard]++;
                            } else if(getHouse()->getNumItems(Structure_HighTechFactory) + buildQueue[Structure_HighTechFactory]  < 1
                                     && pBuilder->isAvailableToBuild(Structure_HighTechFactory)) {
                                itemID = Structure_HighTechFactory;
                                buildQueue[Structure_HighTechFactory]++;
                            } else if(getHouse()->getNumItems(Structure_IX) + buildQueue[Structure_IX] < 1
                                     && pBuilder->isAvailableToBuild(Structure_IX)) {
                                itemID = Structure_IX;
                                buildQueue[Structure_IX]++;
                            } else if( !(pBuilder->isAvailableToBuild(Structure_HeavyFactory)) //There are no heavy factories availables
                                    && pBuilder->isAvailableToBuild(Structure_LightFactory)
                                    && focusFactory()) {
                                itemID = Structure_LightFactory;
                                buildQueue[Structure_LightFactory]++;
                            } else if(focusFactory()) {
                                /*
                                    If we have lots of money, lets increase war production. Here are some different strategies
                                    It makes sense for only one strategy to be used per game due to the lack of a 'combined arms'
                                    attack algorithm. Because all units are simply set to 'Hunt', if they have different speeds
                                    the attack is not consolidated. Therefore we want an attack comprised of only one unit type in
                                    order to maximise impact.

                                    We have too much money, we need more heavy factories to spend it
                                    but we want to make sure we still have plenty of repair yards as they win games
                                */

                                if(pBuilder->isAvailableToBuild(Structure_RepairYard)
                                   && getHouse()->getNumItems(Structure_RepairYard) + buildQueue[Structure_RepairYard]
                                    < (getHouse()->getNumItems(Unit_Tank)
                                       + getHouse()->getNumItems(Unit_SiegeTank)
                                       + getHouse()->getNumItems(Unit_Launcher) )/ 7){
                                    itemID = Structure_RepairYard;
                                    buildQueue[Structure_RepairYard]++;
                                } else {
                                    itemID = Structure_HeavyFactory;
                                    buildQueue[Structure_HeavyFactory]++;
                                }
                            } else if( pBuilder->getCurrentUpgradeLevel()  < 2
                                        && pBuilder->getHealth() >= pBuilder->getMaxHealth()
                                        && pBuilder->isUpgrading() == false
                                        && pBuilder->getCurrentUpgradeLevel() < pBuilder->getMaxUpgradeLevel()
                                        && pBuilder->getBuildListSize() < 1) {
                                // Only upgrade to level 1 and only if concrete slabs are required
                                doUpgrade(pBuilder);
                            } else if (pBuilder->isAvailableToBuild(Structure_RocketTurret)
                                         && focusMilitary()
                                         && difficulty == Difficulty::Defense) {
                                itemID = Structure_RocketTurret;
                            }

                            if(itemID != NONE_ID) {
                                Coord location = findPlaceLocation(itemID);

                                if(location.isValid()) {
                                    Coord placeLocation = location;
                                    if(getGameInitSettings().getGameOptions().concreteRequired) {
                                        int incI;
                                        int incJ;
                                        int startI;
                                        int startJ;

                                        if(getMap().isWithinBuildRange(location.x, location.y, getHouse())) {
                                            startI = location.x; startJ = location.y; incI = 1; incJ = 1;
                                        } else if(getMap().isWithinBuildRange(location.x + getStructureSize(itemID).x - 1, location.y, getHouse())) {
                                            startI = location.x + getStructureSize(itemID).x - 1; startJ = location.y; incI = -1; incJ = 1;
                                        } else if(getMap().isWithinBuildRange(location.x, location.y + getStructureSize(itemID).y - 1, getHouse())) {
                                            startI = location.x; startJ = location.y + getStructureSize(itemID).y - 1; incI = 1; incJ = -1;
                                        } else {
                                            startI = location.x + getStructureSize(itemID).x - 1; startJ = location.y + getStructureSize(itemID).y - 1; incI = -1; incJ = -1;
                                        }

                                        for(int i = startI; abs(i - startI) < getStructureSize(itemID).x; i += incI) {
                                            for(int j = startJ; abs(j - startJ) < getStructureSize(itemID).y; j += incJ) {
                                                const Tile *pTile = getMap().getTile(i, j);

                                                if((getStructureSize(itemID).x > 1) && (getStructureSize(itemID).y > 1)
                                                    && pBuilder->isAvailableToBuild(Structure_Slab4)
                                                    && (abs(i - location.x) < 2) && (abs(j - location.y) < 2)) {
                                                    if( (i == location.x) && (j == location.y) && pTile->getType() != Terrain_Slab) {
                                                        placeLocations.emplace_back(i,j);
                                                        doProduceItem(pBuilder, Structure_Slab4);
                                                    }
                                                } else if(pTile->getType() != Terrain_Slab) {
                                                    placeLocations.emplace_back(i,j);
                                                    doProduceItem(pBuilder, Structure_Slab1);
                                                }
                                            }
                                        }
                                    }

                                    placeLocations.push_back(placeLocation);
                                    doProduceItem(pBuilder, itemID);
                                } else {
                                    // we havn't found a placing location => build some random slabs
                                    location = findPlaceLocation(Structure_Slab1);
                                    if(location.isValid() && getMap().isWithinBuildRange(location.x, location.y, getHouse())) {
                                        placeLocations.push_back(location);
                                        doProduceItem(pBuilder, Structure_Slab1);
                                    }
                                }
                            }

                        }

                        if(pBuilder->isWaitingToPlace()) {
                            //find total region of possible placement and place in random ok position
                            int itemID = pBuilder->getCurrentProducedItem();
                            Coord itemsize = getStructureSize(itemID);

                            //see if there is already a spot to put it stored
                            if(!placeLocations.empty()) {
                                Coord location = placeLocations.front();
                                const ConstructionYard* pConstYard = static_cast<const ConstructionYard*>(pBuilder);
                                if(getMap().okayToPlaceStructure(location.x, location.y, itemsize.x, itemsize.y, false, pConstYard->getOwner())
                                   && getMap().isAStructureGap(location.x, location.y, itemsize.x, itemsize.y)) {
                                    doPlaceStructure(pConstYard, location.x, location.y);
                                    placeLocations.pop_front();
                                } else if(itemID == Structure_Slab1) {
                                    //forget about concrete
                                    doCancelItem(pConstYard, Structure_Slab1);
                                    placeLocations.pop_front();
                                } else if(itemID == Structure_Slab4) {
                                    //forget about concrete
                                    doCancelItem(pConstYard, Structure_Slab4);
                                    placeLocations.pop_front();
                                } else {
                                    //cancel item
                                    doCancelItem(pConstYard, itemID);
                                    placeLocations.pop_front();
                                }
                            }
                        }

                    } break;

                    default: {
                        break;
                    }
                }
            }
        }

    }

    buildTimer = getRandomGen().rand(0,3)*50;
}


void SmartBot::attack() {
    if(difficulty == Difficulty::Defense && getHouse()->getNumItems(Unit_Ornithopter) < 21){
        attackTimer = getRandomGen().rand(10000, 20000);
        return;
    }

    for(const UnitBase *pUnit : getUnitList()) {
        if (pUnit->isRespondable()
            && (pUnit->getOwner() == getHouse())
            && pUnit->isActive()
            && (pUnit->getAttackMode() == AREAGUARD || pUnit->getAttackMode() == GUARD || pUnit->getAttackMode() == AMBUSH)
            && pUnit->getItemID() != Unit_Harvester
            && pUnit->getItemID() != Unit_MCV
            && pUnit->getItemID() != Unit_Carryall
            && (pUnit->getItemID() != Unit_Ornithopter || getHouse()->getNumItems(Unit_Ornithopter) > 20)
            && pUnit->getAttackMode() != HUNT)
        {
            doSetAttackMode(pUnit, HUNT);
        }

    }

    //reset timer for next attack
    attackTimer = getRandomGen().rand(10000, 20000);
}


void SmartBot::checkAllUnits() {
    for(const UnitBase* pUnit : getUnitList()) {
        if(pUnit->getItemID() == Unit_Sandworm) {
                for(const UnitBase* pUnit2 : getUnitList()) {
                    if(pUnit2->getOwner() == getHouse() && pUnit2->getItemID() == Unit_Harvester) {
                        const Harvester* pHarvester = static_cast<const Harvester*>(pUnit2);
                        if( getMap().tileExists(pHarvester->getLocation())
                            && !getMap().getTile(pHarvester->getLocation())->isRock()
                            && blockDistance(pUnit->getLocation(), pHarvester->getLocation()) <= 5) {
                            doReturn(pHarvester);
                            scrambleUnitsAndDefend(pUnit);
                        }
                    }
                }
        }


        if(pUnit->getOwner() != getHouse()) {
            continue;
        }

        switch(pUnit->getItemID()) {
            case Unit_MCV: {
                const MCV* pMCV = static_cast<const MCV*>(pUnit);
                if(!pMCV->isMoving()) {
                    if(pMCV->canDeploy()) {
                        doDeploy(pMCV);
                    } else {
                        Coord pos = findPlaceLocation(Structure_ConstructionYard);
                        doMove2Pos(pMCV, pos.x, pos.y, true);
                    }
                }
            } break;

            case Unit_Harvester: {
                const Harvester* pHarvester = static_cast<const Harvester*>(pUnit);
                if(getHouse()->getCredits() < 1000 && pHarvester->getAmountOfSpice() >= HARVESTERMAXSPICE/2) {
                    doReturn(pHarvester);
                }
            } break;

            default: {
                if(pUnit->getAttackMode() == GUARD){
                    doSetAttackMode(pUnit, AREAGUARD);
                }
            } break;
        }
    }
}


bool SmartBot::focusEconomy(){
    return (getHouse()->getCredits() < getRandomGen().rand(0, 4000));
}


bool SmartBot::focusMilitary(){
    return (getHouse()->getCredits() > getRandomGen().rand(2000, 4000));
}


bool SmartBot::focusFactory(){
    return (getHouse()->getCredits() > getRandomGen().rand(3000, 6000));
}


bool SmartBot::focusBase(){
    return (getHouse()->getCredits() > getRandomGen().rand(6000, 9000));
}


int SmartBot::getMaxHarvester() const {
    return 3*getHouse()->getNumItems(Structure_Refinery);
}
