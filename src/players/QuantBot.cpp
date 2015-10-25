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


#include <players/QuantBot.h>

#include <Game.h>
#include <GameInitSettings.h>
#include <Map.h>
#include <sand.h>
#include <House.h>

#include <structures/StructureBase.h>
#include <structures/BuilderBase.h>
#include <structures/StarPort.h>
#include <structures/ConstructionYard.h>
#include <structures/RepairYard.h>
#include <structures/Palace.h>
#include <units/UnitBase.h>
#include <units/GroundUnit.h>
#include <units/MCV.h>
#include <units/Harvester.h>
#include <units/Saboteur.h>

#include <algorithm>

#define AIUPDATEINTERVAL 50



/**
    TODO
    == Building Placement ==


    ia) build concrete when no placement locations are available == in progress, bugs exist ==
    iii) increase favourability of being near other buildings == 50% done ==

    1. Refinerys near spice == tried but failed ==
    4. Repair yards factories, & Turrets near enemy == 50% done ==
    5. All buildings away from enemy other that silos and turrets


    == buildings ==
    i) stop repair when just on yellow (at 50%) == 50% done, still broken for some buildings as goes into yellow health ==
    ii) silo build broken == fixed ==


    == Units ==
    ii) units that get stuck in buildings should be transported to squadcenter
    vii) fix attack timer =in progress=
    xiii) carryalls get randomly stuck == in progress ==

    1) fix rally point inactivity + add area guard in one go. area guard means no sit at rally point == 75%==

    2) harvester return distance bug been introduced.

    1. Harvesters deploy away from enemy
    3. launchers run from enemy == 75% ==
    5. fix gun turret & gun for rocket turret

    x. Improve squad management

    == Ornithopters ==
    1. Add them with some logic
    2. Make carryalls and ornithopers easier to hit

**/



int hLimit = 4;
int militaryValue;
int initialMilitaryValue;


QuantBot::QuantBot(House* associatedHouse, std::string playername, Uint32 difficulty)
: Player(associatedHouse, playername), difficulty(difficulty) {

	buildTimer = getRandomGen().rand(0,3) * 50;

    attackTimer = MILLI2CYCLES(10000);

    if(currentGame->gameType == GAMETYPE_CUSTOM){
        gameMode = CUSTOM;
    }else{
        gameMode = CAMPAIGN;
    }


    if(gameMode == CAMPAIGN){
        // Wait at least 8 minutes if its a campaign game
        attackTimer = MILLI2CYCLES(480000);
    }

    if(gameMode == CAMPAIGN && currentGame->techLevel == 8){
        // Wait at least 8 minutes if its a campaign game
        attackTimer = MILLI2CYCLES(720000);
    }

    campaignAIAttackFlag = false;
    initialCountComplete = false;
    //militaryValueLimit = 0;



}

QuantBot::QuantBot(InputStream& stream, House* associatedHouse) : Player(stream, associatedHouse) {
    QuantBot::init();

	difficulty = stream.readUint32();
	gameMode = stream.readUint32();
	buildTimer = stream.readSint32();
    attackTimer = stream.readSint32();
    militaryValueLimit = stream.readSint32();

    for (Uint32 i = ItemID_FirstID; i <= Structure_LastID; i++ ){
       initialItemCount[i] = stream.readUint32();
    }

    /**
        Need to add in a building array for when people save and load
        So that it keeps the count of buildings that should be on the map.
    **/

	Uint32 NumPlaceLocations = stream.readUint32();
	for(Uint32 i = 0; i < NumPlaceLocations; i++) {
        Sint32 x = stream.readSint32();
        Sint32 y = stream.readSint32();

		placeLocations.push_back(Coord(x,y));
	}

	campaignAIAttackFlag = stream.readBool();
}

void QuantBot::init() {



}


QuantBot::~QuantBot() {
}

void QuantBot::save(OutputStream& stream) const {
    Player::save(stream);

	stream.writeUint32(difficulty);
    stream.writeUint32(gameMode);
    stream.writeSint32(buildTimer);
    stream.writeSint32(attackTimer);
    stream.writeSint32(militaryValueLimit);

    for (Uint32 i = ItemID_FirstID; i <= Structure_LastID; i++ ){
        stream.writeUint32(initialItemCount[i]);
    }

	stream.writeUint32(placeLocations.size());
	std::list<Coord>::const_iterator iter;
	for(iter = placeLocations.begin(); iter != placeLocations.end(); ++iter) {
		stream.writeSint32(iter->x);
		stream.writeSint32(iter->y);
	}

	stream.writeBool(campaignAIAttackFlag);
}



void QuantBot::update() {
    if( (getGameCylceCount() + getHouse()->getHouseID()) % AIUPDATEINTERVAL != 0) {
        // we are not updating this AI player this cycle
        return;
    }

    // Count the structures once initially for campaign


    // Calculate the total military value of the player
    militaryValue = 0;
    initialMilitaryValue = 0;
    for(Uint32 i = Unit_FirstID; i <= Unit_LastID; i++){
            if(i != Unit_Carryall && i != Unit_Harvester){
                militaryValue += getHouse()->getNumItems(i) * currentGame->objectData.data[i][getHouse()->getHouseID()].price;

                // Used for campaign mode. For some reason I can't get it to 'stick' between cycles, thus why I'm recalculating it
                // each time....
                initialMilitaryValue += initialItemCount[i] * currentGame->objectData.data[i][getHouse()->getHouseID()].price;
            }
    }

    //fprintf(stdout, "Military Value %d  Initial Military Value %d\n", militaryValue, initialMilitaryValue);



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



    fflush(stdout);


}

void QuantBot::onIncrementStructures(int itemID) {
}

void QuantBot::onDecrementStructures(int itemID, const Coord& location) {
}

void QuantBot::onDamage(const ObjectBase* pObject, int damage, Uint32 damagerID) {
    const ObjectBase* pDamager = getObject(damagerID);




    if(pDamager == NULL || pDamager->getOwner() == getHouse()) {
        return;
    }

    // If the human has attacked us then its time to start fighting back...
    if(gameMode == CAMPAIGN && !pDamager->getOwner()->isAI() && !campaignAIAttackFlag){
        campaignAIAttackFlag = true;
    }

    else if (pObject->isAStructure()){

        doRepair(pObject);

        // no point scrambling to defend a missile
        if(pDamager->getItemID() != Structure_Palace){
            scrambleUnitsAndDefend(pObject);
        }

    }

    else if(pObject->isAGroundUnit()){
        Coord squadCenterLocation = findSquadCenter(pObject->getOwner()->getHouseID());

        const GroundUnit* pUnit = dynamic_cast<const GroundUnit*>(pObject);

        if(pUnit == NULL){
            return;
        }

        if(pUnit->isawaitingPickup())
        {
            return;
        }


        /**
            Always keep Harvesters away from harm
        **/
        if (pUnit->getItemID() == Unit_Harvester){

            // Defend the harvester!
            scrambleUnitsAndDefend(pObject);
            const Harvester* pHarvester = dynamic_cast<const Harvester*>(pUnit);
            doReturn(pHarvester);
        }



        /**
            Always keep Launchers away from harm
        **/

        else if (pUnit->getItemID() == Unit_Launcher
            || pUnit->getItemID() == Unit_Deviator){

            doSetAttackMode(pUnit, GUARD);
            doMove2Pos(pUnit, squadCenterLocation.x, squadCenterLocation.y, true);

        }


        /**
            We want to use our light vehicles as raiders.
            This means they are free to engage other light military units
            but should run away from tanks
        **/
        else if((pUnit->getItemID() == Unit_Quad

                // Quads don't run from trikes, infantry, other quads
                && !pDamager->isInfantry()
                && pDamager->getItemID() != Unit_RaiderTrike
                && pDamager->getItemID() != Unit_Trike
                && pDamager->getItemID() != Unit_Quad)

                // Trikes run from quads
                || ((pUnit->getItemID() == Unit_RaiderTrike
                    || pUnit->getItemID() == Unit_Trike)
                    && !pDamager->isInfantry()
                    && pDamager->getItemID() != Unit_RaiderTrike
                    && pDamager->getItemID() != Unit_Trike)){

            doSetAttackMode(pUnit, AREAGUARD);
            doMove2Pos(pUnit, squadCenterLocation.x, squadCenterLocation.y, true);

        }


        /**
            Tanks run from other tanks and Ornithopters
        **/

        else if ((pUnit->getItemID() == Unit_Tank || pUnit->getItemID() == Unit_SiegeTank)
            && pUnit->getHealth() * 100 / pUnit->getMaxHealth() < 90
            && pDamager->getItemID() != Structure_GunTurret
            && pDamager->getItemID() != Structure_RocketTurret
            && pDamager->getItemID() != Unit_RaiderTrike
            && pDamager->getItemID() != Unit_Trike
            && pDamager->getItemID() != Unit_Quad
            && pDamager->getItemID() != Unit_SonicTank
            && pDamager->getItemID() != Unit_Deviator
            && !pDamager->isInfantry()){

            doSetAttackMode(pUnit, AREAGUARD);
            doMove2Pos(pUnit, squadCenterLocation.x, squadCenterLocation.y, true);
        }

        /**
            If the unit is at 65% health or less and is not being forced to move anywhere
            repair them, if they are eligible to be repaired
        **/

        if((pUnit->getHealth() * 100) / pUnit->getMaxHealth() < 60
            && !pUnit->isInfantry()
            && pUnit->isVisible())
        {

            if(getHouse()->hasRepairYard()){
                doRepair(pUnit);
            }else if(gameMode == CUSTOM){
                doSetAttackMode(pUnit, RETREAT);
            }


        }
    }
}

Coord QuantBot::findMcvPlaceLocation(const MCV* pMCV) {
    int bestLocationX, bestLocationY = -1;
    int bestLocationScore = 1000;
    Coord bestLocation = Coord::Invalid();

    bestLocation = findPlaceLocation(Structure_ConstructionYard);

    if(bestLocation == Coord::Invalid()){
        fprintf(stdout,"No MCV deploy location adjacent to existing base structures was found, move to full search | ");

        // Don't place on the vey edge of the map
        for(int placeLocationX = 1; placeLocationX < getMap().getSizeX() -1; placeLocationX++)
        {
            for(int placeLocationY = 1; placeLocationY < getMap().getSizeY() -1; placeLocationY++)
            {
                Coord placeLocation = Coord::Invalid();
                placeLocation.x = placeLocationX;
                placeLocation.y = placeLocationY;

                if(getMap().okayToPlaceStructure(placeLocationX,
                                                placeLocationY,
                                                2,
                                                2,
                                                false,
                                                NULL)){
                    int locationScore = blockDistance(pMCV->getLocation(), placeLocation);
                    if(locationScore < bestLocationScore){
                        bestLocationScore = locationScore;
                        bestLocation.x = placeLocationX;
                        bestLocation.y = placeLocationY;
                    }
                }
            }
        }
    }

    return bestLocation;

}

/*


*/

Coord QuantBot::findPlaceLocation(Uint32 itemID) {

    RobustList<const StructureBase*>::const_iterator iter;

    // Will over allocate space for small maps so its not clean
    // But should allow Richard to compile
    int buildLocationScore[128][128] = {0};

    int bestLocationX, bestLocationY = -1;
    int bestLocationScore = - 10000;
    int newSizeX = getStructureSize(itemID).x;
    int newSizeY = getStructureSize(itemID).y;
    Coord bestLocation = Coord::Invalid();

    Coord baseCentre = findBaseCentre(getHouse()->getHouseID());

    bool newIsBuilder = (itemID == Structure_HeavyFactory
                                   || itemID == Structure_Refinery
                                   || itemID == Structure_RepairYard
                                   || itemID == Structure_LightFactory
                                   || itemID == Structure_WOR
                                   || itemID == Structure_Barracks
                                   || itemID == Structure_StarPort);

    for(iter = getStructureList().begin(); iter != getStructureList().end(); ++iter) {
        const StructureBase* pStructureExisting = *iter;


        if(pStructureExisting->getOwner() == getHouse()) {

            int existingStartX = pStructureExisting->getX();
            int existingStartY = pStructureExisting->getY();

            int existingSizeX = pStructureExisting->getStructureSizeX();
            int existingSizeY = pStructureExisting->getStructureSizeY();

            int existingEndX = existingStartX + existingSizeX;
            int existingEndY = existingStartY + existingSizeY;

            squadRallyLocation = findSquadRallyLocation();

            bool existingIsBuilder = (pStructureExisting->getItemID() == Structure_HeavyFactory
                                   || pStructureExisting->getItemID() == Structure_RepairYard
                                   || pStructureExisting->getItemID() == Structure_LightFactory
                                   || pStructureExisting->getItemID() == Structure_WOR
                                   || pStructureExisting->getItemID() == Structure_Barracks
                                   || pStructureExisting->getItemID() == Structure_StarPort);

            bool sizeMatchX = (existingSizeX == newSizeX);
            bool sizeMatchY = (existingSizeY == newSizeY);


            for(int placeLocationX = existingStartX - newSizeX; placeLocationX <= existingEndX; placeLocationX++){
                for(int placeLocationY = existingStartY - newSizeY; placeLocationY <= existingEndY; placeLocationY++){
                    if(getMap().tileExists(placeLocationX,placeLocationY)){
                        if(getMap().okayToPlaceStructure(placeLocationX,
                                                            placeLocationY,
                                                            newSizeX,
                                                            newSizeY,
                                                            false,
                                                            (itemID == Structure_ConstructionYard) ? NULL : getHouse())){

                                int placeLocationEndX = placeLocationX + newSizeX;
                                int placeLocationEndY = placeLocationY + newSizeY;

                                bool alignedX = (placeLocationX == existingStartX && sizeMatchX);
                                bool alignedY = (placeLocationY == existingStartY && sizeMatchY);

                                /*
                                bool placeGapExists = (placeLocationEndX < existingStartX
                                                       || placeLocationX > existingEndX
                                                       || placeLocationEndY < existingStartY
                                                       || placeLocationY > existingEndY);
                                */

                                // give a point for being valid


                                // How many free spaces the building will have if placed
                                for(int i = placeLocationX-1; i <= placeLocationEndX; i++){
                                    for(int j = placeLocationY-1; j <= placeLocationEndY; j++){
                                        if(getMap().tileExists(i,j)
                                           && (getMap().getSizeX() > i)
                                           && (0 <= i)
                                           && (getMap().getSizeY() > j)
                                           && (0 <= j)){


                                                // Penalise if near edge of map
                                                if(i == 0
                                                   || i == getMap().getSizeX() - 1
                                                   || j == 0
                                                   || j == getMap().getSizeY() - 1){

                                                    buildLocationScore[placeLocationX][placeLocationY] -= 10;
                                                }

                                                if(getMap().getTile(i,j)->hasAStructure()){

                                                    // If one of our buildings is nearby favour the location
                                                    // if it is someone elses building don't favour it
                                                    if(getMap().getTile(i,j)->getOwner() == getHouse()->getHouseID()){
                                                        buildLocationScore[placeLocationX][placeLocationY]+=2;


                                                    } else{
                                                        buildLocationScore[placeLocationX][placeLocationY]-=10;
                                                    }


                                                }


                                                else if(!getMap().getTile(i,j)->isRock()){

                                                    // square isn't rock, favour it
                                                    buildLocationScore[placeLocationX][placeLocationY]+=1;
                                                }


                                                else if(getMap().getTile(i,j)->hasAGroundObject()){

                                                    if(getMap().getTile(i,j)->getOwner() != getHouse()->getHouseID()){
                                                        // try not to build next to units which aren't yours
                                                        buildLocationScore[placeLocationX][placeLocationY]-=100;
                                                    }else{
                                                        buildLocationScore[placeLocationX][placeLocationY]-=20;
                                                    }
                                                }

                                        }else{
                                            // penalise if on edge of map
                                            buildLocationScore[placeLocationX][placeLocationY]-=200;
                                        }
                                    }
                                }

                                //encourage structure alignment


                                if(alignedX){
                                    buildLocationScore[placeLocationX][placeLocationX] += existingSizeX;
                                }

                                if(alignedY){
                                    buildLocationScore[placeLocationX][placeLocationY] += existingSizeY;
                                }



                                /*
                                   Add building specific scores
                                */
                                if(existingIsBuilder || itemID == Structure_GunTurret || itemID == Structure_RocketTurret){
                                    buildLocationScore[placeLocationX][placeLocationY] -= blockDistance(squadRallyLocation, Coord(placeLocationX,placeLocationY));
                                }

                                // Pick this location if it has the best score
                                if (buildLocationScore[placeLocationX][placeLocationY] > bestLocationScore){
                                    bestLocationScore = buildLocationScore[placeLocationX][placeLocationY];
                                    bestLocationX = placeLocationX;
                                    bestLocationY = placeLocationY;
                                    fprintf(stdout, "Build location for item:%d  x:%d y:%d score:%d\n",
                                            itemID,
                                            bestLocationX,
                                            bestLocationY,
                                            bestLocationScore);
                                }
                         }
                    }
                }
            }
        }
    }


    if (bestLocationScore != -10000){
        bestLocation = Coord(bestLocationX, bestLocationY);
    }

	return bestLocation;
}


void QuantBot::build() {

    // First count all the objects we have
    for (int i = ItemID_FirstID; i <= ItemID_LastID; i++ ){
       if(getHouse()->getNumItems(i) > 0){
            itemCount[i] = getHouse()->getNumItems(i);

            if(!initialCountComplete){
                initialItemCount[i] = getHouse()->getNumItems(i);
                fprintf(stdout,"Initial: Item: %d  Count: %d\n", i, initialItemCount[i]);


            }

       } else{

            itemCount[i] = 0;
            if(!initialCountComplete){
                initialItemCount[i] = 0;
                fprintf(stdout,"Initial: Item: %d  Count: %d\n", i, initialItemCount[i]);
            }
       }
    }

    if(initialItemCount[Structure_RepairYard] == 0
       && gameMode == CAMPAIGN
       && currentGame->techLevel > 4){

            initialItemCount[Structure_RepairYard] = 1;
            if(initialItemCount[Structure_Radar] == 0){
                initialItemCount[Structure_Radar] = 1;
            }

            if(initialItemCount[Structure_LightFactory] == 0){
                initialItemCount[Structure_LightFactory] = 1;
            }

            fprintf(stdout, "Allow Campaign AI one Repair Yard\n");

    }


    initialCountComplete = true;

    switch(gameMode){

        case CAMPAIGN:{

             switch(difficulty){
                case EASY: {

                    hLimit = initialItemCount[Structure_Refinery];

                    if(currentGame->techLevel == 8){
                        militaryValueLimit = 4000;

                    }
                    else{
                        militaryValueLimit = initialMilitaryValue;
                    }


                    fprintf(stdout, "Easy Campaign  ");


                } break;

                case MEDIUM: {

                    if(currentGame->techLevel == 8){
                        hLimit = 2;
                        militaryValueLimit = 4000;

                    }else{
                        hLimit = 2 * initialItemCount[Structure_Refinery];
                        militaryValueLimit = initialMilitaryValue * 1.5;

                    }


                    fprintf(stdout, "Medium Campaign  ");


                } break;

                case HARD: {


                    if(currentGame->techLevel == 8){
                        hLimit = 3;
                        initialItemCount[Structure_Refinery] = 2;
                        militaryValueLimit = 5000;
                    }else{

                        hLimit = 3 * initialItemCount[Structure_Refinery];
                        militaryValueLimit = initialMilitaryValue * 2;
                    }


                    fprintf(stdout, "Hard Campaign  ");


                } break;

                case BRUTAL: {


                    hLimit = (currentGameMap->getSizeX() * currentGameMap->getSizeY() / 512);
                    militaryValueLimit = 25000;


                    fprintf(stdout, "Brutal Campaign  ");


                } break;
            }



        } break;

        case CUSTOM:{

             switch(difficulty){


                case BRUTAL: {
                    hLimit = 60;
                    militaryValueLimit = 150000;

                    //fprintf(stdout,"BUILD BRUTAL SKIRM ");
                } break;

                case EASY: {
                    hLimit = (currentGameMap->getSizeX() * currentGameMap->getSizeY() / 2048);
                    militaryValueLimit = 10000;

                    //fprintf(stdout,"BUILD EASY SKIRM ");
                } break;

                case MEDIUM: {
                    hLimit = (currentGameMap->getSizeX() * currentGameMap->getSizeY() / 1024);
                    militaryValueLimit = 25000;

                    //fprintf(stdout,"BUILD MEDIUM SKIRM ");
                } break;

                case HARD: {
                    hLimit = (currentGameMap->getSizeX() * currentGameMap->getSizeY() / 512);
                    militaryValueLimit = 50000;

                    //fprintf(stdout,"BUILD HARD SKIRM ");
                } break;


            }
        }


        if((currentGameMap->getSizeX() * currentGameMap->getSizeY() / 512) < hLimit
           && difficulty != BRUTAL){
            hLimit = currentGameMap->getSizeX() * currentGameMap->getSizeY() / 512;
        }


    }


    int activeHeavyFactoryCount = 0;
    int activeRepairYardCount = 0;




    /*
        Let's try just running this once...
    */
    if(squadRallyLocation.x == 0 && squadRallyLocation.y == 0 ){
        fprintf(stdout,"Set squad rally location\n");
        retreatAllUnits();
    }


    // Next add in the objects we are building
    RobustList<const StructureBase*>::const_iterator iter;

    for(iter = getStructureList().begin(); iter != getStructureList().end(); ++iter) {
        const StructureBase* pStructure = *iter;

        if(pStructure->getOwner() == getHouse()){


            if(pStructure->isABuilder()) {
                const BuilderBase* pBuilder = dynamic_cast<const BuilderBase*>(pStructure);
                if(pBuilder->getProductionQueueSize() > 0){
                    itemCount[pBuilder->getCurrentProducedItem()]++;
                    if(pBuilder->getItemID() == Structure_HeavyFactory){
                        activeHeavyFactoryCount++;
                    }
                }


            }else if(pStructure->getItemID() == Structure_RepairYard){
                const RepairYard* pRepairYard= dynamic_cast<const RepairYard*>(pStructure);
                if(!pRepairYard->isFree()){
                    activeRepairYardCount++;
                }

            }


            // Set unit deployment position
            if(pStructure->getItemID() == Structure_Barracks
               || pStructure->getItemID() == Structure_WOR
               || pStructure->getItemID() == Structure_LightFactory
               || pStructure->getItemID() == Structure_HeavyFactory
               || pStructure->getItemID() == Structure_RepairYard
               || pStructure->getItemID() == Structure_StarPort)
            {
                doSetDeployPosition(pStructure, squadRallyLocation.x, squadRallyLocation.y);

            }
        }


    }


    int money = getHouse()->getCredits();

    fprintf(stdout,"House: %d hvstr: %d hLim: %d mValLim: %d mVal: %d att: %d crdt: %d repair(a/t): %d/%d fact(a/t): %d/%d | alive: %d built: %d kills: %d deaths: %d\n",
            getHouse()->getHouseID(),
            getHouse()->getNumItems(Unit_Harvester),
            hLimit,
            militaryValueLimit,
            militaryValue,
            attackTimer,
            getHouse()->getCredits(),
            activeRepairYardCount,
            getHouse()->getNumItems(Structure_RepairYard),
            activeHeavyFactoryCount,
            getHouse()->getNumItems(Structure_HeavyFactory),
            getHouse()->getNumUnits(),
            getHouse()->getNumBuiltUnits(),
            getHouse()->getNumDestroyedUnits(),
            getHouse()->getNumBuiltUnits() - getHouse()->getNumUnits());

    for(iter = getStructureList().begin(); iter != getStructureList().end(); ++iter) {
        const StructureBase* pStructure = *iter;

        if(pStructure->getOwner() == getHouse()) {

            if((pStructure->isRepairing() == false)
               && (pStructure->getHealth() < pStructure->getMaxHealth())
                && (!getGameInitSettings().getGameOptions().concreteRequired
                     || pStructure->getItemID() == Structure_Palace) // Palace repairs for free
                && (pStructure->getItemID() != Structure_Refinery
                    && pStructure->getItemID() != Structure_Silo
                    && pStructure->getItemID() != Structure_Radar
                    && pStructure->getItemID() != Structure_WindTrap))
            {
                doRepair(pStructure);
            }

            else if(  (pStructure->isRepairing() == false)
                        && (pStructure->getHealth() < pStructure->getMaxHealth() *0.45)
                        && !getGameInitSettings().getGameOptions().concreteRequired
                        && money > 1000)
            {

                doRepair(pStructure);
            }
            // Repair if we are rich
            else if(  (pStructure->isRepairing() == false)
                        && money > 5000)
            {

                doRepair(pStructure);
            } else if(pStructure->getItemID() == Structure_RocketTurret)
            {
                if(!getGameInitSettings().getGameOptions().structuresDegradeOnConcrete
                    || pStructure->hasATarget())
                {

                    doRepair(pStructure);
                }
            }


            /*

                Deathhand launch logic
            */


            if(pStructure->getItemID() == Structure_Palace

               /*&& (getHouse()->getHouseID() != HOUSE_ORDOS
                   && getHouse()->getHouseID() != HOUSE_MERCENARY)*/)
            {

                const Palace* pPalace = dynamic_cast<const Palace*>(pStructure);


                if(pPalace != NULL){
                    if(pPalace->isSpecialWeaponReady()){

                        if(getHouse()->getHouseID() != HOUSE_HARKONNEN
                           && getHouse()->getHouseID() != HOUSE_SARDAUKAR)
                        {
                               doSpecialWeapon(pPalace);
                        }else{

                            int enemyHouseID = -1;
                            int enemyHouseBuildingCount = 0;



                            for(int i = HOUSE_HARKONNEN; i <= HOUSE_MERCENARY; i++){

                                if(getHouse(i) != NULL){
                                    if(getHouse(i)->getTeam() != getHouse()->getTeam()
                                       && getHouse(i)->getNumStructures() > enemyHouseBuildingCount){

                                        enemyHouseBuildingCount = getHouse(i)->getNumStructures();
                                        enemyHouseID = i;

                                    }
                                }

                            }


                            if(enemyHouseID != -1
                               && (getHouse()->getHouseID() == HOUSE_HARKONNEN
                                   || getHouse()->getHouseID() == HOUSE_SARDAUKAR)){
                                Coord target = findBaseCentre(enemyHouseID);
                                doLaunchDeathhand(pPalace, target.x, target.y);
                            }

                        }



                    }
                }

            }


            const BuilderBase* pBuilder = dynamic_cast<const BuilderBase*>(pStructure);
            if(pBuilder != NULL) {

                switch (pStructure->getItemID()) {



                    case Structure_LightFactory: {

                        if(!pBuilder->isUpgrading()
                           && gameMode == CAMPAIGN
                           && money > 1000
                           && (itemCount[Structure_HeavyFactory == 0] || militaryValue < militaryValueLimit * 0.30)
                           && pBuilder->getProductionQueueSize() < 1
                           && pBuilder->getBuildListSize() > 0
                           && militaryValue < militaryValueLimit){

                            if(pBuilder->getCurrentUpgradeLevel() < pBuilder->getMaxUpgradeLevel()
                               && getHouse()->getCredits() > 1500){
                                doUpgrade(pBuilder);
                            }else{

                                Uint32 itemID = NONE;

                                if(pBuilder->isAvailableToBuild(Unit_RaiderTrike)){
                                    itemID = Unit_RaiderTrike;
                                }

                                else if(pBuilder->isAvailableToBuild(Unit_Quad)){
                                    itemID = Unit_Quad;
                                }

                                else if(pBuilder->isAvailableToBuild(Unit_Trike)){
                                    itemID = Unit_Trike;
                                }

                                if(itemID != NONE){
                                    doProduceItem(pBuilder, itemID);
                                    itemCount[itemID]++;
                                }
                            }

                        }
                    } break;


                    case Structure_WOR: {
                        if(!pBuilder->isUpgrading()
                           && gameMode == CAMPAIGN
                           && money > 1000
                           && (itemCount[Structure_HeavyFactory == 0] || militaryValue < militaryValueLimit * 0.30)
                           && pBuilder->getProductionQueueSize() < 1
                           && pBuilder->getBuildListSize() > 0
                           && militaryValue < militaryValueLimit){

                            doProduceItem(pBuilder, Unit_Trooper);
                            itemCount[Unit_Trooper]++;
                        }
                    } break;

                    case Structure_Barracks: {
                        if(!pBuilder->isUpgrading()
                           && gameMode == CAMPAIGN
                           && (itemCount[Structure_HeavyFactory == 0] || militaryValue < militaryValueLimit * 0.30)
                           && itemCount[Structure_WOR == 0]
                           && money > 1000
                           && pBuilder->getProductionQueueSize() < 1
                           && pBuilder->getBuildListSize() > 0
                           && militaryValue < militaryValueLimit){

                            doProduceItem(pBuilder, Unit_Soldier);
                            itemCount[Unit_Soldier]++;
                        }
                    } break;


                    case Structure_HighTechFactory: {
                        if(itemCount[Unit_Carryall] < (militaryValue + itemCount[Unit_Harvester] * 500) / 3000
                           && (pBuilder->getProductionQueueSize() < 1)
                           && money > 1000){
                            doProduceItem(pBuilder, Unit_Carryall);
                            itemCount[Unit_Carryall]++;
                        }
                    } break;


                    case Structure_HeavyFactory: {

                        // only if the factory isn't busy
                        if((pBuilder->isUpgrading() == false)
                           && (pBuilder->getProductionQueueSize() < 1)
                           && (pBuilder->getBuildListSize() > 0)){


                            // we need a construction yard. Build an MCV if we don't have a starport
                            if(itemCount[Unit_MCV]
                               + itemCount[Structure_ConstructionYard]
                               + itemCount[Structure_StarPort] < 1
                                && pBuilder->isAvailableToBuild(Unit_MCV)){

                               doProduceItem(pBuilder, Unit_MCV);
                               itemCount[Unit_MCV]++;
                            }


                            // If we are really rich, like in all against Atriedes
                            else if(gameMode == CUSTOM && (itemCount[Structure_ConstructionYard] + itemCount[Unit_MCV] )*10000 < getHouse()->getCredits()
                                && pBuilder->isAvailableToBuild(Unit_MCV)
                                && itemCount[Structure_ConstructionYard] + itemCount[Unit_MCV] < 10){

                               doProduceItem(pBuilder, Unit_MCV);
                               itemCount[Unit_MCV]++;
                            }

                            // If we are kind of rich make a backup construction yard to spend the excess money
                            else if(gameMode == CUSTOM && (itemCount[Structure_ConstructionYard] + itemCount[Unit_MCV] ) == 1
                                    && getHouse()->getCredits() > 10000
                                    && pBuilder->isAvailableToBuild(Unit_MCV)){

                               doProduceItem(pBuilder, Unit_MCV);
                               itemCount[Unit_MCV]++;
                            }


                            // In case we get given lots of money, it will eventually run out so we need to be prepared
                            else if(gameMode == CUSTOM && itemCount[Unit_Harvester] < militaryValue / 1000
                                    && itemCount[Unit_Harvester] < hLimit ) {
                                    doProduceItem(pBuilder, Unit_Harvester);
                                    itemCount[Unit_Harvester]++;
                            }

                            else if(itemCount[Unit_Harvester] < hLimit
                                   && (money < 2500
                                        || gameMode == CAMPAIGN)) {

                                //fprintf(stdout,"*Building a Harvester.\n",
                                //itemCount[Unit_Harvester], hLimit, money);

                                doProduceItem(pBuilder, Unit_Harvester);
                                itemCount[Unit_Harvester]++;
                            }

                            else if((money > 500)
                                   && (pBuilder->isUpgrading() == false)
                                   && (pBuilder->getCurrentUpgradeLevel() < pBuilder->getMaxUpgradeLevel()))
                            {
                                if (pBuilder->getHealth() >= pBuilder->getMaxHealth()){
                                    doUpgrade(pBuilder);
                                }else{
                                    doRepair(pBuilder);
                                }

                            }

                            else if(money > 1200
                                    && militaryValue < militaryValueLimit) { // Limit enemy military units based on difficulty


                                if(pBuilder->isAvailableToBuild(Unit_Launcher)
                                   &&(itemCount[Unit_Launcher] < militaryValue / 1600
                                      || (gameMode == CAMPAIGN && currentGame->techLevel == 8 && itemCount[Unit_Launcher] < militaryValue / 1100) )){

                                    doProduceItem(pBuilder, Unit_Launcher);
                                    itemCount[Unit_Launcher]++;
                                    money -= currentGame->objectData.data[Unit_Launcher][getHouse()->getHouseID()].price;
                                    militaryValue += currentGame->objectData.data[Unit_Launcher][getHouse()->getHouseID()].price;
                                }


                                else if(pBuilder->isAvailableToBuild(Unit_SonicTank)) {
                                    doProduceItem(pBuilder, Unit_SonicTank);
                                    itemCount[Unit_SonicTank]++;
                                }

                                else if(pBuilder->isAvailableToBuild(Unit_Devastator)) {
                                    doProduceItem(pBuilder, Unit_Devastator);
                                    itemCount[Unit_Devastator]++;
                                }

                                else if(pBuilder->isAvailableToBuild(Unit_SiegeTank)) {
                                    doProduceItem(pBuilder, Unit_SiegeTank);
                                    itemCount[Unit_SiegeTank]++;
                                    money -= currentGame->objectData.data[Unit_SiegeTank][getHouse()->getHouseID()].price;
                                    militaryValue += currentGame->objectData.data[Unit_SiegeTank][getHouse()->getHouseID()].price;
                                }

                                else if(pBuilder->isAvailableToBuild(Unit_Tank)) {
                                    doProduceItem(pBuilder, Unit_Tank);
                                    itemCount[Unit_Tank]++;
                                    money -= currentGame->objectData.data[Unit_Tank][getHouse()->getHouseID()].price;
                                    militaryValue += currentGame->objectData.data[Unit_Tank][getHouse()->getHouseID()].price;
                                }

                            }
                        }

                    } break;

                    case Structure_StarPort: {
                        const StarPort* pStarPort = dynamic_cast<const StarPort*>(pBuilder);
                        if(pStarPort->okToOrder())	{
                            const Choam& choam = getHouse()->getChoam();



                            // We need a construction yard!!
                            if(pStarPort->isAvailableToBuild(Unit_MCV)
                               && choam.getNumAvailable(Unit_MCV) > 0
                               && itemCount[Structure_ConstructionYard] + itemCount[Unit_MCV] < 1){

                                doProduceItem(pBuilder, Unit_MCV);
                                itemCount[Unit_MCV]++;
                                money = money - choam.getPrice(Unit_MCV);
                            }

                            if(money >= choam.getPrice(Unit_Carryall) && itemCount[Unit_Carryall] == 0){
                                doProduceItem(pBuilder, Unit_Carryall);
                                itemCount[Unit_Carryall]++;
                                money = money - choam.getPrice(Unit_Carryall);
                            }

                            else if(militaryValue > (itemCount[Unit_Harvester]*200)){

                                while (money > choam.getPrice(Unit_Harvester)
                                       && choam.getNumAvailable(Unit_Harvester) > 0
                                       && itemCount[Unit_Harvester] < hLimit){

                                    doProduceItem(pBuilder, Unit_Harvester);
                                    itemCount[Unit_Harvester]++;
                                    money = money - choam.getPrice(Unit_Harvester);
                                }

                                while (money > choam.getPrice(Unit_Carryall)
                                       && choam.getNumAvailable(Unit_Carryall) > 0
                                       && itemCount[Unit_Carryall] <
                                          (itemCount[Unit_Tank]
                                          + itemCount[Unit_SiegeTank]
                                          + itemCount[Unit_Launcher]
                                          + itemCount[Unit_Quad]
                                          + itemCount[Unit_Harvester]) / 5)
                                {
                                    doProduceItem(pBuilder, Unit_Carryall);
                                    itemCount[Unit_Carryall]++;
                                    money = money - choam.getPrice(Unit_Carryall);
                                }
                            }

                            /*
                                Get at least one Carryall
                            */
                            if (money > choam.getPrice(Unit_Carryall)
                                   && choam.getNumAvailable(Unit_Carryall) > 0
                                   && itemCount[Unit_Carryall] == 0)
                            {
                                doProduceItem(pBuilder, Unit_Carryall);
                                itemCount[Unit_Carryall]++;
                                money = money - choam.getPrice(Unit_Carryall);
                            }



                            if(militaryValue < militaryValueLimit && itemCount[Unit_Carryall] > 0 ){

                                while (money > choam.getPrice(Unit_SiegeTank) && choam.getNumAvailable(Unit_SiegeTank) > 0
                                       && choam.isCheap(Unit_SiegeTank)
                                       && militaryValue < militaryValueLimit)
                                {
                                    doProduceItem(pBuilder, Unit_SiegeTank);
                                    itemCount[Unit_SiegeTank]++;
                                    money = money - choam.getPrice(Unit_SiegeTank);
                                    militaryValue += currentGame->objectData.data[Unit_SiegeTank][getHouse()->getHouseID()].price;
                                }

                                while (money > choam.getPrice(Unit_Tank) && choam.getNumAvailable(Unit_Tank) > 0
                                       && choam.isCheap(Unit_Tank)
                                       && militaryValue < militaryValueLimit)
                                {
                                    doProduceItem(pBuilder, Unit_Tank);
                                    itemCount[Unit_Tank]++;
                                    money = money - choam.getPrice(Unit_Tank);
                                    militaryValue += currentGame->objectData.data[Unit_Tank][getHouse()->getHouseID()].price;
                                }

                                while (money > choam.getPrice(Unit_Launcher) && choam.getNumAvailable(Unit_Launcher) > 0
                                       && choam.isCheap(Unit_Launcher)
                                       && militaryValue < militaryValueLimit
                                       && militaryValue > 1000)
                                {
                                    doProduceItem(pBuilder, Unit_Launcher);
                                    itemCount[Unit_Launcher]++;
                                    money = money - choam.getPrice(Unit_Launcher);
                                    militaryValue += currentGame->objectData.data[Unit_Launcher][getHouse()->getHouseID()].price;
                                }


                                while (money > choam.getPrice(Unit_Quad) && choam.getNumAvailable(Unit_Quad) > 0
                                       && choam.isCheap(Unit_Quad)
                                       && militaryValue * 10 < militaryValueLimit)
                                {
                                    doProduceItem(pBuilder, Unit_Quad);
                                    itemCount[Unit_Quad]++;
                                    money = money - choam.getPrice(Unit_Quad);
                                    militaryValue += currentGame->objectData.data[Unit_Quad][getHouse()->getHouseID()].price;
                                }

                                while (money > choam.getPrice(Unit_Trike) && choam.getNumAvailable(Unit_Trike) > 0
                                       && choam.isCheap(Unit_Trike)
                                       && militaryValue * 10 < militaryValueLimit)
                                {
                                    doProduceItem(pBuilder, Unit_Trike);
                                    itemCount[Unit_Trike]++;
                                    money = money - choam.getPrice(Unit_Trike);
                                    militaryValue += currentGame->objectData.data[Unit_Trike][getHouse()->getHouseID()].price;
                                }
                            }

                            doPlaceOrder(pStarPort);

                        }

                    } break;

                    case Structure_ConstructionYard: {


                        const ConstructionYard* pConstYard = dynamic_cast<const ConstructionYard*>(pBuilder);

                        if(!pBuilder->isUpgrading()
                           && getHouse()->getCredits() > 100
                           && pBuilder->getProductionQueueSize() < 1
                           && pBuilder->getBuildListSize() > 0){

                            /**
                                Campaign Build order, iterate through the buildings, if the number
                                that exist is less than the number that should exist, then build the one
                                that is missing
                            **/


                            if(gameMode == CAMPAIGN && difficulty != BRUTAL){

                                //fprintf(stdout,"GameMode Campaign.. ");


                                for(int i = Structure_FirstID; i <= Structure_LastID; i++){



                                    if(itemCount[i] < initialItemCount[i]
                                       && pBuilder->isAvailableToBuild(i)
                                       && findPlaceLocation(i).isValid()
                                       && !pBuilder->isUpgrading()
                                       && pBuilder->getProductionQueueSize() < 1){

                                        fprintf(stdout,"***CampAI Build itemID: %o structure count: %o, initial count: %o\n", i, itemCount[i], initialItemCount[i]);

                                        doProduceItem(pBuilder, i);
                                        itemCount[i]++;

                                    }
                                }

                                /**
                                    If Campaign AI can't build military, let it build up its cash reserves
                                    and defenses
                                **/

                                if((getHouse()->getCapacity() < getHouse()->getStoredCredits() + 2000)
                                       && pBuilder->isAvailableToBuild(Structure_Silo)
                                       && findPlaceLocation(Structure_Silo).isValid()
                                       && !pBuilder->isUpgrading()
                                       && pBuilder->getProductionQueueSize() < 1){

                                    doProduceItem(pBuilder, Structure_Silo);
                                    itemCount[Structure_Silo]++;

                                    fprintf(stdout,"***CampAI Build A new Silo increasing count to: %d", itemCount[Structure_Silo]);
                                }


                                if(pStructure->getHealth() < pStructure->getMaxHealth())
                                {

                                    doRepair(pBuilder);
                                } else if(pBuilder->getCurrentUpgradeLevel() < pBuilder->getMaxUpgradeLevel()){
                                    doUpgrade(pBuilder);
                                }




                                buildTimer = getRandomGen().rand(0,3)*5;
                            }

                            /*
                                custom AI starts here:
                            */
                            else
                            {

                                Uint32 itemID = NONE;

                                if(itemCount[Structure_WindTrap] == 0){
                                    itemID = Structure_WindTrap;
                                    itemCount[Structure_WindTrap]++;
                                }

                                else if(itemCount[Structure_Refinery] == 0
                                        || itemCount[Structure_Refinery] < itemCount[Unit_Harvester] / 2){
                                    itemID = Structure_Refinery;
                                    itemCount[Unit_Harvester]++;
                                    itemCount[Structure_Refinery]++;
                                }

                                else if(itemCount[Structure_Refinery] < 6 - (money / 2000) ){
                                    itemID = Structure_Refinery;
                                    itemCount[Unit_Harvester]++;
                                    itemCount[Structure_Refinery]++;

                                }

                                else if(itemCount[Structure_StarPort] == 0
                                        && pBuilder->isAvailableToBuild(Structure_StarPort)
                                        && findPlaceLocation(Structure_StarPort).isValid()){

                                    itemID = Structure_StarPort;
                                }



                                // Focus on the economy
                                else if(itemCount[Unit_Harvester] < (hLimit / 3) && money < 2000
                                        && ((itemCount[Structure_Refinery] < hLimit / 4
                                             && itemCount[Structure_Refinery] < 8)
                                            || itemCount[Structure_HeavyFactory] > 0)) {

                                    itemID = Structure_Refinery;
                                    itemCount[Unit_Harvester]++;

                                }


                                else if(itemCount[Structure_LightFactory] == 0){
                                    if(pBuilder->isAvailableToBuild(Structure_LightFactory)){
                                        itemID = Structure_LightFactory;
                                    }
                                }

                                else if(itemCount[Structure_Radar] == 0){
                                    if(pBuilder->isAvailableToBuild(Structure_Radar)){
                                        itemID = Structure_Radar;
                                    }
                                }

                                else if(itemCount[Structure_RepairYard] < 3
                                        && itemCount[Structure_RepairYard] * 4000 < militaryValue){
                                    if(pBuilder->isAvailableToBuild(Structure_RepairYard)){
                                        itemID = Structure_RepairYard;
                                    }
                                }

                                // Focus on the economy
                                else if(money < 2000 && itemCount[Unit_Harvester] < hLimit){

                                    itemID = Structure_Refinery;
                                    itemCount[Unit_Harvester]++;

                                }

                                else if(itemCount[Structure_HeavyFactory] == 0){
                                    if(pBuilder->isAvailableToBuild(Structure_HeavyFactory)){
                                        itemID = Structure_HeavyFactory;
                                    }
                                }



                                /*
                                    For sonic tanks and in the future ornithopters
                                */
/*
                                else if(itemCount[Structure_IX] == 0
                                        && militaryValue > 3000
                                        && (getHouse()->getHouseID() == HOUSE_HARKONNEN
                                            || getHouse()->getHouseID() == HOUSE_MERCENARY)){
                                    if(pBuilder->isAvailableToBuild(Structure_IX)){
                                        itemID = Structure_IX;
                                    }
                                }
*/

                                /*
                                    Next see if we need anything else
                                    If we have a lot of troops get some repair facilities
                                    Try out some palaces...
                                    If we have a lot of money get more heavy factories
                                */


                                else if(itemCount[Structure_HighTechFactory] == 0){
                                    if(pBuilder->isAvailableToBuild(Structure_HighTechFactory)){
                                        itemID = Structure_HighTechFactory;
                                    }
                                }

                                else if(pBuilder->isAvailableToBuild(Structure_RepairYard)
                                        && itemCount[Structure_RepairYard] - 2 <= activeRepairYardCount){
                                    itemID = Structure_RepairYard;
                                    fprintf(stdout,"Build Repair... active: %d  total: %d\n",
                                                    activeRepairYardCount,
                                                    getHouse()->getNumItems(Structure_RepairYard));

                                }

                                else if(pBuilder->isAvailableToBuild(Structure_HeavyFactory)
                                        && (itemCount[Structure_HeavyFactory] - 2 <= activeHeavyFactoryCount
                                            || money > itemCount[Structure_HeavyFactory]*4000)){
                                    itemID = Structure_HeavyFactory;
                                     fprintf(stdout,"Build Factory... active: %d  total: %d\n",
                                                    activeHeavyFactoryCount,
                                                    getHouse()->getNumItems(Structure_HeavyFactory));
                                }


                                else if(itemCount[Structure_Refinery] * 3.5 < itemCount[Unit_Harvester]) {

                                    itemID = Structure_Refinery;


                                }

                                /*
                                    We are running out of spice storage capacity
                                */

                                else if (getHouse()->getStoredCredits() + 2000 > (itemCount[Structure_Refinery] + itemCount[Structure_Silo]) * 1000){

                                    itemID = Structure_Silo;

                                }


                                /*
                                    Here are our luxury items:
                                    - Rocket Turrets
                                    - Palaces
                                    - Construction yard upgrades

                                    Build one rocket turret for every Spice Silo and two for every palace
                                    Otherwise build palaces

                                    Palace and Turret build is dependent on two game options:
                                    - only one palace limits the number of palaces that can be build
                                    - rocket turrets need power turns of the building of them by the ai
                                */

                                else if(money > 2000 || (money > 500 && militaryValue * 2 > militaryValueLimit)){

                                    if(pBuilder->isAvailableToBuild(Structure_Palace)
                                        && (!getGameInitSettings().getGameOptions().onlyOnePalace || itemCount[Structure_Palace] == 0)
                                        && (itemCount[Structure_RocketTurret] >= itemCount[Structure_Palace] * 2 + itemCount[Structure_Silo]
                                            || getGameInitSettings().getGameOptions().rocketTurretsNeedPower)){

                                        itemID = Structure_Palace;
                                    } else if(pBuilder->getCurrentUpgradeLevel() < pBuilder->getMaxUpgradeLevel()){
                                        doUpgrade(pBuilder);
                                    } else{
                                        itemID = Structure_RocketTurret;
                                    }
                                }



                                /*
                                    Build concrete if we have bad building spots
                                */
                                if(pBuilder->isAvailableToBuild(itemID) && findPlaceLocation(itemID).isValid()){
                                    doProduceItem(pBuilder, itemID);
                                    itemCount[itemID]++;
                                }else if(pBuilder->isAvailableToBuild(Structure_Slab1) && findPlaceLocation(Structure_Slab1).isValid()){
                                    doProduceItem(pBuilder, Structure_Slab1);
                                }

                            }
                        }


                        if(pBuilder->isWaitingToPlace()) {
                            Coord location = findPlaceLocation(pBuilder->getCurrentProducedItem());



                            if(location.isValid()){

                                doPlaceStructure(pConstYard, location.x, location.y);

                            } else{
                                doCancelItem(pConstYard, pBuilder->getCurrentProducedItem());
                            }

                        }
                    } break;

                }
            }
        }
    }

    buildTimer = getRandomGen().rand(0,3)*5;
}


void QuantBot::scrambleUnitsAndDefend(const ObjectBase* pDefend) {
    RobustList<const UnitBase*>::const_iterator iter;
    for(iter = getUnitList().begin(); iter != getUnitList().end(); ++iter) {
        const UnitBase* pUnit = *iter;
        if(pUnit->isRespondable() && (pUnit->getOwner() == getHouse())) {

            if(!pUnit->hasATarget() && !pUnit->wasForced()) {
                Uint32 itemID = pUnit->getItemID();
                if((itemID != Unit_Harvester) && (pUnit->getItemID() != Unit_MCV) && (pUnit->getItemID() != Unit_Carryall)
                    && (pUnit->getItemID() != Unit_Frigate) && (pUnit->getItemID() != Unit_Saboteur) && (pUnit->getItemID() != Unit_Sandworm)) {
                    doSetAttackMode(pUnit, AREAGUARD);
                    doMove2Pos(pUnit, pDefend->getX(), pDefend->getY(), true);

                    if(pUnit->isVisible()
                        && blockDistance(pUnit->getLocation(), pUnit->getDestination()) >= 10.0f
                        && pUnit->isAGroundUnit()
                        && pUnit->getHealthColor() == COLOR_LIGHTGREEN) {

                        const GroundUnit* pGroundUnit = dynamic_cast<const GroundUnit*>(pUnit);

                        if(getGameInitSettings().getGameOptions().manualCarryallDrops
                           && pGroundUnit->getItemID() != Unit_Deviator
                           && pGroundUnit->getItemID() != Unit_Launcher){
                            doRequestCarryallDrop(pGroundUnit); //do request carryall to defend unit
                        }

                    }


                }
            }
        }
    }
}


void QuantBot::attack() {




    float strength = ((float)militaryValue + 1) / ((float)militaryValueLimit) + 0.03;

    //float newAttack = 5000 / strength;

    int newAttack = 50000;

    attackTimer = MILLI2CYCLES(newAttack);




    // only attack if we have assault vehicles
    if((militaryValue < militaryValueLimit * 0.33)){

        return;
    }

    // In campaign mode don't attack if  the attack trigger isn't set
    // And don't attack with less than 40% of your limit
    if((!campaignAIAttackFlag || militaryValue < militaryValueLimit * 0.80) && gameMode == CAMPAIGN){

        return;
    }


    fprintf(stdout,"Attack: house: %d  dif: %d  mStr: %d  mLim: %d  strength: %f  newTimer: %f  attackTimer: %d\n",
            getHouse()->getHouseID(),
            difficulty,
            militaryValue,
            militaryValueLimit,
            strength,
            newAttack,
            attackTimer);

    Coord squadCenterLocation = findSquadCenter(getHouse()->getHouseID());

    const UnitBase* pLeaderUnit = NULL;
    RobustList<const UnitBase*>::const_iterator iter;



    for(iter = getUnitList().begin(); iter != getUnitList().end(); ++iter) {
        const UnitBase *pUnit = *iter;
        if (pUnit->isRespondable()
            && (pUnit->getOwner() == getHouse())
            && pUnit->isActive()
            && (pUnit->getAttackMode() == AREAGUARD)
            && pUnit->getItemID() != Unit_Harvester
            && pUnit->getItemID() != Unit_MCV
            && pUnit->getItemID() != Unit_Carryall
            && (pUnit->getItemID() != Unit_Ornithopter || getHouse()->getNumItems(Unit_Ornithopter) > 20)
            && pUnit->getHealthColor() == COLOR_LIGHTGREEN
            && (pUnit->getHealth() * 100) / pUnit->getMaxHealth() > 60

            /**

                Only units within the squad should hunt, safety in numbers

            **/
            && blockDistance(pUnit->getLocation(), squadCenterLocation) < sqrt(getHouse()->getNumUnits()
                                                                                - getHouse()->getNumItems(Unit_Harvester)
                                                                                - getHouse()->getNumItems(Unit_Carryall)
                                                                                - getHouse()->getNumItems(Unit_Ornithopter)
                                                                                - getHouse()->getNumItems(Unit_Sandworm)
                                                                                - getHouse()->getNumItems(Unit_MCV)) + 6)

        {

            doSetAttackMode(pUnit, HUNT);
        }
    }

}


Coord QuantBot::findSquadRallyLocation(){
    int buildingCount = 0;
    int totalX = 0;
    int totalY = 0;

    int enemyBuildingCount = 0;
    int enemyTotalX = 0;
    int enemyTotalY = 0;


    RobustList<const StructureBase*>::const_iterator currentStructure;

    for(currentStructure = getStructureList().begin(); currentStructure != getStructureList().end(); ++currentStructure) {
        const StructureBase* pCurrentStructure = *currentStructure;


            if(pCurrentStructure->getOwner()->getHouseID() == getHouse()->getHouseID()){

                // Lets find the center of mass of our squad
                buildingCount ++;
                totalX += pCurrentStructure->getX();
                totalY += pCurrentStructure->getY();
            } else if(pCurrentStructure->getOwner()->getTeam() != getHouse()->getTeam()){

                enemyBuildingCount ++;
                enemyTotalX += pCurrentStructure->getX();
                enemyTotalY += pCurrentStructure->getY();
            }
    }

    Coord baseCentreLocation = Coord::Invalid();

    if(enemyBuildingCount > 0 && buildingCount > 0){
        baseCentreLocation.x = (totalX / buildingCount) * 0.70 + (enemyTotalX / enemyBuildingCount) * 0.30;
        baseCentreLocation.y = (totalY / buildingCount) * 0.70 + (enemyTotalY / enemyBuildingCount) * 0.30;

    }

    fprintf(stdout, "Squad rally location: %d, %d \n", baseCentreLocation.x , baseCentreLocation.y );

    return baseCentreLocation;
}

Coord QuantBot::findBaseCentre(int houseID){
    int buildingCount = 0;
    int totalX = 0;
    int totalY = 0;


    RobustList<const StructureBase*>::const_iterator currentStructure;

    for(currentStructure = getStructureList().begin(); currentStructure != getStructureList().end(); ++currentStructure) {
        const StructureBase* pCurrentStructure = *currentStructure;


            if(pCurrentStructure->getOwner()->getHouseID() == houseID){

                // Lets find the center of mass of our squad
                buildingCount ++;
                totalX += pCurrentStructure->getX();
                totalY += pCurrentStructure->getY();
            }
    }

    Coord baseCentreLocation = Coord::Invalid();

    if(buildingCount > 0){
        baseCentreLocation.x = totalX / buildingCount;
        baseCentreLocation.y = totalY / buildingCount;
    }


    return baseCentreLocation;
}





Coord QuantBot::findSquadCenter(int houseID){
    int squadSize = 0;

    int totalX = 0;
    int totalY = 0;

    float closestDistance = 100000000;

    RobustList<const UnitBase*>::const_iterator currentUnit;

    for(currentUnit = getUnitList().begin(); currentUnit != getUnitList().end(); ++currentUnit) {
        const UnitBase* pCurrentUnit = *currentUnit;

            if(pCurrentUnit->getOwner()->getHouseID()  == houseID
                && pCurrentUnit->getItemID() != Unit_Carryall
                && pCurrentUnit->getItemID() != Unit_Harvester
                && pCurrentUnit->getItemID() != Unit_Frigate
                && pCurrentUnit->getItemID() != Unit_MCV

                // Stop freeman making tanks roll forward
                && !(currentGame->techLevel > 6 && pCurrentUnit->getItemID() == Unit_Trooper)
                && pCurrentUnit->getItemID() != Unit_Saboteur
                && pCurrentUnit->getItemID() != Unit_Sandworm

                // Don't let troops moving to rally point contribute
                && pCurrentUnit->getAttackMode() != RETREAT
                && pCurrentUnit->getDestination().x != squadRallyLocation.x
                && pCurrentUnit->getDestination().y != squadRallyLocation.y){
                // Lets find the center of mass of our squad

                squadSize ++;
                totalX += pCurrentUnit->getX();
                totalY += pCurrentUnit->getY();
            }

    }

    Coord squadCenterLocation = Coord::Invalid();

    if(squadSize > 0){
        squadCenterLocation.x = totalX / squadSize;
        squadCenterLocation.y = totalY / squadSize;

    }



    return squadCenterLocation;


}

/*
    Set a rally / retreat location for all our military units.
    This should be near our base but within it
    The retreat mode causes all our military units to move
    to this squad rally location

*/

void QuantBot::retreatAllUnits() {

    // Set the new squad rally location
    squadRallyLocation = findSquadRallyLocation();


    // If no base exists yet, there is no retreat location
    if(squadRallyLocation.x != -1){

        RobustList<const UnitBase*>::const_iterator iter;

        for(iter = getUnitList().begin(); iter != getUnitList().end(); ++iter) {
            const UnitBase* pUnit = *iter;

            if(pUnit->getOwner() == getHouse()
               && pUnit->getItemID() != Unit_Carryall
               && pUnit->getItemID() != Unit_Sandworm
               && pUnit->getItemID() != Unit_Harvester
               && pUnit->getItemID() != Unit_MCV
               && pUnit->getItemID() != Unit_Frigate){

                doSetAttackMode(pUnit, RETREAT);
            }
        }
    }
}


/**
    In dune it is best to mass military units in one location.
    This function determines a squad leader by finding the unit with the most central location
    Amongst all of a players units.

    Rocket launchers and Ornithopters are excluded from having this role as on the
    battle field these units should always have other supporting units to work with

**/

void QuantBot::checkAllUnits() {

    squadCenterLocation = findSquadCenter(getHouse()->getHouseID());



    RobustList<const UnitBase*>::const_iterator iter;

    for(iter = getUnitList().begin(); iter != getUnitList().end(); ++iter) {
        const UnitBase* pUnit = *iter;

        if(pUnit->getOwner() == getHouse()){

            switch(pUnit->getItemID()) {
                case Unit_MCV: {

                    const MCV* pMCV = dynamic_cast<const MCV*>(pUnit);

                    fprintf(stdout,"MCV: forced: %d  moving: %d  canDeploy: %d\n",
                    pMCV->wasForced(), pMCV->isMoving(), pMCV->canDeploy());

                    if (pMCV->canDeploy()
                        && !pMCV->wasForced()
                        && !pMCV->isMoving()) {

                        fprintf(stdout,"MCV: Deployed\n");
                        doDeploy(pMCV);
                    } else if(!pMCV->isMoving() && !pMCV->wasForced()){
                        Coord pos = findMcvPlaceLocation(pMCV);
                        doMove2Pos(pMCV, pos.x, pos.y, true);

                        /*
                        if(getHouse()->getNumItems(Unit_Carryall) > 0){
                            doRequestCarryallDrop(pMCV);
                        }*/
                    }

                } break;

                case Unit_Harvester: {
                    const Harvester* pHarvester = dynamic_cast<const Harvester*>(pUnit);
                    if(getHouse()->getCredits() < 1000 && pHarvester->getAmountOfSpice() >= HARVESTERMAXSPICE/2
                       && getHouse()->getNumItems(Structure_HeavyFactory) == 0) {
                        doReturn(pHarvester);


                    }


                } break;

                case Unit_Carryall:{
                } break;

                case Unit_Frigate:{
                } break;

                case Unit_Sandworm:{
                } break;

                case Unit_Saboteur:{
                    if(pUnit->getAttackMode() != HUNT){
                        doSetAttackMode(pUnit, HUNT);
                    }


                    /*
                    if(getHouse()->hasCarryalls()
                       && pUnit->getDestination() != NULL
                       && getGameInitSettings().getGameOptions().manualCarryallDrops){
                        const Saboteur* pSaboteur = dynamic_cast<const Saboteur*>(pUnit);
                        doRequestCarryallDrop(pSaboteur);
                    }*/
                } break;



                default: {

                    int squadRadius = sqrt(getHouse()->getNumUnits()
                                                        - getHouse()->getNumItems(Unit_Harvester)
                                                        - getHouse()->getNumItems(Unit_Carryall)
                                                        - getHouse()->getNumItems(Unit_Ornithopter)
                                                        - getHouse()->getNumItems(Unit_Sandworm)
                                                        - getHouse()->getNumItems(Unit_MCV)) + 2;


                    // Special logic to keep launchers away from harm
                    if(pUnit->getItemID() == Unit_Launcher
                           && pUnit->hasATarget()){

                        if(pUnit->getTarget() != NULL){

                            if(blockDistance(pUnit->getLocation(), pUnit->getTarget()->getLocation()) < 6
                               && pUnit->getTarget()->getItemID() != Unit_Ornithopter){
                                doSetAttackMode(pUnit, AREAGUARD);
                                doMove2Pos(pUnit, squadCenterLocation.x, squadCenterLocation.y, true );

                            }

                        }


                    }

                    else if(pUnit->getAttackMode() != HUNT
                        && !pUnit->hasATarget()
                        && !pUnit->wasForced()){


                        if(pUnit->getAttackMode() == AREAGUARD
                           && squadCenterLocation.isValid()){


                           if(blockDistance(pUnit->getLocation(),
                                         squadCenterLocation) > squadRadius){

                                doMove2Pos(pUnit, squadCenterLocation.x, squadCenterLocation.y, false );

                            }

                        } else if (pUnit->getAttackMode() == RETREAT){

                           if(blockDistance(pUnit->getLocation(),
                                         squadRallyLocation) > squadRadius){


                                doMove2Pos(pUnit, squadRallyLocation.x, squadRallyLocation.y, true );

                            }

                            // We have finished retreating back to the rally point
                            else{
                                doSetAttackMode(pUnit, AREAGUARD);
                            }


                        // A newly deployed unit has reached the rally point, or has been diverted.
                        // Change it to area guard
                        } else if (pUnit->getAttackMode() == GUARD
                                   && ((pUnit->getDestination().x != squadRallyLocation.x
                                        || pUnit->getDestination().y != squadRallyLocation.y)
                                       || (blockDistance(pUnit->getLocation(),
                                         squadRallyLocation) <= squadRadius))){

                            doSetAttackMode(pUnit, AREAGUARD);

                        }


                    }

                    if (pUnit->getAttackMode() == AREAGUARD
                                   && pUnit->getDestination().x == squadRallyLocation.x
                                   && pUnit->getDestination().y == squadRallyLocation.y
                                   && blockDistance(pUnit->getLocation(), squadRallyLocation) > squadRadius){

                            doSetAttackMode(pUnit, GUARD);
                            doMove2Pos(pUnit, squadRallyLocation.x, squadRallyLocation.y, false );

                    }

                    // Placeholder to make launchers run away from ground enemies which get too close




                } break;
            }
        }
    }
}

