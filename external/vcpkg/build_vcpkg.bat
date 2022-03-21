SET Packages=sdl2 sdl2-mixer sdl2-ttf fmt lodepng ms-gsl gtest soxr

if NOT EXIST "%~dp0vcpkg\vcpkg.exe" call "%~dp0vcpkg\bootstrap-vcpkg.bat"

if [%1] == [] GOTO buildAll

"%~dp0vcpkg\vcpkg.exe" install %* %Packages%

exit 0

:buildAll
"%~dp0vcpkg\vcpkg.exe" install --triplet x64-windows %Packages%
"%~dp0vcpkg\vcpkg.exe" install --triplet x64-windows-ltcg %Packages%
"%~dp0vcpkg\vcpkg.exe" install --triplet x86-windows-ltcg %Packages%
"%~dp0vcpkg\vcpkg.exe" install --triplet arm64-windows-ltcg %Packages%
"%~dp0vcpkg\vcpkg.exe" install --triplet x64-avx2-windows-ltcg %Packages%
setlocal
set ASAN_OPTIONS= windows_hook_rtl_allocators=true
"%~dp0vcpkg\vcpkg.exe" install --triplet x64-windows-asan %Packages%
endlocal
setlocal
set CC=clang
set CXX=clang++
set LD=lld-link
"%~dp0vcpkg\vcpkg.exe" install --triplet x64-windows-ubsan %Packages%
endlocal
