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

#include <CommandManager.h>

#include <Network/NetworkManager.h>
#include <players/HumanPlayer.h>

#include <globals.h>

#include <Game.h>

#include <algorithm>

static bool compareCommands(Command cmd1, Command cmd2) {
    return (cmd1.getPlayerID() < cmd2.getPlayerID());
}

CommandManager::CommandManager() {
    pStream = nullptr;
    bReadOnly = false;
    networkCycleBuffer = 0;
}

CommandManager::~CommandManager() {
    delete pStream;
    pStream = nullptr;
}

void CommandManager::addCommand(Command cmd) {
    Uint32 CycleNumber = currentGame->getGameCycleCount();

    if(pNetworkManager != nullptr) {
        CycleNumber += networkCycleBuffer;
    }
    addCommand(cmd, CycleNumber);
}

void CommandManager::save(OutputStream& stream) const {
    for(unsigned int i=0;i<timeslot.size();i++) {
        const std::vector<Command>& cmdlist = timeslot[i];
        std::vector<Command>::const_iterator iter;

        for(iter = cmdlist.begin(); iter != cmdlist.end(); ++iter) {
            stream.writeUint32(i);
            iter->save(stream);
        }
    }
}

void CommandManager::load(InputStream& stream) {
    try {
        while(1) {
            Uint32 cycle = stream.readUint32();
            addCommand(Command(stream), cycle);
        }
    } catch (InputStream::exception&) {
        ;
    }
}

void CommandManager::update() {
    if(pNetworkManager != nullptr) {
        CommandList commandList;
        for(Uint32 i = std::max((int) currentGame->getGameCycleCount() - MILLI2CYCLES(2500), 0); i < currentGame->getGameCycleCount() + networkCycleBuffer; i++) {
            std::vector<Command> commands;

            if(i < timeslot.size()) {
                std::vector<Command>::const_iterator iter;
                for(iter = timeslot[i].begin(); iter != timeslot[i].end(); ++iter) {
                    if(iter->getPlayerID() == pLocalPlayer->getPlayerID()) {
                        commands.push_back(*iter);
                    }
                }
            }

            commandList.commandList.push_back(CommandList::CommandListEntry(i, commands));
        }

        pNetworkManager->sendCommandList(commandList);
    }
}

void CommandManager::addCommandList(const std::string& playername, const CommandList& commandList) {
    HumanPlayer* pPlayer = dynamic_cast<HumanPlayer*>(currentGame->getPlayerByName(playername));
    if(pPlayer == nullptr) {
        return;
    }

    std::vector<CommandList::CommandListEntry>::const_iterator iter;
    for(iter = commandList.commandList.begin(); iter != commandList.commandList.end(); ++iter) {
        if(pPlayer->nextExpectedCommandsCycle > iter->cycle) {
            continue;
        }

        std::vector<Command>::const_iterator iter2;
        for(iter2 = iter->commands.begin(); iter2 != iter->commands.end(); ++iter2) {
            if(iter2->getPlayerID() != pPlayer->getPlayerID()) {
                fprintf(stderr, "Player '%s' send a command which he is not allowed to give!\n", playername.c_str());
            }

            addCommand(*iter2, iter->cycle);
        }

        pPlayer->nextExpectedCommandsCycle = std::max(pPlayer->nextExpectedCommandsCycle, iter->cycle+1);
    }
}

void CommandManager::addCommand(Command cmd, Uint32 CycleNumber) {
    if(bReadOnly == false) {

        if(CycleNumber >= timeslot.size()) {
            timeslot.resize(CycleNumber+1);
        }

        timeslot[CycleNumber].push_back(cmd);
        std::stable_sort(timeslot[CycleNumber].begin(), timeslot[CycleNumber].end(), compareCommands);

        if(pStream != nullptr) {
            pStream->writeUint32(CycleNumber);
            cmd.save(*pStream);
        }
    }
}

void CommandManager::executeCommands(Uint32 CycleNumber) const {
    if(CycleNumber >= timeslot.size()) {
        return;
    }

    const std::vector<Command>& cmdlist = timeslot[CycleNumber];
    std::vector<Command>::const_iterator iter;

    for(iter = cmdlist.begin(); iter != cmdlist.end(); ++iter) {

        /*
        fprintf(stderr, "Executing Command (GameCycle %d): PlayerID=%d, Cmd=%d, Params=", CycleNumber, iter->getPlayerID(), iter->getCommandID());
        std::vector<Uint32> params = iter->getParameter();
        std::vector<Uint32>::const_iterator paramiter;
        for(paramiter = params.begin(); paramiter != params.end(); ++paramiter) {
            fprintf(stderr, "%d ", *paramiter);
        }
        fprintf(stderr,"\n");
        */

        iter->executeCommand();
    }
}

