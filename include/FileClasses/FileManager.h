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

#include <SDL_rwops.h>
#include <SDL.h>
#include <string>
#include <vector>

/// A class for loading all the PAK-Files.
/**
	This class manages all the PAK-Files and provides access to the contained files through SDL_RWops.
*/
class FileManager {
public:
    /**
        Constructor.
        \param saveMode if true, every loading error is ignored
    */
	FileManager(bool saveMode = false);
	~FileManager();

    static std::vector<std::string> getSearchPath();
	static std::vector<std::string> getNeededFiles();
	static std::vector<std::string> getMissingFiles();

	SDL_RWops* openFile(std::string filename);

	bool exists(std::string filename) const;
private:
    std::string md5FromFilename(std::string filename);

	std::vector<Pakfile*> pakFiles;
};

#endif // FILEMANAGER_H
