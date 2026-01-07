# Dune Legacy

## This project

### Build Status

| [Workflow](../../actions) | Status |
|------------|------------|
|   [Windows](../../actions/workflows/windows.yml)  | [![Windows](../../actions/workflows/windows.yml/badge.svg)](../../actions/workflows/windows.yml) |
|   [Ubuntu](../../actions/workflows/build.yml)    | [![Ubuntu](../../actions/workflows/build.yml/badge.svg)](../../actions/workflows/build.yml) |

### Binaries

Binaries are built automatically from the latest source and can be found here:
**[Windows](../../releases/tag/latest-windows)**

### Summary

The goals of this effort are to modernize the codebase, simplify the build system, and to improve
performance.  The game engine itself will eventually be moved off of the rendering thread, which
will provide for a much more responsive user interface, particularly when many units are on the map.

All builds are now being done through [CMake](https://cmake.org/) with regular testing
on Windows with [Visual Studio 2026](https://visualstudio.microsoft.com/vs/) and Ubuntu with g++.  Windows
development is being done with VS2026's native CMake support, which uses Ninja for the actual build.  (It
is unknown if the CMake Visual Studio generators produce usable `vcxproj` files.)

### Notable Changes

- Builds are done with CMake (and only CMake).
- [ENet](http://enet.bespin.org/) has been updated and is no longer a
normal part of the tree but is included as a [Git submodule](https://git-scm.com/book/en/v2/Git-Tools-Submodules)
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
- Screen/windows management has been changed to allow a window to be used over a Remote Desktop (RDP)
session.  Disconnecting and reconnecting RDP should no longer cause a hang.

At some point, some of these change might be incorporated into the upstream repository.

### Known Issues

- The [`xoshiro256**`](https://prng.di.unimi.it/) random number generator is different
from the one in the official release.  There are also more generator instances used so
that a change to one part of the code that impacts how many times its generator is
called will only impact other users of the same generator instance.
- Systems with older GPUs may need to use the software renderer (add `Renderer = software`
under the `[Video]` section in the INI file).
- With all the rendering changes, there are now some glitches that need to be tracked down.

### Building

#### Prerequisites

- **CMake 3.22** or later (3.26+ recommended for better preset support).
- **Git** with submodule support.
- A C++20 compatible compiler.

#### Windows Build

Install Visual Studio 2026 (or later) including the "Desktop development with C++" workload. Go
to the "Individual Components" of the installer to make sure "C++ CMake tools for Windows" is
enabled (enable Clang as well to enable builds with `clang-cl`).

**Building from Visual Studio (recommended):**

Open the `dunelegacy` folder directly in Visual Studio ("File" -> "Open" -> "Folder..." or
"File" -> "Clone Repository..."). Select a CMake preset/configuration from the toolbar dropdown,
then select "Build" -> "Build All".

**Building from the command line:**

Open a command prompt with access to CMake (e.g., "Developer Command Prompt for Visual Studio 2026"
or "Developer PowerShell for VS 2026").

Clone the repository with submodules:

```bat
git clone --recurse-submodules -j 8 https://github.com/henricj/dunelegacy.git
cd dunelegacy
```

Configure, build, and test using a CMake preset:

```bat
cmake --preset=windows-x64-release
cmake --build --preset=windows-x64-release
ctest --preset=windows-x64-release
```

The executable will be in `out/build/windows-x64-release/src/dunelegacy.exe`.

To see all available CMake presets:

```bat
cmake --list-presets
```

**Note:** If you have multiple Visual Studio versions installed, ensure you use a consistent
toolchain. Mixing compiler versions (e.g., building vcpkg packages with one version and
linking with another) can cause linker errors like `unresolved external symbol __std_rotate`.

#### Linux Build

Install the required packages. On Debian/Ubuntu:

```sh
sudo apt-get install build-essential git ninja-build cmake pkg-config \
    libsdl2-dev libsdl2-mixer-dev libsdl2-ttf-dev libsoxr-dev
```

Clone the repository with submodules:

```sh
git clone --recurse-submodules -j 8 https://github.com/henricj/dunelegacy.git
cd dunelegacy
```

Configure, build, and test using a CMake preset (requires CMake 3.21+):

```sh
cmake --preset=linux-release
cmake --build --preset=linux-release
ctest --preset=linux-release
```

Or configure manually:

```sh
mkdir -p out/build/release
cd out/build/release
cmake -G Ninja -DCMAKE_BUILD_TYPE=Release ../../..
cmake --build .
ctest
```

The executable will be in `out/build/release/src/dunelegacy` (or the preset's output directory).

### Other Platforms

Other platforms may work if the proper
[vcpkg triplet](https://vcpkg.readthedocs.io/en/latest/users/triplets/) is
available.  For example, this has worked on FreeBSD 13.

```sh
mkdir -p out/build/x64-release
cd out/build/x64-release
cmake -G Ninja -DCMAKE_BUILD_TYPE:STRING=Release -DVCPKG_TARGET_TRIPLET:STRING=x64-freebsd -S ../../.. -B .
cmake --build .
ctest
```

If there are linker errors on a BSD platform, check to see if
`/usr/local/lib` is in the link library search path (e.g., check
the LIBRARY_PATH environment variable).

### Useful CMake Targets

There are a few helper targets defined to facilitate development:

- `cmake --build . --target update_locale`: Scans the source code for translatable strings, updates `dunelegacy.pot`, and then merges those changes into the source `.po` files (e.g., `data/locale/German.de.po`).
- `cmake --build . --target clangformat`: Runs `clang-format` on the codebase to enforce code style.

### Notes

Builds for macOS have not been tested, but should be similar to the Linux instructions.

Builds for Linux and other platforms with established package managers should be able
to resolve their dependencies from those packages managers instead of vcpkg.  Some
enhancements to the CMake files will be needed to support this.

### Developer Help / Cheat Codes

While developing, it can be helpful to use the following cheat codes to advance in the game. The cheats
are available in singleplayer modes by typing the following text in chat (case sensitive!):

- "Let me cheat": Enables cheat mode, type this first to activate other cheats
- "Let me win": Wins the current mission,
- "Start debugging": Enables developer mode,
- "Stop debugging": Disables developer mode,
- "Give me some credits": Accumulates 10000 spice to the current player

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
