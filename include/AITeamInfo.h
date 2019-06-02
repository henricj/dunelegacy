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

#ifndef AITEAMINFO_H
#define AITEAMINFO_H

#include <DataTypes.h>
#include <misc/InputStream.h>
#include <misc/OutputStream.h>

class AITeamInfo {
public:
    AITeamInfo(int houseID, AITeamBehavior aiTeamBehavior, AITeamType aiTeamType, int minUnits, int maxUnits)
     : houseID(houseID), aiTeamBehavior(aiTeamBehavior), aiTeamType(aiTeamType), minUnits(minUnits), maxUnits(maxUnits) {
    }

    explicit AITeamInfo(InputStream& stream) {
        houseID = stream.readUint32();
        aiTeamBehavior = static_cast<AITeamBehavior>(stream.readUint32());
        aiTeamType = static_cast<AITeamType>(stream.readUint32());
        minUnits = stream.readUint32();
        maxUnits = stream.readUint32();
    }

    void save(OutputStream& stream) const {
        stream.writeUint32(houseID);
        stream.writeUint32(aiTeamBehavior);
        stream.writeUint32(aiTeamType);
        stream.writeUint32(minUnits);
        stream.writeUint32(maxUnits);
    }

    int          houseID;
    AITeamBehavior aiTeamBehavior;
    AITeamType     aiTeamType;
    int          minUnits;
    int          maxUnits;
};

#endif // AITEAMINFO_H
