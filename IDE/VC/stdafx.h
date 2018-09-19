
#ifndef STDAFX_H
#define STDAFX_H

#ifdef _WIN32
#define WINDDI_VERSION WINDDI_VERSION_VISTASP2
#define _WIN32_WINNT _WIN32_WINNT_VISTA
#endif // _WIN32

#include <config.h>

#ifdef __cplusplus

#include <globals.h>

#include <FileClasses/FileManager.h>
#include <FileClasses/GFXManager.h>
#include <FileClasses/SFXManager.h>
#include <FileClasses/FontManager.h>
#include <FileClasses/TextManager.h>
#include <FileClasses/INIFile.h>
#include <FileClasses/Palfile.h>
#include <FileClasses/music/DirectoryPlayer.h>
#include <FileClasses/music/ADLPlayer.h>
#include <FileClasses/music/XMIPlayer.h>

#include <GUI/GUIStyle.h>
#include <GUI/dune/DuneStyle.h>

#include <Menu/MainMenu.h>

#include <House.h>
#include <Game.h>
#include <Map.h>
#include <Explosion.h>
#include <ScreenBorder.h>
#include <SoundPlayer.h>

#include <misc/fnkdat.h>
#include <misc/FileSystem.h>
#include <misc/Scaler.h>
#include <misc/sdl_support.h>
#include <misc/string_util.h>
#include <misc/exceptions.h>
#include <fmt/format.h>
#include <fmt/printf.h>
#include <misc/Random.h>

#include <SoundPlayer.h>

#include <mmath.h>

#include <algorithm>
#include <array>
#include <bitset>
#include <deque>
#include <chrono>
#include <cinttypes>
#include <ctime>
#include <cmath>
#include <cassert>
#include <cstdarg>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <exception>
#include <future>
#include <functional>
#include <iostream>
#include <list>
#include <limits>
#include <map>
#include <memory>
#include <mutex>
#include <queue>
#include <random>
#include <set>
#include <string>
#include <typeinfo>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <vector>

#endif // __cplusplus

#include <SDL2/SDL.h>
#include <SDL2/SDL_rwops.h>
#include <SDL2/SDL_mixer.h>
#include <SDL2/SDL_endian.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <inttypes.h>

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <winsock2.h>
#include <windows.h>
#include <io.h>
#endif // _WIN32

#include <stdlib.h>
#include <stdio.h>
#include <string.h>


#endif // STDAFX_H
