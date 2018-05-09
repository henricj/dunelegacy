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


CommandManager::CommandManager() {
    pStream = nullptr;
    bReadOnly = false;
    networkCycleBuffer = 0;
}

CommandManager::~CommandManager() = default;

void CommandManager::addCommand(const Command& cmd) {
    Uint32 CycleNumber = currentGame->getGameCycleCount();

    if(pNetworkManager != nullptr) {
        CycleNumber += networkCycleBuffer;
    }
    addCommand(cmd, CycleNumber);
}

void CommandManager::save(OutputStream& stream) const {
    for(unsigned int i=0;i<timeslot.size();i++) {
        for(const Command& command : timeslot[i]) {
            stream.writeUint32(i);
            command.save(stream);
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
                for(Command& command : timeslot[i]) {
                    if(command.getPlayerID() == pLocalPlayer->getPlayerID()) {
                        commands.push_back(command);
                    }
                }
            }

            commandList.commandList.emplace_back(i, commands);
        }

        pNetworkManager->sendCommandList(commandList);
    }
}

void CommandManager::addCommandList(const std::string& playername, const CommandList& commandList) {
    HumanPlayer* pPlayer = dynamic_cast<HumanPlayer*>(currentGame->getPlayerByName(playername));
    if(pPlayer == nullptr) {
        return;
    }

    for(const CommandList::CommandListEntry& commandListEntry : commandList.commandList) {
        if(pPlayer->nextExpectedCommandsCycle > commandListEntry.cycle) {
            continue;
        }

        for(const Command& command : commandListEntry.commands) {
            if(command.getPlayerID() != pPlayer->getPlayerID()) {
                SDL_Log("Warning: Player '%s' send a command which he is not allowed to give!", playername.c_str());
            }

            addCommand(command, commandListEntry.cycle);
        }

        pPlayer->nextExpectedCommandsCycle = std::max(pPlayer->nextExpectedCommandsCycle, commandListEntry.cycle+1);
    }
}

void CommandManager::addCommand(const Command& cmd, Uint32 CycleNumber) {
    if(bReadOnly == false) {

        if(CycleNumber >= timeslot.size()) {
            timeslot.resize(CycleNumber+1);
        }

        timeslot[CycleNumber].push_back(cmd);
        std::stable_sort(   timeslot[CycleNumber].begin(),
                            timeslot[CycleNumber].end(),
                            [](const Command& cmd1, const Command& cmd2) {
                                return (cmd1.getPlayerID() < cmd2.getPlayerID());
                            });

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

    for(const Command& command : timeslot[CycleNumber]) {
        command.executeCommand();
    }
}

