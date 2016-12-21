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

void ObjectManager::save(OutputStream& stream) const {
    stream.writeUint32(nextFreeObjectID);

    stream.writeUint32(objectMap.size());
    for(const auto& objectEntry : objectMap) {
        stream.writeUint32(objectEntry.second->getObjectID());
        currentGame->saveObject(stream, objectEntry.second);
    }
}

void ObjectManager::load(InputStream& stream) {
    nextFreeObjectID = stream.readUint32();

    Uint32 numObjects = stream.readUint32();
    for(Uint32 i=0;i<numObjects;i++) {
        Uint32 objectID = stream.readUint32();

        ObjectBase* pObject = currentGame->loadObject(stream,objectID);
        if(objectID != pObject->getObjectID()) {
            SDL_Log("ObjectManager::load(): The loaded object has a different ID than expected (%d!=%d)!",objectID,pObject->getObjectID());
        }

        objectMap.insert( std::pair<Uint32,ObjectBase*>(objectID, pObject) );
    }
}

Uint32 ObjectManager::addObject(ObjectBase* pObject) {
    std::pair<ObjectMap::iterator,bool> insertPosition;

    insertPosition = objectMap.insert( std::pair<Uint32,ObjectBase*>(nextFreeObjectID, pObject) );

    if(insertPosition.second == false) {
        // there is already such an object in the list
        return NONE_ID;
    } else {
        return nextFreeObjectID++;  // Caution: Old value is returned but value is incremented afterwards
    }
}
