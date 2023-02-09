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

#include <FileClasses/Pakfile.h>
#include <misc/SDL2pp.h>
#include <misc/exceptions.h>

#include <cstdlib>
#include <string>

#include "misc/BufferedReader.h"

#include <gsl/gsl>

#include <utility>

namespace {

inline constexpr auto PAKFILE_RWOP_TYPE = 0x9a5f17ecU;

}

BasePakfile::BasePakfile(std::filesystem::path pakfilename) : filename_{std::move(pakfilename)} { }

BasePakfile::~BasePakfile() = default;

/// Returns the name of the nth file in this pak-File.
/**
    Returns the name of the nth file in this pak-File. index specifies which file.
    \param  index   Index in pak-File
    \return name of the file specified by index
*/
const std::string& BasePakfile::getFilename(unsigned int index) const {
    if (index >= fileEntries.size()) {
        THROW(std::invalid_argument, "Pakfile::getFilename({}): This Pakfile has only {} entries!", index,
              fileEntries.size());
    }

    return fileEntries[index].filename;
}

size_t BasePakfile::WriteFile([[maybe_unused]] SDL_RWops* pRWop, [[maybe_unused]] const void* ptr,
                              [[maybe_unused]] size_t size, [[maybe_unused]] size_t n) {
    return 0;
}

int64_t BasePakfile::SizeFile(SDL_RWops* pRWop) {
    if (pRWop == nullptr || pRWop->hidden.unknown.data1 == nullptr || pRWop->type != PAKFILE_RWOP_TYPE) {
        return -1;
    }

    const auto* const pRWopData = static_cast<RWopData*>(pRWop->hidden.unknown.data1);
    const auto* const pPakfile  = pRWopData->curPakfile;
    if (pPakfile == nullptr) {
        return -1;
    }

    if (pRWopData->fileIndex >= pPakfile->fileEntries.size()) {
        return -1;
    }

    return pPakfile->fileEntries[pRWopData->fileIndex].endOffset
         - pPakfile->fileEntries[pRWopData->fileIndex].startOffset + 1;
}

int64_t BasePakfile::SeekFile(SDL_RWops* pRWop, int64_t offset, int whence) {
    if (pRWop == nullptr || pRWop->hidden.unknown.data1 == nullptr || pRWop->type != PAKFILE_RWOP_TYPE) {
        return -1;
    }

    auto* const pRWopData      = static_cast<RWopData*>(pRWop->hidden.unknown.data1);
    const auto* const pPakfile = pRWopData->curPakfile;
    if (pPakfile == nullptr) {
        return -1;
    }

    if (pRWopData->fileIndex >= pPakfile->fileEntries.size()) {
        return -1;
    }

    int64_t newOffset = 0;

    switch (whence) {
        case SEEK_SET: {
            newOffset = offset;
        } break;

        case SEEK_CUR: {
            newOffset = pRWopData->fileOffset + offset;
        } break;

        case SEEK_END: {
            newOffset = pPakfile->fileEntries[pRWopData->fileIndex].endOffset
                      - pPakfile->fileEntries[pRWopData->fileIndex].startOffset + 1 + offset;
        } break;

        default: {
            return -1;
        }
    }

    if (newOffset > pPakfile->fileEntries[pRWopData->fileIndex].endOffset
                        - pPakfile->fileEntries[pRWopData->fileIndex].startOffset + 1) {
        return -1;
    }

    pRWopData->fileOffset = static_cast<size_t>(newOffset);
    return newOffset;
}

int BasePakfile::CloseFile(SDL_RWops* pRWop) {
    if (pRWop == nullptr || pRWop->hidden.unknown.data1 == nullptr || pRWop->type != PAKFILE_RWOP_TYPE) {
        return -1;
    }

    const auto* const pRWopData = static_cast<RWopData*>(pRWop->hidden.unknown.data1);
    delete pRWopData;
    pRWop->hidden.unknown.data1 = nullptr;
    SDL_FreeRW(pRWop);
    return 0;
}

void BasePakfile::readIndex() {
    BufferedReader reader{fPakFile.get()};

    while (true) {
        PakFileEntry newEntry;

        if (!reader.read_one(&newEntry.startOffset, sizeof newEntry.startOffset)) {
            THROW(std::runtime_error, "Pakfile::readIndex(): SDL_RWread() failed!");
        }

        // pak-files are always little endian encoded
        newEntry.startOffset = SDL_SwapLE32(newEntry.startOffset);
        newEntry.endOffset   = 0;

        if (newEntry.startOffset == 0) {
            break;
        }

        for (;;) {
            char tmp{};
            if (!reader.read_type<char>(tmp))
                THROW(std::runtime_error, "Pakfile::readIndex(): SDL_RWread() failed!");

            if (tmp == '\0')
                break;

            newEntry.filename += tmp;
        }

        if (!fileEntries.empty()) {
            fileEntries.back().endOffset = newEntry.startOffset - 1;
        }

        fileEntries.push_back(newEntry);
    }

    const auto filesize = SDL_RWsize(fPakFile.get());
    if (filesize < 0) {
        THROW(std::runtime_error, "Pakfile::readIndex(): SDL_RWsize() failed!");
    }

    fileEntries.back().endOffset = static_cast<uint32_t>(filesize) - 1u;
}

/// Constructor for Pakfile
/**
    The PAK-File to be read/write is specified by the pakfilename-parameter. If write==false the file is opened for
    read-only and is closed in the destructor. If write==true the file is opened write-only and written out in the
    destructor.
    \param pakfilename  Filename of the *.pak-File.
*/
Pakfile::Pakfile(const std::filesystem::path& pakfilename) : BasePakfile{pakfilename} {

    // Open for reading
    sdl2::RWops_ptr file{SDL_RWFromFile(filename_.u8string(), "rb")};

    if (file == nullptr) {
        THROW(std::invalid_argument, "Pakfile::Pakfile(): Cannot open {}!", pakfilename.string());
    }

    fPakFile = std::move(file);

    readIndex();
}

/// Destructor
/**
    Closes the filehandle and releases all memory.
*/
Pakfile::~Pakfile() = default;

/// Opens a file in this PAK-File.
/**
    This method opens the file specified by filename. It is only allowed if the Pakfile is opened for reading.
    The returned SDL_RWops-structure can be used readonly with SDL_RWread, SDL_RWsize, SDL_RWseek and SDL_RWclose. No
   writing is supported.<br> NOTICE: The returned SDL_RWops-Structure is only valid as long as this Pakfile-Object
   exists. It gets invalid as soon as Pakfile:~Pakfile() is executed.
   \param  filename    The name of this file
   \return SDL_RWops for this file
*/
sdl2::RWops_ptr Pakfile::openFile(const std::string& filename) const {
    // find file
    for (auto i = 0U; i < fileEntries.size(); ++i) {
        if (filename == fileEntries[i].filename)
            return openFile(static_cast<int>(i));
    }

    THROW(io_error, "Pakfile::openFile(): Cannot find file with name '{}' in this PAK file!", filename);
}

sdl2::RWops_ptr Pakfile::openFile(int index) const {
    if (index < 0 || std::cmp_greater_equal(index, fileEntries.size()))
        THROW(io_error, "Pakfile::openFile(): There is not file at index '{}' in this PAK file!", index);

    // alloc RWop

    sdl2::RWops_ptr pRWop{SDL_AllocRW()};

    if (!pRWop) {
        THROW(io_error, "Pakfile::openFile(): Cannot open file at index '{}' in this PAK file!", index);
    }

    // alloc RWopData
    auto pRWopData = std::make_unique<RWopData>();

    pRWop->type           = PAKFILE_RWOP_TYPE;
    pRWopData->curPakfile = this;
    pRWopData->fileOffset = 0;
    pRWopData->fileIndex  = index;

    pRWop->read  = ReadFile;
    pRWop->write = WriteFile;
    pRWop->size  = SizeFile;
    pRWop->seek  = SeekFile;
    pRWop->close = CloseFile;

    pRWop->hidden.unknown.data1 = static_cast<void*>(pRWopData.release());

    return pRWop;
}

bool BasePakfile::exists(const std::string& filename) const {
    return std::ranges::any_of(fileEntries, [&](auto& fe) { return filename == fe.filename; });
}

size_t Pakfile::ReadFile(SDL_RWops* pRWop, void* ptr, size_t size, size_t n) {
    if (pRWop == nullptr || ptr == nullptr || pRWop->hidden.unknown.data1 == nullptr
        || pRWop->type != PAKFILE_RWOP_TYPE) {
        return 0;
    }

    auto bytes2read = size * n;

    auto* const pRWopData      = static_cast<RWopData*>(pRWop->hidden.unknown.data1);
    const auto* const pPakfile = static_cast<const Pakfile*>(pRWopData->curPakfile);
    if (pPakfile == nullptr) {
        return 0;
    }

    if (pRWopData->fileIndex >= pPakfile->fileEntries.size()) {
        return 0;
    }

    const auto readstartoffset = pPakfile->fileEntries[pRWopData->fileIndex].startOffset + pRWopData->fileOffset;

    if (readstartoffset > pPakfile->fileEntries[pRWopData->fileIndex].endOffset) {
        return 0;
    }

    if (readstartoffset + bytes2read > pPakfile->fileEntries[pRWopData->fileIndex].endOffset) {
        bytes2read = pPakfile->fileEntries[pRWopData->fileIndex].endOffset + 1 - readstartoffset;
        // round to last full block
        bytes2read = (bytes2read / size) * size;
        if (bytes2read == 0) {
            return 0;
        }
    }

    if (SDL_RWseek(pPakfile->fPakFile.get(), static_cast<Sint64>(readstartoffset), SEEK_SET) < 0) {
        return 0;
    }

    if (SDL_RWread(pPakfile->fPakFile.get(), ptr, bytes2read, 1) != 1) {
        return 0;
    }

    pRWopData->fileOffset += bytes2read;
    return bytes2read / size;
}

/// Constructor for OutPakfile
/**
    The PAK-File to be written is specified by the pakfilename-parameter. The file is opened write-only
    and written out in the destructor.
    \param pakfilename  Filename of the *.pak-File.
*/
OutPakfile::OutPakfile(const std::filesystem::path& pakfilename) : BasePakfile{pakfilename} {

    // Open for writing
    fPakFile = sdl2::RWops_ptr{SDL_RWFromFile(filename_.u8string().c_str(), "wb")};

    if (!fPakFile) {
        THROW(std::invalid_argument, "Pakfile::Pakfile(): Cannot open {}!", pakfilename.string());
    }
}

/// Destructor
/**
    Closes the filehandle and releases all memory.
*/
OutPakfile::~OutPakfile() {
    // calculate header size
    auto headersize = size_t{0};
    for (const auto& fileEntry : fileEntries) {
        headersize += 4;
        headersize += fileEntry.filename.length() + 1;
    }
    headersize += 4;

    // write out header
    for (const auto& fileEntry : fileEntries) {
        const auto startoffset = SDL_SwapLE32(gsl::narrow<uint32_t>(fileEntry.startOffset + headersize));
        SDL_RWwrite(fPakFile.get(), &startoffset, sizeof(uint32_t), 1);
        SDL_RWwrite(fPakFile.get(), fileEntry.filename.data(), fileEntry.filename.length() + 1, 1);
    }

    static constexpr uint32_t tmp = 0;
    SDL_RWwrite(fPakFile.get(), &tmp, sizeof(uint32_t), 1);

    // write out data
    SDL_RWwrite(fPakFile.get(), writeOutData, numWriteOutData, 1);

    if (writeOutData != nullptr) {
        free(writeOutData);
        writeOutData    = nullptr;
        numWriteOutData = 0;
    }
}

/// Adds a file to this PAK-File
/**
    This methods adds the SDL_RWop File to this PAK-File. The used name is specified by filename. If the Pakfile
    is read-only this method has no effect.
    \param  rwop        Data to add (the SDL_RWop can be read-only but must support seeking)
    \param  filename    This is the filename the data is added with
*/
void OutPakfile::addFile(SDL_RWops* rwop, const std::string& filename) {
    if (rwop == nullptr) {
        THROW(std::invalid_argument, "Pakfile::addFile(): rwop==nullptr is not allowed!");
    }

    const auto filelength1 = SDL_RWsize(rwop);
    if (filelength1 < 0) {
        THROW(std::runtime_error, "Pakfile::addFile(): unable to get file size!");
    }

    const auto filelength = gsl::narrow<size_t>(filelength1);

    char* extendedBuffer = nullptr;
    if ((extendedBuffer = static_cast<char*>(realloc(writeOutData, numWriteOutData + filelength))) == nullptr) {
        THROW(std::runtime_error, "Pakfile::addFile(): realloc failed!");
    }
    writeOutData = extendedBuffer;

    if (SDL_RWread(rwop, writeOutData + numWriteOutData, 1, filelength) != filelength) {
        // revert the buffer to the original size
        char* shrinkedBuffer = nullptr;
        if ((shrinkedBuffer = static_cast<char*>(realloc(writeOutData, numWriteOutData))) == nullptr) {
            // shrinking the buffer should not fail
            THROW(std::runtime_error, "Pakfile::addFile(): realloc failed!");
        }
        writeOutData = shrinkedBuffer;
        THROW(std::runtime_error, "Pakfile::addFile(): SDL_RWread failed!");
    }

    PakFileEntry newPakFileEntry;
    newPakFileEntry.startOffset = gsl::narrow<uint32_t>(numWriteOutData);
    newPakFileEntry.endOffset   = gsl::narrow<uint32_t>(numWriteOutData + filelength - 1);
    newPakFileEntry.filename    = filename;

    fileEntries.push_back(newPakFileEntry);

    numWriteOutData += filelength;

    SDL_RWseek(rwop, 0, SEEK_SET);
}
