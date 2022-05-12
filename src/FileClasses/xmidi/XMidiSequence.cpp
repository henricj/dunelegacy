/*
Copyright (C) 2003-2005  The Pentagram Team
Copyright (C) 2007-2022  The Exult Team

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

#include "FileClasses/xmidi/XMidiSequence.h"

#include "FileClasses/xmidi/XMidiSequenceHandler.h"

namespace {
// Define this to stop the Midisequencer from attempting to
// catch up time if it has missed over 1200 ticks or 1/5th of a second
// This is useful for when debugging, since the Sequencer will not attempt
// to play hundreds of events at the same time if execution is broken, and
// later resumed.
inline constexpr auto XMIDISEQUENCER_NO_CATCHUP_WAIT_OVER = 1200;
} // namespace

constexpr uint16_t XMidiSequence::ChannelShadow::centre_value   = 0x2000;
constexpr uint8_t XMidiSequence::ChannelShadow::fine_value      = centre_value & 127;
constexpr uint8_t XMidiSequence::ChannelShadow::coarse_value    = centre_value >> 7;
constexpr uint16_t XMidiSequence::ChannelShadow::combined_value = (coarse_value << 8) | fine_value;

XMidiSequence::XMidiSequence(XMidiSequenceHandler* Handler, uint16_t seq_id, XMidiEventList* events, bool Repeat,
                             int volume, int branch)
    : handler(Handler), sequence_id(seq_id), evntlist(events), repeat(Repeat), vol_multi(volume) {
    event_ = evntlist->events;

    for (int i = 0; i < 16; i++) {
        shadows[i].reset();
        const auto message = i | (MIDI_STATUS_CONTROLLER << 4) | (XMIDI_CONTROLLER_BANK_CHANGE << 8);
        handler->sequenceSendEvent(sequence_id, message);
    }

    // Jump to branch
    if (auto* const brnch = events->findBranchEvent(branch)) {
        last_tick = brnch->time;
        event_    = brnch;

        for (auto* event = evntlist->events; event != brnch; event = event->next)
            updateShadowForEvent(event);

        for (int i = 0; i < 16; i++)
            gainChannel(i);
    }

    // initClock();
    start = 0xFFFFFFFF;
}

XMidiSequence::~XMidiSequence() {
    // Handle note off's here
    while (const auto* const note = notes_on.Pop())
        handler->sequenceSendEvent(sequence_id, note->status + (note->data[0] << 8));

    for (int i = 0; i < 16; i++) {
        shadows[i].reset();
        applyShadow(i);
    }

    // 'Release' it
    evntlist->decrementCounter();
}

void XMidiSequence::ChannelShadow::reset() {
    pitchWheel = combined_value;

    program = -1;

    // Bank Select
    bank[0] = 0;
    bank[1] = 0;

    // Modulation Wheel
    modWheel[0] = coarse_value;
    modWheel[1] = fine_value;

    // Foot pedal
    footpedal[0] = 0;
    footpedal[1] = 0;

    // Volume
    volumes[0] = coarse_value;
    volumes[1] = fine_value;

    // Pan
    pan[0] = coarse_value;
    pan[1] = fine_value;

    // Balance
    balance[0] = coarse_value;
    balance[1] = fine_value;

    // Expression
    expression[0] = 127;
    expression[1] = 0;

    // sustain
    sustain = 0;

    // Effects (Reverb)
    effects = 0;

    // Chorus
    chorus = 0;

    // Xmidi Bank
    xbank = 0;
}

int XMidiSequence::playEvent() {
    if (start == 0xFFFFFFFF)
        initClock();

    // Handle note off's here
    while (const auto* const note = notes_on.PopTime(getRealTime()))
        handler->sequenceSendEvent(sequence_id, note->status + (note->data[0] << 8));

    // No events left, but we still have notes on, so say we are still playing, if not report we've finished
    if (!event_) {
        return notes_on.GetNotes() ? 1 : -1;
    }

    // Effectively paused, so indicate it
    if (speed <= 0 || paused)
        return 1;

    // Play all waiting notes;
    int32_t aim  = ((event_->time - last_tick) * 5000) / speed;
    int32_t diff = aim - getTime();

    if (diff > 5)
        return 1;

    addOffset(aim);

    last_tick = event_->time;

    if constexpr (XMIDISEQUENCER_NO_CATCHUP_WAIT_OVER > 0) {
        if (diff < -XMIDISEQUENCER_NO_CATCHUP_WAIT_OVER)
            addOffset(-diff);
    }

    // Handle note off's here too
    while (const auto* const note = notes_on.PopTime(getRealTime()))
        handler->sequenceSendEvent(sequence_id, note->status + (note->data[0] << 8));

    // XMidi For Loop
    if ((event_->status >> 4) == MIDI_STATUS_CONTROLLER && event_->data[0] == XMIDI_CONTROLLER_FOR_LOOP) {
        if (loop_num < XMIDI_MAX_FOR_LOOP_COUNT - 1)
            loop_num++;
        else
            THROW(std::runtime_error, "XMIDI: Exceeding maximum loop count");

        loop_count[loop_num] = event_->data[1];
        loop_event[loop_num] = event_;

    } // XMidi Next/Break
    else if ((event_->status >> 4) == MIDI_STATUS_CONTROLLER && event_->data[0] == XMIDI_CONTROLLER_NEXT_BREAK) {
        if (loop_num != -1) {
            if (event_->data[1] < 64) {
                loop_num--;
            }
        } else {
            // See if we can find the branch index
            // If we can, jump to that
            XMidiEvent* branch = evntlist->findBranchEvent(126);

            if (branch) {
                loop_num             = 0;
                loop_count[loop_num] = 1;
                loop_event[loop_num] = branch;
            }
        }
        event_ = nullptr;
    } // XMidi Callback Trigger
    else if ((event_->status >> 4) == MIDI_STATUS_CONTROLLER && event_->data[0] == XMIDI_CONTROLLER_CALLBACK_TRIG) {
        handler->handleCallbackTrigger(sequence_id, event_->data[1]);
    } // Not SysEx
    else if (event_->status < 0xF0) {
        sendEvent();
    }
    // SysEx gets sent immediately
    else if (event_->status != 0xFF) {
        handler->sequenceSendSysEx(sequence_id, event_->status, event_->ex.sysex_data.buffer,
                                   event_->ex.sysex_data.len);
    }

    // If we've got another note, play that next
    if (event_)
        event_ = event_->next;

    // Now, handle what to do when we are at the end
    if (!event_) {
        // If we have for loop events, follow them
        if (loop_num != -1) {
            event_    = loop_event[loop_num]->next;
            last_tick = loop_event[loop_num]->time;

            if (loop_count[loop_num])
                if (!--loop_count[loop_num])
                    loop_num--;
        }
        // Or, if we are repeating, but there hasn't been any for loop events,
        // repeat from the start
        else if (repeat) {
            event_ = evntlist->events;
            if (last_tick == 0)
                return 1;
            last_tick = 0;
        }
        // If we are not repeating, then return saying we are end
        else {
            if (notes_on.GetNotes())
                return 1;
            return -1;
        }
    }

    if (!event_) {
        if (notes_on.GetNotes())
            return 1;
        else
            return -1;
    }

    aim  = ((event_->time - last_tick) * 5000) / speed;
    diff = aim - getTime();

    if (diff < 0)
        return 0; // Next event is ready now!
    return 1;
}

int32_t XMidiSequence::timeTillNext() {
    int32_t sixthoToNext = 0x7FFFFFFF; // Int max

    // Time remaining on notes currently being played
    if (const auto* const note = notes_on.GetNotes()) {
        const int32_t diff = note->ex.note_on.note_time - getRealTime();
        if (diff < sixthoToNext)
            sixthoToNext = diff;
    }

    // Time till the next event, if we are playing
    if (speed > 0 && event_ && !paused) {
        const int32_t aim  = ((event_->time - last_tick) * 5000) / speed;
        const int32_t diff = aim - getTime();

        if (diff < sixthoToNext)
            sixthoToNext = diff;
    }
    return sixthoToNext / 6;
}

void XMidiSequence::updateShadowForEvent(XMidiEvent* event) {
    const unsigned int chan = event->status & 0xF;
    const unsigned int type = event->status >> 4;
    const uint32_t data     = event->data[0] | (event->data[1] << 8);

    // Shouldn't be required. XMidi should automatically detect all anyway
    // evntlist->chan_mask |= 1 << chan;

    // Update the shadows here

    auto& shadow = shadows[chan];

    if (type == MIDI_STATUS_CONTROLLER) {
        // Channel volume
        if (event->data[0] == 7) {
            shadow.volumes[0] = event->data[1];
        } else if (event->data[0] == 39) {
            shadow.volumes[1] = event->data[1];
        }
        // Bank
        else if (event->data[0] == 0 || event->data[0] == 32) {
            shadow.bank[event->data[0] / 32] = event->data[1];
        }
        // modWheel
        else if (event->data[0] == 1 || event->data[0] == 33) {
            shadow.modWheel[event->data[0] / 32] = event->data[1];
        }
        // footpedal
        else if (event->data[0] == 4 || event->data[0] == 36) {
            shadow.footpedal[event->data[0] / 32] = event->data[1];
        }
        // pan
        else if (event->data[0] == 9 || event->data[0] == 41) {
            shadow.pan[event->data[0] / 32] = event->data[1];
        }
        // balance
        else if (event->data[0] == 10 || event->data[0] == 42) {
            shadow.balance[event->data[0] / 32] = event->data[1];
        }
        // expression
        else if (event->data[0] == 11 || event->data[0] == 43) {
            shadow.expression[event->data[0] / 32] = event->data[1];
        }
        // sustain
        else if (event->data[0] == 64) {
            shadow.effects = event->data[1];
        }
        // effect
        else if (event->data[0] == 91) {
            shadow.effects = event->data[1];
        }
        // chorus
        else if (event->data[0] == 93) {
            shadow.chorus = event->data[1];
        }
        // XMidi bank
        else if (event->data[0] == XMIDI_CONTROLLER_BANK_CHANGE) {
            shadow.xbank = event->data[1];
        }
    } else if (type == MIDI_STATUS_PROG_CHANGE) {
        shadow.program = data;
    } else if (type == MIDI_STATUS_PITCH_WHEEL) {
        shadow.pitchWheel = data;
    }
}

void XMidiSequence::sendEvent() {
    // unsigned int chan = event->status & 0xF;
    const unsigned int type = event_->status >> 4;
    uint32_t data           = event_->data[0] | (event_->data[1] << 8);

    // Shouldn't be required. XMidi should automatically detect all anyway
    // evntlist->chan_mask |= 1 << chan;

    // Update the shadows here
    updateShadowForEvent(event_);

    if (type == MIDI_STATUS_CONTROLLER) {
        // Channel volume
        if (event_->data[0] == 7) {
            data = event_->data[0] | (((event_->data[1] * vol_multi) / 0xFF) << 8);
        }
    } else if (type == MIDI_STATUS_AFTERTOUCH) {
        notes_on.SetAftertouch(event_);
    }

    if ((type != MIDI_STATUS_NOTE_ON || event_->data[1]) && type != MIDI_STATUS_NOTE_OFF) {

        if (type == MIDI_STATUS_NOTE_ON) {

            if (!event_->data[1])
                return;

            notes_on.Remove(event_);

            handler->sequenceSendEvent(sequence_id, event_->status | (data << 8));
            event_->ex.note_on.actualvel = event_->data[1];

            notes_on.Push(event_, ((event_->ex.note_on.duration - 1) * 5000 / speed) + getStart());
        }
        // Only send IF the channel has been marked enabled
        else
            handler->sequenceSendEvent(sequence_id, event_->status | (data << 8));
    }
}

#define SendController(ctrl, name)                                                                                     \
    handler->sequenceSendEvent(sequence_id,                                                                            \
                               i | (MIDI_STATUS_CONTROLLER << 4) | ((ctrl) << 8) | (shadows[i].name[0] << 16));        \
    handler->sequenceSendEvent(sequence_id,                                                                            \
                               i | (MIDI_STATUS_CONTROLLER << 4) | (((ctrl) + 32) << 8) | (shadows[i].name[1] << 16));

void XMidiSequence::applyShadow(int i) {
    // Pitch Wheel
    handler->sequenceSendEvent(sequence_id, i | (MIDI_STATUS_PITCH_WHEEL << 4) | (shadows[i].pitchWheel << 8));

    // Modulation Wheel
    SendController(1, modWheel);

    // Footpedal
    SendController(4, footpedal);

    // Volume
    handler->sequenceSendEvent(sequence_id, i | (MIDI_STATUS_CONTROLLER << 4) | (7 << 8)
                                                | (((shadows[i].volumes[0] * vol_multi) / 0xFF) << 16));
    handler->sequenceSendEvent(sequence_id,
                               i | (MIDI_STATUS_CONTROLLER << 4) | (39 << 8) | (shadows[i].volumes[1] << 16));

    // Pan
    SendController(9, pan);

    // Balance
    SendController(10, balance);

    // expression
    SendController(11, expression);

    // Sustain
    handler->sequenceSendEvent(sequence_id, i | (MIDI_STATUS_CONTROLLER << 4) | (64 << 8) | (shadows[i].sustain << 16));

    // Effects (Reverb)
    handler->sequenceSendEvent(sequence_id, i | (MIDI_STATUS_CONTROLLER << 4) | (91 << 8) | (shadows[i].effects << 16));

    // Chorus
    handler->sequenceSendEvent(sequence_id, i | (MIDI_STATUS_CONTROLLER << 4) | (93 << 8) | (shadows[i].chorus << 16));

    // XMidi Bank
    handler->sequenceSendEvent(sequence_id, i | (MIDI_STATUS_CONTROLLER << 4) | (XMIDI_CONTROLLER_BANK_CHANGE << 8)
                                                | (shadows[i].xbank << 16));

    // Bank Select
    if (shadows[i].program != -1)
        handler->sequenceSendEvent(sequence_id, i | (MIDI_STATUS_PROG_CHANGE << 4) | (shadows[i].program << 8));
    SendController(0, bank);
}

void XMidiSequence::setVolume(int new_volume) {
    vol_multi = new_volume;

    // Only update used channels
    for (int i = 0; i < 16; i++)
        if (evntlist->chan_mask & (1 << i)) {
            uint32_t message = i;
            message |= MIDI_STATUS_CONTROLLER << 4;
            message |= 7 << 8;
            message |= ((shadows[i].volumes[0] * vol_multi) / 0xFF) << 16;
            handler->sequenceSendEvent(sequence_id, message);
        }
}

void XMidiSequence::loseChannel(int i) {
    // If the channel matches, send a note off for any note
    const XMidiEvent* note = notes_on.GetNotes();
    while (note) {
        if ((note->status & 0xF) == i)
            handler->sequenceSendEvent(sequence_id, note->status + (note->data[0] << 8));
        note = note->ex.note_on.next_note;
    }
}

void XMidiSequence::gainChannel(int i) {
    applyShadow(i);

    // If the channel matches, send a note on for any note
    const XMidiEvent* note = notes_on.GetNotes();
    while (note) {
        if ((note->status & 0xF) == i)
            handler->sequenceSendEvent(sequence_id,
                                       note->status | (note->data[0] << 8) | (note->ex.note_on.actualvel << 16));
        note = note->ex.note_on.next_note;
    }
}

void XMidiSequence::pause() {
    paused = true;
    for (int i = 0; i < 16; i++)
        if (evntlist->chan_mask & (1 << i))
            loseChannel(i);
}

void XMidiSequence::unpause() {
    paused = false;
    for (int i = 0; i < 16; i++)
        if (evntlist->chan_mask & (1 << i))
            applyShadow(i);
}

int XMidiSequence::countNotesOn(int chan) {
    if (paused)
        return 0;

    int ret = 0;

    for (const XMidiEvent* note = notes_on.GetNotes(); note; note = note->ex.note_on.next_note) {
        if ((note->status & 0xF) == chan)
            ret++;
    }
    return ret;
}

XMidiSequenceHandler::~XMidiSequenceHandler() = default;

// Protection
