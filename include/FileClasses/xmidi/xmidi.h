/*
Copyright (C) 2000, 2001  Ryan Nunn

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

// XMIDI/MIDI Converter/Loader

#ifndef RANDGEN_XMIDI_H
#define RANDGEN_XMIDI_H

#include <cstring>
//#include <windows.h>
#include "databuf.h"

#define BOOL    int

// Conversion types for Midi files
#define XMIDI_CONVERT_NOCONVERSION  0
#define XMIDI_CONVERT_MT32_TO_GM    1
#define XMIDI_CONVERT_MT32_TO_GS    2
#define XMIDI_CONVERT_MT32_TO_GS127 3 // This one is broken so don't use
#define XMIDI_CONVERT_MT32_TO_GS127DRUM 4 // This one is broken so don't use
#define XMIDI_CONVERT_GS127_TO_GS   5

// Midi Status Bytes
#define MIDI_STATUS_NOTE_OFF        0x8
#define MIDI_STATUS_NOTE_ON     0x9
#define MIDI_STATUS_AFTERTOUCH      0xA
#define MIDI_STATUS_CONTROLLER      0xB
#define MIDI_STATUS_PROG_CHANGE     0xC
#define MIDI_STATUS_PRESSURE        0xD
#define MIDI_STATUS_PITCH_WHEEL     0xE
#define MIDI_STATUS_SYSEX       0xF

// XMIDI Controllers
#define XMIDI_CONTROLLER_FOR_LOOP   116
#define XMIDI_CONTROLLER_NEXT_BREAK 117

// Maximum number of for loops we'll allow
#define XMIDI_MAX_FOR_LOOP_COUNT    128


struct midi_event
{
    int     time{0};
    unsigned char   status{0};

    unsigned char   data[2];

    unsigned int    len{0};
    unsigned char   *buffer{nullptr};

    midi_event  *next{nullptr};

    midi_event()  
    = default;

    ~midi_event()
    {
        delete [] buffer;
        buffer = nullptr;
    }
};

class   XMIDI
{
public:
    struct  midi_descriptor
    {
        unsigned short  type;
        unsigned short  tracks;
    };

protected:
    midi_descriptor info;

private:
    midi_event      **events;
    signed short        *timing;

    midi_event      *list;
    midi_event      *current;

    const static char   mt32asgm[128];
    const static char   mt32asgs[256];
    BOOL            bank127[16];
    int         convert_type;
    BOOL            *fixed;

public:
    XMIDI(DataSource *source, int pconvert);
    ~XMIDI();

    int number_of_tracks()
    {
        if (info.type != 1)
            return info.tracks;
                    return 1;
    };

    // Retrieve it to a data source
    int retrieve (unsigned int track, DataSource *dest);

    // External Event list functions
    int retrieve (unsigned int track, midi_event **dest, int &ppqn);
    static void DeleteEventList (midi_event *mlist);

    // Not yet implimented
    // int apply_patch (int track, DataSource *source);

private:
    XMIDI(); // No default constructor

    // List manipulation
    void CreateNewEvent (int time);

    // Variable length quantity
    int GetVLQ (DataSource *source, unsigned int &quant);
    int GetVLQ2 (DataSource *source, unsigned int &quant);
    int PutVLQ(DataSource *dest, unsigned int value);

    void MovePatchVolAndPan (int channel = -1);
    void DuplicateAndMerge (int num = 0);

    int ConvertEvent (const int time, const unsigned char status, DataSource *source, const int size);
    int ConvertSystemMessage (const int time, const unsigned char status, DataSource *source);

    int ConvertFiletoList (DataSource *source, BOOL is_xmi);
    unsigned int ConvertListToMTrk (DataSource *dest, midi_event *mlist);

    int ExtractTracksFromXmi (DataSource *source);
    int ExtractTracksFromMid (DataSource *source);

    int ExtractTracks (DataSource *source);
};

#endif //RANDGEN_XMIDI_H
