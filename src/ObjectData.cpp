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

ObjectData::ObjectData() {
    // set default values
    for (auto& i : data) {
        for (auto& h : i) {
            h.enabled          = false;
            h.hitpoints        = 0;
            h.price            = 0;
            h.power            = 0;
            h.viewrange        = 0;
            h.capacity         = 0;
            h.weapondamage     = 0;
            h.weaponrange      = 0;
            h.weaponreloadtime = 0;
            h.maxspeed         = 0;
            h.turnspeed        = 0;
            h.buildtime        = 0;
            h.infspawnprop     = 0;
            h.builder          = ItemID_Invalid;
            h.prerequisiteStructuresSet.reset();
            h.techLevel    = -1;
            h.upgradeLevel = 0;
        }
    }
}

ObjectData::~ObjectData() = default;

void ObjectData::loadFromINIFile(const std::string& filename) {
    const INIFile objectDataFile(pFileManager->openFile(filename).get());

    // load default structure values
    ObjectDataStruct structureDefaultData[static_cast<int>(HOUSETYPE::NUM_HOUSES)];
    for (int h = 0; h < static_cast<int>(HOUSETYPE::NUM_HOUSES); h++) {
        structureDefaultData[h].enabled = loadBoolValue(objectDataFile, "default structure", "Enabled", houseChar[h]);
        structureDefaultData[h].hitpoints =
            loadIntValue(objectDataFile, "default structure", "HitPoints", houseChar[h]);
        structureDefaultData[h].price = loadIntValue(objectDataFile, "default structure", "Price", houseChar[h]);
        structureDefaultData[h].power = loadIntValue(objectDataFile, "default structure", "Power", houseChar[h]);
        structureDefaultData[h].viewrange =
            loadIntValue(objectDataFile, "default structure", "ViewRange", houseChar[h]);
        structureDefaultData[h].capacity = loadIntValue(objectDataFile, "default structure", "Capacity", houseChar[h]);
        structureDefaultData[h].weapondamage =
            loadIntValue(objectDataFile, "default structure", "WeaponDamage", houseChar[h]);
        structureDefaultData[h].weaponrange =
            loadIntValue(objectDataFile, "default structure", "WeaponRange", houseChar[h]);
        structureDefaultData[h].weaponreloadtime =
            loadIntValue(objectDataFile, "default structure", "WeaponReloadTime", houseChar[h]);
        structureDefaultData[h].maxspeed =
            loadFixPointValue(objectDataFile, "default structure", "MaxSpeed", houseChar[h]);
        structureDefaultData[h].turnspeed =
            loadFixPointValue(objectDataFile, "default structure", "TurnSpeed", houseChar[h]);
        structureDefaultData[h].buildtime =
            loadIntValue(objectDataFile, "default structure", "BuildTime", houseChar[h]);
        structureDefaultData[h].infspawnprop =
            loadIntValue(objectDataFile, "default structure", "InfSpawnProp", houseChar[h]);
        structureDefaultData[h].builder = loadItemID(objectDataFile, "default structure", "Builder", houseChar[h]);
        structureDefaultData[h].prerequisiteStructuresSet =
            loadPrerequisiteStructuresSet(objectDataFile, "default structure", "Prerequisite", houseChar[h]);
        structureDefaultData[h].techLevel =
            loadIntValue(objectDataFile, "default structure", "TechLevel", houseChar[h], -1);
        structureDefaultData[h].upgradeLevel =
            loadIntValue(objectDataFile, "default structure", "UpgradeLevel", houseChar[h]);
    }

    // load default unit values
    ObjectDataStruct unitDefaultData[static_cast<int>(HOUSETYPE::NUM_HOUSES)];
    for (int h = 0; h < static_cast<int>(HOUSETYPE::NUM_HOUSES); h++) {
        unitDefaultData[h].enabled      = loadBoolValue(objectDataFile, "default unit", "Enabled", houseChar[h]);
        unitDefaultData[h].hitpoints    = loadIntValue(objectDataFile, "default unit", "HitPoints", houseChar[h]);
        unitDefaultData[h].price        = loadIntValue(objectDataFile, "default unit", "Price", houseChar[h]);
        unitDefaultData[h].power        = loadIntValue(objectDataFile, "default unit", "Power", houseChar[h]);
        unitDefaultData[h].viewrange    = loadIntValue(objectDataFile, "default unit", "ViewRange", houseChar[h]);
        unitDefaultData[h].capacity     = loadIntValue(objectDataFile, "default unit", "Capacity", houseChar[h]);
        unitDefaultData[h].weapondamage = loadIntValue(objectDataFile, "default unit", "WeaponDamage", houseChar[h]);
        unitDefaultData[h].weaponrange  = loadIntValue(objectDataFile, "default unit", "WeaponRange", houseChar[h]);
        unitDefaultData[h].weaponreloadtime =
            loadIntValue(objectDataFile, "default unit", "WeaponReloadTime", houseChar[h]);
        unitDefaultData[h].maxspeed     = loadFixPointValue(objectDataFile, "default unit", "MaxSpeed", houseChar[h]);
        unitDefaultData[h].turnspeed    = loadFixPointValue(objectDataFile, "default unit", "TurnSpeed", houseChar[h]);
        unitDefaultData[h].buildtime    = loadIntValue(objectDataFile, "default unit", "BuildTime", houseChar[h]);
        unitDefaultData[h].infspawnprop = loadIntValue(objectDataFile, "default unit", "InfSpawnProp", houseChar[h]);
        unitDefaultData[h].builder      = loadItemID(objectDataFile, "default structure", "Builder", houseChar[h]);
        unitDefaultData[h].prerequisiteStructuresSet =
            loadPrerequisiteStructuresSet(objectDataFile, "default unit", "Prerequisite", houseChar[h]);
        unitDefaultData[h].techLevel    = loadIntValue(objectDataFile, "default unit", "TechLevel", houseChar[h], -1);
        unitDefaultData[h].upgradeLevel = loadIntValue(objectDataFile, "default unit", "UpgradeLevel", houseChar[h]);
    }

    // set default values
    for (int i = 0; i < Num_ItemID; i++) {
        for (int h = 0; h < static_cast<int>(HOUSETYPE::NUM_HOUSES); h++) {
            if (isStructure(static_cast<ItemID_enum>(i))) {
                data[i][h] = structureDefaultData[h];
            } else {
                data[i][h] = unitDefaultData[h];
            }
        }
    }

    for (INIFile::Section& section : objectDataFile) {
        const std::string& sectionName = section.getSectionName();

        if (sectionName.empty() || sectionName == "default structure" || sectionName == "default unit") {
            continue;
        }

        const auto itemID = getItemIDByName(sectionName);

        if (itemID == ItemID_Invalid) {
            sdl2::log_info("ObjectData::ObjectData(): '%s' is no valid unit/structure name!", sectionName);
            continue;
        }

        for (int h = 0; h < static_cast<int>(HOUSETYPE::NUM_HOUSES); h++) {

            const ObjectDataStruct& defaultData = isStructure(itemID) ? structureDefaultData[h] : unitDefaultData[h];

            data[itemID][h].enabled =
                loadBoolValue(objectDataFile, sectionName, "Enabled", houseChar[h], defaultData.enabled);
            data[itemID][h].hitpoints =
                loadIntValue(objectDataFile, sectionName, "HitPoints", houseChar[h], defaultData.hitpoints);
            data[itemID][h].price = loadIntValue(objectDataFile, sectionName, "Price", houseChar[h], defaultData.price);
            data[itemID][h].power = loadIntValue(objectDataFile, sectionName, "Power", houseChar[h], defaultData.power);
            data[itemID][h].viewrange =
                loadIntValue(objectDataFile, sectionName, "ViewRange", houseChar[h], defaultData.viewrange);
            data[itemID][h].capacity =
                loadIntValue(objectDataFile, sectionName, "Capacity", houseChar[h], defaultData.capacity);
            data[itemID][h].weapondamage =
                loadIntValue(objectDataFile, sectionName, "WeaponDamage", houseChar[h], defaultData.weapondamage);
            data[itemID][h].weaponrange =
                loadIntValue(objectDataFile, sectionName, "WeaponRange", houseChar[h], defaultData.weaponrange);
            data[itemID][h].weaponreloadtime = loadIntValue(objectDataFile, sectionName, "WeaponReloadTime",
                                                            houseChar[h], defaultData.weaponreloadtime);
            data[itemID][h].maxspeed =
                loadFixPointValue(objectDataFile, sectionName, "MaxSpeed", houseChar[h], defaultData.maxspeed);
            data[itemID][h].turnspeed =
                loadFixPointValue(objectDataFile, sectionName, "TurnSpeed", houseChar[h], defaultData.turnspeed);
            data[itemID][h].buildtime =
                loadIntValue(objectDataFile, sectionName, "BuildTime", houseChar[h], defaultData.buildtime);
            data[itemID][h].infspawnprop =
                loadIntValue(objectDataFile, sectionName, "InfSpawnProp", houseChar[h], defaultData.infspawnprop);
            data[itemID][h].builder =
                loadItemID(objectDataFile, sectionName, "Builder", houseChar[h], defaultData.builder);
            data[itemID][h].prerequisiteStructuresSet = loadPrerequisiteStructuresSet(
                objectDataFile, sectionName, "Prerequisite", houseChar[h], defaultData.prerequisiteStructuresSet);
            data[itemID][h].techLevel =
                loadIntValue(objectDataFile, sectionName, "TechLevel", houseChar[h], defaultData.techLevel);
            data[itemID][h].upgradeLevel =
                loadIntValue(objectDataFile, sectionName, "UpgradeLevel", houseChar[h], defaultData.upgradeLevel);
        }
    }
}

void ObjectData::save(OutputStream& stream) const {
    for (const auto& i : data) {
        for (const auto& h : i) {
            stream.writeBool(h.enabled);
            stream.writeSint32(h.hitpoints);
            stream.writeSint32(h.price);
            stream.writeSint32(h.power);
            stream.writeSint32(h.viewrange);
            stream.writeSint32(h.capacity);
            stream.writeSint32(h.weapondamage);
            stream.writeSint32(h.weaponrange);
            stream.writeSint32(h.weaponreloadtime);
            stream.writeFixPoint(h.maxspeed);
            stream.writeFixPoint(h.turnspeed);
            stream.writeSint32(h.buildtime);
            stream.writeSint32(h.infspawnprop);
            stream.writeSint32(static_cast<int>(h.builder));
            stream.writeUint32(static_cast<uint32_t>(h.prerequisiteStructuresSet.to_ulong()));
            stream.writeSint8(h.techLevel);
            stream.writeSint8(h.upgradeLevel);
        }
    }
}

void ObjectData::load(InputStream& stream) {
    for (auto& i : data) {
        for (auto& h : i) {
            h.enabled                   = stream.readBool();
            h.hitpoints                 = stream.readSint32();
            h.price                     = stream.readSint32();
            h.power                     = stream.readSint32();
            h.viewrange                 = stream.readSint32();
            h.capacity                  = stream.readSint32();
            h.weapondamage              = stream.readSint32();
            h.weaponrange               = stream.readSint32();
            h.weaponreloadtime          = stream.readSint32();
            h.maxspeed                  = stream.readFixPoint();
            h.turnspeed                 = stream.readFixPoint();
            h.buildtime                 = stream.readSint32();
            h.infspawnprop              = stream.readSint32();
            h.builder                   = static_cast<ItemID_enum>(stream.readSint32());
            h.prerequisiteStructuresSet = std::bitset<Structure_LastID + 1>((unsigned long)stream.readUint32());
            h.techLevel                 = stream.readSint8();
            h.upgradeLevel              = stream.readSint8();
        }
    }
}

int ObjectData::loadIntValue(const INIFile& objectDataFile, const std::string& section, const std::string& key,
                             char houseChar, int defaultValue) {
    const std::string specializedKey = key + "(" + houseChar + ")";
    if (objectDataFile.hasKey(section, specializedKey)) {
        return objectDataFile.getIntValue(section, specializedKey, defaultValue);
    }
    return objectDataFile.getIntValue(section, key, defaultValue);
}

bool ObjectData::loadBoolValue(const INIFile& objectDataFile, const std::string& section, const std::string& key,
                               char houseChar, bool defaultValue) {
    const std::string specializedKey = key + "(" + houseChar + ")";
    if (objectDataFile.hasKey(section, specializedKey)) {
        return objectDataFile.getBoolValue(section, specializedKey, defaultValue);
    }
    return objectDataFile.getBoolValue(section, key, defaultValue);
}

FixPoint ObjectData::loadFixPointValue(const INIFile& objectDataFile, const std::string& section,
                                       const std::string& key, char houseChar, FixPoint defaultValue) {
    const std::string specializedKey = key + "(" + houseChar + ")";
    if (objectDataFile.hasKey(section, specializedKey)) {
        return FixPoint(objectDataFile.getStringValue(section, specializedKey, defaultValue.toString()));
    }
    return FixPoint(objectDataFile.getStringValue(section, key, defaultValue.toString()));
}

std::string ObjectData::loadStringValue(const INIFile& objectDataFile, const std::string& section,
                                        const std::string& key, char houseChar, const std::string& defaultValue) {
    const std::string specializedKey = key + "(" + houseChar + ")";
    if (objectDataFile.hasKey(section, specializedKey)) {
        return objectDataFile.getStringValue(section, specializedKey, defaultValue);
    }
    return objectDataFile.getStringValue(section, key, defaultValue);
}

ItemID_enum ObjectData::loadItemID(const INIFile& objectDataFile, const std::string& section, const std::string& key,
                                   char houseChar, ItemID_enum defaultValue) {
    const std::string strItem{trim(loadStringValue(objectDataFile, section, key, houseChar, ""))};

    if (strItem.empty()) {
        return defaultValue;
    }
    if (strToLower(strItem) == "invalid") {

        return ItemID_Invalid;
    }

    const ItemID_enum itemID = getItemIDByName(strItem);

    if (itemID == ItemID_Invalid) {
        sdl2::log_info(
            "Warning: Cannot read object data from section '%s', key '%s': '%s' is no valid structure/unit name!",
            section, key, strItem);
        return defaultValue;
    }

    return itemID;
}

std::bitset<Structure_LastID + 1>
ObjectData::loadPrerequisiteStructuresSet(const INIFile& objectDataFile, const std::string& section,
                                          const std::string& key, char houseChar,
                                          std::bitset<Structure_LastID + 1> defaultValue) {
    std::bitset<Structure_LastID + 1> resultSet;

    const std::string strList = loadStringValue(objectDataFile, section, key, houseChar, "");

    if (strList.empty())
        return defaultValue;

    if (trim(strToLower(strList)) == "invalid")
        return resultSet;

    const std::vector<std::string> strItemList = splitStringToStringVector(strList);

    for (const std::string& strItem : strItemList) {
        std::string strItem2{trim(strItem)};

        const ItemID_enum itemID = getItemIDByName(strItem2);
        if (itemID == ItemID_Invalid || !isStructure(itemID)) {
            sdl2::log_info(
                "Warning: Cannot read object data from section '%s', key '%s': '%s' is no valid structure name!",
                section.c_str(), key.c_str(), strItem2.c_str());
            return defaultValue;
        }

        resultSet.set(itemID);
    }

    return resultSet;
}
