
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

#include <CutScenes/Intro.h>

#include <misc/fnkdat.h>
#include <misc/FileSystem.h>
#include <misc/Scaler.h>
#include <misc/string_util.h>
#include <misc/exceptions.h>
#include <misc/format.h>

#include <SoundPlayer.h>

#include <mmath.h>

#include <ctime>
#include <exception>
#include <future>
#include <iostream>
#include <list>
#include <random>
#include <set>
#include <string>
#include <typeinfo>
#include <vector>

#endif // __cplusplus

#include <SDL.h>
#include <SDL_rwops.h>
#include <SDL_mixer.h>
#include <SDL_endian.h>

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
