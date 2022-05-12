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

#include "FileClasses/xmidi/XMidiEvent.h"

void XMidiEvent::FreeThis() {
    // Free all our children first. Using a loop instead of recursive
    // because it could have nasty effects on the stack otherwise
    for (auto* e = next; e; e = next) {
        next    = e->next;
        e->next = nullptr;
        e->FreeThis();
    }

    // We only do this with sysex
    if ((status >> 4) == 0xF && ex.sysex_data.buffer)
        delete[] ex.sysex_data.buffer;
    delete this;
}
