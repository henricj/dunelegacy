#include <misc/dune_sdl_mixer.h>

#include <globals.h>

#include <Audio/AudioEngine.h>

#include <algorithm>
#include <cstring>

namespace {

[[nodiscard]] SDL_AudioSpec get_default_spec() {
    SDL_AudioSpec spec{};
    if (dune::globals::pAudioEngine && dune::globals::pAudioEngine->isOpen()
        && dune::globals::pAudioEngine->getFormat(spec)) {
        return spec;
    }

    spec.freq     = 11025;
    spec.format   = SDL_AUDIO_U8;
    spec.channels = 2;
    return spec;
}

void destroy_audio(Mix_Chunk* chunk) {
    if (!chunk || !chunk->audio)
        return;

    MIX_DestroyAudio(chunk->audio);
    chunk->audio = nullptr;
}

void set_chunk_spec(Mix_Chunk* chunk, const SDL_AudioSpec* spec) {
    if (!chunk)
        return;

    if (spec) {
        chunk->spec = *spec;
    } else {
        chunk->spec = get_default_spec();
    }
}

} // namespace

Mix_Chunk* Mix_CreateChunk() {
    auto* chunk = static_cast<Mix_Chunk*>(SDL_calloc(1, sizeof(Mix_Chunk)));
    if (!chunk)
        return nullptr;

    chunk->volume = MIX_MAX_VOLUME;
    set_chunk_spec(chunk, nullptr);
    return chunk;
}

Mix_Chunk* Mix_CreateChunkWithData(Uint8* data, Uint32 len, bool takeOwnership, int volume, const SDL_AudioSpec* spec) {
    auto* chunk = Mix_CreateChunk();
    if (!chunk)
        return nullptr;

    if (!Mix_SetChunkData(chunk, data, len, takeOwnership, spec)) {
        Mix_FreeChunk(chunk);
        return nullptr;
    }

    Mix_SetChunkVolume(chunk, volume);
    return chunk;
}

Mix_Chunk* Mix_CreateChunkCopy(const void* data, Uint32 len, int volume, const SDL_AudioSpec* spec) {
    if (!data && len != 0)
        return nullptr;

    Uint8* buffer = nullptr;
    if (len > 0) {
        buffer = static_cast<Uint8*>(SDL_malloc(len));
        if (!buffer)
            return nullptr;
        std::memcpy(buffer, data, len);
    }

    auto* const chunk = Mix_CreateChunkWithData(buffer, len, true, volume, spec);
    if (!chunk && buffer) {
        SDL_free(buffer);
    }
    return chunk;
}

bool Mix_SetChunkData(Mix_Chunk* chunk, Uint8* data, Uint32 len, bool takeOwnership, const SDL_AudioSpec* spec) {
    if (!chunk)
        return false;

    destroy_audio(chunk);

    if (chunk->allocated && chunk->abuf) {
        SDL_free(chunk->abuf);
    }

    chunk->abuf      = data;
    chunk->alen      = len;
    chunk->allocated = takeOwnership ? 1 : 0;
    set_chunk_spec(chunk, spec);

    return Mix_RefreshChunkAudio(chunk);
}

bool Mix_RefreshChunkAudio(Mix_Chunk* chunk) {
    if (!chunk)
        return false;

    destroy_audio(chunk);

    if (!chunk->abuf || chunk->alen == 0)
        return true;

    if (dune::globals::pAudioEngine && dune::globals::pAudioEngine->isOpen()) {
        // Transfer ownership of the MIX_Audio from the unique_ptr into chunk->audio;
        // chunk->audio is the sole owner and will be released in destroy_audio() /
        // Mix_FreeChunk().
        chunk->audio = dune::globals::pAudioEngine->loadRawAudio(chunk->abuf, chunk->alen, chunk->spec).release();
    } else {
        chunk->audio = nullptr;
    }

    return true;
}

bool Mix_SetChunkVolume(Mix_Chunk* chunk, int volume) {
    if (!chunk)
        return false;

    chunk->volume = static_cast<Uint8>(std::clamp(volume, 0, MIX_MAX_VOLUME));
    return true;
}

int Mix_GetChunkVolume(const Mix_Chunk* chunk) {
    if (!chunk)
        return 0;

    return chunk->volume;
}

Uint8* Mix_GetChunkData(Mix_Chunk* chunk) {
    if (!chunk)
        return nullptr;

    return chunk->abuf;
}

const Uint8* Mix_GetChunkData(const Mix_Chunk* chunk) {
    if (!chunk)
        return nullptr;

    return chunk->abuf;
}

Uint32 Mix_GetChunkDataSize(const Mix_Chunk* chunk) {
    if (!chunk)
        return 0;

    return chunk->alen;
}

bool Mix_GetChunkSpec(const Mix_Chunk* chunk, SDL_AudioSpec* spec) {
    if (!chunk || !spec)
        return false;

    *spec = chunk->spec;
    return true;
}

MIX_Audio* Mix_GetChunkAudio(const Mix_Chunk* chunk) {
    if (!chunk)
        return nullptr;

    return chunk->audio;
}
