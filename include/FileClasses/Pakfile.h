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

#include <stdio.h>
#include <string>
#include <vector>
#include <inttypes.h>

#define PAKFILE_RWOP_TYPE   0x9A5F17EC

/// A class for reading PAK-Files.
/**
    This class can be used to read PAK-Files. PAK-Files are archive files used by Dune2.
    The files inside the PAK-File can an be read through SDL_RWops.
*/
class Pakfile
{
private:

    /// Internal structure for representing one file in this PAK-File
    struct PakFileEntry {
        uint32_t startOffset;
        uint32_t endOffset;
        std::string filename;
    };

    /// Internal structure used by opened SDL_RWop
    struct RWopData {
        Pakfile* curPakfile;
        unsigned int fileIndex;
        size_t fileOffset;
    };

public:
    Pakfile(const std::string& pakfilename, bool write = false);
    ~Pakfile();

    const std::string& getFilename(unsigned int index) const;

    /// Number of files in this pak-File.
    /**
        Returns the number of files in this pak-File.
        \return Number of files.
    */
    inline int getNumFiles() const { return fileEntries.size(); };

    sdl2::RWops_ptr openFile(const std::string& filename);

    bool exists(const std::string& filename) const;

    void addFile(SDL_RWops* rwop, const std::string& filename);

private:
    static size_t ReadFile(SDL_RWops* pRWop, void *ptr, size_t size, size_t n);
    static size_t WriteFile(SDL_RWops *pRWop, const void *ptr, size_t size, size_t n);
    static Sint64 SizeFile(SDL_RWops *pRWop);
    static Sint64 SeekFile(SDL_RWops *pRWop, Sint64 offset, int whence);
    static int CloseFile(SDL_RWops *pRWop);

    void readIndex();

    bool write;
    SDL_RWops * fPakFile;
    std::string filename;

    char* writeOutData;
    int numWriteOutData;
    std::vector<PakFileEntry> fileEntries;
};

#endif // PAKFILE_H
