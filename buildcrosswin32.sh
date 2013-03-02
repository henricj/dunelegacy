#!/bin/bash

if [ ! -e data/SDL.dll ]
then
	echo "Downloading SDL.dll..."
	wget -c http://www.libsdl.org/release/SDL-1.2.15-win32.zip -O data/SDL-1.2.15-win32.zip
	unzip -q data/SDL-1.2.15-win32.zip -d data SDL.dll
	rm data/SDL-1.2.15-win32.zip
fi

if [ ! -e data/SDL_mixer.dll ]
then
	echo "Downloading SDL_mixer.dll..."
	wget -c http://www.libsdl.org/projects/SDL_mixer/release/SDL_mixer-1.2.12-win32.zip -O data/SDL_mixer-1.2.12-win32.zip
	unzip -q data/SDL_mixer-1.2.12-win32.zip -d data SDL_mixer.dll smpeg.dll libmikmod-2.dll libvorbisfile-3.dll libvorbis-0.dll libogg-0.dll libFLAC-8.dll
	rm data/SDL_mixer-1.2.12-win32.zip
fi

autoreconf --install
mkdir -p build
mkdir -p build/src
i686-w64-mingw32-windres -o build/src/resource.o resource.rc
cd build
../configure --prefix="" --host=i686-w64-mingw32 --with-sdl-prefix=/usr/i686-w64-mingw32 --disable-sdltest && make $1 && cp src/dunelegacy.exe ../data/
cd ..
makensis dunelegacy.nsi
