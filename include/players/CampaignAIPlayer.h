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

#ifndef CAMPAIGNAIPLAYER_H
#define CAMPAIGNAIPLAYER_H

#include <players/Player.h>

#include <vector>

/**
    This AI player tries to resemble the original Dune II AI:
    - Build speed is dependent on mission number (slower in earlier missions)
    - Wait for the human player to find the AI player before becoming active and doing the following things:
     - Units are build when builders are idle
     - Built units are send out to attack the structure/unit with the highest priority
     - Teams are waiting till they reach a minimum number of units before attacking
     - Structures are only build when they were destroyed before (queue of at most 5 structures)
     - Special weapons are launched as soon as they get ready
*/
class CampaignAIPlayer : public Player
{
public:
    CampaignAIPlayer(House* associatedHouse, const std::string& playername);
    CampaignAIPlayer(InputStream& stream, House* associatedHouse);
    void init();
    ~CampaignAIPlayer();
    void save(OutputStream& stream) const override;

    void update() override;

    void onObjectWasBuilt(const ObjectBase* pObject) override;
    void onDecrementStructures(int itemID, const Coord& location) override;
    void onDamage(const ObjectBase* pObject, int damage, Uint32 damagerID) override;

private:
    class StructureInfo {
    public:
        StructureInfo(int itemID, const Coord& location)
         : itemID(itemID), location(location) {
        }

        StructureInfo(InputStream& stream) {
            itemID = stream.readUint32();
            location.x = stream.readSint32();
            location.y = stream.readSint32();
        }

        void save(OutputStream& stream) const {
            stream.writeUint32(itemID);
            stream.writeSint32(location.x);
            stream.writeSint32(location.y);
        }

        int itemID;
        Coord location;
    };

    void updateStructures();
    void updateUnits();

    int calculateTargetPriority(const UnitBase* pUnit, const ObjectBase* pObject);

    std::vector<StructureInfo> structureQueue;    ///< Last destroyed structures and their location
};

#endif //CAMPAIGNAIPLAYER_H
