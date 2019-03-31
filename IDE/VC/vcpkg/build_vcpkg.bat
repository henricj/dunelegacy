SET Packages=cppunit sdl2 sdl2-mixer sdl2-ttf

copy "%~dp0triplets\*.*" "%~dp0vcpkg\triplets\"

call "%~dp0vcpkg\bootstrap-vcpkg.bat"

"%~dp0vcpkg\vcpkg.exe" install --triplet x64-windows %Packages%
"%~dp0vcpkg\vcpkg.exe" install --triplet x64-windows-ltcg %Packages%
"%~dp0vcpkg\vcpkg.exe" install --triplet x86-windows %Packages%
"%~dp0vcpkg\vcpkg.exe" install --triplet x86-windows-ltcg %Packages%

