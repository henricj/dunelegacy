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

#include <Command.h>

#include <globals.h>

#include <Game.h>
#include <House.h>

#include <misc/exceptions.h>

#include <units/UnitBase.h>
#include <units/GroundUnit.h>
#include <units/Carryall.h>
#include <units/Devastator.h>
#include <units/MCV.h>
#include <units/Harvester.h>
#include <units/InfantryBase.h>
#include <structures/BuilderBase.h>
#include <structures/TurretBase.h>
#include <structures/Palace.h>
#include <structures/StarPort.h>
#include <structures/ConstructionYard.h>

Command::Command(Uint8 playerID, CMDTYPE id)
 : playerID(playerID), commandID(id)
{
}

Command::Command(Uint8 playerID, CMDTYPE id, Uint32 parameter1)
 : playerID(playerID), commandID(id)
{
    parameter.push_back(parameter1);
}

Command::Command(Uint8 playerID, CMDTYPE id, Uint32 parameter1, Uint32 parameter2)
 : playerID(playerID), commandID(id)
{
    parameter.push_back(parameter1);
    parameter.push_back(parameter2);
}

Command::Command(Uint8 playerID, CMDTYPE id, Uint32 parameter1, Uint32 parameter2, Uint32 parameter3)
 : playerID(playerID), commandID(id)
{
    parameter.push_back(parameter1);
    parameter.push_back(parameter2);
    parameter.push_back(parameter3);
}

Command::Command(Uint8 playerID, CMDTYPE id, Uint32 parameter1, Uint32 parameter2, Uint32 parameter3, Uint32 parameter4)
 : playerID(playerID), commandID(id)
{
    parameter.push_back(parameter1);
    parameter.push_back(parameter2);
    parameter.push_back(parameter3);
    parameter.push_back(parameter4);
}

Command::Command(Uint8 playerID, Uint8* data, Uint32 length)
 : playerID(playerID)
{
    if(length % 4 != 0) {
        THROW(std::invalid_argument, "Command::Command(): Length must be multiple of 4!");
    }

    if(length < 4) {
        THROW(std::invalid_argument, "Command::Command(): Command must be at least 4 bytes long!");
    }

    commandID = (CMDTYPE) *((Uint32*) data);

    if(commandID >= CMD_MAX) {
        THROW(std::invalid_argument, "Command::Command(): CommandID unknown!");
    }

    Uint32* pData = (Uint32*) (data+4);
    for(Uint32 i=0;i<(length-4)/4;i++) {
        parameter.push_back(*pData);
        pData++;
    }
}

Command::Command(InputStream& stream) {
    playerID = stream.readUint8();
    commandID = (CMDTYPE) stream.readUint32();
    parameter = stream.readUint32Vector();
}

Command::~Command() = default;

void Command::save(OutputStream& stream) const {
    stream.writeUint8(playerID);
    stream.writeUint32((Uint32) commandID);
    stream.writeUint32Vector(parameter);
    stream.flush();
}

void Command::executeCommand() const {
    switch(commandID) {

        case CMD_PLACE_STRUCTURE: {
            if(parameter.size() != 3) {
                THROW(std::invalid_argument, "Command::executeCommand(): CMD_PLACE_STRUCTURE needs 3 Parameters!");
            }
            ConstructionYard* pConstYard = dynamic_cast<ConstructionYard*>(currentGame->getObjectManager().getObject(parameter[0]));
            if(pConstYard == nullptr) {
                return;
            }
            pConstYard->doPlaceStructure((int) parameter[1], (int) parameter[2]);
        } break;


        case CMD_UNIT_MOVE2POS: {
            if(parameter.size() != 4) {
                THROW(std::invalid_argument, "Command::executeCommand(): CMD_UNIT_MOVE2POS needs 4 Parameters!");
            }
            UnitBase* unit = dynamic_cast<UnitBase*>(currentGame->getObjectManager().getObject(parameter[0]));
            if(unit == nullptr) {
                return;
            }
            unit->doMove2Pos((int) parameter[1], (int) parameter[2], (bool) parameter[3]);
        } break;

        case CMD_UNIT_MOVE2OBJECT: {
            if(parameter.size() != 2) {
                THROW(std::invalid_argument, "Command::executeCommand(): CMD_UNIT_MOVE2OBJECT needs 2 Parameters!");
            }
            UnitBase* unit = dynamic_cast<UnitBase*>(currentGame->getObjectManager().getObject(parameter[0]));
            if(unit == nullptr) {
                return;
            }
            unit->doMove2Object((int) parameter[1]);
        } break;

        case CMD_UNIT_ATTACKPOS: {
            if(parameter.size() != 4) {
                THROW(std::invalid_argument, "Command::executeCommand(): CMD_UNIT_ATTACKPOS needs 4 Parameters!");
            }
            UnitBase* unit = dynamic_cast<UnitBase*>(currentGame->getObjectManager().getObject(parameter[0]));
            if(unit == nullptr) {
                return;
            }
            unit->doAttackPos((int) parameter[1], (int) parameter[2], (bool) parameter[3]);
        } break;

        case CMD_UNIT_ATTACKOBJECT: {
            if(parameter.size() != 2) {
                THROW(std::invalid_argument, "Command::executeCommand(): CMD_UNIT_ATTACKOBJECT needs 2 Parameters!");
            }
            UnitBase* pUnit = dynamic_cast<UnitBase*>(currentGame->getObjectManager().getObject(parameter[0]));
            if(pUnit == nullptr) {
                return;
            }
            pUnit->doAttackObject((int) parameter[1], true);
        } break;

        case CMD_INFANTRY_CAPTURE: {
            if(parameter.size() != 2) {
                THROW(std::invalid_argument, "Command::executeCommand(): CMD_INFANTRY_CAPTURE needs 2 Parameters!");
            }
            InfantryBase* pInfantry = dynamic_cast<InfantryBase*>(currentGame->getObjectManager().getObject(parameter[0]));
            if(pInfantry == nullptr) {
                return;
            }
            pInfantry->doCaptureStructure((int) parameter[1]);
        } break;

        case CMD_UNIT_REQUESTCARRYALLDROP: {
            if(parameter.size() != 3) {
                THROW(std::invalid_argument, "Command::executeCommand(): CMD_UNIT_REQUESTCARRYALLDROP needs 3 Parameters!");
            }
            GroundUnit* pGroundUnit = dynamic_cast<GroundUnit*>(currentGame->getObjectManager().getObject(parameter[0]));
            if(pGroundUnit == nullptr) {
                return;
            }
            pGroundUnit->doRequestCarryallDrop((int) parameter[1], (int) parameter[2]);
        } break;

        case CMD_UNIT_SENDTOREPAIR: {
            if(parameter.size() != 1) {
                THROW(std::invalid_argument, "Command::executeCommand(): CMD_UNIT_SENDTOREPAIR needs 1 Parameter!");
            }
            GroundUnit* pGroundUnit = dynamic_cast<GroundUnit*>(currentGame->getObjectManager().getObject(parameter[0]));
            if(pGroundUnit == nullptr) {
                return;
            }
            pGroundUnit->doRepair();
        } break;

        case CMD_UNIT_SETMODE: {
            if(parameter.size() != 2) {
                THROW(std::invalid_argument, "Command::executeCommand(): CMD_UNIT_SETMODE needs 2 Parameter!");
            }
            UnitBase* pUnit = dynamic_cast<UnitBase*>(currentGame->getObjectManager().getObject(parameter[0]));
            if(pUnit == nullptr) {
                return;
            }
            pUnit->doSetAttackMode((ATTACKMODE) parameter[1]);
        } break;

        case CMD_DEVASTATOR_STARTDEVASTATE: {
            if(parameter.size() != 1) {
                THROW(std::invalid_argument, "Command::executeCommand(): CMD_DEVASTATOR_STARTDEVASTATE needs 1 Parameter!");
            }
            Devastator* pDevastator = dynamic_cast<Devastator*>(currentGame->getObjectManager().getObject(parameter[0]));
            if(pDevastator == nullptr) {
                return;
            }
            pDevastator->doStartDevastate();
        } break;

        case CMD_MCV_DEPLOY: {
            if(parameter.size() != 1) {
                THROW(std::invalid_argument, "Command::executeCommand(): CMD_MCV_DEPLOY needs 1 Parameter!");
            }
            MCV* pMCV = dynamic_cast<MCV*>(currentGame->getObjectManager().getObject(parameter[0]));
            if(pMCV == nullptr) {
                return;
            }
            pMCV->doDeploy();
        } break;

        case CMD_HARVESTER_RETURN: {
            if(parameter.size() != 1) {
                THROW(std::invalid_argument, "Command::executeCommand(): CMD_HARVESTER_RETURN needs 1 Parameter!");
            }
            Harvester* pHarvester = dynamic_cast<Harvester*>(currentGame->getObjectManager().getObject(parameter[0]));
            if(pHarvester == nullptr) {
                return;
            }
            pHarvester->doReturn();
        } break;

        case CMD_STRUCTURE_SETDEPLOYPOSITION: {
            if(parameter.size() != 3) {
                THROW(std::invalid_argument, "Command::executeCommand(): CMD_STRUCTURE_SETDEPLOYPOSITION needs 3 Parameters!");
            }
            StructureBase* pStructure = dynamic_cast<StructureBase*>(currentGame->getObjectManager().getObject(parameter[0]));
            if(pStructure == nullptr) {
                return;
            }
            pStructure->doSetDeployPosition((int) parameter[1],(int) parameter[2]);
        } break;

        case CMD_STRUCTURE_REPAIR: {
            if(parameter.size() != 1) {
                THROW(std::invalid_argument, "Command::executeCommand(): CMD_STRUCTURE_REPAIR needs 1 Parameter!");
            }
            StructureBase* pStructure = dynamic_cast<StructureBase*>(currentGame->getObjectManager().getObject(parameter[0]));
            if(pStructure == nullptr) {
                return;
            }
            pStructure->doRepair();
        } break;

        case CMD_BUILDER_UPGRADE: {
            if(parameter.size() != 1) {
                THROW(std::invalid_argument, "Command::executeCommand(): CMD_BUILDER_UPGRADE needs 1 Parameter!");
            }
            BuilderBase* pBuilder = dynamic_cast<BuilderBase*>(currentGame->getObjectManager().getObject(parameter[0]));
            if(pBuilder == nullptr) {
                return;
            }
            pBuilder->doUpgrade();
        } break;

        case CMD_BUILDER_PRODUCEITEM: {
            if(parameter.size() != 3) {
                THROW(std::invalid_argument, "Command::executeCommand(): CMD_BUILDER_PRODUCEITEM needs 3 Parameter!");
            }
            BuilderBase* pBuilder = dynamic_cast<BuilderBase*>(currentGame->getObjectManager().getObject(parameter[0]));
            if(pBuilder == nullptr) {
                return;
            }
            pBuilder->doProduceItem(parameter[1],(bool) parameter[2]);
        } break;

        case CMD_BUILDER_CANCELITEM: {
            if(parameter.size() != 3) {
                THROW(std::invalid_argument, "Command::executeCommand(): CMD_BUILDER_CANCELITEM needs 3 Parameter!");
            }
            BuilderBase* pBuilder = dynamic_cast<BuilderBase*>(currentGame->getObjectManager().getObject(parameter[0]));
            if(pBuilder == nullptr) {
                return;
            }
            pBuilder->doCancelItem(parameter[1],(bool) parameter[2]);
        } break;

        case CMD_BUILDER_SETONHOLD: {
            if(parameter.size() != 2) {
                THROW(std::invalid_argument, "Command::executeCommand(): CMD_BUILDER_SETONHOLD needs 2 Parameters!");
            }
            BuilderBase* pBuilder = dynamic_cast<BuilderBase*>(currentGame->getObjectManager().getObject(parameter[0]));
            if(pBuilder == nullptr) {
                return;
            }
            pBuilder->doSetOnHold((bool) parameter[1]);
        } break;

        case CMD_PALACE_SPECIALWEAPON: {
            if(parameter.size() != 1) {
                THROW(std::invalid_argument, "Command::executeCommand(): CMD_PALACE_SPECIALWEAPON needs 1 Parameter!");
            }
            Palace* palace = dynamic_cast<Palace*>(currentGame->getObjectManager().getObject(parameter[0]));
            if(palace == nullptr) {
                return;
            }
            palace->doSpecialWeapon();
        } break;

        case CMD_PALACE_DEATHHAND: {
            if(parameter.size() != 3) {
                THROW(std::invalid_argument, "Command::executeCommand(): CMD_PALACE_DEATHHAND needs 3 Parameter!");
            }
            Palace* palace = dynamic_cast<Palace*>(currentGame->getObjectManager().getObject(parameter[0]));
            if(palace == nullptr) {
                return;
            }
            palace->doLaunchDeathhand((int) parameter[1], (int) parameter[2]);
        } break;

        case CMD_STARPORT_PLACEORDER: {
            if(parameter.size() != 1) {
                THROW(std::invalid_argument, "Command::executeCommand(): CMD_STARPORT_PLACEORDER needs 1 Parameter!");
            }
            StarPort* pStarport = dynamic_cast<StarPort*>(currentGame->getObjectManager().getObject(parameter[0]));
            if(pStarport == nullptr) {
                return;
            }
            pStarport->doPlaceOrder();
        } break;

        case CMD_STARPORT_CANCELORDER: {
            if(parameter.size() != 1) {
                THROW(std::invalid_argument, "Command::executeCommand(): CMD_STARPORT_CANCELORDER needs 1 Parameter!");
            }
            StarPort* pStarport = dynamic_cast<StarPort*>(currentGame->getObjectManager().getObject(parameter[0]));
            if(pStarport == nullptr) {
                return;
            }
            pStarport->doCancelOrder();
        } break;

        case CMD_TURRET_ATTACKOBJECT: {
            if(parameter.size() != 2) {
                THROW(std::invalid_argument, "Command::executeCommand(): CMD_TURRET_ATTACKOBJECT needs 2 Parameters!");
            }
            TurretBase* pTurret = dynamic_cast<TurretBase*>(currentGame->getObjectManager().getObject(parameter[0]));
            if(pTurret == nullptr) {
                return;
            }
            pTurret->doAttackObject((int) parameter[1]);
        } break;

        case CMD_TEST_SYNC: {
            if(parameter.size() != 1) {
                THROW(std::invalid_argument, "Command::executeCommand(): CMD_TEST_SYNC needs 1 Parameters!");
            }

            Uint32 currentSeed = currentGame->randomGen.getSeed();
            if(currentSeed != parameter[0]) {
                SDL_Log("Warning: Game is asynchronous in game cycle %d! Saved seed and current seed do not match: %ud != %ud", currentGame->getGameCycleCount(), parameter[0], currentSeed);
#ifdef TEST_SYNC
                currentGame->saveGame("test.sav");
                exit(0);
#endif
            }
        } break;

        default: {
            THROW(std::invalid_argument, "Command::executeCommand(): Unknown CommandID!");
        } break;
    }

}

