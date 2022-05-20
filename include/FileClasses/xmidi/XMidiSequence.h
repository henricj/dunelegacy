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

#ifndef XMIDISEQUENCE_H_INCLUDED
#define XMIDISEQUENCE_H_INCLUDED

#include "XMidiEvent.h"
#include "XMidiEventList.h"
#include "XMidiNoteStack.h"
#include "XMidiSequenceHandler.h"

class XMidiSequence {
public:
    //! Create the XXMidiSequence object
    //! \param handler XMidiSequenceHandler object that the events are to be sent to
    //! \param sequence_id The id num for this sequence
    //! \param events The Midi event list that is to be played
    //! \param repeat Specifies if repeating is enabled or disabled
    //! \param volume Volume level to play at
    //! \param branch The Branch index to start playing at
    XMidiSequence(XMidiSequenceHandler* handler, uint16_t seq_id, XMidiEventList* events, bool repeat, int volume,
                  int branch);

    //! Destructor
    ~XMidiSequence();

    //! Play a single waiting event
    //! \return <0 if there is no more events
    //! \return 0 if there are pending events that can be played imediately
    //! \return >0 if there are pending events in the future
    int playEvent();

    //! Get the time till the next event
    //! \return ms till next event (can be negative if running behind time).
    int32_t timeTillNext();

    //! Set a new volume to use
    //! \param new_vol the new volume level (0-255)
    void setVolume(int new_vol);

    //! Get the current volume level
    //! \return the current volume level (0-255)
    int getVolume() { return vol_multi; }

    //! Set the speed of playback
    //! \param new_speed the new playback speed (percentage)
    void setSpeed(int new_speed) { speed = new_speed; }

    //! Called when the XMidiSequenceHandler is taking a channel from this sequence
    //! The XMidiSequence will gracefully reset the state of the channel
    //! \param c the channel being lost
    void loseChannel(int i);

    //! Called when the XMidiSequenceHandler is giving a channel to this sequence
    //! The XMidiSequence will update the state of the channel to what is stored in the shadow
    //! \param c the channel being gained
    void gainChannel(int i);

    //! Apply a shadow state
    //! \param c the channel to apply the shadow for
    void applyShadow(int i);

    //! Get the channel used mask
    //! \return the mask of used channels
    uint16_t getChanMask() { return evntlist->chan_mask; }

    //! Pause the sequence
    void pause();

    //! Unpause the sequence
    void unpause();

    //! Is the sequence paused
    bool isPaused() { return paused; }

    //! Count the number of notes on for a chan
    //! \param chan The channel to count for
    //! \return the number of notes on
    int countNotesOn(int chan);

private:
    XMidiSequenceHandler* handler; //!< The handler the events are sent to
    uint16_t sequence_id;          //!< The sequence id of this sequence
    XMidiEventList* evntlist;      //!< The Midi event list that is being played
    XMidiEvent* event;             //!< The next event to be played
    bool repeat;                   //!< Specifies if repeating is enabled
    XMidiNoteStack notes_on;       //!< Note stack of notes currently playing, and time to stop
    uint32_t last_tick;            //!< The tick of the previously played notes
    uint32_t start;                //!< XMidi Clock (in 1/6000 seconds)
    int loop_num;                  //!< The level of the loop we are currently in
    int vol_multi;                 //!< Volume multiplier (0-255)
    bool paused;                   //!< Is the sequence paused
    int speed;                     //!< Percentage of speed to playback at

    //! The for loop event that triggered the loop per level
    XMidiEvent* loop_event[XMIDI_MAX_FOR_LOOP_COUNT];

    //! The amount of times we have left that we can loop per level
    int loop_count[XMIDI_MAX_FOR_LOOP_COUNT];

    struct ChannelShadow {
        static const uint16_t centre_value;
        static const uint8_t fine_value;
        static const uint8_t coarse_value;
        static const uint16_t combined_value;

        int pitchWheel;
        int program;

        // Controllers
        int bank[2];       // 0
        int modWheel[2];   // 1
        int footpedal[2];  // 4
        int volumes[2];    // 7
        int pan[2];        // 9
        int balance[2];    // 10
        int expression[2]; // 11
        int sustain;       // 64
        int effects;       // 91
        int chorus;        // 93

        int xbank;
        void reset();
    } shadows[16];

    //! Send the current event to the XMidiSequenceHandler
    void sendEvent();

    //! Update the shadows for an event
    void updateShadowForEvent(XMidiEvent* event);

    //! Initialize the XMidi clock to begin now
    inline void initClock() { start = handler->getTickCount(sequence_id); }

    //! Add an offset to the XMidi clock
    inline void addOffset(uint32_t offset) { start += offset; }

    //! Get the current time of the XMidi clock
    inline uint32_t getTime() { return handler->getTickCount(sequence_id) - start; }

    //! Get the start time of the XMidi clock
    inline uint32_t getStart() { return start; }

    //! Get the real time of the XMidi clock
    inline uint32_t getRealTime() { return handler->getTickCount(sequence_id); }
};

#endif // XMIDISEQUENCE_H_INCLUDED
