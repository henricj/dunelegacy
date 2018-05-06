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

#include <misc/SDL2pp.h>

#include <string>


/**
    Return an appropriate factor between logical and physical resolution.
    \param  physicalWidth   the physical width of the display
    \param  physicalHeight  the physical height of the display
    \return the factor between logical and physical resolution, e.g. 1 for physical resolution below 1280x960
*/
int getLogicalToPhysicalResolutionFactor(int physicalWidth, int physicalHeight);

/**
    This functions sets the video mode according to the settings
*/
void setVideoMode();

/**
    Toggles fullscreen and windowed mode
*/
void toogleFullscreen();

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
void createDefaultConfigFile(const std::string& configfilepath, const std::string& language);

/**
    This function is used by SDL to write out log messages
*/
void logOutputFunction(void *userdata, int category, SDL_LogPriority priority, const char *message);

/**
    This function shows a list of missing pak-files in a message box.
    It returns, when the message box is closed.
*/
void showMissingFilesMessageBox();

/**
    This function tries to determine the system language the user uses.
    \return two character language code (e.g. en, de, fr) or empty if unknown
*/
std::string getUserLanguage();

#endif //MAIN_H
