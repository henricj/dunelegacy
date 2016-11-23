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

#ifndef CHANGEEVENTLIST_H
#define CHANGEEVENTLIST_H

#include <misc/InputStream.h>
#include <misc/OutputStream.h>

#include <SDL.h>
#include <list>

class ChangeEventList {
public:
    class ChangeEvent {
    public:
        enum EventType {
            EventType_ChangeHouse,
            EventType_ChangeTeam,
            EventType_ChangePlayer,
            EventType_SetHumanPlayer
        };


        ChangeEvent(EventType eventType, Uint32 slot, Uint32 newValue)
         : eventType(eventType), slot(slot), newValue(newValue) {

        }

        ChangeEvent(Uint32 slot, std::string newStringValue)
         : eventType(EventType_SetHumanPlayer), slot(slot), newStringValue(newStringValue) {
        }

        explicit ChangeEvent(InputStream& stream) {
            eventType = (EventType) stream.readUint32();
            slot = stream.readUint32();

            if(eventType == EventType_SetHumanPlayer) {
                newStringValue = stream.readString();
            } else {
                newValue = stream.readUint32();
            }
        }

        void save(OutputStream& stream) const {
            stream.writeUint32((Uint32) eventType);
            stream.writeUint32(slot);
            if(eventType == EventType_SetHumanPlayer) {
                stream.writeString(newStringValue);
            } else {
                stream.writeUint32(newValue);
            }
        }

        EventType   eventType;
        Uint32      slot;
        Uint32      newValue;
        std::string newStringValue;
    };

    ChangeEventList() {
    }

    explicit ChangeEventList(InputStream& stream) {
        Uint32 numChangeEvents = stream.readUint32();
        for(Uint32 i = 0; i < numChangeEvents; i++) {
            changeEventList.push_back(ChangeEvent(stream));
        }
    }

    ~ChangeEventList() {

    }

    void save(OutputStream& stream) const {
        stream.writeUint32(changeEventList.size());
        for(const ChangeEvent& changeEvent : changeEventList) {
            changeEvent.save(stream);
        }
    }

    std::list<ChangeEvent> changeEventList;
};

#endif //CHANGEEVENTLIST_H
