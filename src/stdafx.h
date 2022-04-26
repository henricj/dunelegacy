
#ifndef STDAFX_H
#define STDAFX_H

#ifdef _WIN32
#    define WINDDI_VERSION WINDDI_VERSION_VISTASP2
#    define _WIN32_WINNT   _WIN32_WINNT_VISTA
#    ifndef WIN32_LEAN_AND_MEAN
#        define WIN32_LEAN_AND_MEAN
#    endif
#    ifndef NOMINMAX
#        define NOMINMAX
#    endif

#    if DUNE_CRT_HEAP_DEBUG
#        define DEBUG_CLIENTBLOCK new (_CLIENT_BLOCK, __FILE__, __LINE__)
#        include <crtdbg.h>
#        define new DEBUG_CLIENTBLOCK
#    endif
#endif // _WIN32

#include <config.h>

#ifdef __cplusplus

#    include <globals.h>

#    include <FileClasses/FileManager.h>
#    include <FileClasses/FontManager.h>
#    include <FileClasses/GFXManager.h>
#    include <FileClasses/INIFile.h>
#    include <FileClasses/Palfile.h>
#    include <FileClasses/SFXManager.h>
#    include <FileClasses/TextManager.h>
#    include <FileClasses/music/ADLPlayer.h>
#    include <FileClasses/music/DirectoryPlayer.h>
#    include <FileClasses/music/XMIPlayer.h>

#    include <GUI/GUIStyle.h>
#    include <GUI/dune/DuneStyle.h>

#    include <Menu/MainMenu.h>

#    include <Explosion.h>
#    include <Game.h>
#    include <House.h>
#    include <Map.h>
#    include <ScreenBorder.h>
#    include <SoundPlayer.h>

#    include <fmt/format.h>
#    include <fmt/printf.h>
#    include <misc/FileSystem.h>
#    include <misc/Random.h>
#    include <misc/SDL2pp.h>
#    include <misc/Scaler.h>
#    include <misc/exceptions.h>
#    include <misc/fnkdat.h>
#    include <misc/sdl_support.h>
#    include <misc/string_util.h>

#    include <SoundPlayer.h>

#    include <mmath.h>

#    include <algorithm>
#    include <array>
#    include <bitset>
#    include <cassert>
#    include <chrono>
#    include <cinttypes>
#    include <cmath>
#    include <cstdarg>
#    include <cstdint>
#    include <cstdio>
#    include <cstdlib>
#    include <cstring>
#    include <ctime>
#    include <deque>
#    include <exception>
#    include <filesystem>
#    include <functional>
#    include <future>
#    include <iostream>
#    include <limits>
#    include <list>
#    include <map>
#    include <memory>
#    include <mutex>
#    include <numeric>
#    include <optional>
#    include <queue>
#    include <random>
#    include <set>
#    include <string>
#    include <tuple>
#    include <typeinfo>
#    include <unordered_map>
#    include <unordered_set>
#    include <utility>
#    include <vector>

#    include <cinttypes>
#    include <cstdio>
#    include <cstdlib>
#    include <cstring>

#    include <gsl/gsl>

#else // __cplusplus
#    include <inttypes.h>
#    include <stdio.h>
#    include <stdlib.h>
#    include <string.h>
#endif // __cplusplus

#include <SDL2/SDL.h>
#include <SDL2/SDL_endian.h>
#include <SDL2/SDL_mixer.h>
#include <SDL2/SDL_rwops.h>

#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>

#ifdef _WIN32
#    include <Windows.h>
#    include <io.h>
#    include <winsock2.h>
#endif // _WIN32

#endif // STDAFX_H
