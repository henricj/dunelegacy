
#include <misc/sound_util.h>

#include <FileClasses/FileManager.h>
#include <FileClasses/Vocfile.h>

#include <misc/exceptions.h>

#include <SDL2/SDL_mixer.h>
#include <stdlib.h>
#include <memory.h>

extern FileManager* pFileManager;

Mix_Chunk* concat2Chunks(Mix_Chunk* sound1, Mix_Chunk* sound2)
{
    Mix_Chunk* returnChunk;
    if((returnChunk = (Mix_Chunk*) SDL_malloc(sizeof(Mix_Chunk))) == nullptr) {
        return nullptr;
    }

    returnChunk->allocated = 1;
    returnChunk->volume = sound1->volume;
    returnChunk->alen = sound1->alen + sound2->alen;

    if((returnChunk->abuf = (Uint8 *)SDL_malloc(returnChunk->alen)) == nullptr) {
        SDL_free(returnChunk);
        return nullptr;
    }

    memcpy(returnChunk->abuf, sound1->abuf, sound1->alen);
    memcpy(returnChunk->abuf + sound1->alen, sound2->abuf, sound2->alen);

    return returnChunk;
}

Mix_Chunk* concat3Chunks(Mix_Chunk* sound1, Mix_Chunk* sound2, Mix_Chunk* sound3)
{
    Mix_Chunk* returnChunk;
    if((returnChunk = (Mix_Chunk*) SDL_malloc(sizeof(Mix_Chunk))) == nullptr) {
        return nullptr;
    }

    returnChunk->allocated = 1;
    returnChunk->volume = sound1->volume;
    returnChunk->alen = sound1->alen + sound2->alen + sound3->alen;

    if((returnChunk->abuf = (Uint8 *)SDL_malloc(returnChunk->alen)) == nullptr) {
        SDL_free(returnChunk);
        return nullptr;
    }

    memcpy(returnChunk->abuf, sound1->abuf, sound1->alen);
    memcpy(returnChunk->abuf + sound1->alen, sound2->abuf, sound2->alen);
    memcpy(returnChunk->abuf + sound1->alen + sound2->alen, sound3->abuf, sound3->alen);

    return returnChunk;
}

Mix_Chunk* concat4Chunks(Mix_Chunk* sound1, Mix_Chunk* sound2, Mix_Chunk* sound3, Mix_Chunk* sound4)
{
    Mix_Chunk* returnChunk;
    if((returnChunk = (Mix_Chunk*) SDL_malloc(sizeof(Mix_Chunk))) == nullptr) {
        return nullptr;
    }

    returnChunk->allocated = 1;
    returnChunk->volume = sound1->volume;
    returnChunk->alen = sound1->alen + sound2->alen + sound3->alen + sound4->alen;

    if((returnChunk->abuf = (Uint8 *)SDL_malloc(returnChunk->alen)) == nullptr) {
        SDL_free(returnChunk);
        return nullptr;
    }

    memcpy(returnChunk->abuf, sound1->abuf, sound1->alen);
    memcpy(returnChunk->abuf + sound1->alen, sound2->abuf, sound2->alen);
    memcpy(returnChunk->abuf + sound1->alen + sound2->alen, sound3->abuf, sound3->alen);
    memcpy(returnChunk->abuf + sound1->alen + sound2->alen + sound3->alen, sound4->abuf, sound4->alen);

    return returnChunk;
}

Mix_Chunk* createEmptyChunk()
{
    Mix_Chunk* returnChunk;
    if((returnChunk = (Mix_Chunk*) SDL_malloc(sizeof(Mix_Chunk))) == nullptr) {
        return nullptr;
    }

    returnChunk->allocated = 1;
    returnChunk->volume = 0;
    returnChunk->alen = 0;
    returnChunk->abuf = nullptr;

    return returnChunk;
}

Mix_Chunk* createSilenceChunk(int length)
{
    Mix_Chunk* returnChunk;
    if((returnChunk = (Mix_Chunk*) SDL_malloc(sizeof(Mix_Chunk))) == nullptr) {
        return nullptr;
    }

    returnChunk->allocated = 1;
    returnChunk->volume = MIX_MAX_VOLUME;
    returnChunk->alen = length;

    if((returnChunk->abuf = (Uint8 *)SDL_calloc(returnChunk->alen,1)) == nullptr) {
        SDL_free(returnChunk);
        return nullptr;
    }

    return returnChunk;
}

Mix_Chunk* getChunkFromFile(const std::string& filename) {
    SDL_RWops* rwop = pFileManager->openFile(filename);

    Mix_Chunk* returnChunk;
    if((returnChunk = LoadVOC_RW(rwop, 1)) == nullptr) {
        THROW(io_error, "Cannot load '%s' !", filename);
    }

    return returnChunk;
}

Mix_Chunk* getChunkFromFile(const std::string& filename, const std::string& alternativeFilename) {
    if(pFileManager->exists(filename)) {
        return getChunkFromFile(filename);
    } else if(pFileManager->exists(alternativeFilename)) {
        return getChunkFromFile(alternativeFilename);
    } else {
        THROW(io_error, "Cannot open '%s' or '%s'!", filename, alternativeFilename);
    }
    return nullptr;
}
