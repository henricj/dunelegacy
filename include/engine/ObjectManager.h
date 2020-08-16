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

#include "ObjectBase.h"
#include "data.h"

#include <dune_log.h>

#include <cassert>
#include <queue>
#include <unordered_map>

namespace Dune::Engine {

// forward declarations
class ObjectBase;

typedef std::unordered_map<uint32_t, std::unique_ptr<ObjectBase>> ObjectMap;

/// This class holds all objects (structures and units) in the game.
class ObjectManager final {
public:
    /**
        Default constructor
    */
    ObjectManager();

    ObjectManager(const ObjectManager&) = delete;
    ObjectManager(ObjectManager&&)      = delete;
    ObjectManager& operator=(const ObjectManager&) = delete;
    ObjectManager& operator=(ObjectManager&&) = delete;

    /**
        Default destructor
    */
    ~ObjectManager();

    /**
        Saves all objects to a stream
        \param  stream  Stream to save to
    */
    void save(const Game& game, OutputStream& stream) const;

    /**
        Loads all objects from a stream
        \param  stream  Stream to load from
    */
    void load(Game& game, InputStream& stream);

    /**
        This method searches for the object with ObjectID.
        \param  objectID        ID of the object to search for
        \return Pointer to this object (nullptr if not found)
    */
    [[nodiscard]] ObjectBase* getObject(uint32_t objectID) const {
        const auto iter = objectMap.find(objectID);

        if(iter == objectMap.end()) return nullptr;

        return iter->second.get();
    }

    /**
        This method searches for the object with ObjectID.
        \param  objectID        ID of the object to search for
        \return Pointer to this object (nullptr if not found)
    */
    template<typename ObjectType>
    [[nodiscard]] ObjectType* getObject(uint32_t objectID) const {
        static_assert(std::is_base_of<ObjectBase, ObjectType>::value, "ObjectType not derived from ObjectBase");

        return dune_cast<ObjectType>(getObject(objectID));
    }

    /**
        This method removes one object.
        \param  objectID        ID of the object to remove
        \return false if there was no object with this ObjectID, true if it could be removed
    */
    bool removeObject(uint32_t objectID) {
        const auto iter = objectMap.find(objectID);

        if(iter == objectMap.end()) return false;

        pendingDelete.push(std::move(iter->second));

        objectMap.erase(iter);

        return true;
    }

    template<typename F>
    void consume_pending_deletes(F&& f) {
        while(!pendingDelete.empty()) {
            auto object = std::move(pendingDelete.front());

            pendingDelete.pop();

            f(object);
        }
    }

    template<typename Visitor>
    void for_each(Visitor&& visitor) {
        for(auto& pair : objectMap) {
            assert(pair.first == pair.second->getObjectID());
            visitor(pair.second);
        }
    }

    template<typename ObjectType>
    ObjectType* createObjectFromItemId(ItemID_enum itemID, const ObjectInitializer& initializer) {
        static_assert(std::is_base_of<ObjectBase, ObjectType>::value, "ObjectType not derived from ObjectBase");

        auto object = ObjectBase::createObject(itemID, nextFreeObjectID, initializer);
        if(!object) {
            Dune::Logger.log("createObjectFromItemId() could not build item type %d", itemID);
            return nullptr;
        }

        auto* const pObject = dune_cast<ObjectType>(object.get());
        if(!pObject) {
            Dune::Logger.log("createObjectFromItemId() created the wrong type of object for build item type %d",
                             itemID);
            return nullptr;
        }

        if(!addObject(std::move(object))) {
            Dune::Logger.log("createObjectFromItemId() unable to add object of item type %d", itemID);
            return nullptr;
        }

        return pObject;
    }

    template<typename ObjectType>
    ObjectType* createObjectFromType(const ObjectInitializer& initializer) {
        static_assert(std::is_constructible<ObjectType, uint32_t, const ObjectInitializer&>::value,
                      "ObjectType is not constructible");
        static_assert(std::is_base_of<ObjectBase, ObjectType>::value, "ObjectType not derived from ObjectBase");

        return createObjectFromItemId<ObjectType>(ObjectType::item_id, initializer);
    }

private:
    /**
        This method adds one object. The ObjectID is chosen automatically.
        \param  pObject A pointer to the object.
        \return ObjectID of the added object.
    */
    bool                               addObject(std::unique_ptr<ObjectBase> pObject);
    static std::unique_ptr<ObjectBase> loadObject(Game& game, InputStream& stream, uint32_t objectID);

    uint32_t                                nextFreeObjectID = 1;
    ObjectMap                               objectMap;
    std::queue<std::unique_ptr<ObjectBase>> pendingDelete;
};

} // namespace Dune::Engine

#endif //OBJECTMANAGER_H
