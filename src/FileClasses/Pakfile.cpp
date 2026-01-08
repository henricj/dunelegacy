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
        THROW(std::invalid_argument,
              "Pakfile::getFilename({}): This Pakfile has only {} entries!",
              index,
              fileEntries.size());
    }

    return fileEntries[index].filename;
}

// SDL3: WriteFile callback - pakfiles are read-only so this just reports failure
size_t BasePakfile::WriteFile(void* userdata, const void* ptr, size_t size, SDL_IOStatus* status) {
    (void)userdata;
    (void)ptr;
    (void)size;
    if (status)
        *status = SDL_IO_STATUS_ERROR;
    return 0;
}

// SDL3: SizeFile callback - returns size of the virtual file
Sint64 BasePakfile::SizeFile(void* userdata) {
    if (userdata == nullptr) {
        return -1;
    }

    const auto* const pRWopData = static_cast<RWopData*>(userdata);
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

// SDL3: SeekFile callback
Sint64 BasePakfile::SeekFile(void* userdata, Sint64 offset, SDL_IOWhence whence) {
    if (userdata == nullptr) {
        return -1;
    }

    auto* const pRWopData      = static_cast<RWopData*>(userdata);
    const auto* const pPakfile = pRWopData->curPakfile;
    if (pPakfile == nullptr) {
        return -1;
    }

    if (pRWopData->fileIndex >= pPakfile->fileEntries.size()) {
        return -1;
    }

    int64_t newOffset = 0;

    switch (whence) {
        case SDL_IO_SEEK_SET: {
            newOffset = offset;
        } break;

        case SDL_IO_SEEK_CUR: {
            newOffset = pRWopData->fileOffset + offset;
        } break;

        case SDL_IO_SEEK_END: {
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

// SDL3: CloseFile callback
bool BasePakfile::CloseFile(void* userdata) {
    if (userdata == nullptr) {
        return false;
    }

    const auto* const pRWopData = static_cast<RWopData*>(userdata);
    delete pRWopData;
    return true;
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

    const auto filesize = SDL_GetIOSize(fPakFile.get());
    if (filesize < 0) {
        THROW(std::runtime_error, "Pakfile::readIndex(): SDL_GetIOSize() failed!");
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

    // Open for reading - SDL3: use SDL_IOFromFile
    sdl2::IOStream_ptr file{SDL_IOFromFile(filename_.string().c_str(), "rb")};

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
    The returned SDL_IOStream-structure can be used readonly with SDL_ReadIO, SDL_GetIOSize, SDL_SeekIO and SDL_CloseIO.
    No writing is supported.<br>
    NOTICE: The returned SDL_IOStream-Structure is only valid as long as this Pakfile-Object exists. It gets invalid
    as soon as Pakfile:~Pakfile() is executed.
   \param  filename    The name of this file
   \return SDL_IOStream for this file
*/
sdl2::IOStream_ptr Pakfile::openFile(const std::string& filename) const {
    // find file
    for (auto i = 0U; i < fileEntries.size(); ++i) {
        if (filename == fileEntries[i].filename)
            return openFile(static_cast<int>(i));
    }

    THROW(io_error, "Pakfile::openFile(): Cannot find file with name '{}' in this PAK file!", filename);
}

sdl2::IOStream_ptr Pakfile::openFile(int index) const {
    if (index < 0 || std::cmp_greater_equal(index, fileEntries.size()))
        THROW(io_error, "Pakfile::openFile(): There is not file at index '{}' in this PAK file!", index);

    // alloc RWopData - this will be passed to the callbacks as userdata
    auto pRWopData = std::make_unique<RWopData>();

    pRWopData->curPakfile = this;
    pRWopData->fileOffset = 0;
    pRWopData->fileIndex  = index;

    // SDL3: Create SDL_IOStreamInterface with our callbacks
    SDL_IOStreamInterface iface{};
    iface.version = sizeof(SDL_IOStreamInterface);
    iface.size    = SizeFile;
    iface.seek    = SeekFile;
    iface.read    = ReadFile;
    iface.write   = WriteFile;
    iface.close   = CloseFile;

    // SDL3: SDL_OpenIO creates an SDL_IOStream from an interface
    sdl2::IOStream_ptr pRWop{SDL_OpenIO(&iface, pRWopData.release())};

    if (!pRWop) {
        THROW(io_error, "Pakfile::openFile(): Cannot open file at index '{}' in this PAK file!", index);
    }

    return pRWop;
}

bool BasePakfile::exists(const std::string& filename) const {
    return std::ranges::any_of(fileEntries, [&](auto& fe) { return filename == fe.filename; });
}

// SDL3: ReadFile callback
size_t Pakfile::ReadFile(void* userdata, void* ptr, size_t size, SDL_IOStatus* status) {
    if (userdata == nullptr || ptr == nullptr) {
        if (status)
            *status = SDL_IO_STATUS_ERROR;
        return 0;
    }

    auto* const pRWopData      = static_cast<RWopData*>(userdata);
    const auto* const pPakfile = static_cast<const Pakfile*>(pRWopData->curPakfile);
    if (pPakfile == nullptr) {
        if (status)
            *status = SDL_IO_STATUS_ERROR;
        return 0;
    }

    if (pRWopData->fileIndex >= pPakfile->fileEntries.size()) {
        if (status)
            *status = SDL_IO_STATUS_ERROR;
        return 0;
    }

    auto bytes2read = size;

    const auto readstartoffset = pPakfile->fileEntries[pRWopData->fileIndex].startOffset + pRWopData->fileOffset;

    if (readstartoffset > pPakfile->fileEntries[pRWopData->fileIndex].endOffset) {
        if (status)
            *status = SDL_IO_STATUS_EOF;
        return 0;
    }

    if (readstartoffset + bytes2read > pPakfile->fileEntries[pRWopData->fileIndex].endOffset + 1) {
        bytes2read = pPakfile->fileEntries[pRWopData->fileIndex].endOffset + 1 - readstartoffset;
        if (bytes2read == 0) {
            if (status)
                *status = SDL_IO_STATUS_EOF;
            return 0;
        }
    }

    if (SDL_SeekIO(pPakfile->fPakFile.get(), static_cast<Sint64>(readstartoffset), SDL_IO_SEEK_SET) < 0) {
        if (status)
            *status = SDL_IO_STATUS_ERROR;
        return 0;
    }

    const auto bytesRead = SDL_ReadIO(pPakfile->fPakFile.get(), ptr, bytes2read);
    if (bytesRead == 0) {
        if (status)
            *status = SDL_IO_STATUS_ERROR;
        return 0;
    }

    pRWopData->fileOffset += bytesRead;
    if (status)
        *status = SDL_IO_STATUS_READY;
    return bytesRead;
}

/// Constructor for OutPakfile
/**
    The PAK-File to be written is specified by the pakfilename-parameter. The file is opened write-only
    and written out in the destructor.
    \param pakfilename  Filename of the *.pak-File.
*/
OutPakfile::OutPakfile(const std::filesystem::path& pakfilename) : BasePakfile{pakfilename} {

    // Open for writing - SDL3: use SDL_IOFromFile
    fPakFile = sdl2::IOStream_ptr{SDL_IOFromFile(filename_.string().c_str(), "wb")};

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
        SDL_WriteIO(fPakFile.get(), &startoffset, sizeof(uint32_t));
        SDL_WriteIO(fPakFile.get(), fileEntry.filename.data(), fileEntry.filename.length() + 1);
    }

    static constexpr uint32_t tmp = 0;
    SDL_WriteIO(fPakFile.get(), &tmp, sizeof(uint32_t));

    // write out data
    SDL_WriteIO(fPakFile.get(), writeOutData, numWriteOutData);

    if (writeOutData != nullptr) {
        free(writeOutData);
        writeOutData    = nullptr;
        numWriteOutData = 0;
    }
}

/// Adds a file to this PAK-File
/**
    This methods adds the SDL_IOStream File to this PAK-File. The used name is specified by filename. If the Pakfile
    is read-only this method has no effect.
    \param  io          Data to add (the SDL_IOStream can be read-only but must support seeking)
    \param  filename    This is the filename the data is added with
*/
void OutPakfile::addFile(SDL_IOStream* io, const std::string& filename) {
    if (io == nullptr) {
        THROW(std::invalid_argument, "Pakfile::addFile(): io==nullptr is not allowed!");
    }

    const auto filelength1 = SDL_GetIOSize(io);
    if (filelength1 < 0) {
        THROW(std::runtime_error, "Pakfile::addFile(): unable to get file size!");
    }

    const auto filelength = gsl::narrow<size_t>(filelength1);

    char* extendedBuffer = nullptr;
    if ((extendedBuffer = static_cast<char*>(realloc(writeOutData, numWriteOutData + filelength))) == nullptr) {
        THROW(std::runtime_error, "Pakfile::addFile(): realloc failed!");
    }
    writeOutData = extendedBuffer;

    if (SDL_ReadIO(io, writeOutData + numWriteOutData, filelength) != filelength) {
        // revert the buffer to the original size
        char* shrinkedBuffer = nullptr;
        if ((shrinkedBuffer = static_cast<char*>(realloc(writeOutData, numWriteOutData))) == nullptr) {
            // shrinking the buffer should not fail
            THROW(std::runtime_error, "Pakfile::addFile(): realloc failed!");
        }
        writeOutData = shrinkedBuffer;
        THROW(std::runtime_error, "Pakfile::addFile(): SDL_ReadIO failed!");
    }

    PakFileEntry newPakFileEntry;
    newPakFileEntry.startOffset = gsl::narrow<uint32_t>(numWriteOutData);
    newPakFileEntry.endOffset   = gsl::narrow<uint32_t>(numWriteOutData + filelength - 1);
    newPakFileEntry.filename    = filename;

    fileEntries.push_back(newPakFileEntry);

    numWriteOutData += filelength;

    SDL_SeekIO(io, 0, SDL_IO_SEEK_SET);
}
