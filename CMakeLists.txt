cmake_minimum_required(VERSION 3.16)

if(NOT DEFINED VCPKG_TARGET_TRIPLET)
    if(DEFINED ENV{VCPKG_DEFAULT_TRIPLET})
        set(VCPKG_TARGET_TRIPLET "$ENV{VCPKG_DEFAULT_TRIPLET}" CACHE STRING "")
    else()
        set(VCPKG_TARGET_TRIPLET "x64-windows" CACHE STRING "")
    endif()
endif()

if(NOT DEFINED CMAKE_TOOLCHAIN_FILE)
    set(CMAKE_TOOLCHAIN_FILE "IDE/VC/vcpkg/vcpkg/scripts/buildsystems/vcpkg.cmake" CACHE STRING "")
endif()

project(dunelegacy)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_INTERPROCEDURAL_OPTIMIZATION ON)
set(CMAKE_INTERPROCEDURAL_OPTIMIZATION_DEBUG OFF)

if(WIN32)
    add_compile_definitions(WIN32_LEAN_AND_MEAN NOMINMAX)
    add_compile_definitions(NTDDI_VERSION=NTDDI_VISTASP2 _WIN32_WINNT=_WIN32_WINNT_VISTA)
    #add_compile_definitions(_HAS_CXX17) # This is bad and don't build stuff with it, but it makes Intellisense tolerable while active.
endif()

if(MSVC)
	set(AUDACITY_MSVC_FLAGS " /diagnostics:caret /GS /Gy /Gw /utf-8 /permissive- /Zc:__cplusplus /Zc:inline")
	string(APPEND AUDACITY_MSVC_FLAGS " /wd4267")

    string( APPEND CMAKE_C_FLAGS ${AUDACITY_MSVC_FLAGS} )
    string( APPEND CMAKE_CXX_FLAGS ${AUDACITY_MSVC_FLAGS} )
endif()


add_subdirectory(src)

message(STATUS "          CMAKE_C_FLAGS: ${CMAKE_C_FLAGS}")
message(STATUS "        CMAKE_CXX_FLAGS: ${CMAKE_CXX_FLAGS}")
message(STATUS "   VCPKG_TARGET_TRIPLET: ${VCPKG_TARGET_TRIPLET}")
