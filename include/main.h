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

class CaseInsensitiveFilesystemCache;
class INIFile;

// ============================================================================
// Video functions (MainVideo.cpp)
// ============================================================================

/**
    This functions sets the video mode according to the settings
    \param displayIndex The display index to use
*/
void setVideoMode(int displayIndex);

/**
    Updates the display scale based on DPI settings
    \param sdl_window The SDL window
*/
void update_display_scale(SDL_Window* sdl_window);

/**
    This function shows a list of missing pak-files in a message box.
    It returns, when the message box is closed.
    \param filesystemCache The filesystem cache to check
*/
void showMissingFilesMessageBox(const CaseInsensitiveFilesystemCache& filesystemCache);

// ============================================================================
// Configuration functions (MainConfig.cpp)
// ============================================================================

/**
    This function tries to determine the system language the user uses.
    \return two character language code (e.g. en, de, fr) or empty if unknown
*/
std::string getUserLanguage();

/**
    Load settings from INI file into global settings
    \param myINIFile The INI file to load from
*/
void load_settings(const INIFile& myINIFile);

/**
    Configure the game based on settings file and command line arguments
    \param argc Argument count
    \param argv Argument values
    \param bFirstInit True if this is the first initialization
    \param currentDisplayIndex Current display index
    \return True if this is the first gamestart
*/
bool configure_game(int argc, char* argv[], bool bFirstInit, int currentDisplayIndex);

/**
    Parse command line arguments
    \param argc Argument count
    \param argv Argument values
    \param bShowDebugLog Output: whether to show debug log
    \return True if parsing succeeded
*/
bool parseCommandLine(int argc, char* argv[], bool& bShowDebugLog);

// ============================================================================
// Main game functions (main.cpp)
// ============================================================================

/**
    This function is used by SDL to write out log messages
*/
void logOutputFunction(void* userdata, int category, SDL_LogPriority priority, const char* message);

/**
    Run the main game loop
    \param argc Argument count
    \param argv Argument values
    \return True if the game ran successfully
*/
bool run_game(int argc, char* argv[]);

#endif // MAIN_H
