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
#include <misc/SDL2pp.h>

#include <string>
#include <vector>

class ChangeEventList final {
public:
    class ChangeEvent final {
    public:
        enum class EventType { ChangeHouse, ChangeTeam, ChangePlayer, SetHumanPlayer };

        ChangeEvent(EventType eventType, uint32_t slot, uint32_t newValue);

        ChangeEvent(uint32_t slot, std::string newStringValue);

        explicit ChangeEvent(InputStream& stream);

        void save(OutputStream& stream) const;

        EventType eventType_;
        uint32_t slot_     = 0;
        uint32_t newValue_ = 0;
        std::string newStringValue_;
    };

    ChangeEventList();

    explicit ChangeEventList(InputStream& stream);

    ChangeEventList(const ChangeEventList&);
    ChangeEventList(ChangeEventList&&) noexcept;

    ~ChangeEventList();

    ChangeEventList& operator=(const ChangeEventList&);
    ChangeEventList& operator=(ChangeEventList&&) noexcept;

    void save(OutputStream& stream) const;

    std::vector<ChangeEvent> changeEventList_;
};

#endif // CHANGEEVENTLIST_H
