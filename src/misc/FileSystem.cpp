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

#include <stdio.h>
#include <algorithm>
#include <ctype.h>

#include <SDL_rwops.h>

#ifdef _WIN32
#include <io.h>
#include <direct.h>
#include <windows.h>
#else
#include <dirent.h>
#include <errno.h>
#include <sys/stat.h>
#include <string.h>
#endif

// Used by FileManager::getResourcesBundlePath
#if defined(DUNELEGACY_PLATFORM_OSX)
    #include <CoreFoundation/CoreFoundation.h>
#endif

std::list<std::string> getFileNamesList(std::string directory, std::string extension, bool IgnoreCase, FileListOrder fileListOrder)
{
    std::list<FileInfo> Files = getFileList(directory, extension, IgnoreCase, fileListOrder);

    std::list<std::string> FileNames;

    std::list<FileInfo>::iterator iter;
    for(iter = Files.begin(); iter != Files.end(); ++iter) {
        FileNames.push_back(iter->name);
    }

    return FileNames;
}

static bool cmp_Name_Asc(FileInfo& a, FileInfo& b) { return (a.name.compare(b.name) < 0); }
static bool cmp_Name_CaseInsensitive_Asc(FileInfo& a, FileInfo& b) {
  unsigned int i=0;
  while((i < a.name.length()) && (i < b.name.length())) {
    if(tolower(a.name[i]) < tolower(b.name[i])) {
        return true;
    } else if (tolower(a.name[i]) > tolower(b.name[i])) {
        return false;
    }
    i++;
  }

  return (a.name.length() < b.name.length());
}

static bool cmp_Name_Dsc(FileInfo& a, FileInfo& b) { return (a.name.compare(b.name) > 0); }
static bool cmp_Name_CaseInsensitive_Dsc(FileInfo& a, FileInfo& b) {
  unsigned int i=0;
  while((i < a.name.length()) && (i < b.name.length())) {
    if(tolower(a.name[i]) < tolower(b.name[i])) {
        return false;
    } else if (tolower(a.name[i]) > tolower(b.name[i])) {
        return true;
    }
    i++;
  }

  return (a.name.length() > b.name.length());
}

static bool cmp_Size_Asc(FileInfo& a, FileInfo& b) { return a.size < b.size; }
static bool cmp_Size_Dsc(FileInfo& a, FileInfo& b) { return a.size > b.size; }

static bool cmp_ModifyDate_Asc(FileInfo& a, FileInfo& b) { return a.modifydate < b.modifydate; }
static bool cmp_ModifyDate_Dsc(FileInfo& a, FileInfo& b) { return a.modifydate > b.modifydate; }

std::list<FileInfo> getFileList(std::string directory, std::string extension, bool bIgnoreCase, FileListOrder fileListOrder)
{

	std::list<FileInfo> Files;

	if(bIgnoreCase == true) {
		convertToLower(extension);
	}

#ifdef _WIN32
    // on win32 we need an ansi-encoded filepath
    WCHAR szwPath[MAX_PATH];
    char szPath[MAX_PATH];

    if(MultiByteToWideChar(CP_UTF8, 0, directory.c_str(), -1, szwPath, MAX_PATH) == 0) {
        fprintf(stderr, "getFileList(): Conversion of search path from utf-8 to utf-16 failed\n");
        return Files;
    }

    if(WideCharToMultiByte(CP_ACP, 0, szwPath, -1, szPath, MAX_PATH, nullptr, nullptr) == 0) {
        fprintf(stderr, "getFileList(): Conversion of search path from utf-16 to ansi failed\n");
        return Files;
    }

	long hFile;

	_finddata_t fdata;

	std::string searchString = std::string(szPath) + "/*";

	if ((hFile = (long)_findfirst(searchString.c_str(), &fdata)) != -1L) {
		do {
			std::string filename = fdata.name;

            if(filename.length() < extension.length()+1) {
                continue;
			}

			if(filename[filename.length() - extension.length() - 1] != '.') {
                continue;
			}

			std::string ext = filename.substr(filename.length() - extension.length());

			if(bIgnoreCase == true) {
				convertToLower(ext);
            }

			if(ext == extension) {
                // on win32 we get an ansi-encoded filename
                WCHAR szwFilename[MAX_PATH];
                char szFilename[MAX_PATH];

                if(MultiByteToWideChar(CP_ACP, 0, filename.c_str(), -1, szwFilename, MAX_PATH) == 0) {
                    fprintf(stderr, "getFileList(): Conversion of filename from ansi to utf-16 failed\n");
                    continue;
                }

                if(WideCharToMultiByte(CP_UTF8, 0, szwFilename, -1, szFilename, MAX_PATH, nullptr, nullptr) == 0) {
                    fprintf(stderr, "getFileList(): Conversion of search path from utf-16 to utf-8 failed\n");
                    continue;
                }

				Files.push_back(FileInfo(szFilename, fdata.size, fdata.time_write));
			}
		} while(_findnext(hFile, &fdata) == 0);

		_findclose(hFile);
	}

#else

	DIR * dir = opendir(directory.c_str());
	dirent *curEntry;

	if(dir == nullptr) {
		return Files;
	}

	errno = 0;
	while((curEntry = readdir(dir)) != nullptr) {
			std::string filename = curEntry->d_name;

			if(filename.length() < extension.length()+1) {
                continue;
			}

			if(filename[filename.length() - extension.length() - 1] != '.') {
                continue;
			}

			std::string ext = filename.substr(filename.length() - extension.length());

			if(bIgnoreCase == true) {
				convertToLower(ext);
			}

			if(ext == extension) {
			    std::string fullpath = directory + "/" + filename;
			    struct stat fdata;
			    if(stat(fullpath.c_str(), &fdata) != 0) {
                    perror("stat()");
                    continue;
			    }
				Files.push_back(FileInfo(filename, fdata.st_size, fdata.st_mtime));
			}
	}

	if(errno != 0) {
		perror("readdir()");
	}

	closedir(dir);

#endif

    switch(fileListOrder) {
        case FileListOrder_Name_Asc: {
            Files.sort(cmp_Name_Asc);
        } break;

        case FileListOrder_Name_CaseInsensitive_Asc: {
            Files.sort(cmp_Name_CaseInsensitive_Asc);
        } break;

        case FileListOrder_Name_Dsc: {
            Files.sort(cmp_Name_Dsc);
        } break;

        case FileListOrder_Name_CaseInsensitive_Dsc: {
            Files.sort(cmp_Name_CaseInsensitive_Dsc);
        } break;

        case FileListOrder_Size_Asc: {
            Files.sort(cmp_Size_Asc);
        } break;

        case FileListOrder_Size_Dsc: {
            Files.sort(cmp_Size_Dsc);
        } break;

        case FileListOrder_ModifyDate_Asc: {
            Files.sort(cmp_ModifyDate_Asc);
        } break;

        case FileListOrder_ModifyDate_Dsc: {
            Files.sort(cmp_ModifyDate_Dsc);
        } break;

        case FileListOrder_Unsorted:
        default: {
            // do nothing
        } break;
    }

	return Files;

}

bool getCaseInsensitiveFilename(std::string& filepath) {

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


bool existsFile(const std::string& path) {
	// try opening the file
    SDL_RWops* RWopsFile = SDL_RWFromFile(path.c_str(),"r");

    if(RWopsFile == nullptr) {
        return false;
    }

    SDL_RWclose(RWopsFile);

    return true;
}

std::string readCompleteFile(std::string filename) {
    SDL_RWops* RWopsFile = SDL_RWFromFile(filename.c_str(),"r");

    if(RWopsFile == nullptr) {
        return "";
    }

	int filesize = SDL_RWseek(RWopsFile,0,SEEK_END);
	if(filesize < 0) {
		SDL_RWclose(RWopsFile);
		return "";
	}

	if(SDL_RWseek(RWopsFile,0,SEEK_SET) != 0) {
		SDL_RWclose(RWopsFile);
		return "";
	}

    char* filedata = new char[filesize];

	if(SDL_RWread(RWopsFile, filedata, filesize, 1) != 1) {
	    delete [] filedata;
		SDL_RWclose(RWopsFile);
		return "";
	}

	std::string retValue(filedata, filesize);

	delete [] filedata;

    SDL_RWclose(RWopsFile);

    return retValue;
}

std::string getBasename(const std::string& filepath, bool bStripExtension) {

    if(filepath == "/") {
        // special case
        return "/";
    }

    // strip trailing slashes
    size_t nameEndPos = filepath.find_last_not_of("/\\");

    size_t nameStart = filepath.find_last_of("/\\", nameEndPos);
    if(nameStart == std::string::npos) {
        nameStart = 0;
    } else {
        nameStart++;
    }

    size_t extensionStart;
    if(bStripExtension) {
        extensionStart = filepath.find_last_of(".");
        if(extensionStart == std::string::npos) {
            extensionStart = filepath.length();
        }
    } else {
        extensionStart = (nameEndPos == std::string::npos) ? filepath.length() : nameEndPos+1;
    }

    return filepath.substr(nameStart, extensionStart-nameStart);
}

std::string getDirname(const std::string& filepath) {

    if(filepath == "/") {
        // special case
        return "/";
    }

    // strip trailing slashes
    size_t nameEndPos = filepath.find_last_not_of("/\\");

    // strip trailing name
    size_t nameStartPos = filepath.find_last_of("/\\", nameEndPos);

    if(nameStartPos == std::string::npos) {
        return ".";
    }

    // strip separator between dir name and file name
    size_t dirEndPos = filepath.find_last_not_of("/\\", nameStartPos);

    return filepath.substr(0, dirEndPos+1);
}

#if defined(DUNELEGACY_PLATFORM_OSX)
std::string getResourcesBundlePath()
{
    char resources_path[PATH_MAX]; // file-system path
    CFBundleRef bundle;            // bundle type reference

    // Look for the top-level bundle's Resources path
    bundle = CFBundleGetMainBundle();

    CFURLRef resourcesURL = CFBundleCopyResourcesDirectoryURL(bundle);

    if( ! CFURLGetFileSystemRepresentation(resourcesURL, true, (unsigned char*)resources_path, PATH_MAX) ) {
        throw std::runtime_error("Could not find the app bundle's Resources directory path!");
        CFRelease(resourcesURL);

        return "\0";
    }

    CFRelease(resourcesURL);

    return resources_path;
}
#endif // defined DUNELEGACY_PLATFORM_OSX
