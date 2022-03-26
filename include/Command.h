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

#ifndef COMMAND_H
#define COMMAND_H

#include <misc/InputStream.h>
#include <misc/OutputStream.h>

#include <type_traits>
#include <vector>

class GameContext;

enum class CMDTYPE {
    CMD_NONE,
    CMD_PLACE_STRUCTURE,             ///< PLACE_STRUCTURE(BUILDER_ID, X, Y)
    CMD_UNIT_MOVE2POS,               ///< UNIT_MOVE2POS(OBJECT_ID,X,Y,BFORCED)
    CMD_UNIT_MOVE2OBJECT,            ///< UNIT_MOVE2OBJECT(OBJECT_ID,TARGET_OBJECT_ID)
    CMD_UNIT_ATTACKPOS,              ///< UNIT_ATTACKPOS(OBJECT_ID,X,Y)
    CMD_UNIT_ATTACKOBJECT,           ///< UNIT_ATTACKOBJECT(OBJECT_ID,TARGET_OBJECT_ID)
    CMD_INFANTRY_CAPTURE,            ///< INFANTRY_CAPTURE(OBJECT_ID,TARGET_STRUCTURE_ID)
    CMD_UNIT_REQUESTCARRYALLDROP,    ///< UNIT_REQUESTCARRYALLDROP(OBJECT_ID,X,Y)
    CMD_UNIT_SENDTOREPAIR,           ///< UNIT_SENDTOREPAIR(OBJECT_ID)
    CMD_UNIT_SETMODE,                ///< UNIT_SETMODE(OBJECT_ID,MODE)
    CMD_DEVASTATOR_STARTDEVASTATE,   ///< DEVASTATOR_STARTDEVASTATE(OBJECT_ID)
    CMD_MCV_DEPLOY,                  ///< MCV_DEPLOY(OBJECT_ID)
    CMD_HARVESTER_RETURN,            ///< HARVESTER_RETURN(OBJECT_ID)
    CMD_STRUCTURE_SETDEPLOYPOSITION, ///< STRUCTURE_SETDEPLOYPOSITION(OBJECT_ID,X,Y)
    CMD_STRUCTURE_REPAIR,            ///< STRUCTURE_REPAIR(OBJECT_ID)
    CMD_BUILDER_UPGRADE,             ///< BUILDER_UPGRADE(OBJECT_ID,B_START)
    CMD_BUILDER_PRODUCEITEM,         ///< BUILDER_PRODUCEITEM(OBJECT_ID,ITEM_ID,B_MULTIMODE)
    CMD_BUILDER_CANCELITEM,          ///< BUILDER_PRODUCEITEM(OBJECT_ID,ITEM_ID,B_MULTIMODE)
    CMD_BUILDER_SETONHOLD,           ///< BUILDER_SETONHOLD(OBJECT_ID, B_HOLD)
    CMD_PALACE_SPECIALWEAPON,        ///< PALACE_SPECIALWEAPON(OBJECT_ID)
    CMD_PALACE_DEATHHAND,            ///< PALACE_DEATHHAND(OBJECT_ID, X, Y)
    CMD_STARPORT_PLACEORDER,         ///< CMD_STARPORT_PLACEORDER(OBJECT_ID)
    CMD_STARPORT_CANCELORDER,        ///< CMD_STARPORT_CANCELORDER(OBJECT_ID)
    CMD_TURRET_ATTACKOBJECT,         ///< TURRET_ATTACKOBJECT(OBJECT_ID,TARGET_OBJECT_ID)
    CMD_TEST_SYNC,                   ///< TEST_SYNC(SEED)
    CMD_MAX
};

/**
    This class represents one command with all its parameters. The command is specified by CommandID (see CMDTYPE)
    and Parameter holds all its parameters (see the documentation for every CMDTYPE). There can be up to 4 parameters
*/
class Command final {
public:
    /**
        Construct a command with CMDTYPE id and no parameter.
        \param  id  the id of the command
    */
    Command(uint8_t playerID, CMDTYPE id) : playerID {playerID}, commandID {id} { }

    template<typename... Parameters, typename = std::enable_if_t<(std::is_convertible_v<Parameters, uint32_t> && ...)>>
    Command(uint8_t playerID, CMDTYPE id, Parameters&&... parameters) : playerID {playerID}, commandID {id} {
        // Pilfered from https://stackoverflow.com/a/39659128
        parameter.reserve(sizeof...(parameters));
        (parameter.push_back(parameters), ...);
    }

    /**
        Construct a command from raw memory.
        \param  data        pointer to the data
        \param  length      length of the data
    */
    Command(uint8_t playerID, uint8_t* data, uint32_t length);

    /**
        Read a command from stream.
        \param  stream  the stream to read from
    */
    explicit Command(InputStream& stream);

    Command(const Command&) = default;
    Command(Command&&)      = default;

    Command& operator=(const Command&) = default;
    Command& operator=(Command&&) = default;

    /// destructor
    ~Command();

    /**
        Writes the command to a stream.
        \param  stream  the stream to write to
    */
    void save(OutputStream& stream) const;

    /**
        Gets the ID of the player that added this command.
        \return the ID of the player
    */
    [[nodiscard]] uint8_t getPlayerID() const noexcept { return playerID; }

    /**
        Gets the ID of this command.
        \return the ID of this command
    */
    [[nodiscard]] CMDTYPE getCommandID() const noexcept { return commandID; }

    /**
        Executes this command. This takes the appropriate actions to run this command.
    */
    void executeCommand(const GameContext& context) const;

private:
    uint8_t playerID;                ///< the ID of the player that gave the command
    CMDTYPE commandID;               ///< the type of command
    std::vector<uint32_t> parameter; ///< the parameters for this command
};

#endif // COMMAND_H
