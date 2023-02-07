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

CommandManager::CommandManager() = default;

CommandManager::~CommandManager() = default;

void CommandManager::addCommand(const Command& cmd) {
    auto CycleNumber = dune::globals::currentGame->getGameCycleCount();

    if (dune::globals::pNetworkManager != nullptr) {
        CycleNumber += networkCycleBuffer;
    }
    addCommand(cmd, CycleNumber);
}

void CommandManager::addCommand(Command&& cmd) {
    auto CycleNumber = dune::globals::currentGame->getGameCycleCount();

    if (dune::globals::pNetworkManager != nullptr) {
        CycleNumber += networkCycleBuffer;
    }
    addCommand(std::move(cmd), CycleNumber);
}

void CommandManager::save(OutputStream& stream) const {
    for (auto i = 0u; i < timeslot.size(); i++) {
        for (const auto& command : timeslot[i]) {
            stream.writeUint32(i);
            command.save(stream);
        }
    }
}

void CommandManager::load(InputStream& stream) {
    try {
        while (stream.bytesLeft() > 0) {
            const auto cycle = stream.readUint32();
            addCommand(Command{stream}, cycle);
        }
    } catch (InputStream::exception& e) {
        sdl2::log_info("Warning: Unexpected input stream exception in CommandManager::load: {}", e.what());
    }
}

void CommandManager::update() {
    auto* const network_manager = dune::globals::pNetworkManager.get();
    const auto* const game      = dune::globals::currentGame.get();

    if (network_manager == nullptr)
        return;

    CommandList commandList;

    for (uint32_t i = std::max(static_cast<int>(game->getGameCycleCount()) - MILLI2CYCLES(2500), 0);
         i < game->getGameCycleCount() + networkCycleBuffer; i++) {

        std::vector<Command> commands;

        if (i < timeslot.size()) {
            for (auto& command : timeslot[i]) {
                if (command.getPlayerID() == dune::globals::pLocalPlayer->getPlayerID()) {
                    commands.push_back(command);
                }
            }
        }

        commandList.commandList.emplace_back(i, std::move(commands));
    }

    network_manager->sendCommandList(commandList);
}

void CommandManager::addCommandList(const std::string& playername, const CommandList& commandList) {
    auto* const pPlayer = dynamic_cast<HumanPlayer*>(dune::globals::currentGame->getPlayerByName(playername));
    if (pPlayer == nullptr) {
        return;
    }

    for (const auto& commandListEntry : commandList.commandList) {
        if (pPlayer->nextExpectedCommandsCycle > commandListEntry.cycle) {
            continue;
        }

        for (const auto& command : commandListEntry.commands) {
            if (command.getPlayerID() != pPlayer->getPlayerID()) {
                sdl2::log_info("Warning: Player '{}' send a command which he is not allowed to give!", playername);
            }

            addCommand(command, commandListEntry.cycle);
        }

        pPlayer->nextExpectedCommandsCycle = std::max(pPlayer->nextExpectedCommandsCycle, commandListEntry.cycle + 1);
    }
}

void CommandManager::addCommand(const Command& cmd, uint32_t CycleNumber) {
    if (bReadOnly)
        return;

    if (CycleNumber >= timeslot.size()) {
        timeslot.resize(CycleNumber + 1);
    }

    timeslot[CycleNumber].push_back(cmd);
    std::ranges::stable_sort(timeslot[CycleNumber], [](const Command& cmd1, const Command& cmd2) {
        return (cmd1.getPlayerID() < cmd2.getPlayerID());
    });

    if (pStream != nullptr) {
        pStream->writeUint32(CycleNumber);
        cmd.save(*pStream);
    }
}

void CommandManager::addCommand(Command&& cmd, uint32_t CycleNumber) {
    if (bReadOnly)
        return;

    if (CycleNumber >= timeslot.size()) {
        timeslot.resize(CycleNumber + 1);
    }

    if (pStream != nullptr) {
        pStream->writeUint32(CycleNumber);
        cmd.save(*pStream);
    }

    timeslot[CycleNumber].push_back(std::move(cmd));
    std::ranges::stable_sort(timeslot[CycleNumber], [](const Command& cmd1, const Command& cmd2) {
        return (cmd1.getPlayerID() < cmd2.getPlayerID());
    });
}

void CommandManager::executeCommands(const GameContext& context, uint32_t CycleNumber) const {
    if (CycleNumber >= timeslot.size()) {
        return;
    }

    for (const Command& command : timeslot[CycleNumber]) {
        command.executeCommand(context);
    }
}
