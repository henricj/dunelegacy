
#include <misc/sound_util.h>

#include <FileClasses/FileManager.h>
#include <FileClasses/Vocfile.h>

#include <misc/exceptions.h>
#include <misc/SDL2pp.h>

#include <globals.h>

#include <SDL2/SDL_mixer.h>

sdl2::mix_chunk_ptr create_chunk()
{
    return sdl2::mix_chunk_ptr{ static_cast<Mix_Chunk*>(SDL_malloc(sizeof(Mix_Chunk))) };
}

sdl2::mix_chunk_ptr concat2Chunks(Mix_Chunk* sound1, Mix_Chunk* sound2)
{
    auto returnChunk = create_chunk();
    if(returnChunk == nullptr) {
        return nullptr;
    }

    returnChunk->allocated = 1;
    returnChunk->volume = sound1->volume;
    returnChunk->alen = sound1->alen + sound2->alen;

    sdl2::sdl_ptr<Uint8> buffer{ static_cast<Uint8 *>(SDL_malloc(returnChunk->alen)) };
    if (buffer == nullptr) {
        return nullptr;
    }

    auto p = buffer.get();

    memcpy(p, sound1->abuf, sound1->alen);
    p += sound1->alen;
    memcpy(p, sound2->abuf, sound2->alen);

    returnChunk->abuf = buffer.release();

    return returnChunk;
}

sdl2::mix_chunk_ptr concat3Chunks(Mix_Chunk* sound1, Mix_Chunk* sound2, Mix_Chunk* sound3)
{
    auto returnChunk = create_chunk();
    if(returnChunk == nullptr) {
        return nullptr;
    }

    returnChunk->allocated = 1;
    returnChunk->volume = sound1->volume;
    returnChunk->alen = sound1->alen + sound2->alen + sound3->alen;

    sdl2::sdl_ptr<Uint8> buffer{ static_cast<Uint8 *>(SDL_malloc(returnChunk->alen)) };
    if(buffer == nullptr) {
        return nullptr;
    }

    auto p = buffer.get();

    memcpy(p, sound1->abuf, sound1->alen);
    p += sound1->alen;
    memcpy(p, sound2->abuf, sound2->alen);
    p += sound2->alen;
    memcpy(p, sound3->abuf, sound3->alen);

    returnChunk->abuf = buffer.release();

    return returnChunk;
}

sdl2::mix_chunk_ptr concat4Chunks(Mix_Chunk* sound1, Mix_Chunk* sound2, Mix_Chunk* sound3, Mix_Chunk* sound4)
{
    auto returnChunk = create_chunk();
    if (returnChunk == nullptr) {
        return nullptr;
    }

    returnChunk->allocated = 1;
    returnChunk->volume = sound1->volume;
    returnChunk->alen = sound1->alen + sound2->alen + sound3->alen + sound4->alen;

    sdl2::sdl_ptr<Uint8> buffer{ static_cast<Uint8 *>(SDL_malloc(returnChunk->alen)) };
    if (buffer == nullptr) {
        return nullptr;
    }

    auto p = buffer.get();

    memcpy(p, sound1->abuf, sound1->alen);
    p += sound1->alen;
    memcpy(p, sound2->abuf, sound2->alen);
    p += sound2->alen;
    memcpy(p, sound3->abuf, sound3->alen);
    p += sound3->alen;
    memcpy(p, sound4->abuf, sound4->alen);

    returnChunk->abuf = buffer.release();

    return returnChunk;
}

sdl2::mix_chunk_ptr createEmptyChunk()
{
    auto returnChunk = create_chunk();
    if (returnChunk == nullptr) {
        return nullptr;
    }

    returnChunk->allocated = 1;
    returnChunk->volume = 0;
    returnChunk->alen = 0;
    returnChunk->abuf = nullptr;

    return returnChunk;
}

sdl2::mix_chunk_ptr createSilenceChunk(int length)
{
    auto returnChunk = create_chunk();
    if (returnChunk == nullptr) {
        return nullptr;
    }

    returnChunk->allocated = 1;
    returnChunk->volume = MIX_MAX_VOLUME;
    returnChunk->alen = length;

    sdl2::sdl_ptr<Uint8> buffer{ static_cast<Uint8 *>(SDL_calloc(returnChunk->alen, 1)) };
    if (buffer == nullptr) {
        return nullptr;
    }

    returnChunk->abuf = buffer.release();

    return returnChunk;
}

sdl2::mix_chunk_ptr getChunkFromFile(const std::string& filename) {
    auto returnChunk = LoadVOC_RW(pFileManager->openFile(filename).get());
    if(returnChunk == nullptr) {
        THROW(io_error, "Cannot load '%s'!", filename);
    }

    return returnChunk;
}

sdl2::mix_chunk_ptr getChunkFromFile(const std::string& filename, const std::string& alternativeFilename) {
    if(pFileManager->exists(filename)) {
        return getChunkFromFile(filename);
    } else if(pFileManager->exists(alternativeFilename)) {
        return getChunkFromFile(alternativeFilename);
    } else {
        THROW(io_error, "Cannot open '%s' or '%s'!", filename, alternativeFilename);
    }
    return nullptr;
}
