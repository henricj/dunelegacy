set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake" ${CMAKE_MODULE_PATH})

find_package(ZLIB REQUIRED)

find_package(SDL3 CONFIG REQUIRED)
find_package(SDL3_mixer CONFIG REQUIRED)
find_package(SDL3_ttf CONFIG REQUIRED)
find_package(fmt CONFIG REQUIRED)
find_package(lodepng CONFIG REQUIRED)
find_package(Microsoft.GSL CONFIG REQUIRED)

if(UNIX AND NOT APPLE)
    find_package(X11 REQUIRED)
endif()

# If we want to really support UTF-8...
#find_path(ICU_INCLUDE_DIR unicode/unistr.h REQUIRED)
#find_library(ICU_COMMON_LIBRARY lib/icuuc.lib REQUIRED)

find_path(SOXR_INCLUDE_DIR soxr.h)
find_library(SOXR_LIBRARY_DIR soxr)

if(WIN32)
	string(FIND ${VCPKG_TARGET_TRIPLET} "-static" DUNE_STATIC_VCPKG_TRIPLET_POSITION)

	if(NOT DUNE_STATIC_VCPKG_TRIPLET_POSITION EQUAL -1)
		set(DUNE_LIBRARY_POSTFIX "-static")
	endif()
else()
	set(DUNE_LIBRARY_POSTFIX "-static")
endif()

if(APPLE)
    add_subdirectory(macOS)
endif()

set(LIBS
	SDL3_mixer::SDL3_mixer
	SDL3_ttf::SDL3_ttf
	SDL3::SDL3
	fmt::fmt
	lodepng
	Microsoft.GSL::GSL
	adplug::adplug
	libbinio::libbinio
	digestpp
	rectpack2D
	${SOXR_LIBRARY_DIR}
	enet
	harden_interface
)


if(WIN32)
	list(APPEND LIBS Psapi shlwapi ws2_32 winmm)
    # Note: These cannot simply be added via target_sources if they are unconditional or need special handling
    # But using target_sources for C++ files is preferred.
    # fixmath/fixmath.natvis is in fixmath/
    # FileClasses/INIFile.natvis is in FileClasses/
    # We will handle subdirectories shortly.
    # For now, let's keep win32 specific logic simple or push it down.
	# We'll use target_sources(dune PRIVATE ...) for these later.
endif()

if(UNIX AND NOT APPLE)
	list(APPEND LIBS ${X11_LIBRARIES})
endif()

if(APPLE)
	list(APPEND LIBS dunelegacyObjc)
endif()

set(dune_flags ${DUNE_TARGET_COMPILE_FLAGS})
separate_arguments(dune_flags)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Define library target FIRST
add_library(dune STATIC)

# Add sources in current directory
target_sources(dune PRIVATE
	Audio/SDLAudioEngine.cpp
	AStarSearch.cpp
	Bullet.cpp
	Choam.cpp
	Command.cpp
	CommandManager.cpp
	Explosion.cpp
	Game.cpp
	GameInput.cpp
	GameRender.cpp
	GameSaveLoad.cpp
	GameSelection.cpp
	GameUpdate.cpp
	GameInitSettings.cpp
	GameInterface.cpp
	globals.cpp
	House.cpp
	Map.cpp
	MapSeed.cpp
	mmath.cpp
	ObjectBase.cpp
	ObjectData.cpp
	ObjectManager.cpp
	ObjectPointer.cpp
	RadarView.cpp
	RadarViewBase.cpp
	sand.cpp
	ScreenBorder.cpp
	SoundPlayer.cpp
	Tile.cpp
)

# Add subdirectories
add_subdirectory(CutScenes)
add_subdirectory(FileClasses)
add_subdirectory(fixmath)
add_subdirectory(GUI)
add_subdirectory(INIMap)
add_subdirectory(MapEditor)
add_subdirectory(Menu)
add_subdirectory(misc)
add_subdirectory(Network)
add_subdirectory(platforms)
add_subdirectory(players)
add_subdirectory(Renderer)
add_subdirectory(structures)
add_subdirectory(Trigger)
add_subdirectory(units)

# Add headers from include directory
add_subdirectory(../include include)

target_compile_options(dune PRIVATE ${dune_flags})
target_include_directories(dune PUBLIC ../include PRIVATE ../external/enet/include ${SOXR_INCLUDE_DIR})
target_link_libraries(dune PUBLIC ${LIBS})

if(TARGET dune_gitversion)
  target_link_libraries(dune PRIVATE "$<BUILD_INTERFACE:dune_gitversion>")
endif()

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

if(HAVE_PARENTHESIZED_INITIALIZATION_OF_AGGREGATES)
	target_compile_definitions(dune PUBLIC HAVE_PARENTHESIZED_INITIALIZATION_OF_AGGREGATES)
endif()

if(HAVE_STD_ASYNC)
	target_compile_definitions(dune PUBLIC HAVE_STD_ASYNC)
endif()

if(DUNE_PRECOMPILED_HEADERS)
	target_precompile_headers(dune PRIVATE stdafx.h)
endif()

# Define Executable
# Note: main.h and logging.h are in ../include, but belong to the executable
add_executable(dunelegacy 
    main.cpp
    MainConfig.cpp
    MainVideo.cpp
    logging.cpp
    ../include/main.h 
    ../include/logging.h
)

if(WIN32)
    # Add windows specific sources/manifests to executable
    target_sources(dunelegacy PRIVATE 
        ../Windows/DuneLegacy.exe.manifest 
        ../resource.rc
    )
    # Add natvis files to library
    target_sources(dune PRIVATE
        fixmath/fixmath.natvis 
        FileClasses/INIFile.natvis
    )
endif()

target_compile_options(dunelegacy PRIVATE ${dune_flags})
target_link_libraries(dunelegacy PRIVATE dune harden_interface)

if(TARGET dune_gitversion)
  target_link_libraries(dunelegacy PRIVATE "$<BUILD_INTERFACE:dune_gitversion>")
endif()

if(HAVE_CXXABI_H)
	target_compile_definitions(dunelegacy PUBLIC HAVE_CXXABI_H)
endif()

if(MSVC)
   if(${DUNE_TARGET_ARCHITECTURE} STREQUAL "x86")
      target_link_options(dunelegacy PRIVATE /LARGEADDRESSAWARE )
   endif()
endif()

if(DUNE_PRECOMPILED_HEADERS)
	if(MSVC)
		target_precompile_headers(dunelegacy PRIVATE stdafx.h)
	else()
		target_precompile_headers(dunelegacy REUSE_FROM dune)
	endif()
endif()

add_custom_target(copy_locale_and_maps ALL)

add_custom_command(
    TARGET copy_locale_and_maps POST_BUILD
	COMMENT "Copying locale"
    COMMAND "${CMAKE_COMMAND}" -E copy_directory
            "${CMAKE_SOURCE_DIR}/data/locale"
            "${CMAKE_CURRENT_BINARY_DIR}/locale"
    VERBATIM
)

add_custom_command(
    TARGET copy_locale_and_maps POST_BUILD
	COMMENT "Copying maps"
    COMMAND "${CMAKE_COMMAND}" -E copy_directory
            "${CMAKE_SOURCE_DIR}/data/maps"
            "${CMAKE_CURRENT_BINARY_DIR}/maps"
    VERBATIM
)

if(WIN32)
   set_target_properties(dunelegacy PROPERTIES WIN32_EXECUTABLE ON)
endif()

install(TARGETS dunelegacy RUNTIME DESTINATION .)

# clangformat uses the explicit target source lists (no globbing).
get_target_property(DUNE_SOURCES dune SOURCES)
get_target_property(DUNELEGACY_SOURCES dunelegacy SOURCES)
set(CLANGFORMAT_SOURCES ${DUNE_SOURCES} ${DUNELEGACY_SOURCES})
list(FILTER CLANGFORMAT_SOURCES INCLUDE REGEX ".*\\.(cpp|h)$")
list(REMOVE_DUPLICATES CLANGFORMAT_SOURCES)

add_custom_target(
	clangformat
	COMMAND clang-format
		--style=file
		-i
		${CLANGFORMAT_SOURCES}
	WORKING_DIRECTORY
		${CMAKE_CURRENT_SOURCE_DIR}
)

# Extract SOURCES from the dune target for locale extraction
get_target_property(SOURCES dune SOURCES)

include(locale-update)
