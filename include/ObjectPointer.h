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

#ifndef OBJECTPOINTER_H
#define OBJECTPOINTER_H

#include <Definitions.h>
#include <misc/InputStream.h>
#include <misc/OutputStream.h>

class ObjectBase;
class UnitBase;
class StructureBase;

class ObjectPointer
{
public:
    ObjectPointer() { objectID = NONE_ID; };
    explicit ObjectPointer(Uint32 newItemID) : objectID(newItemID) { };
    ObjectPointer(const ObjectBase* newObject) { pointTo(newObject); };
    ~ObjectPointer() = default;

    ObjectPointer(const ObjectPointer &) = default;
    ObjectPointer(ObjectPointer &&) = default;
    ObjectPointer& operator=(const ObjectPointer &) = default;
    ObjectPointer& operator=(ObjectPointer &&) = default;

    void pointTo(Uint32 newItemID) { objectID = newItemID; };
    void pointTo(const ObjectBase* newObject);

    Uint32 getObjectID() const noexcept { return objectID; };
    ObjectBase* getObjPointer() const;
    UnitBase* getUnitPointer() const;
    StructureBase* getStructurePointer() const;

    void save(OutputStream& stream) const;
    void load(InputStream& stream);

    operator bool() const {
        return (objectID != NONE_ID);
    };

private:
    mutable Uint32 objectID;
};


#endif // OBJECTPOINTER_H
