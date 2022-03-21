# Dune Legacy

## This project

### Build Status

| [Workflow](../../actions) | Status |
|------------|------------|
|   [Windows](../../actions/workflows/windows.yml)  | [![Windows](../../actions/workflows/windows.yml/badge.svg)](../../actions/workflows/windows.yml) |
|   [Ubuntu](../../actions/workflows/ubuntu.yml)    | [![Ubuntu](../../actions/workflows/ubuntu.yml/badge.svg)](../../actions/workflows/ubuntu.yml) |

### Binaries

Binaries are built automatically from the latest source and can be found here:

**[Windows arm64](../../releases/tag/latest-arm64)**  
**[Windows x86](../../releases/tag/latest-x86)**  
**[Windows x64](../../releases/tag/latest-x64)**  
**[Windows x64/avx2](../../releases/tag/latest-x64-avx2)**  

### Summary

The goals of this effort are to modernize the codebase, simplify the build system, and to improve
performance.  The game engine itself will eventually be moved off of the rendering thread, which
will provide for a much more responsive user interface, particularly when many units are on the map.

All builds are now being done through [CMake](https://cmake.org/) with regular testing
on Windows with [Visual Studio 2022](https://visualstudio.microsoft.com/vs/) and Ubuntu with g++.  Windows
development is being done with VS2022's native CMake support, which uses Ninja for the actual build.  (It
is unknown if the CMake Visual Studio generators produce usable `vcxproj` files.)

### Notable Changes

- Builds are done with CMake (and only CMake).
- [ENet](http://enet.bespin.org/) has been updated and is no longer a
normal part of the tree but is included as [Git submodules](https://git-scm.com/book/en/v2/Git-Tools-Submodules)
instead.
- [vcpkg](https://github.com/microsoft/vcpkg) is used to provide [SDL2](https://www.libsdl.org/),
[googletest](https://github.com/google/googletest), [fmt](https://fmt.dev/), [lodepng](https://lodev.org/lodepng/),
 [soxr](https://sourceforge.net/projects/soxr/), and [ms-gsl](https://github.com/microsoft/GSL).
- Pausing after the mission has completed but before the statistics page is displayed will let the user
explore the map for as long as they like.
- On Windows, DirectX 11 is now the preferred renderer. An option has been added to the INI file to configure
the preferred renderer.  Add `Renderer = direct3d` or `Renderer = software` under the `[Video]` section
to request [SDL2](https://www.libsdl.org/)'s DX9 or software renderer, respectively.
- A texture atlas (sprite-sheet) is created during startup that drastically reduces the number of texture
switches during rendering.  Some more work needs to be done for it to not fail when faced with an older
GPU.  To improve startup times, these texture atlases should probably be cached on disk.
- Screen/windows managment has been changed to allow a window to be used over a Remote Desktop (RDP)
session.  Disconnecting and reconnecting RDP should no longer cause a hang.

At some point, some of these change might be incorporated into the upstream repository.

### Known Issues

- The random number generator is different than the one in the official release.
- Systems with older GPUs may need to use the software renderer (add `Renderer = software` under
the `[Video]` section in the INI file).
- With all the rendering changes, there are now some glitches that need to be tracked down.

### Building

#### Windows Build

Install Visual Studio 2022 including the "Desktop development with C++" workload.  Go
to the "Individual Components" of the installer to make sure "C++ CMake tools for Windows" is
enabled (enable clang as well to enable builds with "clang-cl").

To build from inside Visual Studio, open the "dunelegacy" folder, go to the menu to select
"Project" -> "CMake Settings for dunelegacy", and create a release configuration, set it as the
current configuration, then select "Build" -> "Build All".  It should be possible to open the
repository directly by going to "File" -> "Clone Repository..." on the menu.

From the command line (a bit faster, but requries more typing):

First open a command prompt with access to CMake (for example, "Developer Command Prompt
for Visual Studio 2022").  Get a copy of the code repository, along with the submodules:

```bat
git clone --recurse-submodules -j 8 https://github.com/henricj/dunelegacy.git
cd dunelegacy
```

Build the vcpkg packages,

```bat
external\vcpkg\build_vcpkg.bat
```

Configure CMake's build folder, compile the source, and run the tests (alas, there are far
too few tests):

```bat
mkdir out\build\x64-avx2-Release
cd out\build\x64-avx2-Release
cmake -G Ninja -profile=windows-x64-avx2-release -B . -S ../../..
cmake --build .
ctest
```

There should now be a working dunelegacy executable in `out\build\x64-avx2-Release\src`.

To see the full list of CMake presets (from the top level dunelegacy directory):
```bat
cmake --list-presets
```

#### Linux Build

The build-essential, git, ninja-build, and cmake packages are needed.  Either g++ or clang
can be used for the build (tested with GCC 11 and Clang 13).

```sh
git clone --recurse-submodules -j 8 https://github.com/henricj/dunelegacy.git
cd dunelegacy
```

Build the vcpkg packages,

```sh
external/vcpkg/build_vcpkg.sh
```

Configure CMake's build folder, compile the source, and run the tests (alas, there are far
too few tests):

```sh
mkdir -p out/build/x64-Release
cd out/build/x64-Release
cmake -G Ninja -DCMAKE_BUILD_TYPE:STRING=Release -DVCPKG_TARGET_TRIPLET:STRING=x64-linux ../../..
cmake --build .
ctest
```

There should now be a working dunelegacy executable in `out/build/x64-Release/src`.

### Notes

Builds for macOS have not been tested, but should be similar to the Linux instructions.

---

## Original README

```text
              /-------------------------------\
              |                               |
              |    Dune Legacy 0.96 readme    |
              |    -----------------------    |
              |                               |
              \-------------------------------/
```

IMPORTANT:
This software is provided as is, and you are running it at your own risk.  I'm not responsible if any harm results
from you aquiring, or running this software.  If you distribute this software, make sure this readme file is included.
The program Dune Legacy is an modernized clone of the excellent Westwood Studios game "Dune 2". It is ridiculusly easy to find
Dune2 on the net anyways, but I won't provide it.  I think you can still even buy it from Westwood studios, so do that.
There exists a mod called Superdune 2. The PAK-Files Superdune 2 provides are nearly the same except SCENARIO.PAK.

This program would not have been created without the use of the excellent SDL library and its extensions.  Thanks guys :).

## Getting Started

The following PAK-files from the original Dune 2 are needed to play Dune Legacy:

- HARK.PAK
- ATRE.PAK
- ORDOS.PAK
- ENGLISH.PAK
- DUNE.PAK
- SCENARIO.PAK
- MENTAT.PAK
- VOC.PAK
- MERC.PAK
- FINALE.PAK
- INTRO.PAK
- INTROVOC.PAK
- SOUND.PAK
- GERMAN.PAK (for playing in German)
- FRENCH.PAK (for playing in French)

It depends on your system and installation where to put these files. LEGACY.PAK is supplied with Dune Legacy and is already
in the data directory. All the other files should be copied there too. If you are not allowed to copy files there you may
copy these files inside the dunelegacy configuration directory (e.g. ~/.config/dunelegacy/data/ on unix).

### Linux

It depends on how the game was compiled. Normally you should put these files under /usr/share/dunelegacy/ or /usr/local/share/dunelegacy/ .
Just look for LEGACY.PAK. If you do not have root access to your system you should put them in your home directory under ~/.config/dunelegacy/data/ .

### Windows

The installer has already asked for the files and put them in the installation directory. If not put the PAK-files inside your installation folder or
if you do not have administrator privileges you should put them to C:\Documents and Settings\<YourName>\Application Data\dunelegacy\data\ .

### MAC OS X

The PAK-files have to be copied inside the application bundle. If you have followed the steps in the supplied dmg you have already copied them there.
Otherwise just right-click on the bundle and select "Show Bundle Content". Then navigate into "Contents" and then into "Resources". There you will
find LEGACY.PAK. Put the other PAK-files there too. Alternativly you can put them in your home directory under ~/.config/dunelegacy/data/ but putting
them inside the application bundle is the preferred way.

## Keyboard Shortcuts

 General Keyboard Shortcuts:

| Key | Effect |
|--|--|
| Escape                        |         Go to menu                |
| Space                         |         Pause game                |
| Alt + Enter                   |    Toggle fullscreen              |
| Alt + Tab                     |    Switch to other application    |
| Print Key or Ctrl + P         |  Save screenshot as ScreenshotX.bmp with increasing numbers for X |
| Enter                         |    Start/Stop chatting            |
| | |
| Key F1                        |    Zoomlevel x1                   |
| Key F2                        |    Zoomlevel x2                   |
| Key F3                        |    Zoomlevel x3                   |
| | |
| Key T                         |    Toggle display of current game time    |
| Key F10                       |    Toggle sound effects and voice         |
| Key F11                       |    Toggle music                           |
| Key F12                       |    Toggle display of FPS                  |
| Key Up, Down, Left or Right   |    Move on the map                        |
| | |
| Key G                         |    Cycle through construction yards           |
| Key F                         |    Cycle through factories/other builders     |
| | |
| Key F4                        |    Skip 10 seconds (only in singleplayer)     |
| Key F5                        |    Skip 30 seconds (only in singleplayer)     |
| Key F6                        |    Skip 2 minutes (only in singleplayer)      |
| Key -                         |    Decrease gamespeed (only in singleplayer)  |
| Key +                         |    Increase gamespeed (only in singleplayer)  |
| | |
| Ctrl + (Key 1 to Key 9)       |    Save the list of selected units as unit group 1 to 9   |
| Key 1 to Key 9                |    Select units from unit group 1 to 9                    |
| Shift + (Key 1 to Key 9)      |    Add all units from unit group 1 to 9 to the list of currently selected units |
| Key 0                         |    Deselect all currently selected units                  |
| Ctrl + Key 0                  |    Remove currently selected units form all unit groups (group 1-9) |
| | |
| Key M                         |    Order unit to move to some position                                |
| Key A                         |    Order selected units to attack some unit, structure or position    |
| Key C                         |    Order selected infantry units to capture some structure            |
| Key R                         |    Repair selected structure or send selected units to repair yard    |
| Key H                         |    Return selected harvester                                          |
| Key D                         |    Request a carryall drop (only if this game option is enabled)      |
| Key U                         |    Upgrade selected structure                                         |
| Key P                         |    Place a structure (if a construction yard is selected)             |

Map Editor:

| Key | Effect |
|--|--|
| Print Key or Ctrl + P         |    Save a picture of the whole map as &lt;Mapname&gt;.bmp |
| Ctrl + Z                      |    Undo last edit                                   |
| Ctrl + Y                      |    Redo last edit                                   |

## Configuration file

If you want to fine tune the configuration of Dune Legacy you might want to take a look at the configuration file "Dune Legacy.ini". Depending on your system it
is either placed in ~/.config/dunelegacy (on Linux), ~/Library/Application Support/Dune Legacy (on Mac OS X) or in C:\Documents and Settings\<YourName>\Application Data\dunelegacy\ (on Windows).

## Internet Game

To play online via Internet you have to manually enable port forwarding if you use a NAT Router. Forward the Dune Legacy Server Port (Default is 28747) from your NAT Router to your computer. Use the same port on your router as configured in Dune Legacy.
Example: If your machine has IP 192.168.123.1 and your using the default Dune Legacy Port, than forward port 28747 from your router to 192.168.123.1:28747.

---
IRC: #dunelegacy @ irc.freenode.net  
Web: <http://sourceforge.net/projects/dunelegacy>
