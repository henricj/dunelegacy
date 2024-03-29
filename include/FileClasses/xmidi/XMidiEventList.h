/*
Copyright (C) 2003  The Pentagram Team
Copyright (C) 2010-2022  The Exult Team

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*/

// Tab Size = 4

#ifndef XMIDIEVENTLIST_H_INCLUDED
#define XMIDIEVENTLIST_H_INCLUDED

class ODataSource;

#include "XMidiEvent.h"

class XMidiEventList final {
    int counter = 0;

    // Helper funcs for Write
    int putVLQ(ODataSource* dest, uint32_t value);
    uint32_t convertListToMTrk(ODataSource* dest);

public:
    uint16_t chan_mask;
    XMidiEvent* events{};

    //! XMidiFile Seq Branch Index controllers. For Ult126 = loop start
    XMidiEvent* branches{};

    //! Patch and Bank change events
    XMidiEvent* x_patch_bank{};

    //! Write the list to a DataSource
    int write(ODataSource* dest);

    //! Increments the counter
    void incrementCounter() { counter++; }

    //! Decrement the counter and delete the event list, if possible
    void decrementCounter();

    //! Find the Sequence Branch Event for the index
    //! \param index The index to search for
    //! \return The event found, or 0
    [[nodiscard]] XMidiEvent* findBranchEvent(int index) const {

        auto* b = branches;

        while (b) {
            if (b->data[1] == (index & 0xFF))
                break;
            b = b->ex.branch_index.next_branch;
        }

        return b;
    }
};

#endif // XMIDIEVENTLIST_H_INCLUDED
