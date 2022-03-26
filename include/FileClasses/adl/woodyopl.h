/*
 *  Copyright (C) 2002-2008  The DOSBox Team
 *  OPL2/OPL3 emulation library
 *
 *  Based on ADLIBEMU.C, an AdLib emulation library by Ken Silverman
 *  Copyright (C) 1998-2001 Ken Silverman
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License as published by the Free Software Foundation; either
 *  version 2.1 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 */

#ifndef WOODYOPL_H
#define WOODYOPL_H

#include <cstdint>

using fltype = double;
//#using fltype = float;

/*
    define Bits, Bitu, Bit32s, Bit32u, Bit16s, Bit16u, Bit8s, Bit8u here
*/

using Bitu   = uintptr_t;
using Bits   = intptr_t;
using Bit32u = uint32_t;
using Bit32s = int32_t;
using Bit16u = uint16_t;
using Bit16s = int16_t;
using Bit8u  = uint8_t;
using Bit8s  = int8_t;

#undef NUM_CHANNELS
#define NUM_CHANNELS 9

inline constexpr auto MAXCELLS = NUM_CHANNELS * 2;

inline constexpr auto FL0  = (fltype)0.0;
inline constexpr auto FL05 = (fltype)0.5;
inline constexpr auto FL1  = (fltype)1.0;
inline constexpr auto FL2  = (fltype)2.0;
inline constexpr auto PI   = (fltype)3.141592653589793238462643;

inline constexpr auto WAVPREC = 1024;

inline constexpr auto AMPVOL = (fltype)(8192.0 / 2.0);
// inline constexpr auto FRQSCALE = (fltype)(49716/512.0);
inline constexpr auto INTFREQU = 50000;

inline constexpr auto MODFACTOR = (fltype)(WAVPREC * 4.0); // How much of modulator cell goes into carrier

inline constexpr auto CF_TYPE_ATT        = 0;
inline constexpr auto CF_TYPE_DEC        = 1;
inline constexpr auto CF_TYPE_REL        = 2;
inline constexpr auto CF_TYPE_SUS        = 3;
inline constexpr auto CF_TYPE_SUS_NOKEEP = 4;
inline constexpr auto CF_TYPE_OFF        = 5;

inline constexpr auto ARC_CONTROL     = 0x00;
inline constexpr auto ARC_TVS_KSR_MUL = 0x20;
inline constexpr auto ARC_KSL_OUTLEV  = 0x40;
inline constexpr auto ARC_ATTR_DECR   = 0x60;
inline constexpr auto ARC_SUSL_RELR   = 0x80;
inline constexpr auto ARC_FREQ_NUM    = 0xa0;
inline constexpr auto ARC_KON_BNUM    = 0xb0;
inline constexpr auto ARC_PERC_MODE   = 0xbd;
inline constexpr auto ARC_FEEDBACK    = 0xc0;
inline constexpr auto ARC_WAVE_SEL    = 0xe0;

inline constexpr auto ARC_SECONDSET = 0x100; // second cell set for OPL3

inline constexpr auto FIFOSIZE = 512;

// vibrato constants
inline constexpr auto VIBTAB_SIZE = 8;
inline constexpr auto VIB_FREQ =
    (fltype)(INTFREQU / 8192); // vibrato at 6.1hz ?? (opl3 docs say 6.1, opl4 docs say 6.0, y8950 docs say 6.4)

// tremolo constants and table
inline constexpr auto TREMTAB_SIZE = 53;
inline constexpr auto TREM_FREQ    = (fltype)3.7; // tremolo at 3.7hz

/* cell struct definition
     For OPL2 all 9 channels consist of two cells each, carrier and modulator.
     Channel x has cell x as modulator and cell (9+x) as carrier.
     For OPL3 all 18 channels consist either of two cells (2op mode) or four
     cells (4op mode) which is determined through register4 of the second
     adlib register set.
     Only the channels 0,1,2 (first set) and 9,10,11 (second set) can act as
     4op channels. The two additional operators (cells) for a channel y come
     from the 2op channel y+3 so the cells y, (9+y), y+3, (9+y)+3 make up a 4op
     channel.
*/
typedef struct cell_type_struct {
    fltype val, lastval;         // current output/last output (used for feedback)
    fltype t, tinc;              // time (position in waveform) and time increment
    fltype vol, amp, step_amp;   // volume and amplification (cell envelope)
    fltype sustain_level, mfb;   // sustain level, feedback amount
    fltype a0, a1, a2, a3;       // attack rate function coefficients
    fltype decaymul, releasemul; // decay/release rate functions
    Bitu cf_sel;                 // current state of cell (attack/decay/sustain/release/off)
    Bits toff;
    Bits freq_high;        // highest three bits of the frequency, used for vibrato calculations
    Bit16s* cur_wform;     // start of selected waveform
    Bitu cur_wmask;        // mask for selected waveform
    bool sus_keep;         // keep sustain level when decay finished
    bool vibrato, tremolo; // vibrato/tremolo enable bits

    // variables used to provide non-continuous envelopes
    fltype generator_pos; // for non-standard sample rates we need to determine how many samples have passed
    Bits cur_env_step;    // current (standardized) sample position
    Bits env_step_a, env_step_d, env_step_r; // number of std samples of one step (for attack/decay/release mode)
    Bit8u step_skip_pos;                     // position of 8-cyclic step skipping (always 2^x to check against mask)
    Bits env_step_skip_a; // bitmask that determines if a step is skipped (respective bit is zero then)

} celltype;

class OPLChipClass {
public:
    // per-chip variables
    Bitu chip_num;
    celltype cell[MAXCELLS];

    Bits int_samplerate, ext_samplerate;

    Bit8u status, index;
    Bit8u adlibreg[256]; // adlib register set
    Bit8u wave_sel[22];  // waveform selection
    Bit8u timer[2];

    // vibrato/tremolo increment/counter
    fltype vibtab_pos;
    fltype vibtab_add;
    fltype tremtab_pos;
    fltype tremtab_add;

    OPLChipClass(Bitu cnum);

    // enable a cell
    void cellon(Bitu regbase, celltype* c);

    // functions to change parameters of a cell
    void change_cellfreq(Bitu chanbase, Bitu regbase, celltype* c);

    void change_attackrate(Bitu regbase, celltype* c);
    void change_decayrate(Bitu regbase, celltype* c);
    void change_releaserate(Bitu regbase, celltype* c);
    void change_sustainlevel(Bitu regbase, celltype* c);
    void change_waveform(Bitu regbase, celltype* c);
    void change_keepsustain(Bitu regbase, celltype* c);
    void change_vibrato(Bitu regbase, celltype* c);

    void change_feedback(Bitu chanbase, celltype* c);

    // general functions
    void adlib_init(Bits samplerate, bool highprec = false);
    void adlib_write(Bitu idx, Bit8u val, Bitu second_set);
    void adlib_getsample(Bit16s* sndptr, Bits numsamples);
};

extern OPLChipClass* oplchip[2];

Bitu adlib_reg_read(Bitu oplnum, Bitu port);
void adlib_reg_write(Bitu oplnum, Bitu port, Bit8u val);
#endif // WOODYOPL_H
