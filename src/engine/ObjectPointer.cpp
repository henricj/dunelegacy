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

#include <ObjectPointer.h>

#include <Game.h>
#include <ObjectBase.h>

#include <units/UnitBase.h>
#include <structures/StructureBase.h>

namespace Dune::Engine {

ObjectBase* ObjectPointer::getObjPointer(const ObjectManager& objectManager) const {
    if(objectID == NONE_ID) { return nullptr; }

    auto* ObjPointer = objectManager.getObject(objectID);
    if(ObjPointer == nullptr) { objectID = NONE_ID; }

    return ObjPointer;
}

UnitBase* ObjectPointer::getUnitPointer(const ObjectManager& objectManager) const {
    return dune_cast<UnitBase>(getObjPointer(objectManager));
}

StructureBase* ObjectPointer::getStructurePointer(const ObjectManager& objectManager) const {
    return dune_cast<StructureBase>(getObjPointer(objectManager));
}

void ObjectPointer::save(OutputStream& stream) const { stream.writeUint32(objectID); }

void ObjectPointer::load(InputStream& stream) { pointTo(stream.readUint32()); }

void ObjectPointer::pointTo(const ObjectBase* newObject) {
    if(newObject != nullptr) {
        objectID = newObject->getObjectID();
    } else {
        objectID = NONE_ID;
    }
}

} // namespace Dune::Engine
