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

#if 0
#    ifndef PAKFILE_CACHE_READS
#        define PAKFILE_CACHE_READS 1
#    endif
#endif // defined(_DEBUG)

/// Constructor for Pakfile
/**
    The PAK-File to be read/write is specified by the pakfilename-parameter. If write==false the file is opened for
    read-only and is closed in the destructor. If write==true the file is opened write-only and written out in the
    destructor.
    \param pakfilename  Filename of the *.pak-File.
    \param write        Specified if the PAK-File is opened for reading or writing (default is false).
*/
Pakfile::Pakfile(const std::filesystem::path& pakfilename, bool write)
    : write(write), fPakFile(nullptr), filename(pakfilename), writeOutData(nullptr), numWriteOutData(0) {

    if (!write) {
        // Open for reading
        sdl2::RWops_ptr file{SDL_RWFromFile(filename.u8string().c_str(), "rb")};

        if (file == nullptr) {
            THROW(std::invalid_argument, "Pakfile::Pakfile(): Cannot open " + pakfilename.string() + "!");
        }

#if defined(PAKFILE_CACHE_READS)
        const auto size = SDL_RWsize(file.get());

        read_buffer_.resize(size);

        if (1 != SDL_RWread(file.get(), read_buffer_.data(), read_buffer_.size(), 1))
            THROW(std::invalid_argument, "Pakfile::Pakfile(): Cannot read " + pakfilename.string() + "!");

        fPakFile = sdl2::RWops_ptr{SDL_RWFromConstMem(read_buffer_.data(), read_buffer_.size())};

        if (!fPakFile) {
            THROW(std::invalid_argument, "Pakfile::Pakfile(): Cannot open " + pakfilename.string() + "!");
        }
#else
        fPakFile = std::move(file);
#endif // defined(PAKFILE_CACHE_READS)

        readIndex();
    } else {
        // Open for writing
        fPakFile = sdl2::RWops_ptr{SDL_RWFromFile(filename.u8string().c_str(), "wb")};

        if (!fPakFile) {
            THROW(std::invalid_argument, "Pakfile::Pakfile(): Cannot open " + pakfilename.string() + "!");
        }
    }
}

/// Destructor
/**
    Closes the filehandle and releases all memory.
*/
Pakfile::~Pakfile() {
    if (write) {
        // calculate header size
        int headersize = 0;
        for (auto& fileEntrie : fileEntries) {
            headersize += 4;
            headersize += fileEntrie.filename.u8string().length() + 1;
        }
        headersize += 4;

        // write out header
        for (auto& fileEntrie : fileEntries) {
            uint32_t startoffset = SDL_SwapLE32(fileEntrie.startOffset + headersize);
            SDL_RWwrite(fPakFile.get(), &startoffset, sizeof(uint32_t), 1);
            SDL_RWwrite(fPakFile.get(), fileEntrie.filename.u8string().c_str(),
                        fileEntrie.filename.u8string().length() + 1, 1);
        }
        static constexpr uint32_t tmp = 0;
        SDL_RWwrite(fPakFile.get(), &tmp, sizeof(uint32_t), 1);

        // write out data
        SDL_RWwrite(fPakFile.get(), writeOutData, numWriteOutData, 1);
    }

    if (writeOutData != nullptr) {
        free(writeOutData);
        writeOutData    = nullptr;
        numWriteOutData = 0;
    }
}

/// Returns the name of the nth file in this pak-File.
/**
    Returns the name of the nth file in this pak-File. index specifies which file.
    \param  index   Index in pak-File
    \return name of the file specified by index
*/
const std::filesystem::path& Pakfile::getFilename(unsigned int index) const {
    if (index >= fileEntries.size()) {
        THROW(std::invalid_argument, "Pakfile::getFilename(%ud): This Pakfile has only %ud entries!", index,
              fileEntries.size());
    }

    return fileEntries[index].filename;
}

/// Adds a file to this PAK-File
/**
    This methods adds the SDL_RWop File to this PAK-File. The used name is specified by filename. If the Pakfile
    is read-only this method has no effect.
    \param  rwop        Data to add (the SDL_RWop can be read-only but must support seeking)
    \param  filename    This is the filename the data is added with
*/
void Pakfile::addFile(SDL_RWops* rwop, const std::string& filename) {
    if (!write) {
        THROW(std::runtime_error, "Pakfile::addFile(): Pakfile is opened for read-only!");
    }

    if (rwop == nullptr) {
        THROW(std::invalid_argument, "Pakfile::addFile(): rwop==nullptr is not allowed!");
    }

    const auto filelength = static_cast<size_t>(SDL_RWsize(rwop));

    char* extendedBuffer = nullptr;
    if ((extendedBuffer = static_cast<char*>(realloc(writeOutData, numWriteOutData + filelength))) == nullptr) {
        throw std::bad_alloc();
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
    newPakFileEntry.startOffset = numWriteOutData;
    newPakFileEntry.endOffset   = numWriteOutData + filelength - 1;
    newPakFileEntry.filename    = filename;

    fileEntries.push_back(newPakFileEntry);

    numWriteOutData += filelength;

    SDL_RWseek(rwop, 0, SEEK_SET);
}

/// Opens a file in this PAK-File.
/**
    This method opens the file specified by filename. It is only allowed if the Pakfile is opened for reading.
    The returned SDL_RWops-structure can be used readonly with SDL_RWread, SDL_RWsize, SDL_RWseek and SDL_RWclose. No
   writing is supported.<br> NOTICE: The returned SDL_RWops-Structure is only valid as long as this Pakfile-Object
   exists. It gets invalid as soon as Pakfile:~Pakfile() is executed. \param  filename    The name of this file \return
   SDL_RWops for this file
*/
sdl2::RWops_ptr Pakfile::openFile(const std::filesystem::path& filename) {
    if (write) {
        THROW(std::runtime_error, "Pakfile::openFile(): Writing files is not supported!");
    }

    PakFileEntry* file_entry = nullptr;

    // find file
    int index = -1;
    for (unsigned int i = 0; i < fileEntries.size(); i++) {
        if (filename == fileEntries[i].filename) {
            index      = i;
            file_entry = &fileEntries[i];
            break;
        }
    }

    if (index == -1 || file_entry == nullptr) {
        THROW(io_error, "Pakfile::openFile(): Cannot find file with name '%s' in this PAK file!",
              filename.string().c_str());
    }

#if defined(PAKFILE_CACHE_READS)
    auto* const p   = read_buffer_.data() + file_entry->startOffset;
    const auto size = file_entry->endOffset - file_entry->startOffset + 1;

    sdl2::RWops_ptr pRWop{SDL_RWFromConstMem(p, size)};
#else  // defined(PAKFILE_CACHE_READS)
    // alloc RWop

    sdl2::RWops_ptr pRWop{SDL_AllocRW()};

    if (!pRWop)
        throw std::bad_alloc();

    // alloc RWopData
    auto* pRWopData = new RWopData();

    pRWop->type = PAKFILE_RWOP_TYPE;
    pRWopData->curPakfile = this;
    pRWopData->fileOffset = 0;
    pRWopData->fileIndex = index;

    pRWop->read = Pakfile::ReadFile;
    pRWop->write = Pakfile::WriteFile;
    pRWop->size = Pakfile::SizeFile;
    pRWop->seek = Pakfile::SeekFile;
    pRWop->close = Pakfile::CloseFile;
    pRWop->hidden.unknown.data1 = static_cast<void*>(pRWopData);
#endif // defined(PAKFILE_CACHE_READS)

    if (!pRWop) {
        THROW(io_error, "Pakfile::openFile(): Cannot open file with name '%s' in this PAK file!",
              filename.string().c_str());
    }

    return pRWop;
}

bool Pakfile::exists(const std::filesystem::path& filename) const {
    for (const auto& fileEntrie : fileEntries) {
        if (filename == fileEntrie.filename) {
            return true;
        }
    }

    return false;
}

size_t Pakfile::ReadFile(SDL_RWops* pRWop, void* ptr, size_t size, size_t n) {
    if (pRWop == nullptr || ptr == nullptr || pRWop->hidden.unknown.data1 == nullptr
        || pRWop->type != PAKFILE_RWOP_TYPE) {
        return 0;
    }

    int bytes2read = size * n;

    auto* pRWopData         = static_cast<RWopData*>(pRWop->hidden.unknown.data1);
    const Pakfile* pPakfile = pRWopData->curPakfile;
    if (pPakfile == nullptr) {
        return 0;
    }

    if (pRWopData->fileIndex >= pPakfile->fileEntries.size()) {
        return 0;
    }

    const uint32_t readstartoffset = pPakfile->fileEntries[pRWopData->fileIndex].startOffset + pRWopData->fileOffset;

    if (readstartoffset > pPakfile->fileEntries[pRWopData->fileIndex].endOffset) {
        return 0;
    }

    if (readstartoffset + bytes2read > pPakfile->fileEntries[pRWopData->fileIndex].endOffset) {
        bytes2read = pPakfile->fileEntries[pRWopData->fileIndex].endOffset + 1 - readstartoffset;
        // round to last full block
        bytes2read /= size;
        bytes2read *= size;
        if (bytes2read == 0) {
            return 0;
        }
    }

    if (SDL_RWseek(pPakfile->fPakFile.get(), readstartoffset, SEEK_SET) < 0) {
        return 0;
    }

    if (SDL_RWread(pPakfile->fPakFile.get(), ptr, bytes2read, 1) != 1) {
        return 0;
    }

    pRWopData->fileOffset += bytes2read;
    return bytes2read / size;
}

size_t Pakfile::WriteFile(SDL_RWops* pRWop, const void* ptr, size_t size, size_t n) {
    return 0;
}

int64_t Pakfile::SizeFile(SDL_RWops* pRWop) {
    if (pRWop == nullptr || pRWop->hidden.unknown.data1 == nullptr || pRWop->type != PAKFILE_RWOP_TYPE) {
        return -1;
    }

    const auto* pRWopData   = static_cast<RWopData*>(pRWop->hidden.unknown.data1);
    const Pakfile* pPakfile = pRWopData->curPakfile;
    if (pPakfile == nullptr) {
        return -1;
    }

    if (pRWopData->fileIndex >= pPakfile->fileEntries.size()) {
        return -1;
    }

    return pPakfile->fileEntries[pRWopData->fileIndex].endOffset
         - pPakfile->fileEntries[pRWopData->fileIndex].startOffset + 1;
}

int64_t Pakfile::SeekFile(SDL_RWops* pRWop, int64_t offset, int whence) {
    if (pRWop == nullptr || pRWop->hidden.unknown.data1 == nullptr || pRWop->type != PAKFILE_RWOP_TYPE) {
        return -1;
    }

    auto* pRWopData         = static_cast<RWopData*>(pRWop->hidden.unknown.data1);
    const Pakfile* pPakfile = pRWopData->curPakfile;
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

int Pakfile::CloseFile(SDL_RWops* pRWop) {
    if (pRWop == nullptr || pRWop->hidden.unknown.data1 == nullptr || pRWop->type != PAKFILE_RWOP_TYPE) {
        return -1;
    }

    const auto* pRWopData = static_cast<RWopData*>(pRWop->hidden.unknown.data1);
    delete pRWopData;
    pRWop->hidden.unknown.data1 = nullptr;
    SDL_FreeRW(pRWop);
    return 0;
}

void Pakfile::readIndex() {
    while (true) {
        PakFileEntry newEntry;

        if (SDL_RWread(fPakFile.get(), &newEntry.startOffset, sizeof newEntry.startOffset, 1) != 1) {
            THROW(std::runtime_error, "Pakfile::readIndex(): SDL_RWread() failed!");
        }

        // pak-files are always little endian encoded
        newEntry.startOffset = SDL_SwapLE32(newEntry.startOffset);
        newEntry.endOffset   = 0;

        if (newEntry.startOffset == 0) {
            break;
        }

        while (true) {
            char tmp = 0;
            if (SDL_RWread(fPakFile.get(), &tmp, 1, 1) != 1) {
                THROW(std::runtime_error, "Pakfile::readIndex(): SDL_RWread() failed!");
            }

            if (tmp == '\0') {
                break;
            }
            newEntry.filename += tmp;
        }

        if (!fileEntries.empty()) {
            fileEntries.back().endOffset = newEntry.startOffset - 1;
        }

        fileEntries.push_back(newEntry);
    }

    const int64_t filesize = SDL_RWsize(fPakFile.get());
    if (filesize < 0) {
        THROW(std::runtime_error, "Pakfile::readIndex(): SDL_RWsize() failed!");
    }

    fileEntries.back().endOffset = static_cast<size_t>(filesize) - 1;
}
