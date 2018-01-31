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

#ifndef OBJECTMANAGER_H
#define OBJECTMANAGER_H

#include <misc/InputStream.h>
#include <misc/OutputStream.h>
#include <misc/SDL2pp.h>

#include <map>

// forward declarations
class ObjectBase;

typedef std::unordered_map<Uint32,ObjectBase*> ObjectMap;

/// This class holds all objects (structures and units) in the game.
class ObjectManager {
public:
    /**
        Default constructor
    */
    ObjectManager() : nextFreeObjectID(1)
    {
    }

    ObjectManager(const ObjectManager &) = delete;
    ObjectManager(ObjectManager &&) = delete;
    ObjectManager& operator=(const ObjectManager &) = delete;
    ObjectManager& operator=(ObjectManager &&) = delete;

    /**
        Default destructor
    */
    ~ObjectManager() = default;


    /**
        Saves all objects to a stream
        \param  stream  Stream to save to
    */
    void save(OutputStream& stream) const;

    /**
        Loads all objects from a stream
        \param  stream  Stream to load from
    */
    void load(InputStream& stream);

    /**
        This method adds one object. The ObjectID is choosen automatically.
        \param  pObject A pointer to the object.
        \return ObjectID of the added object.
    */
    Uint32 addObject(ObjectBase* pObject);

    /**
        This method searches for the object with ObjectID.
        \param  objectID        ID of the object to search for
        \return Pointer to this object (nullptr if not found)
    */
    ObjectBase* getObject(Uint32 objectID) const {
        const auto iter = objectMap.find(objectID);

        if(iter == objectMap.end()) {
            return nullptr;
        }
  
        return iter->second;
    }

    /**
        This method removes one object.
        \param  objectID        ID of the object to remove
        \return false if there was no object with this ObjectID, true if it could be removed
    */
    bool removeObject(Uint32 objectID) {
        return (objectMap.erase(objectID) != 0);
    }

    template<typename Visitor>
    void for_each(Visitor&& visitor) {
        for (auto& pair : objectMap) {
            assert(pair.first == pair.second->getObjectID());
            visitor(pair.second);
        }
    }
private:
    Uint32 nextFreeObjectID;
    ObjectMap objectMap;
};

#endif //OBJECTMANAGER_H
