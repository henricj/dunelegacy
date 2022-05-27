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

#include "mmath.h"
#include <globals.h>
#include <sand.h>

#include <FileClasses/TextManager.h>

#include <players/AIPlayer.h>
#include <players/HumanPlayer.h>
#include <players/PlayerFactory.h>
#include <players/QuantBot.h>

#include <Game.h>
#include <GameInterface.h>
#include <Map.h>
#include <SoundPlayer.h>

#include <structures/BuilderBase.h>
#include <structures/ConstructionYard.h>
#include <structures/Refinery.h>
#include <structures/StructureBase.h>
#include <units/Carryall.h>
#include <units/Harvester.h>

#include <misc/exceptions.h>

#include <fmt/printf.h>

#include <algorithm>
#include <numeric>
#include <stdexcept>

House::House(const GameContext& context) : ai_{true}, choam_(this), context_(context) { }

House::~House() = default;

House::House(const GameContext& context, HOUSETYPE newHouse, int newCredits, int maxUnits, uint8_t teamID, int quota)
    : House(context) {
    const auto is_valid_house = static_cast<int>(newHouse) >= 0 && newHouse < HOUSETYPE::NUM_HOUSES;

    houseID_      = is_valid_house ? newHouse : static_cast<HOUSETYPE>(0);
    this->teamID_ = teamID;

    startingCredits_ = newCredits;
    oldCredits_      = lround(storedCredits_ + startingCredits_);

    this->maxUnits_ = maxUnits;
    this->quota_    = quota;
}

House::House(const GameContext& context, InputStream& stream) : House(context) {
    houseID_ = static_cast<HOUSETYPE>(stream.readUint8());
    teamID_  = stream.readUint8();

    storedCredits_   = stream.readFixPoint();
    startingCredits_ = stream.readFixPoint();
    oldCredits_      = lround(storedCredits_ + startingCredits_);
    maxUnits_        = stream.readSint32();
    quota_           = stream.readSint32();

    stream.readBools(&bHadContactWithEnemy_, &bHadDirectContactWithEnemy_);

    unitBuiltValue_         = stream.readUint32();
    structureBuiltValue_    = stream.readUint32();
    militaryValue           = stream.readUint32();
    killValue_              = stream.readUint32();
    lossValue_              = stream.readUint32();
    numBuiltUnits_          = stream.readUint32();
    numBuiltStructures_     = stream.readUint32();
    destroyedValue_         = stream.readUint32();
    numDestroyedUnits_      = stream.readUint32();
    numDestroyedStructures_ = stream.readUint32();
    harvestedSpice_         = stream.readFixPoint();
    producedPower_          = stream.readSint32();
    powerUsageTimer_        = stream.readSint32();

    choam_.load(stream);

    const auto numAITeams = stream.readUint32();
    for (auto i = 0U; i < numAITeams; i++) {
        aiteams_.emplace_back(stream);
    }

    const auto numPlayers = stream.readUint32();
    for (auto i = 0U; i < numPlayers; i++) {
        const auto playerclass  = stream.readString();
        const auto* pPlayerData = PlayerFactory::getByPlayerClass(playerclass);
        if (pPlayerData == nullptr) {
            sdl2::log_info("Warning: Cannot load player '%s'", playerclass.c_str());
        } else {
            addPlayer(pPlayerData->load(context, stream, this));
        }
    }
}

UnitBase* House::createUnit(ItemID_enum itemID, bool byScenario) {
    return context_.objectManager.createObjectFromItemId<UnitBase>(itemID,
                                                                   ObjectInitializer{context_.game, this, byScenario});
}

StructureBase* House::createStructure(ItemID_enum itemID, bool byScenario) {
    return context_.objectManager.createObjectFromItemId<StructureBase>(
        itemID, ObjectInitializer{context_.game, this, byScenario});
}

void House::save(OutputStream& stream) const {
    stream.writeUint8(static_cast<uint8_t>(houseID_));
    stream.writeUint8(teamID_);

    stream.writeFixPoint(storedCredits_);
    stream.writeFixPoint(startingCredits_);
    stream.writeSint32(maxUnits_);
    stream.writeSint32(quota_);

    stream.writeBools(bHadContactWithEnemy_, bHadDirectContactWithEnemy_);

    stream.writeUint32(unitBuiltValue_);
    stream.writeUint32(structureBuiltValue_);
    stream.writeUint32(militaryValue);
    stream.writeUint32(killValue_);
    stream.writeUint32(lossValue_);
    stream.writeUint32(numBuiltUnits_);
    stream.writeUint32(numBuiltStructures_);
    stream.writeUint32(destroyedValue_);
    stream.writeUint32(numDestroyedUnits_);
    stream.writeUint32(numDestroyedStructures_);
    stream.writeFixPoint(harvestedSpice_);
    stream.writeSint32(producedPower_);
    stream.writeSint32(powerUsageTimer_);

    choam_.save(stream);

    stream.writeUint32(aiteams_.size());
    for (const auto& aiteam : aiteams_) {
        aiteam.save(stream);
    }

    stream.writeUint32(players_.size());
    for (const auto& pPlayer : players_) {
        stream.writeString(pPlayer->getPlayerclass());
        pPlayer->save(stream);
    }
}

void House::addPlayer(std::unique_ptr<Player> newPlayer) {
    auto* const pNewPlayer = newPlayer.get();

    if (nullptr == pNewPlayer)
        THROW(std::invalid_argument, "House::addPlayer passed null newPlayer!");

    players_.push_back(std::move(newPlayer));

    ai_ = !(dynamic_cast<HumanPlayer*>(pNewPlayer) != nullptr && players_.empty());

    const auto newPlayerID =
        static_cast<uint8_t>((static_cast<uint8_t>(houseID_) << 4U) | static_cast<uint8_t>(players_.size()));
    pNewPlayer->playerID = newPlayerID;

    context_.game.registerPlayer(pNewPlayer);
}

bool House::isAlive() const noexcept {
    return (teamID_ == 0)
        || !(((numStructures_ - numItem_[Structure_Wall]) <= 0)
             && (((numUnits_ - numItem_[Unit_Carryall] - numItem_[Unit_Harvester] - numItem_[Unit_Frigate]
                   - numItem_[Unit_Sandworm])
                  <= 0)));
}

void House::setProducedPower(int newPower) {
    producedPower_ = newPower;
}

bool House::isGroundUnitLimitReached() const {
    const int numGroundUnit = numUnits_ - numItem_[Unit_Soldier] - numItem_[Unit_Trooper] - numItem_[Unit_Carryall]
                            - numItem_[Unit_Ornithopter];
    return (numGroundUnit + (numItem_[Unit_Soldier] + 2) / 3 + (numItem_[Unit_Trooper] + 2) / 3 >= maxUnits_);
}

bool House::isInfantryUnitLimitReached() const {
    const auto numGroundUnit = numUnits_ - numItem_[Unit_Soldier] - numItem_[Unit_Trooper] - numItem_[Unit_Carryall]
                             - numItem_[Unit_Ornithopter];
    return (numGroundUnit + numItem_[Unit_Soldier] / 3 + numItem_[Unit_Trooper] / 3 >= maxUnits_);
}

bool House::isAirUnitLimitReached() const {
    return (numItem_[Unit_Carryall] + numItem_[Unit_Ornithopter] >= 11 * std::max(maxUnits_, 25) / 25);
}

void House::addCredits(FixPoint newCredits, bool wasRefined) {
    if (newCredits <= 0)
        return;

    if (wasRefined) {
        harvestedSpice_ += newCredits;
    }

    storedCredits_ += newCredits;
    if (this == dune::globals::pLocalHouse) {
        if (((context_.game.winFlags & WINLOSEFLAGS_QUOTA) != 0) && (quota_ != 0)) {
            if (storedCredits_ >= quota_) {
                win();
            }
        }
    }
}

void House::returnCredits(FixPoint newCredits) {
    if (newCredits <= 0)
        return;

    const auto leftCapacity = capacity_ - storedCredits_;
    if (newCredits <= leftCapacity) {
        addCredits(newCredits, false);
    } else {
        addCredits(leftCapacity, false);
        startingCredits_ += (newCredits - leftCapacity);
    }
}

FixPoint House::takeCredits(FixPoint amount) {
    FixPoint taken = 0;

    if (getCredits() >= 1) {
        if (storedCredits_ > amount) {
            taken = amount;
            storedCredits_ -= amount;
        } else {
            taken          = storedCredits_;
            storedCredits_ = 0;

            if (startingCredits_ > (amount - taken)) {
                startingCredits_ -= (amount - taken);
                taken = amount;
            } else {
                taken += startingCredits_;
                startingCredits_ = 0;
            }
        }
    }

    return taken; // the amount that was actually withdrawn
}

void House::printStat() const {
    sdl2::log_info("House %s: (Number of Units: %d, Number of Structures: %d)",
                   getHouseNameByNumber(getHouseID()).c_str(), numUnits_, numStructures_);
    sdl2::log_info("Barracks: %d\t\tWORs: %d", numItem_[Structure_Barracks], numItem_[Structure_WOR]);
    sdl2::log_info("Light Factories: %d\tHeavy Factories: %d", numItem_[Structure_LightFactory],
                   numItem_[Structure_HeavyFactory]);
    sdl2::log_info("IXs: %d\t\t\tPalaces: %d", numItem_[Structure_IX], numItem_[Structure_Palace]);
    sdl2::log_info("Repair Yards: %d\t\tHigh-Tech Factories: %d", numItem_[Structure_RepairYard],
                   numItem_[Structure_HighTechFactory]);
    sdl2::log_info("Refineries: %d\t\tStarports: %d", numItem_[Structure_Refinery], numItem_[Structure_StarPort]);
    sdl2::log_info("Walls: %d\t\tRocket Turrets: %d", numItem_[Structure_Wall], numItem_[Structure_RocketTurret]);
    sdl2::log_info("Gun Turrets: %d\t\tConstruction Yards: %d", numItem_[Structure_GunTurret],
                   numItem_[Structure_ConstructionYard]);
    sdl2::log_info("Windtraps: %d\t\tRadars: %d", numItem_[Structure_WindTrap], numItem_[Structure_Radar]);
    sdl2::log_info("Silos: %d", numItem_[Structure_Silo]);
    sdl2::log_info("Carryalls: %d\t\tFrigates: %d", numItem_[Unit_Carryall], numItem_[Unit_Frigate]);
    sdl2::log_info("Devastators: %d\t\tDeviators: %d", numItem_[Unit_Devastator], numItem_[Unit_Deviator]);
    sdl2::log_info("Soldiers: %d\t\tTrooper: %d", numItem_[Unit_Soldier], numItem_[Unit_Trooper]);
    sdl2::log_info("Saboteur: %d\t\tSandworms: %d", numItem_[Unit_Saboteur], numItem_[Unit_Sandworm]);
    sdl2::log_info("Quads: %d\t\tTrikes: %d", numItem_[Unit_Quad], numItem_[Unit_Trike]);
    sdl2::log_info("Raiders: %d\t\tTanks: %d", numItem_[Unit_RaiderTrike], numItem_[Unit_Tank]);
    sdl2::log_info("Siege Tanks : %d\t\tSonic Tanks: %d", numItem_[Unit_SiegeTank], numItem_[Unit_SonicTank]);
    sdl2::log_info("Harvesters: %d\t\tMCVs: %d", numItem_[Unit_Harvester], numItem_[Unit_MCV]);
    sdl2::log_info("Ornithopters: %d\t\tRocket Launchers: %d", numItem_[Unit_Ornithopter], numItem_[Unit_Launcher]);
}

void House::updateBuildLists() {
    for (auto* pStructure : dune::globals::structureList) {
        auto* builder = dune_cast<BuilderBase>(pStructure);
        if (!builder || builder->getOwner() != this)
            continue;

        builder->updateBuildList();
    }
}

void House::update() {
    numVisibleEnemyUnits_    = 0;
    numVisibleFriendlyUnits_ = 0;

    const auto credits = getCredits();
    if (oldCredits_ != credits) {
        if ((this == dune::globals::pLocalHouse) && (credits > 0)) {
            dune::globals::soundPlayer->playSound(credits > oldCredits_ ? Sound_enum::Sound_CreditsTick
                                                                        : Sound_enum::Sound_CreditsTickDown);
        }
        oldCredits_ = credits;
    }

    if (storedCredits_ > capacity_) {
        --storedCredits_;
        if (storedCredits_ < 0) {
            storedCredits_ = 0;
        }

        if (this == dune::globals::pLocalHouse) {
            context_.game.addToNewsTicker(
                _("@DUNE.ENG|145#As insufficient spice storage is available, spice is lost."));
        }
    }

    powerUsageTimer_--;
    if (powerUsageTimer_ <= 0) {
        powerUsageTimer_ = MILLI2CYCLES(15 * 1000);
        takeCredits(FixPoint(getPowerRequirement()) / 32);
    }

    choam_.update(context_);

    for (const auto& pPlayer : players_) {
        pPlayer->update();
    }
}

void House::incrementUnits(ItemID_enum itemID) {
    numUnits_++;
    numItem_[itemID]++;

    assert(numUnits_ + numStructures_ == std::accumulate(std::begin(numItem_), std::end(numItem_), 0));

    if (itemID != Unit_Saboteur && itemID != Unit_Frigate && itemID != Unit_Carryall && itemID != Unit_MCV
        && itemID != Unit_Harvester && itemID != Unit_Sandworm) {

        militaryValue += context_.game.objectData.data[itemID][static_cast<int>(houseID_)].price;
    }
}

void House::decrementUnits(ItemID_enum itemID) {
    if (numUnits_ < 1)
        THROW(std::runtime_error, "Cannot decrement number of units %d (itemId %d)", numUnits_, itemID);

    numUnits_--;
    numItemLosses_[itemID]++;

    if (itemID == Unit_Harvester) {
        decrementHarvesters();
    } else {
        numItem_[itemID]--;
    }

    for (const auto& pPlayer : players_) {
        pPlayer->onDecrementUnits(itemID);
    }

    if (itemID != Unit_Saboteur && itemID != Unit_Frigate && itemID != Unit_Carryall && itemID != Unit_MCV
        && itemID != Unit_Harvester && itemID != Unit_Sandworm) {

        lossValue_ += context_.game.objectData.data[itemID][static_cast<int>(houseID_)].price;
    }

    if (!isAlive())
        lose();
}

void House::incrementStructures(ItemID_enum itemID) {
    numStructures_++;
    numItem_[itemID]++;

    assert(numUnits_ + numStructures_ == std::accumulate(std::begin(numItem_), std::end(numItem_), 0));

    // change power requirements
    const auto currentItemPower = context_.game.objectData.data[itemID][static_cast<int>(houseID_)].power;
    if (currentItemPower >= 0) {
        powerRequirement_ += currentItemPower;
    }

    // change spice capacity
    capacity_ += context_.game.objectData.data[itemID][static_cast<int>(houseID_)].capacity;

    if (context_.game.gameState != GameState::Loading) {
        // do not check selection lists if we are loading
        updateBuildLists();
    }
}

void House::decrementStructures(ItemID_enum itemID, const Coord& location) {
    if (numStructures_ < 1)
        THROW(std::runtime_error, "Cannot decrement number of structures %d (itemId %d)", numStructures_, itemID);

    numStructures_--;
    numItem_[itemID]--;
    numItemLosses_[itemID]++;

    assert(numUnits_ + numStructures_ == std::accumulate(std::begin(numItem_), std::end(numItem_), 0));

    // change power requirements
    const auto currentItemPower = context_.game.objectData.data[itemID][static_cast<int>(houseID_)].power;
    if (currentItemPower >= 0) {
        powerRequirement_ -= currentItemPower;
    }

    // change spice capacity
    capacity_ -= context_.game.objectData.data[itemID][static_cast<int>(houseID_)].capacity;

    if (context_.game.gameState != GameState::Loading) {
        // do not check selection lists if we are loading
        updateBuildLists();
    }

    if (!isAlive())
        lose();

    for (const auto& pPlayer : players_) {
        pPlayer->onDecrementStructures(itemID, location);
    }
}

void House::noteDamageLocation(ObjectBase* pObject, int damage, uint32_t damagerID) {
    for (const auto& pPlayer : players_) {
        pPlayer->onDamage(pObject, damage, damagerID);
    }
}

/**
    This method informs this house that a new unit or structure was built
    \param pObject   the object that was built
*/
void House::informWasBuilt(ObjectBase* pObject) {
    const auto itemID = pObject->getItemID();
    if (pObject->isAStructure()) {
        structureBuiltValue_ += context_.game.objectData.data[itemID][static_cast<int>(houseID_)].price;
        numBuiltStructures_++;
    } else {
        unitBuiltValue_ += context_.game.objectData.data[itemID][static_cast<int>(houseID_)].price;
        numBuiltUnits_++;
    }

    numItemBuilt_[itemID]++;

    for (const auto& pPlayer : players_) {
        pPlayer->onObjectWasBuilt(pObject);
    }
}

/**
    This method informs this house that one of its units has killed an enemy unit or structure
    \param itemID   the ID of the enemy unit or structure
*/
void House::informHasKilled(ItemID_enum itemID) {
    destroyedValue_ += std::max(context_.game.objectData.data[itemID][static_cast<int>(houseID_)].price / 100, 1);
    if (isStructure(itemID)) {
        numDestroyedStructures_++;
    } else {
        numDestroyedUnits_++;

        if (itemID != Unit_Saboteur && itemID != Unit_Frigate && itemID != Unit_Carryall && itemID != Unit_MCV
            && itemID != Unit_Harvester && itemID != Unit_Sandworm) {

            killValue_ += context_.game.objectData.data[itemID][static_cast<int>(houseID_)].price;
        }
    }

    numItemKills_[itemID]++;

    for (const auto& pPlayer : players_) {
        pPlayer->onIncrementUnitKills(itemID);
    }
}

/**
 This method informs this house that one of its units has damaged an enemy unit or structure
 \param itemID   the ID of the enemy unit or structure
 */

void House::informHasDamaged(ItemID_enum itemID, uint32_t damage) {
    numItemDamageInflicted_[itemID] += damage;
}

void House::win() {
    if (getTeamID() == dune::globals::pLocalHouse->getTeamID()) {
        context_.game.setGameWon();
    } else {
        context_.game.setGameLost();
    }
}

void House::lose(bool bSilent) const {
    if (!bSilent) {
        try {
            context_.game.addToNewsTicker(
                fmt::sprintf(_("House '%s' has been defeated."), getHouseNameByNumber(getHouseID())));
        } catch (std::exception& e) {
            sdl2::log_info("House::lose(): %s", e.what());
        }
    }

    if ((getTeamID() == dune::globals::pLocalHouse->getTeamID())
        && ((context_.game.winFlags & WINLOSEFLAGS_HUMAN_HAS_BUILDINGS) != 0)) {

        bool finished = true;

        for (auto i = 0; i < NUM_HOUSES; i++) {
            const auto* const pHouse = context_.game.getHouse(static_cast<HOUSETYPE>(i));
            if (pHouse != nullptr && pHouse->isAlive()
                && pHouse->getTeamID() == dune::globals::pLocalHouse->getTeamID()) {
                finished = false;
                break;
            }
        }

        if (finished) {
            // pLocalHouse is destroyed and this is a game finish condition
            if ((context_.game.loseFlags & WINLOSEFLAGS_HUMAN_HAS_BUILDINGS) != 0) {
                // house has won
                context_.game.setGameWon();
            } else {
                // house has lost
                context_.game.setGameLost();
            }
        }

    } else if ((context_.game.winFlags & WINLOSEFLAGS_AI_NO_BUILDINGS) != 0) {
        // if the only players left are on the thisPlayers team, pLocalHouse has won
        auto finished = true;

        for (auto i = 0; i < NUM_HOUSES; i++) {
            const auto* const pHouse = context_.game.getHouse(static_cast<HOUSETYPE>(i));
            if (pHouse != nullptr && pHouse->isAlive() && pHouse->getTeamID() != 0
                && pHouse->getTeamID() != dune::globals::pLocalHouse->getTeamID()) {
                finished = false;
                break;
            }
        }

        if (finished) {
            // all AI players are destroyed and this is a game finish condition
            if ((context_.game.loseFlags & WINLOSEFLAGS_AI_NO_BUILDINGS) != 0) {
                // house has won
                context_.game.setGameWon();
            } else {
                // house has lost
                context_.game.setGameLost();
            }
        }
    }
}

void House::freeHarvester(int xPos, int yPos) {
    const auto* const tile = context_.map.tryGetTile(xPos, yPos);

    if (!tile)
        return;

    const auto* const refinery = tile->getGroundObject<Refinery>(context_.objectManager);
    if (!refinery)
        return;

    const Coord closestPos = context_.map.findClosestEdgePoint(refinery->getLocation() + Coord(2, 0), Coord(1, 1));

    auto* carryall  = createUnit<Carryall>();
    auto* harvester = createUnit<Harvester>();
    harvester->setAmountOfSpice(5);
    carryall->setOwned(false);
    carryall->giveCargo(context_, harvester);
    carryall->deploy(context_, closestPos);
    carryall->setDropOfferer(true);

    if (closestPos.x == 0)
        carryall->setAngle(ANGLETYPE::RIGHT);
    else if (closestPos.x == context_.map.getSizeX() - 1)
        carryall->setAngle(ANGLETYPE::LEFT);
    else if (closestPos.y == 0)
        carryall->setAngle(ANGLETYPE::DOWN);
    else if (closestPos.y == context_.map.getSizeY() - 1)
        carryall->setAngle(ANGLETYPE::UP);

    harvester->setTarget(refinery);
    harvester->setActive(false);
    carryall->setTarget(refinery);
}

StructureBase*
House::placeStructure(uint32_t builderID, ItemID_enum itemID, int xPos, int yPos, bool byScenario, bool bForcePlacing) {
    const auto& [game, map, objectManager] = context_;

    auto* const tile = map.tryGetTile(xPos, yPos);
    if (!tile)
        return nullptr;

    auto* pBuilder = builderID == NONE_ID ? nullptr : objectManager.getObject<BuilderBase>(builderID);

    if (game.getGameInitSettings().getGameOptions().onlyOnePalace && pBuilder != nullptr && itemID == Structure_Palace
        && getNumItems(Structure_Palace) > 0) {
        if (this == dune::globals::pLocalHouse && pBuilder->isSelected()) {
            game.currentCursorMode = Game::CursorMode_Normal;
        }
        return nullptr;
    }

    switch (itemID) {
        case Structure_Slab1: {

            // Slabs are no normal buildings
            tile->setType(context_, Terrain_Slab);
            tile->setOwner(getHouseID());
            map.viewMap(getHouseID(), xPos, yPos,
                        game.objectData.data[Structure_Slab1][static_cast<int>(houseID_)].viewrange);
            //      context.map.getTile(xPos, yPos)->clearTerrain();

            if (pBuilder != nullptr) {
                pBuilder->unSetWaitingToPlace();

                if (this == dune::globals::pLocalHouse) {
                    if (pBuilder->isSelected()) {
                        context_.game.currentCursorMode = Game::CursorMode_Normal;
                    }

                    dune::globals::pLocalPlayer->onPlaceStructure(nullptr);
                }
            }

            return nullptr;
        }

        case Structure_Slab4: {
            // Slabs are no normal buildings
            map.for_each(xPos, yPos, xPos + 2, yPos + 2, [&](Tile& t) {
                if (t.hasAGroundObject() || !t.isRock() || t.isMountain())
                    return;

                t.setType(context_, Terrain_Slab);
                t.setOwner(houseID_);
                context_.map.viewMap(
                    getHouseID(), t.getLocation().x, t.getLocation().y,
                    context_.game.objectData.data[Structure_Slab4][static_cast<int>(houseID_)].viewrange);
                // pTile->clearTerrain();
            });

            if (pBuilder != nullptr) {
                pBuilder->unSetWaitingToPlace();

                if (this == dune::globals::pLocalHouse) {
                    if (pBuilder->isSelected()) {
                        context_.game.currentCursorMode = Game::CursorMode_Normal;
                    }

                    dune::globals::pLocalPlayer->onPlaceStructure(nullptr);
                }
            }

            return nullptr;
        }

        default: {
            auto* newStructure = createStructure(itemID, byScenario);
            if (newStructure == nullptr) {
                THROW(std::runtime_error, "Cannot create structure with itemID %d!", itemID);
            }

            auto cleanup = gsl::finally([&] {
                if (!newStructure)
                    return;

                context_.objectManager.removeObject(newStructure->getObjectID());
                newStructure = nullptr;
            });

            // Make sure the whole building fits on the map.
            if (!map.tileExists(xPos + newStructure->getStructureSizeX() - 1,
                                yPos + newStructure->getStructureSizeY() - 1))
                return nullptr;

            if (!bForcePlacing) {
                // check if there is already something on this tile

                if (map.find(xPos, yPos, xPos + newStructure->getStructureSizeX(), newStructure->getStructureSizeY(),
                             [](Tile& tile) { return tile.hasAGroundObject(); })) {
                    return nullptr;
                }
            }

            map.for_each(xPos, yPos, xPos + newStructure->getStructureSizeX(), yPos + newStructure->getStructureSizeY(),
                         [](auto& tile) { tile.clearTerrain(); });

            newStructure->setLocation(context_, xPos, yPos);

            if ((builderID != NONE_ID) && (itemID != Structure_Wall)) {
                newStructure->setJustPlaced();
            }

            // at the beginning of the game the first refinery gets a harvester for free (brought by a carryall)
            if ((itemID == Structure_Refinery)
                && (((context_.game.gameState == GameState::Start) && (numItem_[Unit_Harvester] <= 0))
                    || (builderID != NONE_ID))) {
                freeHarvester(xPos, yPos);
            }

            // if this structure was built by a construction yard this construction yard must be informed
            if (pBuilder != nullptr) {
                pBuilder->unSetWaitingToPlace();

                if (itemID == Structure_Palace) {
                    // cancel all other palaces
                    for (auto* pStructure : dune::globals::structureList) {
                        if (pStructure->getOwner() != this)
                            continue;

                        auto* pConstructionYard = dune_cast<ConstructionYard>(pStructure);
                        if (pConstructionYard && pBuilder != pConstructionYard) {
                            pConstructionYard->doCancelItem(Structure_Palace, false);
                        }
                    }
                }

                if (this == dune::globals::pLocalHouse) {
                    if (pBuilder->isSelected()) {
                        game.currentCursorMode = Game::CursorMode_Normal;
                    }
                    dune::globals::pLocalPlayer->onPlaceStructure(newStructure);
                }

                // only if we were constructed by construction yard
                // => inform house of the building
                pBuilder->getOwner()->informWasBuilt(newStructure);
            }

            if (auto* builder = dune_cast<BuilderBase>(newStructure)) {
                builder->updateBuildList();
            }

            auto* const ret = newStructure;

            newStructure = nullptr; // Prevent gsl::finally() from deleting it.

            return ret;
        }
    }
}

UnitBase* House::placeUnit(ItemID_enum itemID, int xPos, int yPos, bool byScenario) {

    const auto* pTile = context_.map.tryGetTile(xPos, yPos);
    if (!pTile)
        return nullptr;

    if (itemID == Unit_Saboteur || itemID == Unit_Soldier || itemID == Unit_Trooper) {
        if ((pTile->hasANonInfantryGroundObject()) || (!pTile->infantryNotFull())) {
            // infantry units can not placed on non-infantry units or structures (or the tile is already full of
            // infantry units)
            return nullptr;
        }
    } else {
        if (pTile->hasAGroundObject()) {
            // non-infantry units can not placed on a tile where already some other unit or structure is placed on
            return nullptr;
        }
    }

    auto* newUnit = createUnit(itemID, byScenario);
    if (!newUnit)
        return nullptr;

    const Coord pos{xPos, yPos};
    if (newUnit->canPass(xPos, yPos)) {
        newUnit->deploy(context_, pos);
    } else {
        newUnit->setVisible(VIS_ALL, false);
        newUnit->destroy(context_);
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
    for (const StructureBase* pStructure : dune::globals::structureList) {
        if (pStructure->getOwner() == this) {
            center += pStructure->getLocation();
            numStructures++;
        }
    }

    if (numStructures == 0) {
        return Coord::Invalid();
    }
    return center / numStructures;
}

/**
    This method returns the position of the strongest unit
    \return the coordinate of the strongest unit
*/
Coord House::getStrongestUnitPosition() const {
    Coord strongestUnitPosition = Coord::Invalid();
    int32_t strongestUnitCost   = 0;
    for (const UnitBase* pUnit : dune::globals::unitList) {
        if (pUnit->getOwner() == this) {
            const int32_t currentCost =
                context_.game.objectData.data[pUnit->getItemID()][static_cast<int>(houseID_)].price;

            if (currentCost > strongestUnitCost) {
                strongestUnitPosition = pUnit->getLocation();
                strongestUnitCost     = currentCost;
            }
        }
    }

    return strongestUnitPosition;
}

void House::decrementHarvesters() {
    numItem_[Unit_Harvester]--;

    if (numItem_[Unit_Harvester] <= 0) {
        numItem_[Unit_Harvester] = 0;

        if (numItem_[Structure_Refinery]) {
            Coord closestPos;
            FixPoint closestDistance              = FixPt_MAX;
            const StructureBase* pClosestRefinery = nullptr;

            for (const auto* pStructure : dune::globals::structureList) {
                if ((pStructure->getItemID() == Structure_Refinery) && (pStructure->getOwner() == this)
                    && (pStructure->getHealth() > 0)) {
                    Coord pos = pStructure->getLocation();

                    Coord closestPoint        = pStructure->getClosestPoint(pos);
                    FixPoint refineryDistance = blockDistance(pos, closestPoint);
                    if (!pClosestRefinery || (refineryDistance < closestDistance)) {
                        closestDistance  = refineryDistance;
                        pClosestRefinery = pStructure;
                        closestPos       = pos;
                    }
                }
            }

            if (pClosestRefinery && (context_.game.gameState == GameState::Running)) {
                freeHarvester(pClosestRefinery->getLocation());
            }
        }
    }
}
