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

#include "FileClasses/xmidi/XMidiNoteStack.h"

void XMidiNoteStack::clear() {
    notes         = nullptr;
    polyphony     = 0;
    max_polyphony = 0;
}

XMidiEvent* XMidiNoteStack::PopTime(uint32_t time) {
    if (notes && notes->ex.note_on.note_time <= time) {
        XMidiEvent* note           = notes;
        notes                      = note->ex.note_on.next_note;
        note->ex.note_on.next_note = nullptr;
        polyphony--;
        return note;
    }

    return nullptr;
}

XMidiEvent* XMidiNoteStack::Pop() {
    if (notes) {
        XMidiEvent* note           = notes;
        notes                      = note->ex.note_on.next_note;
        note->ex.note_on.next_note = nullptr;
        polyphony--;
        return note;
    }

    return nullptr;
}

XMidiEvent* XMidiNoteStack::Remove(XMidiEvent* event) {
    XMidiEvent* prev = nullptr;
    XMidiEvent* note = notes;
    while (note) {

        if (note == event) {
            if (prev)
                prev->ex.note_on.next_note = note->ex.note_on.next_note;
            else
                notes = note->ex.note_on.next_note;
            note->ex.note_on.next_note = nullptr;
            polyphony--;
            return note;
        }
        prev = note;
        note = note->ex.note_on.next_note;
    }
    return nullptr;
}

XMidiEvent* XMidiNoteStack::FindAndPop(XMidiEvent* event) {

    XMidiEvent* prev = nullptr;
    XMidiEvent* note = notes;
    while (note) {

        if ((note->status & 0xF) == (event->status & 0xF) && note->data[0] == event->data[0]) {
            if (prev)
                prev->ex.note_on.next_note = note->ex.note_on.next_note;
            else
                notes = note->ex.note_on.next_note;
            note->ex.note_on.next_note = nullptr;
            polyphony--;
            return note;
        }
        prev = note;
        note = note->ex.note_on.next_note;
    }
    return nullptr;
}

void XMidiNoteStack::Push(XMidiEvent* event) {
    event->ex.note_on.next_note = notes;
    notes                       = event;
    polyphony++;
    if (max_polyphony < polyphony)
        max_polyphony = polyphony;
}

void XMidiNoteStack::Push(XMidiEvent* event, uint32_t time) {
    event->ex.note_on.note_time = time;
    event->ex.note_on.next_note = nullptr;

    polyphony++;
    if (max_polyphony < polyphony)
        max_polyphony = polyphony;

    if (!notes || time <= notes->ex.note_on.note_time) {
        event->ex.note_on.next_note = notes;
        notes                       = event;
    } else {
        XMidiEvent* prev = notes;
        while (prev) {
            XMidiEvent* note = prev->ex.note_on.next_note;

            if (!note || time <= note->ex.note_on.note_time) {
                event->ex.note_on.next_note = note;
                prev->ex.note_on.next_note  = event;
                return;
            }
            prev = note;
        }
    }
}

void XMidiNoteStack::SetAftertouch(XMidiEvent* event) const {

    // XMidiEvent *prev = 0;
    XMidiEvent* note = notes;
    while (note) {

        if ((note->status & 0xF) == (event->status & 0xF) && note->data[0] == event->data[0]) {
            note->ex.note_on.actualvel = event->data[1];
            return;
        }
        // prev = note;
        note = note->ex.note_on.next_note;
    }
}
