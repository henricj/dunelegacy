/*
 *  This file is part of Dune Legacy.
 *
 *  Dune Legacy is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  Dune Legacy is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with Dune Legacy.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef DUNE_LEGACY_SDL_MIXER_H
#define DUNE_LEGACY_SDL_MIXER_H

/**
 * @file dune_sdl_mixer.h
 * @brief Centralized SDL_mixer include header for Dune Legacy.
 *
 * CRITICAL: SDL3_mixer is a COMPLETE API REWRITE from SDL2_mixer.
 * The entire audio system needs to be rewritten for SDL3_mixer.
 * 
 * This header provides both the real SDL3_mixer API (MIX_Init, MIX_Mixer, etc.)
 * and STUB implementations for the old SDL2_mixer API (Mix_Chunk, Mix_Music, etc.)
 * that allow compilation but AUDIO WILL NOT WORK until the audio subsystem is rewritten.
 */

#include <misc/dune_sdl.h>

// Include the real SDL3_mixer header for MIX_Init, MIX_Quit, SDL_MIXER_VERSION, etc.
#include <SDL3_mixer/SDL_mixer.h>

// SDL2_mixer compatibility: MIX_MAX_VOLUME
#ifndef MIX_MAX_VOLUME
#define MIX_MAX_VOLUME 128
#endif

// MIX_VERSION macro - use SDL3_mixer's SDL_MIXER_VERSION
#ifndef MIX_VERSION
#define MIX_VERSION SDL_MIXER_VERSION
#endif

// MIX_INIT flags for SDL2_mixer compatibility - SDL3_mixer doesn't use these
#ifndef MIX_INIT_FLAC
#define MIX_INIT_FLAC   0x00000001
#define MIX_INIT_MOD    0x00000002
#define MIX_INIT_MP3    0x00000008
#define MIX_INIT_OGG    0x00000010
#define MIX_INIT_MID    0x00000020
#define MIX_INIT_OPUS   0x00000040
#define MIX_INIT_FLUIDSYNTH MIX_INIT_MID
#endif

// Music type enum for Mix_LoadMUSType_RW compatibility
#ifndef MUS_MID
typedef enum {
    MUS_NONE,
    MUS_CMD,
    MUS_WAV,
    MUS_MOD,
    MUS_MID,
    MUS_OGG,
    MUS_MP3,
    MUS_MP3_MAD_UNUSED,
    MUS_FLAC,
    MUS_MODPLUG_UNUSED,
    MUS_OPUS
} Mix_MusicType;
#endif

#ifdef __cplusplus

// ============================================================================
// SDL2_mixer COMPATIBILITY STRUCTURES AND FUNCTIONS
// These provide the old SDL2_mixer API as stubs.
// Audio using these will NOT work until properly rewritten for SDL3_mixer.
// ============================================================================

// Mix_Chunk - the old SDL2_mixer sound effect structure
struct Mix_Chunk {
    int allocated;      // Was this allocated by SDL_mixer?
    Uint8* abuf;        // Audio buffer
    Uint32 alen;        // Length of audio buffer
    Uint8 volume;       // 0-128
    SDL_AudioSpec spec; // Format for abuf/alen
    // Owning pointer: must be released via MIX_DestroyAudio() before the chunk
    // is freed. Lifecycle is managed exclusively through Mix_FreeChunk() and
    // Mix_RefreshChunkAudio(); callers must not free this directly.
    MIX_Audio* audio;   // Backend-ready decoded payload
};

Mix_Chunk* Mix_CreateChunk();
Mix_Chunk* Mix_CreateChunkWithData(Uint8* data, Uint32 len, bool takeOwnership, int volume, const SDL_AudioSpec* spec);
Mix_Chunk* Mix_CreateChunkCopy(const void* data, Uint32 len, int volume, const SDL_AudioSpec* spec);
bool Mix_SetChunkData(Mix_Chunk* chunk, Uint8* data, Uint32 len, bool takeOwnership, const SDL_AudioSpec* spec);
bool Mix_RefreshChunkAudio(Mix_Chunk* chunk);
bool Mix_SetChunkVolume(Mix_Chunk* chunk, int volume);
int Mix_GetChunkVolume(const Mix_Chunk* chunk);
Uint8* Mix_GetChunkData(Mix_Chunk* chunk);
const Uint8* Mix_GetChunkData(const Mix_Chunk* chunk);
Uint32 Mix_GetChunkDataSize(const Mix_Chunk* chunk);
bool Mix_GetChunkSpec(const Mix_Chunk* chunk, SDL_AudioSpec* spec);
// Returns a non-owning pointer to the chunk's backend audio payload.
// The returned pointer is valid only for the lifetime of the chunk;
// callers must not free it.
MIX_Audio* Mix_GetChunkAudio(const Mix_Chunk* chunk);

// Mix_Music - old opaque music structure (stubbed)
struct Mix_Music {
    void* data;
};

// Mix_FreeChunk
inline void Mix_FreeChunk(Mix_Chunk* chunk) {
    if (chunk) {
        if (chunk->audio) {
            MIX_DestroyAudio(chunk->audio);
            chunk->audio = nullptr;
        }
        if (chunk->allocated && chunk->abuf) {
            SDL_free(chunk->abuf);
        }
        SDL_free(chunk);
    }
}

// Mix_FreeMusic
inline void Mix_FreeMusic(Mix_Music* music) {
    if (music) {
        SDL_free(music);
    }
}

// ============================================================================
// SDL2_mixer STUB FUNCTIONS - These do nothing but allow compilation
// Note: lowercase Mix_* functions are SDL2_mixer API
//       uppercase MIX_* functions are SDL3_mixer API (from the real header)
// ============================================================================

// Mix_Init - SDL2_mixer version (takes flags)
inline int Mix_Init(int flags) {
    (void)flags;
    return flags;
}

// Mix_Quit - SDL2_mixer version
inline void Mix_Quit() {
}

inline int Mix_VolumeMusic(int volume) {
    (void)volume;
    return volume;
}

inline int Mix_Volume(int channel, int volume) {
    (void)channel;
    (void)volume;
    return volume;
}

inline int Mix_ReserveChannels(int num) {
    (void)num;
    return num;
}

inline int Mix_GroupChannels(int from, int to, int tag) {
    (void)from;
    (void)to;
    (void)tag;
    return 0;
}

inline int Mix_GroupAvailable(int tag) {
    (void)tag;
    return -1;
}

inline int Mix_PlayChannel(int channel, Mix_Chunk* chunk, int loops) {
    (void)channel;
    (void)chunk;
    (void)loops;
    return -1;
}

inline void Mix_HookMusic(void (*mix_func)(void*, Uint8*, int), void* arg) {
    (void)mix_func;
    (void)arg;
}

inline bool Mix_QuerySpec(int* frequency, SDL_AudioFormat* format, int* channels) {
    if (frequency) *frequency = 44100;
    if (format) *format = SDL_AUDIO_S16;
    if (channels) *channels = 2;
    return true;
}

inline bool Mix_PlayingMusic() {
    return false;
}

inline void Mix_HaltMusic() {
}

inline Mix_Music* Mix_LoadMUS(const char* file) {
    (void)file;
    return nullptr;
}

// Mix_LoadMUSType_RW - SDL2_mixer API for loading music from RWops with type hint
inline Mix_Music* Mix_LoadMUSType_RW(SDL_IOStream* src, Mix_MusicType type, int freesrc) {
    (void)src;
    (void)type;
    if (freesrc && src) {
        SDL_CloseIO(src);
    }
    return nullptr;
}

inline bool Mix_PlayMusic(Mix_Music* music, int loops) {
    (void)music;
    (void)loops;
    return false;
}

inline const char* Mix_GetError() {
    return "SDL2_mixer audio system is stubbed - SDL3_mixer requires rewrite";
}

#endif // __cplusplus

#endif // DUNE_LEGACY_SDL_MIXER_H
