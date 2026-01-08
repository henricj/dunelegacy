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

#ifndef PAKFILE_H
#define PAKFILE_H

#include <misc/SDL2pp.h>

#include <filesystem>
#include <string>
#include <vector>

#include <cstddef>

class BasePakfile {
protected:
    explicit BasePakfile(std::filesystem::path pakfilename);
    ~BasePakfile();

public:
    /// Number of files in this pak-File.
    /**
        Returns the number of files in this pak-File.
        \return Number of files.
    */
    [[nodiscard]] int getNumFiles() const { return fileEntries.size(); }

    [[nodiscard]] const std::string& getFilename(unsigned int index) const;

    [[nodiscard]] bool exists(const std::string& filename) const;

protected:
    /// Internal structure used by opened SDL_IOStream
    struct RWopData {
        const BasePakfile* curPakfile;
        unsigned int fileIndex;
        size_t fileOffset;
    };
    /// Internal structure for representing one file in this PAK-File
    struct PakFileEntry final {
        uint32_t startOffset;
        uint32_t endOffset;
        std::string filename;
    };

    static Sint64 SizeFile(void* userdata);
    static Sint64 SeekFile(void* userdata, Sint64 offset, SDL_IOWhence whence);
    static size_t WriteFile(void* userdata, const void* ptr, size_t size, SDL_IOStatus* status);
    static bool CloseFile(void* userdata);

    void readIndex();

    sdl2::IOStream_ptr fPakFile;
    std::filesystem::path filename_;

    std::vector<PakFileEntry> fileEntries;
};

/// A class for reading PAK-Files.
/**
    This class can be used to read PAK-Files. PAK-Files are archive files used by Dune2.
    The files inside the PAK-File can an be read through SDL_IOStream.
*/
class Pakfile final : public BasePakfile {
public:
    explicit Pakfile(const std::filesystem::path& pakfilename);
    ~Pakfile();

    sdl2::IOStream_ptr openFile(const std::string& filename) const;
    sdl2::IOStream_ptr openFile(int index) const;

private:
    static size_t ReadFile(void* userdata, void* ptr, size_t size, SDL_IOStatus* status);
};

/**
    This class can be used to write PAK-Files. PAK-Files are archive files used by Dune2.
*/
class OutPakfile : public BasePakfile {
public:
    explicit OutPakfile(const std::filesystem::path& pakfilename);
    ~OutPakfile();

    void addFile(SDL_IOStream* io, const std::string& filename);

private:
    char* writeOutData{};
    size_t numWriteOutData{};
};

#endif // PAKFILE_H
