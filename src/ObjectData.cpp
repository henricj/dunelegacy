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

#include "ObjectData.h"

#include <globals.h>

#include <FileClasses/FileManager.h>
#include <FileClasses/INIFile.h>
#include <sand.h>

#include <misc/string_util.h>

ObjectData::ObjectData()
{
    // set default values
    for(int i=0;i<Num_ItemID;i++) {
        for(int h=0;h<NUM_HOUSES;h++) {
            data[i][h].enabled = false;
            data[i][h].hitpoints = 0;
            data[i][h].price = 0;
            data[i][h].power = 0;
            data[i][h].viewrange = 0;
            data[i][h].capacity = 0;
            data[i][h].weapondamage = 0;
            data[i][h].weaponrange = 0;
            data[i][h].weaponreloadtime = 0;
            data[i][h].maxspeed = 0;
            data[i][h].turnspeed = 0;
            data[i][h].buildtime = 0;
            data[i][h].infspawnprop = 0;
            data[i][h].builder = ItemID_Invalid;
            data[i][h].prerequisiteStructuresSet.reset();
            data[i][h].techLevel = -1;
            data[i][h].upgradeLevel = 0;
        }
    }
}

ObjectData::~ObjectData() = default;

void ObjectData::loadFromINIFile(const std::string& filename)
{
    INIFile objectDataFile(pFileManager->openFile(filename).get());

    // load default structure values
    ObjectDataStruct structureDefaultData[NUM_HOUSES];
    for(int h=0;h<NUM_HOUSES;h++) {
        structureDefaultData[h].enabled = loadBoolValue(objectDataFile, "default structure", "Enabled", houseChar[h]);
        structureDefaultData[h].hitpoints = loadIntValue(objectDataFile, "default structure", "HitPoints", houseChar[h]);
        structureDefaultData[h].price = loadIntValue(objectDataFile, "default structure", "Price", houseChar[h]);
        structureDefaultData[h].power = loadIntValue(objectDataFile, "default structure", "Power", houseChar[h]);
        structureDefaultData[h].viewrange = loadIntValue(objectDataFile, "default structure", "ViewRange", houseChar[h]);
        structureDefaultData[h].capacity = loadIntValue(objectDataFile, "default structure", "Capacity", houseChar[h]);
        structureDefaultData[h].weapondamage = loadIntValue(objectDataFile, "default structure", "WeaponDamage", houseChar[h]);
        structureDefaultData[h].weaponrange = loadIntValue(objectDataFile, "default structure", "WeaponRange", houseChar[h]);
        structureDefaultData[h].weaponreloadtime = loadIntValue(objectDataFile, "default structure", "WeaponReloadTime", houseChar[h]);
        structureDefaultData[h].maxspeed = loadFixPointValue(objectDataFile, "default structure", "MaxSpeed", houseChar[h]);
        structureDefaultData[h].turnspeed = loadFixPointValue(objectDataFile, "default structure", "TurnSpeed", houseChar[h]);
        structureDefaultData[h].buildtime = loadIntValue(objectDataFile, "default structure", "BuildTime", houseChar[h]);
        structureDefaultData[h].infspawnprop = loadIntValue(objectDataFile, "default structure", "InfSpawnProp", houseChar[h]);
        structureDefaultData[h].builder = loadItemID(objectDataFile, "default structure", "Builder", houseChar[h]);
        structureDefaultData[h].prerequisiteStructuresSet = loadPrerequisiteStructuresSet(objectDataFile, "default structure", "Prerequisite", houseChar[h]);
        structureDefaultData[h].techLevel = loadIntValue(objectDataFile, "default structure", "TechLevel", houseChar[h], -1);
        structureDefaultData[h].upgradeLevel = loadIntValue(objectDataFile, "default structure", "UpgradeLevel", houseChar[h]);
    }

    // load default unit values
    ObjectDataStruct unitDefaultData[NUM_HOUSES];
    for(int h=0;h<NUM_HOUSES;h++) {
        unitDefaultData[h].enabled = loadBoolValue(objectDataFile, "default unit", "Enabled", houseChar[h]);
        unitDefaultData[h].hitpoints = loadIntValue(objectDataFile, "default unit", "HitPoints", houseChar[h]);
        unitDefaultData[h].price = loadIntValue(objectDataFile, "default unit", "Price", houseChar[h]);
        unitDefaultData[h].power = loadIntValue(objectDataFile, "default unit", "Power", houseChar[h]);
        unitDefaultData[h].viewrange = loadIntValue(objectDataFile, "default unit", "ViewRange", houseChar[h]);
        unitDefaultData[h].capacity = loadIntValue(objectDataFile, "default unit", "Capacity", houseChar[h]);
        unitDefaultData[h].weapondamage = loadIntValue(objectDataFile, "default unit", "WeaponDamage", houseChar[h]);
        unitDefaultData[h].weaponrange = loadIntValue(objectDataFile, "default unit", "WeaponRange", houseChar[h]);
        unitDefaultData[h].weaponreloadtime = loadIntValue(objectDataFile, "default unit", "WeaponReloadTime", houseChar[h]);
        unitDefaultData[h].maxspeed = loadFixPointValue(objectDataFile, "default unit", "MaxSpeed", houseChar[h]);
        unitDefaultData[h].turnspeed = loadFixPointValue(objectDataFile, "default unit", "TurnSpeed", houseChar[h]);
        unitDefaultData[h].buildtime = loadIntValue(objectDataFile, "default unit", "BuildTime", houseChar[h]);
        unitDefaultData[h].infspawnprop = loadIntValue(objectDataFile, "default unit", "InfSpawnProp", houseChar[h]);
        unitDefaultData[h].builder = loadItemID(objectDataFile, "default structure", "Builder", houseChar[h]);
        unitDefaultData[h].prerequisiteStructuresSet = loadPrerequisiteStructuresSet(objectDataFile, "default unit", "Prerequisite", houseChar[h]);
        unitDefaultData[h].techLevel = loadIntValue(objectDataFile, "default unit", "TechLevel", houseChar[h], -1);
        unitDefaultData[h].upgradeLevel = loadIntValue(objectDataFile, "default unit", "UpgradeLevel", houseChar[h]);
    }

    // set default values
    for(int i=0;i<Num_ItemID;i++) {
        for(int h=0;h<NUM_HOUSES;h++) {
            if(isStructure(i)) {
                data[i][h] = structureDefaultData[h];
            } else {
                data[i][h] = unitDefaultData[h];
            }
        }
    }

    for(INIFile::Section& section : objectDataFile) {
        const std::string& sectionName = section.getSectionName();

        if(sectionName == "" || sectionName == "default structure" || sectionName == "default unit") {
            continue;
        }

        Uint32 itemID = getItemIDByName(sectionName);

        if(itemID == ItemID_Invalid) {
            SDL_Log("ObjectData::ObjectData(): '%s' is no valid unit/structure name!", sectionName.c_str());
            continue;
        }

        for(int h=0;h<NUM_HOUSES;h++) {

            ObjectDataStruct& defaultData = isStructure(itemID) ? structureDefaultData[h] : unitDefaultData[h];

            data[itemID][h].enabled = loadBoolValue(objectDataFile, sectionName, "Enabled", houseChar[h], defaultData.enabled);
            data[itemID][h].hitpoints = loadIntValue(objectDataFile, sectionName, "HitPoints", houseChar[h], defaultData.hitpoints);
            data[itemID][h].price = loadIntValue(objectDataFile, sectionName, "Price", houseChar[h], defaultData.price);
            data[itemID][h].power = loadIntValue(objectDataFile, sectionName, "Power", houseChar[h], defaultData.power);
            data[itemID][h].viewrange = loadIntValue(objectDataFile, sectionName, "ViewRange", houseChar[h], defaultData.viewrange);
            data[itemID][h].capacity = loadIntValue(objectDataFile, sectionName, "Capacity", houseChar[h], defaultData.capacity);
            data[itemID][h].weapondamage = loadIntValue(objectDataFile, sectionName, "WeaponDamage", houseChar[h], defaultData.weapondamage);
            data[itemID][h].weaponrange = loadIntValue(objectDataFile, sectionName, "WeaponRange", houseChar[h], defaultData.weaponrange);
            data[itemID][h].weaponreloadtime = loadIntValue(objectDataFile, sectionName, "WeaponReloadTime", houseChar[h], defaultData.weaponreloadtime);
            data[itemID][h].maxspeed = loadFixPointValue(objectDataFile, sectionName, "MaxSpeed", houseChar[h], defaultData.maxspeed);
            data[itemID][h].turnspeed = loadFixPointValue(objectDataFile, sectionName, "TurnSpeed", houseChar[h], defaultData.turnspeed);
            data[itemID][h].buildtime = loadIntValue(objectDataFile, sectionName, "BuildTime", houseChar[h], defaultData.buildtime);
            data[itemID][h].infspawnprop = loadIntValue(objectDataFile, sectionName, "InfSpawnProp", houseChar[h], defaultData.infspawnprop);
            data[itemID][h].builder = loadItemID(objectDataFile, sectionName, "Builder", houseChar[h], defaultData.builder);
            data[itemID][h].prerequisiteStructuresSet = loadPrerequisiteStructuresSet(objectDataFile, sectionName, "Prerequisite", houseChar[h], defaultData.prerequisiteStructuresSet);
            data[itemID][h].techLevel = loadIntValue(objectDataFile, sectionName, "TechLevel", houseChar[h], defaultData.techLevel);
            data[itemID][h].upgradeLevel = loadIntValue(objectDataFile, sectionName, "UpgradeLevel", houseChar[h], defaultData.upgradeLevel);
        }
    }
}

void ObjectData::save(OutputStream& stream) const
{
    for(int i=0;i<Num_ItemID;i++) {
        for(int h=0;h<NUM_HOUSES;h++) {
            stream.writeBool(data[i][h].enabled);
            stream.writeSint32(data[i][h].hitpoints);
            stream.writeSint32(data[i][h].price);
            stream.writeSint32(data[i][h].power);
            stream.writeSint32(data[i][h].viewrange);
            stream.writeSint32(data[i][h].capacity);
            stream.writeSint32(data[i][h].weapondamage);
            stream.writeSint32(data[i][h].weaponrange);
            stream.writeSint32(data[i][h].weaponreloadtime);
            stream.writeFixPoint(data[i][h].maxspeed);
            stream.writeFixPoint(data[i][h].turnspeed);
            stream.writeSint32(data[i][h].buildtime);
            stream.writeSint32(data[i][h].infspawnprop);
            stream.writeSint32(data[i][h].builder);
            stream.writeUint32((Uint32) data[i][h].prerequisiteStructuresSet.to_ulong());
            stream.writeSint8(data[i][h].techLevel);
            stream.writeSint8(data[i][h].upgradeLevel);
        }
    }
}

void ObjectData::load(InputStream& stream)
{
    for(int i=0;i<Num_ItemID;i++) {
        for(int h=0;h<NUM_HOUSES;h++) {
            data[i][h].enabled = stream.readBool();
            data[i][h].hitpoints = stream.readSint32();
            data[i][h].price = stream.readSint32();
            data[i][h].power = stream.readSint32();
            data[i][h].viewrange = stream.readSint32();
            data[i][h].capacity = stream.readSint32();
            data[i][h].weapondamage = stream.readSint32();
            data[i][h].weaponrange = stream.readSint32();
            data[i][h].weaponreloadtime = stream.readSint32();
            data[i][h].maxspeed = stream.readFixPoint();
            data[i][h].turnspeed = stream.readFixPoint();
            data[i][h].buildtime = stream.readSint32();
            data[i][h].infspawnprop = stream.readSint32();
            data[i][h].builder = stream.readSint32();
            data[i][h].prerequisiteStructuresSet = std::bitset<Structure_LastID + 1>( (unsigned long) stream.readUint32());
            data[i][h].techLevel = stream.readSint8();
            data[i][h].upgradeLevel = stream.readSint8();
        }
    }
}

int ObjectData::loadIntValue(const INIFile& objectDataFile, const std::string& section, const std::string& key, char houseChar, int defaultValue) {
    std::string specializedKey = key + "(" + houseChar + ")";
    if(objectDataFile.hasKey(section, specializedKey)) {
        return objectDataFile.getIntValue(section, specializedKey, defaultValue);
    } else {
        return objectDataFile.getIntValue(section, key, defaultValue);
    }
}

bool ObjectData::loadBoolValue(const INIFile& objectDataFile, const std::string& section, const std::string& key, char houseChar, bool defaultValue) {
    std::string specializedKey = key + "(" + houseChar + ")";
    if(objectDataFile.hasKey(section, specializedKey)) {
        return objectDataFile.getBoolValue(section, specializedKey, defaultValue);
    } else {
        return objectDataFile.getBoolValue(section, key, defaultValue);
    }
}

FixPoint ObjectData::loadFixPointValue(const INIFile& objectDataFile, const std::string& section, const std::string& key, char houseChar, FixPoint defaultValue) {
    std::string specializedKey = key + "(" + houseChar + ")";
    if(objectDataFile.hasKey(section, specializedKey)) {
        return FixPoint(objectDataFile.getStringValue(section, specializedKey, defaultValue.toString()));
    } else {
        return FixPoint(objectDataFile.getStringValue(section, key, defaultValue.toString()));
    }
}

std::string ObjectData::loadStringValue(const INIFile& objectDataFile, const std::string& section, const std::string& key, char houseChar, const std::string& defaultValue) {
    std::string specializedKey = key + "(" + houseChar + ")";
    if(objectDataFile.hasKey(section, specializedKey)) {
        return objectDataFile.getStringValue(section, specializedKey, defaultValue);
    } else {
        return objectDataFile.getStringValue(section, key, defaultValue);
    }
}

int ObjectData::loadItemID(const INIFile& objectDataFile, const std::string& section, const std::string& key, char houseChar, int defaultValue) {
    std::string strItem = trim(loadStringValue(objectDataFile, section, key, houseChar, ""));

    if(strItem == "") {
        return defaultValue;
    } else if(strToLower(strItem) == "invalid") {
        return ItemID_Invalid;
    }

    int itemID = getItemIDByName(strItem);

    if(itemID == ItemID_Invalid) {
        SDL_Log("Warning: Cannot read object data from section '%s', key '%s': '%s' is no valid structure/unit name!", section.c_str(), key.c_str(), strItem.c_str() );
        return defaultValue;
    }

    return itemID;
}


std::bitset<Structure_LastID + 1> ObjectData::loadPrerequisiteStructuresSet(const INIFile& objectDataFile, const std::string& section, const std::string& key, char houseChar, std::bitset<Structure_LastID + 1> defaultValue) {
    std::bitset<Structure_LastID + 1> resultSet;

    std::string strList = loadStringValue(objectDataFile, section, key, houseChar, "");

    if(strList == "") {
        return defaultValue;
    } else if(trim(strToLower(strList)) == "invalid") {
        return resultSet;
    }

    std::vector<std::string> strItemList = splitStringToStringVector(strList);

    for(const std::string& strItem : strItemList) {
        std::string strItem2 = trim(strItem);

        int itemID = getItemIDByName(strItem2);
        if(itemID == ItemID_Invalid || !isStructure(itemID)) {
            SDL_Log("Warning: Cannot read object data from section '%s', key '%s': '%s' is no valid structure name!", section.c_str(), key.c_str(), strItem2.c_str() );
            return defaultValue;
        }

        resultSet.set(itemID);
    }

    return resultSet;
}
