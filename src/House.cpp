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

#include <House.h>

#include <globals.h>
#include <sand.h>

#include <FileClasses/TextManager.h>

#include <players/PlayerFactory.h>
#include <players/AIPlayer.h>
#include <players/QuantBot.h>
#include <players/HumanPlayer.h>


#include <Game.h>
#include <GameInterface.h>
#include <Map.h>
#include <SoundPlayer.h>

#include <structures/StructureBase.h>
#include <structures/BuilderBase.h>
#include <structures/Refinery.h>
#include <structures/ConstructionYard.h>
#include <units/Carryall.h>
#include <units/Harvester.h>

#include <misc/exceptions.h>
#include <fmt/printf.h>

#include <algorithm>
#include <numeric>


House::House(const GameContext& context) : choam(this), context(context) {
    ai = true;

    numUnits      = 0;
    numStructures = 0;
    for(int i = 0; i < Num_ItemID; i++) {
        numItem[i]                = 0;
        numItemBuilt[i]           = 0;
        numItemKills[i]           = 0;
        numItemLosses[i]          = 0;
        numItemDamageInflicted[i] = 0;
    }

    capacity         = 0;
    powerRequirement = 0;

    numVisibleEnemyUnits    = 0;
    numVisibleFriendlyUnits = 0;
}


House::~House() = default;


House::House(const GameContext& context, HOUSETYPE newHouse, int newCredits, int maxUnits, Uint8 teamID, int quota) : House(context) {
    houseID = ((static_cast<int>(newHouse) >= 0) && (newHouse < HOUSETYPE::NUM_HOUSES)) ? newHouse :  static_cast<HOUSETYPE>(0);
    this->teamID = teamID;

    storedCredits = 0;
    startingCredits = newCredits;
    oldCredits = lround(storedCredits+startingCredits);

    this->maxUnits = maxUnits;
    this->quota = quota;

    bHadContactWithEnemy = false;
    bHadDirectContactWithEnemy = false;

    unitBuiltValue = 0;
    structureBuiltValue = 0;
    militaryValue = 0;
    killValue = 0;
    lossValue = 0;
    numBuiltUnits = 0;
    numBuiltStructures = 0;
    destroyedValue = 0;
    numDestroyedUnits = 0;
    numDestroyedStructures = 0;
    harvestedSpice = 0;
    producedPower = 0;
    powerUsageTimer = 0;
}


House::House(const GameContext& context, InputStream& stream) : House(context) {
    houseID = static_cast<HOUSETYPE>(stream.readUint8());
    teamID = stream.readUint8();

    storedCredits = stream.readFixPoint();
    startingCredits = stream.readFixPoint();
    oldCredits = lround(storedCredits+startingCredits);
    maxUnits = stream.readSint32();
    quota = stream.readSint32();

    stream.readBools(&bHadContactWithEnemy, &bHadDirectContactWithEnemy);

    unitBuiltValue = stream.readUint32();
    structureBuiltValue = stream.readUint32();
    militaryValue = stream.readUint32();
    killValue = stream.readUint32();
    lossValue = stream.readUint32();
    numBuiltUnits = stream.readUint32();
    numBuiltStructures = stream.readUint32();
    destroyedValue = stream.readUint32();
    numDestroyedUnits = stream.readUint32();
    numDestroyedStructures = stream.readUint32();
    harvestedSpice = stream.readFixPoint();
    producedPower = stream.readSint32();
    powerUsageTimer = stream.readSint32();

    choam.load(stream);

    Uint32 numAITeams = stream.readUint32();
    for(Uint32 i = 0; i < numAITeams; i++) {
        aiteams.emplace_back(stream);
    }

    Uint32 numPlayers = stream.readUint32();
    for(Uint32 i = 0; i < numPlayers; i++) {
        std::string playerclass = stream.readString();
        const auto* pPlayerData = PlayerFactory::getByPlayerClass(playerclass);
        if(pPlayerData == nullptr) {
            sdl2::log_info("Warning: Cannot load player '%s'", playerclass.c_str());
        } else {
            addPlayer(pPlayerData->load(context, stream, this));
        }
    }
}

UnitBase* House::createUnit(ItemID_enum itemID, bool byScenario) {
    return context.objectManager.createObjectFromItemId<UnitBase>(itemID, ObjectInitializer{this, byScenario});
}

StructureBase* House::createStructure(ItemID_enum itemID, bool byScenario) {
    return context.objectManager.createObjectFromItemId<StructureBase>(itemID, ObjectInitializer{this, byScenario});
}


void House::save(OutputStream& stream) const {
    stream.writeUint8(static_cast<Uint8>(houseID));
    stream.writeUint8(teamID);

    stream.writeFixPoint(storedCredits);
    stream.writeFixPoint(startingCredits);
    stream.writeSint32(maxUnits);
    stream.writeSint32(quota);

    stream.writeBools(bHadContactWithEnemy, bHadDirectContactWithEnemy);

    stream.writeUint32(unitBuiltValue);
    stream.writeUint32(structureBuiltValue);
    stream.writeUint32(militaryValue);
    stream.writeUint32(killValue);
    stream.writeUint32(lossValue);
    stream.writeUint32(numBuiltUnits);
    stream.writeUint32(numBuiltStructures);
    stream.writeUint32(destroyedValue);
    stream.writeUint32(numDestroyedUnits);
    stream.writeUint32(numDestroyedStructures);
    stream.writeFixPoint(harvestedSpice);
    stream.writeSint32(producedPower);
    stream.writeSint32(powerUsageTimer);

    choam.save(stream);

    stream.writeUint32(aiteams.size());
    for(const auto& aiteam : aiteams) {
        aiteam.save(stream);
    }

    stream.writeUint32(players.size());
    for(const auto& pPlayer : players) {
        stream.writeString(pPlayer->getPlayerclass());
        pPlayer->save(stream);
    }
}




void House::addPlayer(std::unique_ptr<Player> newPlayer) {
    Player* pNewPlayer = newPlayer.get();

    ai = !(dynamic_cast<HumanPlayer*>(pNewPlayer) != nullptr && players.empty());

    players.push_back(std::move(newPlayer));

    auto newPlayerID = static_cast<Uint8>((static_cast<Uint8>(houseID) << 4) | players.size());
    pNewPlayer->playerID = newPlayerID;

    context.game.registerPlayer(pNewPlayer);
}


void House::setProducedPower(int newPower) {
    producedPower = newPower;
}


void House::addCredits(FixPoint newCredits, bool wasRefined) {
    if(newCredits <= 0) return;

    if(wasRefined) {
        harvestedSpice += newCredits;
    }

    storedCredits += newCredits;
    if(this == pLocalHouse) {
        if(((context.game.winFlags & WINLOSEFLAGS_QUOTA) != 0) && (quota != 0)) {
            if(storedCredits >= quota) {
                win();
            }
        }
    }
}




void House::returnCredits(FixPoint newCredits) {
    if(newCredits <= 0) return;

    const auto leftCapacity = capacity - storedCredits;
    if(newCredits <= leftCapacity) {
        addCredits(newCredits, false);
    } else {
        addCredits(leftCapacity, false);
        startingCredits += (newCredits - leftCapacity);
    }
}




FixPoint House::takeCredits(FixPoint amount) {
    FixPoint taken = 0;

    if(getCredits() >= 1) {
        if(storedCredits > amount) {
            taken = amount;
            storedCredits -= amount;
        } else {
            taken = storedCredits;
            storedCredits = 0;

            if(startingCredits > (amount - taken)) {
                startingCredits -= (amount - taken);
                taken = amount;
            } else {
                taken += startingCredits;
                startingCredits = 0;
            }
        }
    }

    return taken;   //the amount that was actually withdrawn
}




void House::printStat() const {
    sdl2::log_info("House %s: (Number of Units: %d, Number of Structures: %d)", getHouseNameByNumber(getHouseID()).c_str(),
            numUnits, numStructures);
    sdl2::log_info("Barracks: %d\t\tWORs: %d", numItem[Structure_Barracks], numItem[Structure_WOR]);
    sdl2::log_info("Light Factories: %d\tHeavy Factories: %d", numItem[Structure_LightFactory],
            numItem[Structure_HeavyFactory]);
    sdl2::log_info("IXs: %d\t\t\tPalaces: %d", numItem[Structure_IX], numItem[Structure_Palace]);
    sdl2::log_info("Repair Yards: %d\t\tHigh-Tech Factories: %d", numItem[Structure_RepairYard],
            numItem[Structure_HighTechFactory]);
    sdl2::log_info("Refineries: %d\t\tStarports: %d", numItem[Structure_Refinery], numItem[Structure_StarPort]);
    sdl2::log_info("Walls: %d\t\tRocket Turrets: %d", numItem[Structure_Wall], numItem[Structure_RocketTurret]);
    sdl2::log_info("Gun Turrets: %d\t\tConstruction Yards: %d", numItem[Structure_GunTurret],
            numItem[Structure_ConstructionYard]);
    sdl2::log_info("Windtraps: %d\t\tRadars: %d", numItem[Structure_WindTrap], numItem[Structure_Radar]);
    sdl2::log_info("Silos: %d", numItem[Structure_Silo]);
    sdl2::log_info("Carryalls: %d\t\tFrigates: %d", numItem[Unit_Carryall], numItem[Unit_Frigate]);
    sdl2::log_info("Devastators: %d\t\tDeviators: %d", numItem[Unit_Devastator], numItem[Unit_Deviator]);
    sdl2::log_info("Soldiers: %d\t\tTrooper: %d", numItem[Unit_Soldier], numItem[Unit_Trooper]);
    sdl2::log_info("Saboteur: %d\t\tSandworms: %d", numItem[Unit_Saboteur], numItem[Unit_Sandworm]);
    sdl2::log_info("Quads: %d\t\tTrikes: %d", numItem[Unit_Quad], numItem[Unit_Trike]);
    sdl2::log_info("Raiders: %d\t\tTanks: %d", numItem[Unit_RaiderTrike], numItem[Unit_Tank]);
    sdl2::log_info("Siege Tanks : %d\t\tSonic Tanks: %d", numItem[Unit_SiegeTank], numItem[Unit_SonicTank]);
    sdl2::log_info("Harvesters: %d\t\tMCVs: %d", numItem[Unit_Harvester], numItem[Unit_MCV]);
    sdl2::log_info("Ornithopters: %d\t\tRocket Launchers: %d", numItem[Unit_Ornithopter], numItem[Unit_Launcher]);
}




void House::updateBuildLists() {
    for(auto* pStructure : structureList) {
        auto* builder = dune_cast<BuilderBase>(pStructure);
        if(!builder || builder->getOwner() != this) continue;

        builder->updateBuildList();
    }
}




void House::update() {
    numVisibleEnemyUnits = 0;
    numVisibleFriendlyUnits = 0;

    if (oldCredits != getCredits()) {
        if((this == pLocalHouse) && (getCredits() > 0)) {
            soundPlayer->playSound(Sound_CreditsTick);
        }
        oldCredits = getCredits();
    }

    if(storedCredits > capacity) {
        --storedCredits;
        if(storedCredits < 0) {
         storedCredits = 0;
        }

        if(this == pLocalHouse) {
            context.game.addToNewsTicker(_("@DUNE.ENG|145#As insufficient spice storage is available, spice is lost."));
        }
    }

    powerUsageTimer--;
    if(powerUsageTimer <= 0) {
        powerUsageTimer = MILLI2CYCLES(15*1000);
        takeCredits(FixPoint(getPowerRequirement()) / 32);
    }

    choam.update(context);

    for(auto& pPlayer : players) {
        pPlayer->update();
    }
}




void House::incrementUnits(ItemID_enum itemID) {
    numUnits++;
    numItem[itemID]++;

    assert(numUnits + numStructures == std::accumulate(std::begin(numItem), std::end(numItem), 0));

    if(itemID != Unit_Saboteur && itemID != Unit_Frigate && itemID != Unit_Carryall && itemID != Unit_MCV &&
       itemID != Unit_Harvester && itemID != Unit_Sandworm) {

        militaryValue += context.game.objectData.data[itemID][static_cast<int>(houseID)].price;
    }
}



void House::decrementUnits(ItemID_enum itemID) {
    if(numUnits < 1) THROW(std::runtime_error, "Cannot decrement number of units %d (itemId %d)", numUnits, itemID);

    numUnits--;
    numItemLosses[itemID]++;

    if(itemID == Unit_Harvester) {
        decrementHarvesters();
    } else {
        numItem[itemID]--;
    }

    for(auto& pPlayer : players) {
        pPlayer->onDecrementUnits(itemID);
    }

    if(itemID != Unit_Saboteur && itemID != Unit_Frigate && itemID != Unit_Carryall && itemID != Unit_MCV &&
       itemID != Unit_Harvester && itemID != Unit_Sandworm) {

        lossValue += context.game.objectData.data[itemID][static_cast<int>(houseID)].price;
    }

    if(!isAlive()) lose();
}




void House::incrementStructures(ItemID_enum itemID) {
    numStructures++;
    numItem[itemID]++;

    assert(numUnits + numStructures == std::accumulate(std::begin(numItem), std::end(numItem), 0));

    // change power requirements
    const auto currentItemPower = context.game.objectData.data[itemID][static_cast<int>(houseID)].power;
    if(currentItemPower >= 0) {
        powerRequirement += currentItemPower;
    }

    // change spice capacity
    capacity += context.game.objectData.data[itemID][static_cast<int>(houseID)].capacity;

    if(context.game.gameState != GameState::Loading) {
        // do not check selection lists if we are loading
        updateBuildLists();
    }
}




void House::decrementStructures(ItemID_enum itemID, const Coord& location) {
    if (numStructures < 1)
        THROW(std::runtime_error, "Cannot decrement number of structures %d (itemId %d)", numStructures, itemID);

    numStructures--;
    numItem[itemID]--;
    numItemLosses[itemID]++;

    assert(numUnits + numStructures == std::accumulate(std::begin(numItem), std::end(numItem), 0));

    // change power requirements
    const auto currentItemPower = context.game.objectData.data[itemID][static_cast<int>(houseID)].power;
    if(currentItemPower >= 0) {
        powerRequirement -= currentItemPower;
    }

    // change spice capacity
    capacity -= context.game.objectData.data[itemID][static_cast<int>(houseID)].capacity;

    if(context.game.gameState != GameState::Loading) {
        // do not check selection lists if we are loading
        updateBuildLists();
    }

    if (!isAlive())
        lose();

    for(auto& pPlayer : players) {
        pPlayer->onDecrementStructures(itemID, location);
    }
}




void House::noteDamageLocation(ObjectBase* pObject, int damage, Uint32 damagerID) {
    for(auto& pPlayer : players) {
        pPlayer->onDamage(pObject, damage, damagerID);
    }
}


/**
    This method informs this house that a new unit or structure was built
    \param pObject   the object that was built
*/
void House::informWasBuilt(ObjectBase* pObject) {
    auto itemID = pObject->getItemID();
    if(pObject->isAStructure()) {
        structureBuiltValue += context.game.objectData.data[itemID][static_cast<int>(houseID)].price;
        numBuiltStructures++;
    } else {
        unitBuiltValue += context.game.objectData.data[itemID][static_cast<int>(houseID)].price;
        numBuiltUnits++;
    }

    numItemBuilt[itemID]++;

    for(auto& pPlayer : players) {
        pPlayer->onObjectWasBuilt(pObject);
    }
}



/**
    This method informs this house that one of its units has killed an enemy unit or structure
    \param itemID   the ID of the enemy unit or structure
*/
void House::informHasKilled(ItemID_enum itemID) {
    destroyedValue += std::max(context.game.objectData.data[itemID][static_cast<int>(houseID)].price/100, 1);
    if(isStructure(itemID)) {
        numDestroyedStructures++;
    } else {
        numDestroyedUnits++;

        if(itemID != Unit_Saboteur
           && itemID != Unit_Frigate
           && itemID != Unit_Carryall
           && itemID != Unit_MCV
           && itemID != Unit_Harvester
           && itemID != Unit_Sandworm) {

                killValue += context.game.objectData.data[itemID][static_cast<int>(houseID)].price;

        }

    }

    numItemKills[itemID]++;


    for(auto& pPlayer : players) {
        pPlayer->onIncrementUnitKills(itemID);
    }
}

/**
 This method informs this house that one of its units has damaged an enemy unit or structure
 \param itemID   the ID of the enemy unit or structure
 */

void House::informHasDamaged(ItemID_enum itemID, Uint32 damage) {
    numItemDamageInflicted[itemID] += damage;
}


void House::win() {
    if(getTeamID() == pLocalHouse->getTeamID()) {
        context.game.setGameWon();
    } else {
        context.game.setGameLost();
    }
}




void House::lose(bool bSilent) const {
    if(!bSilent) {
        try {
            context.game.addToNewsTicker(fmt::sprintf(_("House '%s' has been defeated."), getHouseNameByNumber(getHouseID())));
        } catch (std::exception& e) {
            sdl2::log_info("House::lose(): %s", e.what());
        }
    }

    if((getTeamID() == pLocalHouse->getTeamID()) && ((context.game.winFlags & WINLOSEFLAGS_HUMAN_HAS_BUILDINGS) != 0)) {

        bool finished = true;

        for(auto i=0; i < static_cast<int>(HOUSETYPE::NUM_HOUSES); i++) {
            auto *const pHouse = context.game.getHouse(static_cast<HOUSETYPE>(i));
            if(pHouse != nullptr && pHouse->isAlive() && pHouse->getTeamID() == pLocalHouse->getTeamID()) {
                finished = false;
                break;
            }
        }

        if(finished) {
            // pLocalHouse is destroyed and this is a game finish condition
            if((context.game.loseFlags & WINLOSEFLAGS_HUMAN_HAS_BUILDINGS) != 0) {
                // house has won
                context.game.setGameWon();
            } else {
                // house has lost
                context.game.setGameLost();
            }
        }

    } else if((context.game.winFlags & WINLOSEFLAGS_AI_NO_BUILDINGS) != 0) {
        //if the only players left are on the thisPlayers team, pLocalHouse has won
        auto finished = true;

        for(auto i=0; i < static_cast<int>(HOUSETYPE::NUM_HOUSES); i++) {
            auto *const pHouse = context.game.getHouse(static_cast<HOUSETYPE>(i));
            if(pHouse != nullptr && pHouse->isAlive() && pHouse->getTeamID() != 0 && pHouse->getTeamID() != pLocalHouse->getTeamID()) {
                finished = false;
                break;
            }
        }

        if(finished) {
            // all AI players are destroyed and this is a game finish condition
            if((context.game.loseFlags & WINLOSEFLAGS_AI_NO_BUILDINGS) != 0) {
                // house has won
                context.game.setGameWon();
            } else {
                // house has lost
                context.game.setGameLost();
            }
        }
    }
}




void House::freeHarvester(int xPos, int yPos) {
    auto* const tile = context.map.tryGetTile(xPos, yPos);

    if(!tile) return;

    auto* const refinery = tile->getGroundObject<Refinery>(context.objectManager);
    if(!refinery) return;

    const Coord closestPos = context.map.findClosestEdgePoint(refinery->getLocation() + Coord(2,0), Coord(1,1));

    auto* carryall = createUnit<Carryall>();
    auto* harvester = createUnit<Harvester>();
    harvester->setAmountOfSpice(5);
    carryall->setOwned(false);
    carryall->giveCargo(context, harvester);
    carryall->deploy(context, closestPos);
    carryall->setDropOfferer(true);

    if (closestPos.x == 0)
        carryall->setAngle(ANGLETYPE::RIGHT);
    else if (closestPos.x == context.map.getSizeX()-1)
        carryall->setAngle(ANGLETYPE::LEFT);
    else if (closestPos.y == 0)
        carryall->setAngle(ANGLETYPE::DOWN);
    else if (closestPos.y == context.map.getSizeY()-1)
        carryall->setAngle(ANGLETYPE::UP);

    harvester->setTarget(refinery);
    harvester->setActive(false);
    carryall->setTarget(refinery);
}


StructureBase* House::placeStructure(Uint32 builderID, ItemID_enum itemID, int xPos, int yPos,
                                     bool byScenario, bool bForcePlacing) {
    const auto& [game, map, objectManager] = context;

    auto* const tile = map.tryGetTile(xPos, yPos);
    if(!tile) return nullptr;

    auto* pBuilder = builderID == NONE_ID ? nullptr : objectManager.getObject<BuilderBase>(builderID);

    if(game.getGameInitSettings().getGameOptions().onlyOnePalace && pBuilder != nullptr && itemID == Structure_Palace && getNumItems(Structure_Palace) > 0) {
        if(this == pLocalHouse && pBuilder->isSelected()) {
            game.currentCursorMode = Game::CursorMode_Normal;
        }
        return nullptr;
    }

    switch (itemID) {
        case Structure_Slab1: {

            // Slabs are no normal buildings
            tile->setType(context, Terrain_Slab);
            tile->setOwner(getHouseID());
            map.viewMap(getHouseID(), xPos, yPos, game.objectData.data[Structure_Slab1][static_cast<int>(houseID)].viewrange);
    //      context.map.getTile(xPos, yPos)->clearTerrain();

            if(pBuilder != nullptr) {
                pBuilder->unSetWaitingToPlace();

                if(this == pLocalHouse) {
                    if(pBuilder->isSelected()) {
                        context.game.currentCursorMode = Game::CursorMode_Normal;
                    }

                    pLocalPlayer->onPlaceStructure(nullptr);
                }
            }

            return nullptr;

        }

        case Structure_Slab4: {
            // Slabs are no normal buildings
            map.for_each(xPos, yPos, xPos + 2, yPos + 2,
                [&](Tile& t) {
                    if (t.hasAGroundObject() || !t.isRock() || t.isMountain())
                        return;

                    t.setType(context, Terrain_Slab);
                    t.setOwner(houseID);
                    context.map.viewMap(getHouseID(), t.getLocation().x, t.getLocation().y, context.game.objectData.data[Structure_Slab4][static_cast<int>(houseID)].viewrange);
                    //pTile->clearTerrain();
                });

            if(pBuilder != nullptr) {
                pBuilder->unSetWaitingToPlace();

                if(this == pLocalHouse) {
                    if(pBuilder->isSelected()) {
                        context.game.currentCursorMode = Game::CursorMode_Normal;
                    }

                    pLocalPlayer->onPlaceStructure(nullptr);
                }
            }

            return nullptr;

        }

        default: {
            auto* newStructure = createStructure(itemID, byScenario);
            if(newStructure == nullptr) {
                THROW(std::runtime_error, "Cannot create structure with itemID %d!", itemID);
            }

            auto cleanup = gsl::finally([&] {
                if(!newStructure) return;

                context.objectManager.removeObject(newStructure->getObjectID());
                newStructure = nullptr;
            });

            // Make sure the whole building fits on the map.
            if(!map.tileExists(xPos + newStructure->getStructureSizeX() - 1,
                               yPos + newStructure->getStructureSizeY() - 1))
                return nullptr;

            if(!bForcePlacing) {
                // check if there is already something on this tile

                if(map.find(xPos, yPos, xPos + newStructure->getStructureSizeX(), newStructure->getStructureSizeY(),
                            [](Tile& tile) { return tile.hasAGroundObject(); })) {
                    return nullptr;
                }
            }

            map.for_each(xPos, yPos, xPos + newStructure->getStructureSizeX(), yPos + newStructure->getStructureSizeY(),
                         [](auto& tile) { tile.clearTerrain(); });

            newStructure->setLocation(context, xPos, yPos);

            if ((builderID != NONE_ID) && (itemID != Structure_Wall)) {
                newStructure->setJustPlaced();
            }

            // at the beginning of the game the first refinery gets a harvester for free (brought by a carryall)
            if((itemID == Structure_Refinery) && ( ((context.game.gameState == GameState::Start) && (numItem[Unit_Harvester] <= 0)) || (builderID != NONE_ID)) ) {
                freeHarvester(xPos, yPos);
            }

            // if this structure was built by a construction yard this construction yard must be informed
            if(pBuilder != nullptr) {
                pBuilder->unSetWaitingToPlace();

                if(itemID == Structure_Palace) {
                    // cancel all other palaces
                    for(auto* pStructure : structureList) {
                        if(pStructure->getOwner() != this) continue;

                        auto* pConstructionYard = dune_cast<ConstructionYard>(pStructure);
                        if(pConstructionYard && pBuilder != pConstructionYard) {
                            pConstructionYard->doCancelItem(Structure_Palace, false);
                        }
                    }
                }

                if (this == pLocalHouse) {
                    if(pBuilder->isSelected()) {
                        game.currentCursorMode = Game::CursorMode_Normal;
                    }
                    pLocalPlayer->onPlaceStructure(newStructure);
                }

                // only if we were constructed by construction yard
                // => inform house of the building
                pBuilder->getOwner()->informWasBuilt(newStructure);
            }

            if(auto* builder = dune_cast<BuilderBase>(newStructure)) {
                builder->updateBuildList();
            }

            auto* const ret = newStructure;

            newStructure = nullptr; // Prevent gsl::finally() from deleting it.

            return ret;
        }
    }
}




UnitBase* House::placeUnit(ItemID_enum itemID, int xPos, int yPos, bool byScenario) {

    auto* pTile = context.map.tryGetTile(xPos, yPos);
    if(!pTile) return nullptr;

    if(itemID == Unit_Saboteur || itemID == Unit_Soldier || itemID == Unit_Trooper) {
        if((pTile->hasANonInfantryGroundObject()) || (!pTile->infantryNotFull())) {
            // infantry units can not placed on non-infantry units or structures (or the tile is already full of
            // infantry units)
            return nullptr;
        }
    } else {
        if(pTile->hasAGroundObject()) {
            // non-infantry units can not placed on a tile where already some other unit or structure is placed on
            return nullptr;
        }
    }

    auto* newUnit = createUnit(itemID, byScenario);
    if(!newUnit) return nullptr;

    Coord pos = Coord(xPos, yPos);
    if(newUnit->canPass(xPos, yPos)) {
        newUnit->deploy(context, pos);
    } else {
        newUnit->setVisible(VIS_ALL, false);
        newUnit->destroy(context);
        newUnit = nullptr;
    }

    return newUnit;
}


/**
    This method returns the center of the base of this house.
    \return the coordinate of the center in tile coordinates
*/
Coord House::getCenterOfMainBase() const {
    Coord center;
    int numStructures = 0;
    for(const StructureBase* pStructure : structureList) {
        if(pStructure->getOwner() == this) {
            center += pStructure->getLocation();
            numStructures++;
        }
    }

    if(numStructures == 0) {
        return Coord::Invalid();
    }         return center / numStructures;

   
}


/**
    This method returns the position of the strongest unit
    \return the coordinate of the strongest unit
*/
Coord House::getStrongestUnitPosition() const {
    Coord strongestUnitPosition = Coord::Invalid();
    Sint32 strongestUnitCost = 0;
    for(const UnitBase* pUnit : unitList) {
        if(pUnit->getOwner() == this) {
            Sint32 currentCost = context.game.objectData.data[pUnit->getItemID()][static_cast<int>(houseID)].price;

            if(currentCost > strongestUnitCost) {
                strongestUnitPosition = pUnit->getLocation();
                strongestUnitCost = currentCost;
            }
        }
    }

    return strongestUnitPosition;
}




void House::decrementHarvesters() {
    numItem[Unit_Harvester]--;

    if(numItem[Unit_Harvester] <= 0) {
        numItem[Unit_Harvester] = 0;

        if(numItem[Structure_Refinery]) {
            Coord          closestPos;
            FixPoint       closestDistance  = FixPt_MAX;
            StructureBase* pClosestRefinery = nullptr;

            for(StructureBase* pStructure : structureList) {
                if((pStructure->getItemID() == Structure_Refinery) && (pStructure->getOwner() == this) && (pStructure->getHealth() > 0)) {
                    Coord pos = pStructure->getLocation();

                    Coord closestPoint = pStructure->getClosestPoint(pos);
                    FixPoint refineryDistance = blockDistance(pos, closestPoint);
                    if(!pClosestRefinery || (refineryDistance < closestDistance)) {
                            closestDistance = refineryDistance;
                            pClosestRefinery = pStructure;
                            closestPos = pos;
                    }
                }
            }

            if(pClosestRefinery && (context.game.gameState == GameState::Running)) {
                freeHarvester(pClosestRefinery->getLocation());
            }
        }
    }
}
