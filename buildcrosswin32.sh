#!/bin/bash

mkdir -p build

mkdir -p build/x86
if [ ! -e build/x86/SDL2.dll ]
then
	echo "Downloading 32-bit SDL2.dll..."
	wget -c http://libsdl.org/release/SDL2-2.0.5-win32-x86.zip -O build/x86/SDL2-2.0.5-win32-x86.zip
	unzip -q build/x86/SDL2-2.0.5-win32-x86.zip -d build/x86 SDL2.dll
	rm build/x86/SDL2-2.0.5-win32-x86.zip
fi

if [ ! -e build/x86/SDL2_mixer.dll ]
then
	echo "Downloading 32-bit SDL2_mixer.dll..."
	wget -c https://www.libsdl.org/projects/SDL_mixer/release/SDL2_mixer-2.0.1-win32-x86.zip -O build/x86/SDL2_mixer-2.0.1-win32-x86.zip
	unzip -q build/x86/SDL2_mixer-2.0.1-win32-x86.zip -d build/x86 SDL2_mixer.dll smpeg2.dll libvorbisfile-3.dll libvorbis-0.dll libogg-0.dll libFLAC-8.dll libmodplug-1.dll
	rm build/x86/SDL2_mixer-2.0.1-win32-x86.zip
fi

mkdir -p build/x64
if [ ! -e build/x64/SDL2.dll ]
then
	echo "Downloading 64-bit SDL2.dll..."
	wget -c http://libsdl.org/release/SDL2-2.0.5-win32-x64.zip -O build/x64/SDL2-2.0.5-win32-x64.zip
	unzip -q build/x64/SDL2-2.0.5-win32-x64.zip -d build/x64 SDL2.dll
	rm build/x64/SDL2-2.0.5-win32-x64.zip
fi

if [ ! -e build/x64/SDL2_mixer.dll ]
then
	echo "Downloading 64-bit SDL2_mixer.dll..."
	wget -c https://www.libsdl.org/projects/SDL_mixer/release/SDL2_mixer-2.0.1-win32-x64.zip -O build/x64/SDL2_mixer-2.0.1-win32-x64.zip
	unzip -q build/x64/SDL2_mixer-2.0.1-win32-x64.zip -d build/x64 SDL2_mixer.dll smpeg2.dll libvorbisfile-3.dll libvorbis-0.dll libogg-0.dll libFLAC-8.dll libmodplug-1.dll
	rm build/x64/SDL2_mixer-2.0.1-win32-x64.zip
fi

autoreconf --install
mkdir -p build/src
i686-w64-mingw32-windres -o build/src/resource.o resource.rc
cd build
../configure --prefix="" --host=i686-w64-mingw32 --with-sdl-prefix=/usr/i686-w64-mingw32 --disable-sdltest && make $1 && cp src/dunelegacy.exe x86/
cd ..

cd build
make distclean
cd ..

autoreconf --install
mkdir -p build/src
x86_64-w64-mingw32-windres -o build/src/resource.o resource.rc
cd build
../configure --prefix="" --host=x86_64-w64-mingw32 --with-sdl-prefix=/usr/x86_64-w64-mingw32 --disable-sdltest && make $1 && cp src/dunelegacy.exe x64/
cd ..

makensis dunelegacy.nsi
