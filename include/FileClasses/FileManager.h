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

#ifndef FILEMANAGER_H
#define FILEMANAGER_H

#include "Pakfile.h"
#include <misc/SDL2pp.h>

#include <filesystem>
#include <memory>
#include <optional>
#include <span>
#include <string>
#include <unordered_map>
#include <vector>

class CaseInsensitiveFilesystemCache final {
public:
    explicit CaseInsensitiveFilesystemCache(std::span<const std::filesystem::path> paths);

    [[nodiscard]] std::optional<std::filesystem::path> find(const std::u8string& filename) const;

    void refresh(std::span<const std::filesystem::path> paths);

private:
    struct CaseInsensitiveEqualTo {
        bool operator()(const std::u8string& lhs, const std::u8string& rhs) const;
    };
    struct CaseInsensitiveHash {
        size_t operator()(const std::u8string& v) const;
    };

    using filesystem_directory_type =
        std::unordered_map<std::u8string, std::filesystem::path, CaseInsensitiveHash, CaseInsensitiveEqualTo>;

    [[nodiscard]] filesystem_directory_type
    createFilesystemDirectory(std::span<const std::filesystem::path> paths) const;

    filesystem_directory_type files_;
};

class PakFileConfiguration final {
public:
    [[nodiscard]] static std::vector<std::string> getNeededFiles();
    [[nodiscard]] static std::vector<std::filesystem::path>
    getMissingFiles(const CaseInsensitiveFilesystemCache& cache);
};

class PakFileManager final {
public:
    explicit PakFileManager(const CaseInsensitiveFilesystemCache& cache, std::span<const std::string> files);

    [[nodiscard]] std::tuple<const Pakfile*, int> find(const std::string& filename) const;

    bool exists(const std::string& filename) const;

private:
    using pak_directory_type = std::unordered_map<std::string, std::tuple<Pakfile*, int>>;

    [[nodiscard]] static std::string md5FromFilename(std::filesystem::path filename);

    [[nodiscard]] static std::vector<std::unique_ptr<Pakfile>>
    loadPakFiles(const CaseInsensitiveFilesystemCache& cache, std::span<const std::string> files);

    [[nodiscard]] pak_directory_type createPakDirectory() const;

    const std::vector<std::unique_ptr<Pakfile>> pakFiles_;
    const pak_directory_type pak_directory_;
};

/// A class for loading all the PAK-Files.
/**
    This class manages all the PAK-Files and provides access to the contained files through SDL_RWops.
*/
class FileManager final {
public:
    /**
        Constructor.
    */
    FileManager();

    FileManager(const FileManager& fileManager) = delete;
    FileManager(FileManager&& fileManager)      = delete;

    ~FileManager();

    FileManager& operator=(const FileManager& fileManager) = delete;
    FileManager& operator=(FileManager&& fileManager)      = delete;

    [[nodiscard]] static const std::vector<std::filesystem::path>& getSearchPath();

    /**
        Opens the file specified via filename. This method first tries to open the file in one of the
        search paths (see getSearchPath()). If no file exists with the given name the content of all
        pak files is considered. In case the file cannot be found an io_error is thrown.
        \param  filename    the filename to look for
        \return a rwop to read the content of the specified file. Use SDL_RWclose() to close the file after usage.

    */
    [[nodiscard]] sdl2::RWops_ptr openFile(std::filesystem::path filename) const;

    [[nodiscard]] bool exists(std::filesystem::path filename) const;

private:
    CaseInsensitiveFilesystemCache filesystem_cache_;
    PakFileManager pak_files_;
};

#endif // FILEMANAGER_H
