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

#include <ObjectManager.h>

#include <globals.h>

#include <Game.h>
#include <ObjectBase.h>

ObjectManager::ObjectManager() {
    objectMap.reserve(100);
}

ObjectManager::~ObjectManager() = default;

void ObjectManager::save(OutputStream& stream) const {
    stream.writeUint32(nextFreeObjectID);

    stream.writeUint32(objectMap.size());
    for (const auto& objectEntry : objectMap) {
        stream.writeUint32(objectEntry.second->getObjectID());
        currentGame->saveObject(stream, objectEntry.second.get());
    }
}

void ObjectManager::load(InputStream& stream) {
    objectMap.clear();

    nextFreeObjectID = stream.readUint32();

    const auto numObjects = stream.readUint32();

    objectMap.reserve(numObjects);

    for (auto i = decltype(numObjects) {0}; i < numObjects; i++) {
        auto objectID = stream.readUint32();

        auto pObject = loadObject(stream, objectID);
        if (objectID != pObject->getObjectID()) {
            sdl2::log_info("ObjectManager::load(): The loaded object has a different ID than expected (%d!=%d)!",
                           objectID, pObject->getObjectID());
        }

        const auto& [_, ok] = objectMap.emplace(objectID, std::move(pObject));
        if (!ok) {
            // there is already such an object
            sdl2::log_info("ObjectManager::load(): The object with this id already exists (%d)!", objectID);
        }
    }
}

bool ObjectManager::addObject(std::unique_ptr<ObjectBase> object) {
    const auto& [_, ok] = objectMap.emplace(nextFreeObjectID, std::move(object));

    if (!ok) {
        // there is already such an object in the list
        sdl2::log_info("ObjectManager::addObject(): The object with this id already exists (%d)!", nextFreeObjectID);
        return false;
    }

    ++nextFreeObjectID;

    return true;
}

std::unique_ptr<ObjectBase> ObjectManager::loadObject(InputStream& stream, uint32_t objectID) {
    const auto itemID = static_cast<ItemID_enum>(stream.readUint32());

    auto newObject = ObjectBase::loadObject(itemID, objectID, ObjectStreamInitializer {stream});
    if (!newObject) {
        THROW(std::runtime_error, "Error while loading an object!");
    }

    return newObject;
}
