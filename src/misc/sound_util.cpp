#include <misc/sound_util.h>

#include <FileClasses/FileManager.h>
#include <FileClasses/Vocfile.h>

#include <misc/dune_sdl_mixer.h>
#include <misc/dune_sdlpp.h>
#include <misc/exceptions.h>

#include <globals.h>

#include <cstring>

sdl2::mix_chunk_ptr create_chunk() {
    return sdl2::mix_chunk_ptr{Mix_CreateChunk()};
}

sdl2::mix_chunk_ptr concat2Chunks(Mix_Chunk* sound1, Mix_Chunk* sound2) {
    if (!sound1 || !sound2) {
        return nullptr;
    }

    SDL_AudioSpec spec{};
    Mix_GetChunkSpec(sound1, &spec);

    const auto size1 = Mix_GetChunkDataSize(sound1);
    const auto size2 = Mix_GetChunkDataSize(sound2);
    const auto total = static_cast<size_t>(size1) + static_cast<size_t>(size2);

    sdl2::sdl_ptr<uint8_t> buffer{static_cast<uint8_t*>(SDL_malloc(total))};
    if (buffer == nullptr) {
        return nullptr;
    }

    auto* p = buffer.get();

    std::memcpy(p, Mix_GetChunkData(sound1), size1);
    p += size1;
    std::memcpy(p, Mix_GetChunkData(sound2), size2);

    auto* raw_buffer = buffer.release();
    auto* chunk =
        Mix_CreateChunkWithData(raw_buffer, static_cast<Uint32>(total), true, Mix_GetChunkVolume(sound1), &spec);
    if (!chunk) {
        SDL_free(raw_buffer);
        return nullptr;
    }

    return sdl2::mix_chunk_ptr{chunk};
}

sdl2::mix_chunk_ptr concat3Chunks(Mix_Chunk* sound1, Mix_Chunk* sound2, Mix_Chunk* sound3) {
    if (!sound1 || !sound2 || !sound3) {
        return nullptr;
    }

    SDL_AudioSpec spec{};
    Mix_GetChunkSpec(sound1, &spec);

    const auto size1 = Mix_GetChunkDataSize(sound1);
    const auto size2 = Mix_GetChunkDataSize(sound2);
    const auto size3 = Mix_GetChunkDataSize(sound3);
    const auto total = static_cast<size_t>(size1) + static_cast<size_t>(size2) + static_cast<size_t>(size3);

    sdl2::sdl_ptr<uint8_t> buffer{static_cast<uint8_t*>(SDL_malloc(total))};
    if (buffer == nullptr) {
        return nullptr;
    }

    auto* p = buffer.get();

    std::memcpy(p, Mix_GetChunkData(sound1), size1);
    p += size1;
    std::memcpy(p, Mix_GetChunkData(sound2), size2);
    p += size2;
    std::memcpy(p, Mix_GetChunkData(sound3), size3);

    auto* raw_buffer = buffer.release();
    auto* chunk =
        Mix_CreateChunkWithData(raw_buffer, static_cast<Uint32>(total), true, Mix_GetChunkVolume(sound1), &spec);
    if (!chunk) {
        SDL_free(raw_buffer);
        return nullptr;
    }

    return sdl2::mix_chunk_ptr{chunk};
}

sdl2::mix_chunk_ptr concat4Chunks(Mix_Chunk* sound1, Mix_Chunk* sound2, Mix_Chunk* sound3, Mix_Chunk* sound4) {
    if (!sound1 || !sound2 || !sound3 || !sound4) {
        return nullptr;
    }

    SDL_AudioSpec spec{};
    Mix_GetChunkSpec(sound1, &spec);

    const auto size1 = Mix_GetChunkDataSize(sound1);
    const auto size2 = Mix_GetChunkDataSize(sound2);
    const auto size3 = Mix_GetChunkDataSize(sound3);
    const auto size4 = Mix_GetChunkDataSize(sound4);
    const auto total = static_cast<size_t>(size1) + static_cast<size_t>(size2) + static_cast<size_t>(size3)
                     + static_cast<size_t>(size4);

    sdl2::sdl_ptr<uint8_t> buffer{static_cast<uint8_t*>(SDL_malloc(total))};
    if (buffer == nullptr) {
        return nullptr;
    }

    auto* p = buffer.get();

    std::memcpy(p, Mix_GetChunkData(sound1), size1);
    p += size1;
    std::memcpy(p, Mix_GetChunkData(sound2), size2);
    p += size2;
    std::memcpy(p, Mix_GetChunkData(sound3), size3);
    p += size3;
    std::memcpy(p, Mix_GetChunkData(sound4), size4);

    auto* raw_buffer = buffer.release();
    auto* chunk =
        Mix_CreateChunkWithData(raw_buffer, static_cast<Uint32>(total), true, Mix_GetChunkVolume(sound1), &spec);
    if (!chunk) {
        SDL_free(raw_buffer);
        return nullptr;
    }

    return sdl2::mix_chunk_ptr{chunk};
}

sdl2::mix_chunk_ptr createEmptyChunk() {
    return sdl2::mix_chunk_ptr{Mix_CreateChunkWithData(nullptr, 0, false, 0, nullptr)};
}

sdl2::mix_chunk_ptr createSilenceChunk(int length) {
    if (length < 0) {
        return nullptr;
    }

    sdl2::sdl_ptr<uint8_t> buffer{static_cast<uint8_t*>(SDL_calloc(length, 1))};
    if (buffer == nullptr) {
        return nullptr;
    }

    auto* raw_buffer = buffer.release();
    auto* chunk      = Mix_CreateChunkWithData(raw_buffer, static_cast<Uint32>(length), true, MIX_MAX_VOLUME, nullptr);
    if (!chunk) {
        SDL_free(raw_buffer);
        return nullptr;
    }

    return sdl2::mix_chunk_ptr{chunk};
}

sdl2::mix_chunk_ptr getChunkFromFile(std::string_view filename) {
    auto returnChunk = LoadVOC_RW(dune::globals::pFileManager->openFile(filename).get());
    if (returnChunk == nullptr) {
        THROW(io_error, "Cannot load '{}'!", filename);
    }

    return returnChunk;
}

sdl2::mix_chunk_ptr getChunkFromFile(std::string_view filename, std::string_view alternativeFilename) {
    const auto* const file_manager = dune::globals::pFileManager.get();

    if (file_manager->exists(filename)) {
        return getChunkFromFile(filename);
    }
    if (file_manager->exists(alternativeFilename)) {

        return getChunkFromFile(alternativeFilename);
    }
    THROW(io_error, "Cannot open '{}' or '{}'!", filename, alternativeFilename);
}
