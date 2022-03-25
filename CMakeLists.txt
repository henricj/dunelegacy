cmake_minimum_required(VERSION 3.16..3.22)

set(CMAKE_POLICY_DEFAULT_CMP0091 NEW)   # Respect CMAKE_MSVC_RUNTIME_LIBRARY

if(NOT DEFINED VCPKG_TARGET_TRIPLET)
    if(DEFINED ENV{VCPKG_DEFAULT_TRIPLET})
        set(VCPKG_TARGET_TRIPLET "$ENV{VCPKG_DEFAULT_TRIPLET}" CACHE STRING "")
    else()
        set(VCPKG_TARGET_TRIPLET "x64-windows" CACHE STRING "")
    endif()
endif()

if(NOT DEFINED CMAKE_TOOLCHAIN_FILE)
    set(CMAKE_TOOLCHAIN_FILE "external/vcpkg/vcpkg/scripts/buildsystems/vcpkg.cmake" CACHE STRING "")
endif()

project(dunelegacy VERSION 0.96.4)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_INTERPROCEDURAL_OPTIMIZATION ON)
set(CMAKE_INTERPROCEDURAL_OPTIMIZATION_DEBUG OFF)

if(CMAKE_BUILD_TYPE)
    message(STATUS "CMAKE_BUILD_TYPE ${CMAKE_BUILD_TYPE}")
    set(build_list ${CMAKE_BUILD_TYPE})
elseif(CMAKE_CONFIGURATION_TYPES)
    message(STATUS "CMAKE_CONFIGURATION_TYPES ${CMAKE_CONFIGURATION_TYPES}")
    set(build_list ${CMAKE_CONFIGURATION_TYPES})
endif()

add_compile_definitions($<$<CONFIG:Debug>:DEBUG>)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_CURRENT_SOURCE_DIR}/cmake)

if(DUNE_ASAN)
    message(STATUS "Enabling ASAN")

    if(DUNE_ASAN_FLAGS)
        add_compile_definitions(${DUNE_ASAN_FLAGS})
    endif()

    if(MSVC)
        add_compile_options(/fsanitize=address)
    else()
        set(asan_flags -fsanitize=address -fno-omit-frame-pointer)
        add_compile_options(${asan_flags})
        add_link_options(${asan_flags} -lpthread -ldl)
    endif()
endif()

if(DUNE_UBSAN)
    if(MSVC)
        error("UBSAN is not supported")
    else()
        message(STATUS "Enabling UBSAN")
        set(ubsan_flags -fsanitize=undefined -fno-omit-frame-pointer)
        add_compile_options(${ubsan_flags})
        add_link_options(${ubsan_flags})
    endif()
endif()

if(WIN32)
    add_compile_definitions(WIN32_LEAN_AND_MEAN NOMINMAX)
    add_compile_definitions(NTDDI_VERSION=NTDDI_VISTASP2 _WIN32_WINNT=_WIN32_WINNT_VISTA)

    set(DUNE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:DebugDLL>" CACHE STRING "MSVC Runtime")
    set_property(CACHE DUNE_MSVC_RUNTIME_LIBRARY PROPERTY STRINGS MultiThreaded MultiThreadedDLL MultiThreadedDebug MultiThreadedDLL)

    set(CMAKE_MSVC_RUNTIME_LIBRARY ${DUNE_MSVC_RUNTIME_LIBRARY})
endif()

if(MSVC)
    set(DUNE_MSVC_FLAGS " /diagnostics:caret /GS /Gy /Gw /utf-8 /volatile:iso /permissive- /Zc:__cplusplus /Zc:inline /fp:fast")
    string(APPEND DUNE_MSVC_FLAGS " /wd4267")

    set(DUNE_MSVC_DEBUG_FLAGS " /ZI /Ob0 /Od /RTC1 /JMC" CACHE STRING "Debug compiler flags")
    set(DUNE_MSVC_RELEASE_FLAGS " /Zi /O2 /Ob3 /DNDEBUG" CACHE STRING  "Release compiler flags")

    set(DUNE_TARGET_ARCHITECTURE ${CMAKE_SYSTEM_PROCESSOR} CACHE STRING "Target processor architecture")
    set_property(CACHE DUNE_TARGET_ARCHITECTURE PROPERTY STRINGS x64 x86 arm64)

    if(${DUNE_TARGET_ARCHITECTURE} MATCHES "^(x86|x64)$")
      set(DUNE_TARGET_ARCHITECTURE_EXTENSION "" CACHE STRING "Enable the avx, avx2, or avx512 exensions (avx2 implies avx, avx512 implies avx and avx2")
      set_property(CACHE DUNE_TARGET_ARCHITECTURE_EXTENSION PROPERTY STRINGS "" avx avx2 avx512)

      if("${DUNE_TARGET_ARCHITECTURE_EXTENSION}" STREQUAL "avx")
         set(HAVE_AVX ON)
         message(STATUS "Enabling AVX instructions")
         string(APPEND DUNE_MSVC_FLAGS " /arch:AVX")
      elseif("${DUNE_TARGET_ARCHITECTURE_EXTENSION}" STREQUAL "avx2")
         set(HAVE_AVX2 ON)
         message(STATUS "Enabling AVX and AVX2 instructions")
         string(APPEND DUNE_MSVC_FLAGS " /arch:AVX2")
      elseif("${DUNE_TARGET_ARCHITECTURE_EXTENSION}" STREQUAL "avx512")
         set(HAVE_AVX512 ON)
         message(STATUS "Enabling AVX, AVX2, and AVX512 instructions")
         string(APPEND DUNE_MSVC_FLAGS " /arch:AVX512")
      elseif (NOT "${DUNE_TARGET_ARCHITECTURE_EXTENSION}" STREQUAL "")
         message (FATAL_ERROR "Unknown architecture extensions: ${DUNE_TARGET_ARCHITECTURE_EXTENSION}")
      else()
         message(STATUS "Using default instructions (SSE2)")
      endif()
    endif()

    if(${DUNE_TARGET_ARCHITECTURE} STREQUAL "x86")
        string(APPEND CMAKE_EXE_LINKER_FLAGS " /LARGEADDRESSAWARE")
    endif()

    foreach(config ${build_list})
        string(TOUPPER "${config}" config)

        if(config MATCHES "DEBUG")
            message(STATUS "Appending Debug build flags")
            set(CMAKE_C_FLAGS_${config} "${DUNE_MSVC_DEBUG_FLAGS}")
            set(CMAKE_CXX_FLAGS_${config} "${DUNE_MSVC_DEBUG_FLAGS}")
            continue()
        endif()

        message(STATUS "Appending Release build flags")
        set(CMAKE_C_FLAGS_${config} "${DUNE_MSVC_RELEASE_FLAGS}")
        set(CMAKE_CXX_FLAGS_${config} "${DUNE_MSVC_RELEASE_FLAGS}")
        string(APPEND CMAKE_EXE_LINKER_FLAGS_${config} " /OPT:REF,ICF=3")
    endforeach()

    string(APPEND CMAKE_C_FLAGS ${DUNE_MSVC_FLAGS})
    string(APPEND CMAKE_CXX_FLAGS ${DUNE_MSVC_FLAGS})
endif()


include(CheckSymbolExists)
include(CheckCXXSourceCompiles)

check_symbol_exists(strerror_s "string.h" HAVE_STRERROR_S)

check_cxx_source_compiles(
    "#include <time.h>
    int main(int argc, char* argv[]) {
        time_t rawtime{};
        tm timeinfo{};
        int error = localtime_s(&timeinfo, &rawtime);
        return 0;
    }"
    HAVE_MS_LOCALTIME_S)

check_cxx_source_compiles(
    "#include <time.h>
    int main(int argc, char* argv[]) {
        time_t rawtime{};
        tm timeinfo{};
        tm* result = localtime_r(&rawtime, &timeinfo);
        return 0;
    }"
    HAVE_MS_LOCALTIME_R)


check_cxx_source_compiles(
    "#include <string.h>
    char buffer[64];
    int main(int argc, char* argv[]) {
        int error = strerror_r(7, buffer, sizeof(buffer));
        return 0;
    }"
    HAVE_STRERROR_R)

check_cxx_source_compiles(
    "#include <string.h>
    char buffer[64];
    int main(int argc, char* argv[]) {
        char* error = strerror_r(7, buffer, sizeof(buffer));
        return 0;
    }"
    HAVE_GNU_STRERROR_R)

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

find_package(Git)

include(GitVersion)

create_git_version_target()

add_subdirectory(external)
add_subdirectory(src)
add_subdirectory(data)

install(FILES README DESTINATION .)

# googletest and the #define "new" used for heap debugging don't get along.
if(NOT DUNE_CRT_HEAP_DEBUG)
    enable_testing()

    add_subdirectory(tests)
endif()

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
message(STATUS " CMAKE_EXE_LINKER_FLAGS: ${CMAKE_EXE_LINKER_FLAGS}")
message(STATUS "   VCPKG_TARGET_TRIPLET: ${VCPKG_TARGET_TRIPLET}")

if(CMAKE_MSVC_RUNTIME_LIBRARY)
    message(STATUS "           MSVC Runtime: ${CMAKE_MSVC_RUNTIME_LIBRARY}")
endif()

foreach(config ${build_list})
    string(TOUPPER "${config}" config)

    message(STATUS "          CMAKE_C_FLAGS_${config}: ${CMAKE_C_FLAGS_${config}}")
    message(STATUS "        CMAKE_CXX_FLAGS_${config}: ${CMAKE_CXX_FLAGS_${config}}")
    message(STATUS " CMAKE_EXE_LINKER_FLAGS_${config}: ${CMAKE_EXE_LINKER_FLAGS_${config}}")
endforeach()
