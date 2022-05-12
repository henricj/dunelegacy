/*
Copyright (C) 2003  The Pentagram Team
Copyright (C) 2013-2022  The Exult Team

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

#ifndef XMIDINOTESTACK_H_INCLUDED
#define XMIDINOTESTACK_H_INCLUDED

#include "XMidiEvent.h"

class XMidiNoteStack final {
    XMidiEvent* notes = nullptr; // Top of the stack
    int polyphony     = 0;
    int max_polyphony = 0;

public:
    // Just clear it. Don't care about what's actually in it
    void clear();

    // Pops the top of the stack if its off_time is <= time (6000th of second)
    XMidiEvent* PopTime(uint32_t time);

    // Pops the top of the stack
    XMidiEvent* Pop();

    // Pops the top of the stack
    XMidiEvent* Remove(XMidiEvent* event);

    // Finds the note that has same pitch and channel, and pops it
    XMidiEvent* FindAndPop(XMidiEvent* event);

    // Pushes a note onto the top of the stack
    void Push(XMidiEvent* event);

    void Push(XMidiEvent* event, uint32_t time);

    // Finds the note that has same pitch and channel, and sets its after touch to our velocity
    void SetAftertouch(XMidiEvent* event) const;

    [[nodiscard]] XMidiEvent* GetNotes() const { return notes; }

    [[nodiscard]] int GetPolyphony() const { return polyphony; }

    [[nodiscard]] int GetMaxPolyphony() const { return max_polyphony; }
};

#endif // XMIDINOTESTACK_H_INCLUDED
