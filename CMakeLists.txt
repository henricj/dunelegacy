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

project(dunelegacy VERSION 0.96.4)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_INTERPROCEDURAL_OPTIMIZATION ON)
set(CMAKE_INTERPROCEDURAL_OPTIMIZATION_DEBUG OFF)

if(CMAKE_BUILD_TYPE)
    message(STATUS "CMAKE_BUILD_TYPE ${CMAKE_BUILD_TYPE}")
    set(build_list ${CMAKE_BUILD_TYPE} )
elseif(CMAKE_CONFIGURATION_TYPES )
    message( STATUS "CMAKE_CONFIGURATION_TYPES ${CMAKE_CONFIGURATION_TYPES}")
    set(build_list ${CMAKE_CONFIGURATION_TYPES})
endif()


if(WIN32)
    add_compile_definitions(WIN32_LEAN_AND_MEAN NOMINMAX)
    add_compile_definitions(NTDDI_VERSION=NTDDI_VISTASP2 _WIN32_WINNT=_WIN32_WINNT_VISTA)
    #add_compile_definitions(_HAS_CXX17) # This is bad and don't build stuff with it, but it makes Intellisense tolerable while active.
endif()

if(MSVC)
	set(DUNELEGACY_MSVC_FLAGS " /diagnostics:caret /GS /Gy /Gw /utf-8 /volatile:iso /permissive- /Zc:__cplusplus /Zc:inline")
	string(APPEND DUNELEGACY_MSVC_FLAGS " /wd4267")
    set(DUNELEGACY_MSVC_DEBUG_FLAGS " /Zi /Ob0 /Od /RTC1 /JMC")
    set(DUNELEGACY_MSVC_RELEASE_FLAGS " /Zi /O2 /Ob3 /fp:fast /DNDEBUG")

    set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:DebugDLL>")

    set(DUNE_TARGET_ARCHITECTURE ${CMAKE_SYSTEM_PROCESSOR} CACHE STRING "Target processor architecture")
    set_property(CACHE DUNE_TARGET_ARCHITECTURE PROPERTY STRINGS x64 x86 arm64)

    if(${DUNE_TARGET_ARCHITECTURE} MATCHES "^(x86|x64)$")
      set(DUNE_TARGET_ARCHITECTURE_EXTENSION "" CACHE STRING "Enable the avx, avx2, or avx512 exensions (avx2 implies avx, avx512 implies avx and avx2")
      set_property(CACHE DUNE_TARGET_ARCHITECTURE_EXTENSION PROPERTY STRINGS "" avx avx2 avx512)

      if("${DUNE_TARGET_ARCHITECTURE_EXTENSION}" STREQUAL "avx")
         set(HAVE_AVX ON)
         message(STATUS "Enabling AVX instructions")
         string(APPEND DUNELEGACY_MSVC_FLAGS " /arch:AVX")
      elseif("${DUNE_TARGET_ARCHITECTURE_EXTENSION}" STREQUAL "avx2")
         set(HAVE_AVX2 ON)
         message(STATUS "Enabling AVX and AVX2 instructions")
         string(APPEND DUNELEGACY_MSVC_FLAGS " /arch:AVX2")
      elseif("${DUNE_TARGET_ARCHITECTURE_EXTENSION}" STREQUAL "avx512")
         set(HAVE_AVX512 ON)
         message(STATUS "Enabling AVX, AVX2, and AVX512 instructions")
         string(APPEND DUNELEGACY_MSVC_FLAGS " /arch:AVX512")
      elseif (NOT "${DUNE_TARGET_ARCHITECTURE_EXTENSION}" STREQUAL "")
         message (FATAL_ERROR "Unknown architecture extensions: ${DUNE_TARGET_ARCHITECTURE_EXTENSION}")
      else()
         message(STATUS "Using default instructions (SSE2)")
      endif()
   endif()

    foreach( config ${build_list} )
        string( TOUPPER "${config}" config )

        if( DUNELEGACY_asan )
            if( config MATCHES "DEBUG"  )
                message( ERROR_FAIL "ASAN is not supported with debug builds" )
            endif()

            message( STATUS "Appending ASAN build flags" )
            set( CMAKE_C_FLAGS_${config} ${DUNELEGACY_MSVC_ASAN_FLAGS} )
            set( CMAKE_CXX_FLAGS_${config} ${DUNELEGACY_MSVC_ASAN_FLAGS} )
            continue()
        endif()

        if( config MATCHES "DEBUG"  )
            message( STATUS "Appending Debug build flags" )
            set( CMAKE_C_FLAGS_${config} "${DUNELEGACY_MSVC_DEBUG_FLAGS}" )
            set( CMAKE_CXX_FLAGS_${config} "${DUNELEGACY_MSVC_DEBUG_FLAGS}" )
            continue()
        endif()

        message( STATUS "Appending Release build flags" )
        set( CMAKE_C_FLAGS_${config} "${DUNELEGACY_MSVC_RELEASE_FLAGS}" )
        set( CMAKE_CXX_FLAGS_${config} "${DUNELEGACY_MSVC_RELEASE_FLAGS}" )
    endforeach()

    string( APPEND CMAKE_C_FLAGS ${DUNELEGACY_MSVC_FLAGS} )
    string( APPEND CMAKE_CXX_FLAGS ${DUNELEGACY_MSVC_FLAGS} )
endif()


include(CheckCXXSourceCompiles)

check_cxx_source_compiles(
    "#include <charconv>
    char buffer[64];
    int main(int argc, char* argv[]) {
        auto [ptr, ec] = std::to_chars(&buffer[0], &buffer[sizeof(buffer)], 1.0f);
        return 0;
    }"
    HAVE_FLOAT_TO_CHARS)

check_cxx_source_compiles(
    "#include <charconv>
    char buffer[64];
    int main(int argc, char* argv[]) {
        auto [ptr, ec] = std::to_chars(&buffer[0], &buffer[sizeof(buffer)], 1.0);
        return 0;
    }"
    HAVE_DOUBLE_TO_CHARS)

check_cxx_source_compiles(
    "#include <charconv>
    const char number[] = \"1.0\";
    int main(int argc, char* argv[]) {
        float value;
        auto [ptr, ec] = std::from_chars(&number[0], &number[sizeof(number) - 1], value);
        return 0;
    }"
    HAVE_FLOAT_FROM_CHARS)

check_cxx_source_compiles(
    "#include <charconv>
    const char number[] = \"1.0\";
    int main(int argc, char* argv[]) {
        double value;
        auto [ptr, ec] = std::from_chars(&number[0], &number[sizeof(number) - 1], value);
        return 0;
    }"
    HAVE_DOUBLE_FROM_CHARS)

add_subdirectory(external)
add_subdirectory(src)
add_subdirectory(data)

install(FILES README DESTINATION .)

enable_testing()

add_subdirectory(tests)

set(CPACK_INCLUDE_TOPLEVEL_DIRECTORY OFF)

if(WIN32)
    if(TARGET_MSI_NAME)
        set(CPACK_PACKAGE_FILE_NAME "${TARGET_MSI_NAME}")
    else()
        set(CPACK_PACKAGE_FILE_NAME "${CMAKE_PROJECT_NAME}-${DUNE_TARGET_ARCHITECTURE}")
    endif()
endif()

include(CPack)

message(STATUS "          CMAKE_C_FLAGS: ${CMAKE_C_FLAGS}")
message(STATUS "        CMAKE_CXX_FLAGS: ${CMAKE_CXX_FLAGS}")
message(STATUS "   VCPKG_TARGET_TRIPLET: ${VCPKG_TARGET_TRIPLET}")
