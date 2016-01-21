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

#include <stdlib.h>
#include <iostream>

ObjectData::ObjectData()
{
    // set default values
    for(int i=0;i<Num_ItemID;i++) {
        for(int h=0;h<NUM_HOUSES;h++) {
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

ObjectData::~ObjectData()
{
}

void ObjectData::loadFromINIFile(std::string filename)
{
    SDL_RWops *file = pFileManager->openFile(filename);
    INIFile objectDataFile(file);
    SDL_RWclose(file);

    // load default structure values
    ObjectDataStruct structureDefaultData[NUM_HOUSES];
    for(int h=0;h<NUM_HOUSES;h++) {
        structureDefaultData[h].hitpoints = loadIntValue(objectDataFile, "default structure", "HitPoints", houseChar[h]);
        structureDefaultData[h].price = loadIntValue(objectDataFile, "default structure", "Price", houseChar[h]);
        structureDefaultData[h].power = loadIntValue(objectDataFile, "default structure", "Power", houseChar[h]);
        structureDefaultData[h].viewrange = loadIntValue(objectDataFile, "default structure", "ViewRange", houseChar[h]);
        structureDefaultData[h].capacity = loadIntValue(objectDataFile, "default structure", "Capacity", houseChar[h]);
        structureDefaultData[h].weapondamage = loadIntValue(objectDataFile, "default structure", "WeaponDamage", houseChar[h]);
        structureDefaultData[h].weaponrange = loadIntValue(objectDataFile, "default structure", "WeaponRange", houseChar[h]);
        structureDefaultData[h].weaponreloadtime = loadIntValue(objectDataFile, "default structure", "WeaponReloadTime", houseChar[h]);
        structureDefaultData[h].maxspeed = loadFloatValue(objectDataFile, "default structure", "MaxSpeed", houseChar[h]);
        structureDefaultData[h].turnspeed = loadFloatValue(objectDataFile, "default structure", "TurnSpeed", houseChar[h]);
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
        unitDefaultData[h].hitpoints = loadIntValue(objectDataFile, "default unit", "HitPoints", houseChar[h]);
        unitDefaultData[h].price = loadIntValue(objectDataFile, "default unit", "Price", houseChar[h]);
        unitDefaultData[h].power = loadIntValue(objectDataFile, "default unit", "Power", houseChar[h]);
        unitDefaultData[h].viewrange = loadIntValue(objectDataFile, "default unit", "ViewRange", houseChar[h]);
        unitDefaultData[h].capacity = loadIntValue(objectDataFile, "default unit", "Capacity", houseChar[h]);
        unitDefaultData[h].weapondamage = loadIntValue(objectDataFile, "default unit", "WeaponDamage", houseChar[h]);
        unitDefaultData[h].weaponrange = loadIntValue(objectDataFile, "default unit", "WeaponRange", houseChar[h]);
        unitDefaultData[h].weaponreloadtime = loadIntValue(objectDataFile, "default unit", "WeaponReloadTime", houseChar[h]);
        unitDefaultData[h].maxspeed = loadFloatValue(objectDataFile, "default unit", "MaxSpeed", houseChar[h]);
        unitDefaultData[h].turnspeed = loadFloatValue(objectDataFile, "default unit", "TurnSpeed", houseChar[h]);
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

    INIFile::SectionIterator iter;
    for(iter = objectDataFile.begin(); iter != objectDataFile.end(); ++iter) {
        std::string sectionName = iter->getSectionName();

        if(sectionName == "" || sectionName == "default structure" || sectionName == "default unit") {
            continue;
        }

        Uint32 itemID = getItemIDByName(sectionName);

        if(itemID == ItemID_Invalid) {
            std::cerr << "ObjectData::ObjectData(): \"" << sectionName << "\" is no valid unit/structure name!" << std::endl;
            continue;
        }

        for(int h=0;h<NUM_HOUSES;h++) {

            ObjectDataStruct& defaultData = isStructure(itemID) ? structureDefaultData[h] : unitDefaultData[h];

            data[itemID][h].hitpoints = loadIntValue(objectDataFile, sectionName, "HitPoints", houseChar[h], defaultData.hitpoints);
            data[itemID][h].price = loadIntValue(objectDataFile, sectionName, "Price", houseChar[h], defaultData.price);
            data[itemID][h].power = loadIntValue(objectDataFile, sectionName, "Power", houseChar[h], defaultData.power);
            data[itemID][h].viewrange = loadIntValue(objectDataFile, sectionName, "ViewRange", houseChar[h], defaultData.viewrange);
            data[itemID][h].capacity = loadIntValue(objectDataFile, sectionName, "Capacity", houseChar[h], defaultData.capacity);
            data[itemID][h].weapondamage = loadIntValue(objectDataFile, sectionName, "WeaponDamage", houseChar[h], defaultData.weapondamage);
            data[itemID][h].weaponrange = loadIntValue(objectDataFile, sectionName, "WeaponRange", houseChar[h], defaultData.weaponrange);
            data[itemID][h].weaponreloadtime = loadIntValue(objectDataFile, sectionName, "WeaponReloadTime", houseChar[h], defaultData.weaponreloadtime);
            data[itemID][h].maxspeed = loadFloatValue(objectDataFile, sectionName, "MaxSpeed", houseChar[h], defaultData.maxspeed);
            data[itemID][h].turnspeed = loadFloatValue(objectDataFile, sectionName, "TurnSpeed", houseChar[h], defaultData.turnspeed);
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
            stream.writeSint32(data[i][h].hitpoints);
            stream.writeSint32(data[i][h].price);
            stream.writeSint32(data[i][h].power);
            stream.writeSint32(data[i][h].viewrange);
            stream.writeSint32(data[i][h].capacity);
            stream.writeSint32(data[i][h].weapondamage);
            stream.writeSint32(data[i][h].weaponrange);
            stream.writeSint32(data[i][h].weaponreloadtime);
            stream.writeFloat(data[i][h].maxspeed);
            stream.writeFloat(data[i][h].turnspeed);
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
            data[i][h].hitpoints = stream.readSint32();
            data[i][h].price = stream.readSint32();
            data[i][h].power = stream.readSint32();
            data[i][h].viewrange = stream.readSint32();
            data[i][h].capacity = stream.readSint32();
            data[i][h].weapondamage = stream.readSint32();
            data[i][h].weaponrange = stream.readSint32();
            data[i][h].weaponreloadtime = stream.readSint32();
            data[i][h].maxspeed = stream.readFloat();
            data[i][h].turnspeed = stream.readFloat();
            data[i][h].buildtime = stream.readSint32();
            data[i][h].infspawnprop = stream.readSint32();
            data[i][h].builder = stream.readSint32();
            data[i][h].prerequisiteStructuresSet = std::bitset<Structure_LastID>( (unsigned long) stream.readUint32());
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

float ObjectData::loadFloatValue(const INIFile& objectDataFile, const std::string& section, const std::string& key, char houseChar, float defaultValue) {
    std::string specializedKey = key + "(" + houseChar + ")";
    if(objectDataFile.hasKey(section, specializedKey)) {
        return objectDataFile.getFloatValue(section, specializedKey, defaultValue);
    } else {
        return objectDataFile.getFloatValue(section, key, defaultValue);
    }
}

std::string ObjectData::loadStringValue(const INIFile& objectDataFile, const std::string& section, const std::string& key, char houseChar, std::string defaultValue) {
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
        fprintf(stderr, "Warning: Cannot read object data from section '%s', key '%s': '%s' is no valid structure/unit name!\n", section.c_str(), key.c_str(), strItem.c_str() );
        return defaultValue;
    }

    return itemID;
}


std::bitset<Structure_LastID> ObjectData::loadPrerequisiteStructuresSet(const INIFile& objectDataFile, const std::string& section, const std::string& key, char houseChar, std::bitset<Structure_LastID> defaultValue) {
    std::bitset<Structure_LastID> resultSet;

    std::string strList = loadStringValue(objectDataFile, section, key, houseChar, "");

    if(strList == "") {
        return defaultValue;
    } else if(trim(strToLower(strList)) == "invalid") {
        return resultSet;
    }

    std::vector<std::string> strItemList = splitString(strList);

    std::vector<std::string>::const_iterator iter;
    for(iter = strItemList.begin(); iter != strItemList.end(); ++iter) {
        std::string strItem = trim(*iter);

        int itemID = getItemIDByName(strItem);
        if(itemID == ItemID_Invalid || !isStructure(itemID)) {
            fprintf(stderr, "Warning: Cannot read object data from section '%s', key '%s': '%s' is no valid structure name!\n", section.c_str(), key.c_str(), strItem.c_str() );
            return defaultValue;
        }

        resultSet.set(itemID);
    }

    return resultSet;
}
