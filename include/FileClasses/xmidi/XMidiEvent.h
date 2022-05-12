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

#ifndef XMIDIEVENT_H_INCLUDED
#define XMIDIEVENT_H_INCLUDED

#include <array>

// Midi Status Bytes
inline static constexpr auto MIDI_STATUS_NOTE_OFF    = 0x8;
inline static constexpr auto MIDI_STATUS_NOTE_ON     = 0x9;
inline static constexpr auto MIDI_STATUS_AFTERTOUCH  = 0xA;
inline static constexpr auto MIDI_STATUS_CONTROLLER  = 0xB;
inline static constexpr auto MIDI_STATUS_PROG_CHANGE = 0xC;
inline static constexpr auto MIDI_STATUS_PRESSURE    = 0xD;
inline static constexpr auto MIDI_STATUS_PITCH_WHEEL = 0xE;
inline static constexpr auto MIDI_STATUS_SYSEX       = 0xF;

//
// XMidiFile Controllers
//

//
// Channel Lock	(110)
//  < 64 : Release Lock
// >= 64 : Lock an unlocked unprotected physical channel to be used exclusively
//         by this logical channel. Traditionally the physical channel would be
//         between 1 and 9, and the logical channel between 11 and 16
//
// When a channel is locked, any notes already playing on it are turned off.
// When the lock is released, the previous state of the channel is restored.
// Locks are automatically released when the sequences finishes playing
//
inline static constexpr auto XMIDI_CONTROLLER_CHAN_LOCK = 0x6e;

inline static constexpr auto XMIDI_CONTROLLER_CHAN_LOCK_PROT   = 0x6f; // Channel Lock Protect
inline static constexpr auto XMIDI_CONTROLLER_VOICE_PROT       = 0x70; // Voice Protect
inline static constexpr auto XMIDI_CONTROLLER_TIMBRE_PROT      = 0x71; // Timbre Protect
inline static constexpr auto XMIDI_CONTROLLER_BANK_CHANGE      = 0x72; // Bank Change
inline static constexpr auto XMIDI_CONTROLLER_IND_CTRL_PREFIX  = 0x73; // Indirect Controller Prefix
inline static constexpr auto XMIDI_CONTROLLER_FOR_LOOP         = 0x74; // For Loop
inline static constexpr auto XMIDI_CONTROLLER_NEXT_BREAK       = 0x75; // Next/Break
inline static constexpr auto XMIDI_CONTROLLER_CLEAR_BB_COUNT   = 0x76; // Clear Beat/Bar Count
inline static constexpr auto XMIDI_CONTROLLER_CALLBACK_TRIG    = 0x77; // Callback Trigger
inline static constexpr auto XMIDI_CONTROLLER_SEQ_BRANCH_INDEX = 0x78; // Sequence Branch Index

// Maximum number of for loops we'll allow (used by LowLevelMidiDriver)
// The specs say 4, so that is what we;ll use
inline static constexpr auto XMIDI_MAX_FOR_LOOP_COUNT = 4;

struct XMidiEvent final {
    int time;
    unsigned char status;

    std::array<unsigned char, 2> data;

    union {
        struct {
            uint32_t len;          // Length of SysEx Data
            unsigned char* buffer; // SysEx Data
        } sysex_data;

        struct {
            int duration;          // Duration of note (120 Hz)
            XMidiEvent* next_note; // The next note on the stack
            uint32_t note_time;    // Time note stops playing (6000th of second)
            uint8_t actualvel;     // Actual velocity of playing note
        } note_on;

        struct {
            XMidiEvent* next_branch; // Next branch index controller
        } branch_index;

    } ex;

    XMidiEvent* next;

    XMidiEvent* next_patch_bank; // next patch or bank change event

    void FreeThis();
};

#endif // XMIDIEVENT_H_INCLUDED
