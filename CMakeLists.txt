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
    set(CMAKE_TOOLCHAIN_FILE "${CMAKE_CURRENT_SOURCE_DIR}/external/vcpkg/vcpkg/scripts/buildsystems/vcpkg.cmake" CACHE STRING "")
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
    include(setup/asan-setup)
endif()

if(DUNE_UBSAN)
    include(setup/ubsan-setup)
endif()

if(WIN32)
    include(setup/win32-setup)
endif()

if(MSVC)
    include(setup/msvc-setup)
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

include(AddSources)

find_package(Git)

include(GitVersion)

create_git_version_target()

add_subdirectory(external)
add_subdirectory(src)
add_subdirectory(data)

install(FILES README DESTINATION .)

# googletest and the #define "new" used for heap debugging don't get along.
if(DUNE_CRT_HEAP_DEBUG)
    message(STATUS "Skipping tests")
else()
    message(STATUS "Enabling tests")

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
