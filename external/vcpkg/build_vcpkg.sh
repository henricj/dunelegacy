#!/bin/sh

Packages="sdl2 sdl2-mixer sdl2-ttf fmt lodepng ms-gsl gtest soxr"

dir=$(CDPATH= cd -- "$(dirname -- "$0")" && pwd -P)

"$dir/vcpkg/bootstrap-vcpkg.sh"

"$dir/vcpkg/vcpkg" install --triplet x64-linux $Packages
"$dir/vcpkg/vcpkg" install --triplet x64-linux-asan $Packages
"$dir/vcpkg/vcpkg" install --triplet x64-linux-ubsan $Packages

