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

#include <algorithm>
#include <cctype>
#include <cstdio>
#include <filesystem>

#include <SDL2/SDL_filesystem.h>

#ifdef _WIN32
#    ifndef WIN32_LEAN_AND_MEAN
#        define WIN32_LEAN_AND_MEAN
#    endif
#include <io.h>
#include <direct.h>
#include <Windows.h>
#else
#include <dirent.h>
#include <sys/stat.h>
#endif


std::vector<std::filesystem::path> getFileNamesList(const std::filesystem::path& directory, const std::string& extension, bool IgnoreCase, FileListOrder fileListOrder)
{
    const auto files = getFileList(directory, extension, IgnoreCase, fileListOrder);

    std::vector<std::filesystem::path> fileNames;
    fileNames.reserve(files.size());

    for(const auto& fileInfo : files) {
        fileNames.push_back(fileInfo.name);
    }

    return fileNames;
}

namespace {
char32_t safe_tolower(char32_t c) {
    // See here for why what this code isn't right:
    //   https://stackoverflow.com/a/50407375
    // However, short of bringing in ICU, this is probably about as
    // good as we can make things. Note that "std::tolower()" is
    // undefined for anything that can't be represented by "unsigned
    // char" or is EOF.
    // https://en.cppreference.com/w/cpp/string/byte/tolower
    using nlw = std::numeric_limits<wchar_t>;

    if(c < nlw::min() || c > nlw::max()) return c;

    return towlower(static_cast<wchar_t>(c));
}

void safe_tolower_inplace(std::u32string& s32) {
    std::transform(s32.begin(), s32.end(), s32.begin(), [](char32_t c) { return safe_tolower(c); });
}

// Work around standard brain damage
// http://www.open-std.org/jtc1/sc22/wg21/docs/lwg-closed.html#721
template<class I, class E, class S>
struct codecvt : std::codecvt<I, E, S> {
    ~codecvt() = default;
};

std::string safe_tolower(std::string_view s) {
    std::wstring_convert<codecvt<char32_t, char, mbstate_t>, char32_t> conv;

    auto s32 = conv.from_bytes(s.data(), s.data() + s.size());

    safe_tolower_inplace(s32);

    return conv.to_bytes(s32);
}


} // namespace

static bool cmp_Name_Asc(const FileInfo& a, const FileInfo& b) { return (a.name.compare(b.name) < 0); }
static bool cmp_Name_CaseInsensitive_Asc(const FileInfo& a, const FileInfo& b) {
    const auto a32 = a.name.u32string();
    const auto b32 = b.name.u32string();

    for(auto i = decltype(a32.length()){0}; i < a32.length() && i < b32.length(); ++i) {
        const auto la = safe_tolower(a32[i]);
        const auto lb = safe_tolower(b32[i]);

        if(la < lb) return true;
        if(la > lb) return false;
    }

    return a32.length() < b32.length();
}

static bool cmp_Name_Dsc(const FileInfo& a, const FileInfo& b) { return (a.name.compare(b.name) > 0); }
static bool cmp_Name_CaseInsensitive_Dsc(const FileInfo& a, const FileInfo& b) {
    const auto a32 = a.name.u32string();
    const auto b32 = b.name.u32string();

    for(auto i = decltype(a32.length()){0}; i < a32.length() && i < b32.length(); ++i) {
        const auto la = safe_tolower(a32[i]);
        const auto lb = safe_tolower(b32[i]);

        if(la < lb) return false;
        if(la > lb) return true;
    }

    return a32.length() > b32.length();
}

static bool cmp_Size_Asc(const FileInfo& a, const FileInfo& b) { return a.size < b.size; }
static bool cmp_Size_Dsc(const FileInfo& a, const FileInfo& b) { return a.size > b.size; }

static bool cmp_ModifyDate_Asc(const FileInfo& a, const FileInfo& b) { return a.modifydate < b.modifydate; }
static bool cmp_ModifyDate_Dsc(const FileInfo& a, const FileInfo& b) { return a.modifydate > b.modifydate; }

std::vector<FileInfo> getFileList(const std::filesystem::path& directory, const std::string& extension,
                                  bool bIgnoreCase, FileListOrder fileListOrder) {

    std::filesystem::path target_extension_path;

    target_extension_path.replace_extension(extension);

    auto target_extension = target_extension_path.u32string();

    if(bIgnoreCase) safe_tolower_inplace(target_extension);

    std::vector<FileInfo> files;

    std::error_code ec;
    for(const auto& entry : std::filesystem::directory_iterator(directory, ec)) {
        if(ec) {
            sdl2::log_info("Scanning directory %s failed with %s", directory.u8string().c_str(), ec.message().c_str());
            break;
        }

        if(!entry.is_regular_file()) continue;

        const auto& path     = entry.path();
        const auto  filename = path.filename().u32string();

        // Make sure we have the extension, a dot, and a non-empty stem.
        if(filename.length() < target_extension.length() + 1) continue;

        const auto match = bIgnoreCase
                               ? std::equal(target_extension.rbegin(), target_extension.rend(), filename.rbegin(),
                                            [](auto a, auto b) { return a == safe_tolower(b); })
                               : std::equal(target_extension.rbegin(), target_extension.rend(), filename.rbegin(),
                                            [](auto a, auto b) { return a == b; });

        if(match) {
            const auto full_path = std::filesystem::canonical(path);

            const auto size = std::filesystem::file_size(full_path, ec);

            if(ec) {
                sdl2::log_info("Getting size of %s failed with %s", full_path.u8string().c_str(), ec.message().c_str());
                continue;
            }

            const auto modified = std::filesystem::last_write_time(full_path, ec);
            if(ec) {
                sdl2::log_info("Getting last modified time of %s failed with %s", full_path.u8string().c_str(),
                        ec.message().c_str());
                continue;
            }

            files.emplace_back(full_path.filename().u8string(), size, modified);
        }
    }

    switch(fileListOrder) {
        case FileListOrder_Name_Asc: {
            std::sort(files.begin(), files.end(), cmp_Name_Asc);
        } break;

        case FileListOrder_Name_CaseInsensitive_Asc: {
            std::sort(files.begin(), files.end(), cmp_Name_CaseInsensitive_Asc);
        } break;

        case FileListOrder_Name_Dsc: {
            std::sort(files.begin(), files.end(), cmp_Name_Dsc);
        } break;

        case FileListOrder_Name_CaseInsensitive_Dsc: {
            std::sort(files.begin(), files.end(), cmp_Name_CaseInsensitive_Dsc);
        } break;

        case FileListOrder_Size_Asc: {
            std::sort(files.begin(), files.end(), cmp_Size_Asc);
        } break;

        case FileListOrder_Size_Dsc: {
            std::sort(files.begin(), files.end(), cmp_Size_Dsc);
        } break;

        case FileListOrder_ModifyDate_Asc: {
            std::sort(files.begin(), files.end(), cmp_ModifyDate_Asc);
        } break;

        case FileListOrder_ModifyDate_Dsc: {
            std::sort(files.begin(), files.end(), cmp_ModifyDate_Dsc);
        } break;

        case FileListOrder_Unsorted:
        default: {
            // do nothing
        } break;
    }

    return files;
}

bool getCaseInsensitiveFilename(std::filesystem::path& filepath) {
    std::error_code ec;
    const auto      cpath = std::filesystem::canonical(filepath, ec);

    if(!ec) {
        filepath = cpath;
        return true;
    }

    const auto has_parent = filepath.has_parent_path();
    const auto parent     = has_parent ? filepath.parent_path() : std::filesystem::current_path();
    const auto wanted     = filepath.filename();
    const auto wanted_u8  = wanted.u8string();

    for(const auto& p : std::filesystem::directory_iterator(parent)) {
        if(!p.is_regular_file()) continue;

        const auto filename = p.path().filename();

        if(wanted != filename) {
            const auto filename_u8 = filename.u8string();

            if(wanted_u8.length() != filename_u8.length()) continue;

            const auto match =
                std::equal(wanted_u8.begin(), wanted_u8.end(), filename_u8.begin(), filename_u8.end(),
                           [](const auto a, const auto b) { return a == b || std::toupper(a) == std::toupper(b); });

            if(!match) continue;
        }

        filepath = has_parent ? p.path() : filename;

        return true;
    }

    return false;
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

    const int64_t filesize = SDL_RWsize(RWopsFile.get());
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
            const sdl2::sdl_ptr<char> basePath{ SDL_GetBasePath() };

            if(basePath == nullptr) {
                THROW(sdl_error, "SDL_GetBasePath() failed: %s!", SDL_GetError());
            }
            dataDir = basePath.get();
        }

        duneLegacyDataDir = dataDir.lexically_normal().make_preferred();
    }
    return duneLegacyDataDir;
}
