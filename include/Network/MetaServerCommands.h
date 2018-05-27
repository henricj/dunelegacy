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

#ifndef METASERVERCOMMANDS_H
#define METASERVERCOMMANDS_H

#include <misc/SDL2pp.h>
#include <string>


#define METASERVERCOMMAND_ADD       1
#define METASERVERCOMMAND_UPDATE    2
#define METASERVERCOMMAND_REMOVE    3
#define METASERVERCOMMAND_LIST      4
#define METASERVERCOMMAND_EXIT      5



class MetaServerCommand {
public:
    explicit MetaServerCommand(int type) : type(type) {
    }

    virtual ~MetaServerCommand() = default;

    virtual bool operator==(const MetaServerCommand& metaServerCommand) const {
        return (type == metaServerCommand.type);
    }

    int type;
};

class MetaServerAdd : public MetaServerCommand {
public:
    MetaServerAdd(const std::string& serverName, int serverPort, const std::string& secret, const std::string& mapName, Uint8 numPlayers, Uint8 maxPlayers)
     : MetaServerCommand(METASERVERCOMMAND_ADD), serverName(serverName), serverPort(serverPort), secret(secret),
       mapName(mapName), numPlayers(numPlayers), maxPlayers(maxPlayers) {
    }

    bool operator==(const MetaServerCommand& metaServerCommand) const override
    {
        const MetaServerAdd* pMetaServerAdd = dynamic_cast<const MetaServerAdd*>(&metaServerCommand);
        if(pMetaServerAdd == nullptr) {
            return false;
        } else {

            return ((serverName == pMetaServerAdd->serverName)
                     && (serverPort == pMetaServerAdd->serverPort)
                     && (secret == pMetaServerAdd->secret)
                     && (mapName == pMetaServerAdd->mapName)
                     && (numPlayers == pMetaServerAdd->numPlayers)
                     && (maxPlayers == pMetaServerAdd->maxPlayers));
        }
    }

    std::string serverName;
    int serverPort;
    std::string secret;
    std::string mapName;
    Uint8 numPlayers;
    Uint8 maxPlayers;
};

class MetaServerUpdate : public MetaServerCommand {
public:
    MetaServerUpdate(const std::string& serverName, int serverPort, const std::string& secret, const std::string& mapName, Uint8 numPlayers, Uint8 maxPlayers)
     : MetaServerCommand(METASERVERCOMMAND_UPDATE), serverName(serverName), serverPort(serverPort), secret(secret),
       mapName(mapName), numPlayers(numPlayers), maxPlayers(maxPlayers) {
    }

    bool operator==(const MetaServerCommand& metaServerCommand) const override
    {
        const MetaServerUpdate* pMetaServerUpdate = dynamic_cast<const MetaServerUpdate*>(&metaServerCommand);
        if(pMetaServerUpdate == nullptr) {
            return false;
        } else {

            return ((serverName == pMetaServerUpdate->serverName)
                     && (serverPort == pMetaServerUpdate->serverPort)
                     && (secret == pMetaServerUpdate->secret)
                     && (mapName == pMetaServerUpdate->mapName)
                     && (numPlayers == pMetaServerUpdate->numPlayers)
                     && (maxPlayers == pMetaServerUpdate->maxPlayers));
        }
    }

    std::string serverName;
    int serverPort;
    std::string secret;
    std::string mapName;
    Uint8 numPlayers;
    Uint8 maxPlayers;
};

class MetaServerRemove : public MetaServerCommand {
public:
    MetaServerRemove(int serverPort, const std::string& secret)
     : MetaServerCommand(METASERVERCOMMAND_REMOVE), serverPort(serverPort), secret(secret) {
    }

    bool operator==(const MetaServerCommand& metaServerCommand) const override
    {
        const MetaServerRemove* pMetaServerRemove = dynamic_cast<const MetaServerRemove*>(&metaServerCommand);
        if(pMetaServerRemove == nullptr) {
            return false;
        } else {

            return ((serverPort == pMetaServerRemove->serverPort)
                     && (secret == pMetaServerRemove->secret));
        }
    }

    int serverPort;
    std::string secret;
};

class MetaServerList : public MetaServerCommand {
public:
    MetaServerList()
     : MetaServerCommand(METASERVERCOMMAND_LIST) {
    }
};

class MetaServerExit : public MetaServerCommand {
public:
    MetaServerExit()
     : MetaServerCommand(METASERVERCOMMAND_EXIT) {
    }
};

#endif // METASERVERCOMMANDS_H
