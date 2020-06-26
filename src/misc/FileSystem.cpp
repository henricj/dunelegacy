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

#include <misc/FileSystem.h>
#include <misc/string_util.h>
#include <misc/exceptions.h>
#include <misc/SDL2pp.h>

#include <cstdio>
#include <algorithm>
#include <filesystem>
#include <ctype.h>

#include <SDL2/SDL_filesystem.h>

#ifdef _WIN32
#include <io.h>
#include <direct.h>
#include <windows.h>
#else
#include <dirent.h>
#include <sys/stat.h>
#endif


std::vector<std::filesystem::path> getFileNamesList(const std::filesystem::path& directory, const std::string& extension, bool IgnoreCase, FileListOrder fileListOrder)
{
    std::vector<FileInfo> files = getFileList(directory, extension, IgnoreCase, fileListOrder);

    std::vector<std::filesystem::path> fileNames;

    for(const auto& fileInfo : files) {
        fileNames.push_back(fileInfo.name);
    }

    return fileNames;
}

static bool cmp_Name_Asc(const FileInfo& a, const FileInfo& b) { return (a.name.compare(b.name) < 0); }
static bool cmp_Name_CaseInsensitive_Asc(const FileInfo& a, const FileInfo& b) {
    const auto a32 = a.name.u32string();
    const auto b32 = b.name.u32string();

    unsigned int i = 0;
    while ((i < a32.length()) && (i < b32.length())) {
        if (tolower(a32[i]) < tolower(b32[i])) {
            return true;
        }
        else if (tolower(a32[i]) > tolower(b32[i])) {
            return false;
        }
        i++;
    }

    return (a32.length() < b32.length());
}

static bool cmp_Name_Dsc(const FileInfo& a, const FileInfo& b) { return (a.name.compare(b.name) > 0); }
static bool cmp_Name_CaseInsensitive_Dsc(const FileInfo& a, const FileInfo& b) {
    const auto a32 = a.name.u32string();
    const auto b32 = b.name.u32string();

    unsigned int i = 0;
    while ((i < a32.length()) && (i < b32.length())) {
        if (tolower(a32[i]) < tolower(b32[i])) {
            return false;
        }
        else if (tolower(a32[i]) > tolower(b32[i])) {
            return true;
        }
        i++;
    }

    return (a32.length() > b32.length());
}

static bool cmp_Size_Asc(const FileInfo& a, const FileInfo& b) { return a.size < b.size; }
static bool cmp_Size_Dsc(const FileInfo& a, const FileInfo& b) { return a.size > b.size; }

static bool cmp_ModifyDate_Asc(const FileInfo& a, const FileInfo& b) { return a.modifydate < b.modifydate; }
static bool cmp_ModifyDate_Dsc(const FileInfo& a, const FileInfo& b) { return a.modifydate > b.modifydate; }

std::vector<FileInfo> getFileList(const std::filesystem::path& directory, const std::string& extension, bool bIgnoreCase, FileListOrder fileListOrder)
{
    std::vector<FileInfo> Files;
    std::filesystem::path lowerExtension;

    lowerExtension.replace_extension(bIgnoreCase ? strToLower(extension) : extension);

    std::error_code ec;

    for (const auto& entry : std::filesystem::directory_iterator(directory, ec))
    {
        if (!entry.is_regular_file()) continue;

        //const auto& status = entry.status();

        const auto& path = entry.path();
        const auto filename = path.filename();

        auto ext = filename.extension();

        if (bIgnoreCase == true) {
            auto tmp = ext.u8string();
            convertToLower(tmp);
            ext = std::filesystem::u8path(tmp);
        }

        if (ext == lowerExtension) {
            auto fullpath = std::filesystem::canonical(path);

            std::error_code ec;
            const auto size = std::filesystem::file_size(fullpath, ec);

            if (ec) {
                SDL_Log("Getting size of %s failed with %s", fullpath.u8string().c_str(), strerror(ec.value()));
                continue;
            }

            const auto modified = std::filesystem::last_write_time(fullpath, ec);
            if (ec) {
                SDL_Log("Getting last modified time of %s failed with %s", fullpath.u8string().c_str(), strerror(ec.value()));
                continue;
            }

            Files.emplace_back(filename, size, modified);
        }
    }

    if (!Files.empty())
    switch(fileListOrder) {
        case FileListOrder_Name_Asc: {
            std::sort(Files.begin(), Files.end(), cmp_Name_Asc);
        } break;

        case FileListOrder_Name_CaseInsensitive_Asc: {
            std::sort(Files.begin(), Files.end(), cmp_Name_CaseInsensitive_Asc);
        } break;

        case FileListOrder_Name_Dsc: {
            std::sort(Files.begin(), Files.end(), cmp_Name_Dsc);
        } break;

        case FileListOrder_Name_CaseInsensitive_Dsc: {
            std::sort(Files.begin(), Files.end(), cmp_Name_CaseInsensitive_Dsc);
        } break;

        case FileListOrder_Size_Asc: {
            std::sort(Files.begin(), Files.end(), cmp_Size_Asc);
        } break;

        case FileListOrder_Size_Dsc: {
            std::sort(Files.begin(), Files.end(), cmp_Size_Dsc);
        } break;

        case FileListOrder_ModifyDate_Asc: {
            std::sort(Files.begin(), Files.end(), cmp_ModifyDate_Asc);
        } break;

        case FileListOrder_ModifyDate_Dsc: {
            std::sort(Files.begin(), Files.end(), cmp_ModifyDate_Dsc);
        } break;

        case FileListOrder_Unsorted:
        default: {
            // do nothing
        } break;
    }

    return Files;

}

bool getCaseInsensitiveFilename(std::filesystem::path& filepath) {

#ifdef _WIN32
    return existsFile(filepath);

#else


    std::string filename;
    std::string path;
    size_t separatorPos = filepath.rfind('/');
    if(separatorPos == std::string::npos) {
        // There is no '/' in the filepath => only filename in current working directory specified
        filename = filepath;
        path = ".";
    } else if(separatorPos == filepath.length()-1) {
        // filepath has an '/' at the end => no filename specified
        return false;
    } else {
        filename = filepath.substr(separatorPos+1, std::string::npos);
        path = filepath.substr(0,separatorPos+1); // path with tailing '/'
    }

    DIR* directory = opendir(path.c_str());
    if(directory == nullptr) {
        return false;
    }

    while(true) {
        errno = 0;
        dirent* directory_entry = readdir(directory);
        if(directory_entry == nullptr) {
            if (errno != 0) {
                closedir(directory);
                return false;
            } else {
                // EOF
                break;
            }
        }

        bool entry_OK = true;
        const char* pEntryName = directory_entry->d_name;
        const char* pFilename = filename.c_str();
        while(true) {
            if((*pEntryName == '\0') && (*pFilename == '\0')) {
                break;
            }

            if(tolower(*pEntryName) != tolower(*pFilename)) {
                entry_OK = false;
                break;
            }
            pEntryName++;
            pFilename++;
        }

        if(entry_OK == true) {
            if(path == ".") {
                filepath = directory_entry->d_name;
            } else {
                filepath = path + directory_entry->d_name;
            }
            closedir(directory);
            return true;
        }
    }
    closedir(directory);
    return false;

#endif

}


bool existsFile(const std::filesystem::path& path) {
    std::error_code ec;
    return std::filesystem::exists(path, ec);
}

std::string readCompleteFile(const std::filesystem::path& filename) {
    auto RWopsFile = sdl2::RWops_ptr{ SDL_RWFromFile(filename.u8string().c_str(),"r") };

    if(!RWopsFile) {
        return "";
    }

    const Sint64 filesize = SDL_RWsize(RWopsFile.get());
    if(filesize < 0) {
        return "";
    }

    std::unique_ptr<char[]> filedata = std::make_unique<char[]>((size_t) filesize);

    if(SDL_RWread(RWopsFile.get(), filedata.get(), (size_t) filesize, 1) != 1) {
        return "";
    }

    std::string retValue(filedata.get(), (size_t) filesize);

    return retValue;
}

std::filesystem::path getBasename(const std::filesystem::path& filepath, bool bStripExtension) {

    if(filepath == "/") {
        // special case
        return "/";
    }

    auto path = filepath.lexically_normal();

    return bStripExtension ? path.stem() : path.filename();
}

std::filesystem::path getDirname(const std::filesystem::path& filepath) {

    if(filepath == "/") {
        // special case
        return "/";
    }

    auto path = filepath.lexically_normal();

    return path.parent_path();
}

static std::filesystem::path duneLegacyDataDir;

std::filesystem::path getDuneLegacyDataDir() {
    if(duneLegacyDataDir.empty()) {

        std::filesystem::path dataDir;
#ifdef DUNELEGACY_DATADIR
        dataDir = DUNELEGACY_DATADIR;
        dataDir = dataDir.lexically_normal();
#endif

        if((dataDir.empty()) || (dataDir == ".") || (dataDir == "./") || (dataDir == ".\\")) {
            auto basePath = sdl2::sdl_ptr<char>{ SDL_GetBasePath() };

            if(basePath.get() == nullptr) {
                THROW(sdl_error, "SDL_GetBasePath() failed: %s!", SDL_GetError());
            }
            dataDir = basePath.get();
        }

        duneLegacyDataDir = dataDir.lexically_normal();
    }
    return duneLegacyDataDir;
}
