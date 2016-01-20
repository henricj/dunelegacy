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

#ifndef HOUSE_H
#define HOUSE_H

#include <misc/InputStream.h>
#include <misc/OutputStream.h>
#include <Definitions.h>
#include <DataTypes.h>
#include <data.h>
#include <Choam.h>

#include <players/Player.h>

#include <memory>

// forward declarations
class UnitBase;
class StructureBase;
class ObjectBase;
class HumanPlayer;

class House
{
public:
	House(int newHouse, int newCredits, Uint8 team = 0, int quota = 0);
	House(InputStream& stream);
	void init();
	virtual ~House();
	virtual void save(OutputStream& stream) const;

	void addPlayer(std::shared_ptr<Player> newPlayer);

	inline int getHouseID() const { return houseID; }
	inline int getTeam() const { return team; }

    inline bool isAI() const { return ai; }
	inline bool isAlive() const { return (team == 0) || !(((numStructures - numItem[Structure_Wall]) <= 0) && (((numUnits - numItem[Unit_Carryall] - numItem[Unit_Harvester] - numItem[Unit_Frigate] - numItem[Unit_Sandworm]) <= 0))); }

	inline bool hasCarryalls() const { return (numItem[Unit_Carryall] > 0); }
	inline bool hasBarracks() const { return (numItem[Structure_Barracks] > 0); }
	inline bool hasIX() const { return (numItem[Structure_IX] > 0); }
	inline bool hasLightFactory() const { return (numItem[Structure_LightFactory] > 0); }
	inline bool hasHeavyFactory() const { return (numItem[Structure_HeavyFactory] > 0); }
	inline bool hasRefinery() const { return (numItem[Structure_Refinery] > 0); }
	inline bool hasRepairYard() const { return (numItem[Structure_RepairYard] > 0); }
	inline bool hasStarPort() const { return (numItem[Structure_StarPort] > 0); }
	inline bool hasWindTrap() const { return (numItem[Structure_WindTrap] > 0); }
	inline bool hasSandworm() const { return (numItem[Unit_Sandworm] > 0); }
    inline bool hasRadar() const { return (numItem[Structure_Radar] > 0); }

    inline bool hasRadarOn() const { return (hasRadar() && hasPower()); }
	inline bool hasPower() const { return (producedPower >= powerRequirement); }

	inline int getNumStructures() const { return numStructures; };
	inline int getNumUnits() const { return numUnits; };
	inline int getNumItems(int itemID) const { return (isStructure(itemID) || isUnit(itemID)) ? numItem[itemID] : 0; };

	inline int getCapacity() const { return capacity; }

	inline int getProducedPower() const { return producedPower; }
    void setProducedPower(int newPower);
	inline int getPowerRequirement() const { return powerRequirement; }

	inline int getBuiltValue() const { return unitBuiltValue + structureBuiltValue; }
	inline int getUnitBuiltValue() const { return unitBuiltValue; }
	inline int getMilitaryValue() const { return militaryValue; }
	inline int getKillValue() const { return killValue; }
	inline int getLossValue() const { return lossValue; }
	inline int getStructureBuiltValue() const { return structureBuiltValue; }
	inline int getNumBuiltUnits() const { return numBuiltUnits; }
	inline int getNumBuiltStructures() const { return numBuiltStructures; }
	inline int getDestroyedValue() const { return destroyedValue; }
	inline int getNumDestroyedUnits() const { return numDestroyedUnits; }
	inline int getNumDestroyedStructures() const { return numDestroyedStructures; }
	inline int getNumBuiltItems(int itemID) const { return numItemBuilt[itemID]; }
	inline int getNumKilledItems(int itemID) const { return numItemKills[itemID]; }
	inline int getNumLostItems(int itemID) const { return numItemLosses[itemID]; }
	inline float getHarvestedSpice() const { return harvestedSpice; }

	inline int getQuota() const { return quota; };

	inline Choam& getChoam() { return choam; };
	inline const Choam& getChoam() const { return choam; };


    inline float getStartingCredits() const { return startingCredits; }
	inline float getStoredCredits() const { return storedCredits; }
    inline int getCredits() const { return lround(storedCredits+startingCredits); }
	void addCredits(float newCredits, bool wasRefined = false);
    void returnCredits(float newCredits);
	float takeCredits(float amount);

	void printStat() const;

	void updateBuildLists();

    void update();

	void incrementUnits(int itemID);
	void decrementUnits(int itemID);
	void incrementStructures(int itemID);
	void decrementStructures(int itemID, const Coord& location);

    /**
        An object was hit by something or damaged somehow else.
        \param  pObject     the object that was damaged
        \param  damage      the damage taken
        \param  damagerID   the shooter of the bullet, rocket, etc. if known; NONE otherwise
    */
    void noteDamageLocation(ObjectBase* pObject, int damage, Uint32 damagerID);

    void informWasBuilt(Uint32 itemID);
	void informHasKilled(Uint32 itemID);

	void lose(bool bSilent = false);
	void win();

	void freeHarvester(int xPos, int yPos);
	StructureBase* placeStructure(Uint32 builderID, int itemID, int xPos, int yPos, bool bForcePlacing = false);
	UnitBase* createUnit(int itemID);
	UnitBase* placeUnit(int itemID, int xPos, int yPos);

	Coord getCenterOfMainBase() const;

	Coord getStrongestUnitPosition() const;

	const std::list<std::shared_ptr<Player> >& getPlayerList() const { return players; };

protected:
	void decrementHarvesters();

	std::list<std::shared_ptr<Player> > players;        ///< List of associated players that control this house

	bool    ai;             ///< Is this an ai player?

	Uint8   houseID;        ///< The house number
	Uint8   team;           ///< The team number

    int numStructures;          ///< How many structures does this player have?
    int numUnits;               ///< How many units does this player have?
    int numItem[Num_ItemID];    ///< This array contains the number of structures/units of a certain type this player has
    int numItemBuilt[Num_ItemID];  /// Number of items built by player
    int numItemKills[Num_ItemID]; /// Number of items killed by player
    int numItemLosses [Num_ItemID]; /// Number of items lost by player

    int capacity;           ///< Total spice capacity
    int producedPower;      ///< Power prodoced by this player
    int powerRequirement;   ///< How much power does this player use?

	float storedCredits;   ///< current number of credits that are stored in refineries/silos
    float startingCredits; ///< number of starting credits this player still has
    int oldCredits;         ///< amount of credits in the last game cycle (used for playing the credits tick sound)

    int quota;              ///< number of credits to win

    Choam   choam;          ///< the things that are deliverable at the starport

    int powerUsageTimer;    ///< every N ticks you have to pay for your power usage

    // statistic
    int unitBuiltValue;
    int structureBuiltValue;
    int militaryValue;
    int killValue;
    int lossValue;
    int numBuiltUnits;
    int numBuiltStructures;
    int destroyedValue;
    int numDestroyedUnits;
    int numDestroyedStructures;
    float harvestedSpice;
};

#endif // HOUSE_H
