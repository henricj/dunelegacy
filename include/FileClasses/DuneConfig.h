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

#ifndef DUNECONFIG_H
#define DUNECONFIG_H

/**
    This function returns the configfile path
    \return the full path to the config file
*/
std::filesystem::path getConfigFilepath();

/**
    This function returns the logfile path
    \return the full path to the logfile
*/
std::filesystem::path getLogFilepath();

/**
    Return an appropriate factor between logical and physical resolution.
    \param  physicalWidth   the physical width of the display
    \param  physicalHeight  the physical height of the display
    \return the factor between logical and physical resolution, e.g. 1 for physical resolution below 1280x960
*/
int getLogicalToPhysicalResolutionFactor(int physicalWidth, int physicalHeight);

/**
    This function creates a new default config file.
    \param configfilepath the path to the config file
    \param language the language to use as default (e.g. en, de, fr)
*/
void createDefaultConfigFile(std::filesystem::path configfilepath, const std::string& language);

#endif // DUNECONFIG_H
