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

# dune_core: simulation engine and game logic
add_library(dune_core STATIC)

# Add sources in current directory
# Keep translation units in dune_core while they still satisfy dune_core symbols.
target_sources(dune_core PRIVATE
	AStarSearch.cpp
	Bullet.cpp
	Choam.cpp
	Command.cpp
	CommandManager.cpp
	Explosion.cpp
	GameInitSettings.cpp
	GameSaveLoad.cpp
	globals.cpp
	House.cpp
	Map.cpp
	MapSeed.cpp
	mmath.cpp
	ObjectBase.cpp
	ObjectData.cpp
	ObjectManager.cpp
	ObjectPointer.cpp
	sand_core.cpp
	Tile.cpp
	View/PublishedFrame.cpp
)

# Non-UI subdirectories (contribute to dune_core)
add_subdirectory(FileClasses)
add_subdirectory(fixmath)
add_subdirectory(INIMap)
add_subdirectory(misc)
add_subdirectory(Network)
add_subdirectory(platforms)
add_subdirectory(players)
add_subdirectory(Renderer)
add_subdirectory(structures)
add_subdirectory(Trigger)
add_subdirectory(units)
add_subdirectory(View)

# Add headers from include directory
add_subdirectory(../include include)

# dune: UI shell — depends on dune_core for all simulation and game logic
add_library(dune STATIC)

# Game/UI integration files that no longer satisfy dune_core references.
target_sources(dune PRIVATE
	Game.cpp
	GameCore.cpp
	GameInterface.cpp
	GameSelection.cpp
	GameUpdate.cpp
	GameRender.cpp
	GameInput.cpp
	GameUIController.cpp
	RadarView.cpp
	RadarViewBase.cpp
	ScreenBorder.cpp
	Audio/SDLAudioEngine.cpp
	SoundPlayer.cpp
	sand.cpp
	View/PublishedFrameBuilder.cpp
)

# UI subdirectories (contribute to dune)
add_subdirectory(CutScenes)
add_subdirectory(GUI)
add_subdirectory(MapEditor)
add_subdirectory(Menu)

target_compile_options(dune_core PRIVATE ${dune_flags})
target_include_directories(dune_core PUBLIC ../include PRIVATE ../external/enet/include ${SOXR_INCLUDE_DIR})
target_link_libraries(dune_core PUBLIC ${LIBS})

if(TARGET dune_gitversion)
  target_link_libraries(dune_core PRIVATE "$<BUILD_INTERFACE:dune_gitversion>")
endif()

if(DUNE_CRT_HEAP_DEBUG)
	target_compile_definitions(dune_core PUBLIC DUNE_CRT_HEAP_DEBUG)
	if(WIN32)
		target_compile_definitions(dune_core PUBLIC _CRTDBG_MAP_ALLOC)
	endif()
endif()

include(CheckIncludeFile)

CHECK_INCLUDE_FILE_CXX(cxxabi.h, HAVE_CXXABI_H)

if(HAVE_MS_LOCALTIME_S)
	target_compile_definitions(dune_core PUBLIC HAVE_MS_LOCALTIME_S)
endif()
if(HAVE_LOCALTIME_R)
	target_compile_definitions(dune_core PUBLIC HAVE_LOCALTIME_R)
endif()

if(HAVE_STRERROR_S)
	target_compile_definitions(dune_core PUBLIC HAVE_STRERROR_S)
elseif(HAVE_STRERROR_R)
	target_compile_definitions(dune_core PUBLIC HAVE_STRERROR_R)
elseif(HAVE_GNU_STRERROR_R)
	target_compile_definitions(dune_core PUBLIC HAVE_GNU_STRERROR_R)
endif()

if(NOT HAVE_FLOAT_TO_CHARS)
	target_compile_definitions(dune_core PUBLIC NEED_FLOAT_TO_CHARS)
endif()
if(NOT HAVE_DOUBLE_TO_CHARS)
	target_compile_definitions(dune_core PUBLIC NEED_DOUBLE_TO_CHARS)
endif()
if(NOT HAVE_FLOAT_FROM_CHARS)
	target_compile_definitions(dune_core PUBLIC NEED_FLOAT_FROM_CHARS)
endif()
if(NOT HAVE_DOUBLE_FROM_CHARS)
	target_compile_definitions(dune_core PUBLIC NEED_DOUBLE_FROM_CHARS)
endif()

if(HAVE_PARENTHESIZED_INITIALIZATION_OF_AGGREGATES)
	target_compile_definitions(dune_core PUBLIC HAVE_PARENTHESIZED_INITIALIZATION_OF_AGGREGATES)
endif()

if(HAVE_STD_ASYNC)
	target_compile_definitions(dune_core PUBLIC HAVE_STD_ASYNC)
endif()

if(DUNE_PRECOMPILED_HEADERS)
	target_precompile_headers(dune_core PRIVATE stdafx.h)
endif()

target_compile_options(dune PRIVATE ${dune_flags})
target_link_libraries(dune PUBLIC dune_core)

if(TARGET dune_gitversion)
  target_link_libraries(dune PRIVATE "$<BUILD_INTERFACE:dune_gitversion>")
endif()

if(DUNE_PRECOMPILED_HEADERS)
	if(MSVC)
		target_precompile_headers(dune PRIVATE stdafx.h)
	else()
		target_precompile_headers(dune REUSE_FROM dune_core)
	endif()
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
    # Add natvis files to dune_core for debugger visualization
    target_sources(dune_core PRIVATE
        fixmath/fixmath.natvis
        FileClasses/INIFile.natvis
    )
endif()

target_compile_options(dunelegacy PRIVATE ${dune_flags})
dune_link_dune_and_core(dunelegacy)
target_link_libraries(dunelegacy PRIVATE harden_interface)

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
		target_precompile_headers(dunelegacy REUSE_FROM dune_core)
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
get_target_property(DUNE_CORE_SOURCES dune_core SOURCES)
get_target_property(DUNE_SOURCES dune SOURCES)
get_target_property(DUNELEGACY_SOURCES dunelegacy SOURCES)
set(CLANGFORMAT_SOURCES ${DUNE_CORE_SOURCES} ${DUNE_SOURCES} ${DUNELEGACY_SOURCES})
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

# Extract SOURCES from both targets for locale extraction
get_target_property(DUNE_CORE_LOCALE_SOURCES dune_core SOURCES)
get_target_property(DUNE_LOCALE_SOURCES dune SOURCES)
set(SOURCES ${DUNE_CORE_LOCALE_SOURCES} ${DUNE_LOCALE_SOURCES})

include(locale-update)
