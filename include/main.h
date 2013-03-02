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

#ifndef MAIN_H
#define MAIN_H

#include <string>

/**
	This functions sets the video mode according to the settings
*/
void setVideoMode();

/**
	This function returns the configfile path
	\return the full path to the config file
*/
std::string getConfigFilepath();

/**
	This function returns the logfile path
	\return the full path to the logfile
*/
std::string getLogFilepath();

/**
	This function creates a new default config file.
	\param configfilepath the path to the config file
	\param language the language to use as default (e.g. en, de, fr)
*/
void createDefaultConfigFile(std::string configfilepath, std::string language);

/**
    This function prints a list of missing pak-files on the screen.
    It returns, when a key is pressed.
*/
void printMissingFilesToScreen();

/**
    This function tries to determine the system language the user uses.
    \return two character language code (e.g. en, de, fr) or empty if unknown
*/
std::string getUserLanguage();

#endif //MAIN_H
