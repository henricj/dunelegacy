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

#include "FileClasses/DuneConfig.h"

#include "misc/fnkdat.h"

#include <filesystem>

#ifndef _WIN32
#    include <cstring>
#    include <sys/types.h>

#    include <pwd.h>
#    include <unistd.h>
#endif // _WIN32

std::filesystem::path getConfigFilepath() {
    // determine path to config file
    auto [ok, tmp] = fnkdat(CONFIGFILENAME, FNKDAT_USER | FNKDAT_CREAT);

    return tmp;
}

std::filesystem::path getLogFilepath() {
    // determine path to config file
    auto [ok, tmp] = fnkdat(LOGFILENAME, FNKDAT_USER | FNKDAT_CREAT);

    if (!ok) {
        THROW(std::runtime_error, "fnkdat() failed!");
    }

    return tmp;
}

int getLogicalToPhysicalResolutionFactor(int physicalWidth, int physicalHeight) {
    if (physicalWidth >= 1280 * 3 && physicalHeight >= 720 * 3) {
        return 3;
    }
    if (physicalWidth >= 640 * 2 && physicalHeight >= 480 * 2) {

        return 2;
    }
    return 1;
}

void createDefaultConfigFile(std::filesystem::path configfilepath, const std::string& language) {
    configfilepath.make_preferred();

    sdl2::log_info("Creating config file '%s'", reinterpret_cast<const char*>(configfilepath.u8string().c_str()));

    const auto file = sdl2::RWops_ptr{SDL_RWFromFile(configfilepath.u8string().c_str(), "w")};
    if (!file) {
        THROW(sdl_error, "Opening config file failed: {}!", SDL_GetError());
    }

    // clang-format off
    static constexpr char configfile[] =
                                "[General]\n"
                                "Play Intro = false          # Play the intro when starting the game?\n"
                                "Player Name = %s            # The name of the player\n"
                                "Language = %s               # en = English, fr = French, de = German\n"
                                "Scroll Speed = 50           # Amount to scroll the map when the cursor is near the screen border\n"
                                "Show Tutorial Hints = true  # Show tutorial hints during the game\n"
                                "\n"
                                "[Video]\n"
                                "# Minimum resolution is 640x480\n"
                                "Width = 640\n"
                                "Height = 480\n"
                                "Physical Width = 640\n"
                                "Physical Height = 480\n"
                                "Fullscreen = true\n"
                                "FrameLimit = true           # Limit the frame rate to save energy?\n"
                                "Preferred Zoom Level = 1    # 0 = no zooming, 1 = 2x, 2 = 3x\n"
                                "Scaler = ScaleHD            # Scaler to use: ScaleHD = apply manual drawn mask to upscale, Scale2x = smooth edges, ScaleNN = nearest neighbour, \n"
                                "RotateUnitGraphics = false  # Freely rotate unit graphics, e.g. carryall graphics\n"
                                "\n"
                                "[Audio]\n"
                                "# There are three different possibilities to play music\n"
                                "#  adl       - This option will use the Dune 2 music as used on e.g. SoundBlaster16 cards\n"
                                "#  xmi       - This option plays the xmi files of Dune 2. Sounds more midi-like\n"
                                "#  directory - Plays music from the \"music\"-directory inside your configuration directory\n"
                                "#              The \"music\"-directory should contain 5 subdirectories named attack, intro, peace, win and lose\n"
                                "#              Put any mp3, ogg or mid file there and it will be played in the particular situation\n"
                                "Music Type = adl\n"
                                "Play Music = true\n"
                                "Music Volume = 64           # Volume between 0 and 128\n"
                                "Play SFX = true\n"
                                "SFX Volume = 64             # Volume between 0 and 128\n"
                                "\n"
                                "[Network]\n"
                                "ServerPort = %d\n"
                                "MetaServer = %s\n"
                                "\n"
                                "[AI]\n"
                                "Campaign AI = qBotMedium\n"
                                "\n"
                                "[Game Options]\n"
                                "Game Speed = 16                         # The default speed of the game: 32 = very slow, 8 = very fast, 16 = default\n"
                                "Concrete Required = true                # If true building on bare rock will result in 50%% structure health penalty\n"
                                "Structures Degrade On Concrete = true   # If true structures will degrade on power shortage even if built on concrete\n"
                                "Fog of War = false                      # If true explored terrain will become foggy when no unit or structure is next to it\n"
                                "Start with Explored Map = false         # If true the complete map is unhidden at the beginning of the game\n"
                                "Instant Build = false                   # If true the building of structures and units does not take any time\n"
                                "Only One Palace = false                 # If true, only one palace can be build per house\n"
                                "Rocket-Turrets Need Power = false       # If true, rocket turrets are dysfunctional on power shortage\n"
                                "Sandworms Respawn = false               # If true, killed sandworms respawn after some time\n"
                                "Killed Sandworms Drop Spice = false     # If true, killed sandworms drop some spice\n"
                                "Manual Carryall Drops = false           # If true, player can request carryall to transport units\n"
                                "Maximum Number of Units Override = -1   # Override the maximum number of units each house is allowed to build (-1 = do not override)\n";
    // clang-format on

    char playername[MAX_PLAYERNAMELENGTH + 1] = "Player";

#ifdef _WIN32
    DWORD playernameLength = MAX_PLAYERNAMELENGTH + 1;
    GetUserNameA(playername, &playernameLength);
#else
    if (auto* const pwent = getpwuid(getuid())) {
        strncpy(playername, pwent->pw_name, MAX_PLAYERNAMELENGTH + 1);
        playername[MAX_PLAYERNAMELENGTH] = '\0';
    }
#endif

    playername[0] = static_cast<char>(std::toupper(static_cast<unsigned char>(playername[0])));

    // replace player name, language, server port and metaserver
    const std::string strConfigfile = fmt::sprintf(configfile, playername, language, DEFAULT_PORT, DEFAULT_METASERVER);

    if (SDL_RWwrite(file.get(), strConfigfile.c_str(), 1, strConfigfile.length()) == 0) {
        THROW(sdl_error, "Writing config file failed: {}!", SDL_GetError());
    }
}
