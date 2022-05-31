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

#include <Network/ChangeEventList.h>

#include <string>
#include <utility>

ChangeEventList::ChangeEvent::ChangeEvent(EventType eventType, uint32_t slot, uint32_t newValue)
    : eventType_(eventType), slot_(slot), newValue_(newValue) { }

ChangeEventList::ChangeEvent::ChangeEvent(uint32_t slot, std::string newStringValue)
    : eventType_(EventType::SetHumanPlayer), slot_(slot), newStringValue_(std::move(newStringValue)) { }

ChangeEventList::ChangeEvent::ChangeEvent(InputStream& stream) {
    eventType_ = static_cast<EventType>(stream.readUint32());
    slot_      = stream.readUint32();

    if (eventType_ == EventType::SetHumanPlayer) {
        newStringValue_ = stream.readString();
    } else {
        newValue_ = stream.readUint32();
    }
}

void ChangeEventList::ChangeEvent::save(OutputStream& stream) const {
    stream.writeUint32(static_cast<uint32_t>(eventType_));
    stream.writeUint32(slot_);
    if (eventType_ == EventType::SetHumanPlayer) {
        stream.writeString(newStringValue_);
    } else {
        stream.writeUint32(newValue_);
    }
}

ChangeEventList::ChangeEventList() = default;

ChangeEventList::ChangeEventList(InputStream& stream) {
    const auto numChangeEvents = stream.readUint32();
    for (auto i = 0U; i < numChangeEvents; ++i) {
        changeEventList_.emplace_back(stream);
    }
}

ChangeEventList::ChangeEventList(const ChangeEventList&)     = default;
ChangeEventList::ChangeEventList(ChangeEventList&&) noexcept = default;

ChangeEventList::~ChangeEventList() = default;

ChangeEventList& ChangeEventList::operator=(const ChangeEventList&)     = default;
ChangeEventList& ChangeEventList::operator=(ChangeEventList&&) noexcept = default;

void ChangeEventList::save(OutputStream& stream) const {
    stream.writeUint32(changeEventList_.size());
    for (const auto& changeEvent : changeEventList_) {
        changeEvent.save(stream);
    }
}
