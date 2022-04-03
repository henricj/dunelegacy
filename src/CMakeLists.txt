cmake_minimum_required(VERSION 3.16)

set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake" ${CMAKE_MODULE_PATH})

include(sources.cmake)
include(../include/sources.cmake)

set(SOURCES
	${TOP_SOURCES}
	${CUT_SCENES_SOURCES}
	${FILE_CLASSES_SOURCES}
	${GUI_SOURCES}
	${INIMAP_SOURCES}
	${MAP_EDITOR_SOURCES}
	${MENU_SOURCES}
	${NETWORK_SOURCES}
	${TRIGGER_SOURCES}
	${FIX_MATH_SOURCES}
	${MISC_SOURCES}
	${PLAYERS_SOURCES}
	${STRUCTURES_SOURCES}
	${RENDERER_SOURCES}
	${UNITS_SOURCES})

find_package(SDL2 CONFIG REQUIRED)
find_package(sdl2-mixer CONFIG REQUIRED)
find_package(sdl2-ttf CONFIG REQUIRED)
find_package(fmt CONFIG REQUIRED)
find_package(lodepng CONFIG REQUIRED)
find_package(Microsoft.GSL CONFIG REQUIRED)

if(UNIX)
    find_package(X11 REQUIRED)
endif()

# If we want to really support UTF-8...
#find_path(ICU_INCLUDE_DIR unicode/unistr.h REQUIRED)
#find_library(ICU_COMMON_LIBRARY lib/icuuc.lib REQUIRED)

find_path(SOXR_INCLUDE_DIR soxr.h)
find_library(SOXR_LIBRARY_DIR soxr)

set(LIBS
	SDL2::SDL2
	SDL2::SDL2_mixer
	SDL2::SDL2_ttf
	fmt::fmt
	lodepng
	Microsoft.GSL::GSL
	digestpp
	rectpack2D
	${SOXR_LIBRARY_DIR}
	enet)


if(WIN32)
	list(APPEND LIBS Psapi shlwapi ws2_32 winmm)
	list(APPEND SOURCES fixmath/fixmath.natvis)
	list(APPEND EXE_SOURCES ../Windows/DuneLegacy.exe.manifest ../resource.rc)
endif()

if(UNIX)
	list(APPEND LIBS ${X11_LIBRARIES})
endif()


add_library(dune ${SOURCES} ${HEADERS})
target_include_directories(dune PUBLIC ../include PRIVATE ../external/enet/include ${SOXR_INCLUDE_DIR})
target_link_libraries(dune PUBLIC git_version ${LIBS})

if(DUNE_CRT_HEAP_DEBUG)
	target_compile_definitions(dune PUBLIC DUNE_CRT_HEAP_DEBUG)
	if(WIN32)
		target_compile_definitions(dune PUBLIC _CRTDBG_MAP_ALLOC)
	endif()
endif()

include(CheckIncludeFile)

CHECK_INCLUDE_FILE_CXX(cxxabi.h, HAVE_CXXABI_H)

if(HAVE_MS_LOCALTIME_S)
	target_compile_definitions(dune PUBLIC HAVE_MS_LOCALTIME_S)
endif()
if(HAVE_LOCALTIME_R)
	target_compile_definitions(dune PUBLIC HAVE_LOCALTIME_R)
endif()

if(HAVE_STRERROR_S)
	target_compile_definitions(dune PUBLIC HAVE_STRERROR_S)
elseif(HAVE_STRERROR_R)
	target_compile_definitions(dune PUBLIC HAVE_STRERROR_R)
elseif(HAVE_GNU_STRERROR_R)
	target_compile_definitions(dune PUBLIC HAVE_GNU_STRERROR_R)
endif()

if(NOT HAVE_FLOAT_TO_CHARS)
	target_compile_definitions(dune PUBLIC NEED_FLOAT_TO_CHARS)
endif()
if(NOT HAVE_DOUBLE_TO_CHARS)
	target_compile_definitions(dune PUBLIC NEED_DOUBLE_TO_CHARS)
endif()
if(NOT HAVE_FLOAT_FROM_CHARS)
	target_compile_definitions(dune PUBLIC NEED_FLOAT_FROM_CHARS)
endif()
if(NOT HAVE_DOUBLE_FROM_CHARS)
	target_compile_definitions(dune PUBLIC NEED_DOUBLE_FROM_CHARS)
endif()

target_precompile_headers(dune PRIVATE stdafx.h)

add_executable(dunelegacy ${EXE_SOURCES} ${EXE_HEADERS})
target_link_libraries(dunelegacy PRIVATE dune SDL2::SDL2main)

if(HAVE_CXXABI_H)
	target_compile_definitions(dunelegacy PUBLIC HAVE_CXXABI_H)
endif()

if(MSVC)
   if(${DUNE_TARGET_ARCHITECTURE} STREQUAL "x86")
      target_link_options(dunelegacy PRIVATE /LARGEADDRESSAWARE )
   endif()
endif()


target_precompile_headers(dunelegacy REUSE_FROM dune)

add_custom_command(
        TARGET dunelegacy POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
                ${CMAKE_SOURCE_DIR}/data/locale
                ${CMAKE_CURRENT_BINARY_DIR}/locale)

add_custom_command(
        TARGET dunelegacy POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
                ${CMAKE_SOURCE_DIR}/data/maps
                ${CMAKE_CURRENT_BINARY_DIR}/maps)

if(WIN32)
   set_target_properties(dunelegacy PROPERTIES WIN32_EXECUTABLE ON)
endif()

install(TARGETS dunelegacy RUNTIME DESTINATION .)

set(CLANGFORMAT_SOURCES ${SOURCES} ${EXE_SOURCES} ${HEADERS} ${EXE_HEADERS})

add_custom_target(
	clangformat
	COMMAND clang-format
		--style=file
		-i
		${CLANGFORMAT_SOURCES}
	WORKING_DIRECTORY
		${CMAKE_CURRENT_SOURCE_DIR}
)

include(locale-update)

