/*
Copyright (C) 2003-2005  The Pentagram Team
Copyright (C) 2009-2022  The Exult Team

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

#ifndef XMIDIFILE_H_INCLUDED
#define XMIDIFILE_H_INCLUDED

#include "gamma.h"

class IDataSource;
class OIDataSource;
struct XMidiEvent;
class XMidiEventList;

// Conversion types for Midi files
#define XMIDIFILE_CONVERT_NOCONVERSION  4
#define XMIDIFILE_CONVERT_MT32_TO_GM    1
#define XMIDIFILE_CONVERT_MT32_TO_GS    2
#define XMIDIFILE_CONVERT_MT32_TO_GS127 3
#define XMIDIFILE_CONVERT_GM_TO_MT32    0

#define XMIDIFILE_CONVERT_GS127_TO_GS 5
#define XMIDIFILE_HINT_XMIDI_MT_FILE  7
#define XMIDIFILE_HINT_SYX_FILE       8
#define XMIDIFILE_HINT_SYSEX_IN_MID   9

class XMidiFile {
protected:
    uint16_t num_tracks;

private:
    XMidiEventList** events;

    XMidiEvent* list;
    XMidiEvent* branches;
    XMidiEvent* current;
    XMidiEvent* x_patch_bank_cur;
    XMidiEvent* x_patch_bank_first;

    const static char mt32asgm[128];
    const static char mt32asgs[256];
    const static char gmasmt32[128];
    bool bank127[16];
    int convert_type;

    bool do_reverb;
    bool do_chorus;
    int chorus_value;
    int reverb_value;

    // Midi Volume Curve Modification
    static GammaTable<unsigned char> VolumeCurve;

public:
    XMidiFile() = delete; // No default constructor
    XMidiFile(IDataSource* source, int pconvert);
    ~XMidiFile();

    int number_of_tracks() { return num_tracks; }

    // External Event list functions
    XMidiEventList* GetEventList(uint32_t track);
    XMidiEventList* StealEventList() {
        XMidiEventList* tmp = GetEventList(0);
        events              = nullptr;
        return tmp;
    }

    // Not yet implemented
    // int apply_patch (int track, DataSource *source);

private:
    struct first_state {       // Status,	Data[0]
        XMidiEvent* patch[16]; // 0xC
        XMidiEvent* bank[16];  // 0xB,		0
        XMidiEvent* pan[16];   // 0xB,		7
        XMidiEvent* vol[16];   // 0xB,		10
    };

    // List manipulation
    void CreateNewEvent(int time);

    // Variable length quantity
    int GetVLQ(IDataSource* source, uint32_t& quant);
    int GetVLQ2(IDataSource* source, uint32_t& quant);

    void AdjustTimings(uint32_t ppqn); // This is used by Midi's ONLY!
    void ApplyFirstState(first_state& fs, int chan_mask);

    int ConvertNote(const int time, const unsigned char status, IDataSource* source, const int size);
    int ConvertEvent(const int time, const unsigned char status, IDataSource* source, const int size, first_state& fs);
    int ConvertSystemMessage(const int time, const unsigned char status, IDataSource* source);
    int CreateMT32SystemMessage(const int time, uint32_t address_base, uint16_t address_offset, uint32_t len,
                                const void* data = nullptr, IDataSource* source = nullptr);

    int ConvertFiletoList(IDataSource* source, const bool is_xmi, first_state& fs);

    int ExtractTracksFromXmi(IDataSource* source);
    int ExtractTracksFromMid(IDataSource* source, const uint32_t ppqn, const int num_tracks, const bool type1);

    int ExtractTracks(IDataSource* source);

    int ExtractTracksFromXMIDIMT(IDataSource* source);

    void CreateEventList();
    void DestroyEventList();
};

#endif // XMIDIFILE_H_INCLUDED
