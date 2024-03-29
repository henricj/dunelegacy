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
	${PLATFORMS_SOURCES}
	${PLAYERS_SOURCES}
	${STRUCTURES_SOURCES}
	${RENDERER_SOURCES}
	${UNITS_SOURCES}
)

find_package(SDL2 CONFIG REQUIRED)
find_package(SDL2_mixer CONFIG REQUIRED)
find_package(SDL2_ttf CONFIG REQUIRED)
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
	SDL2_mixer::SDL2_mixer${DUNE_LIBRARY_POSTFIX}
	SDL2_ttf::SDL2_ttf${DUNE_LIBRARY_POSTFIX}
	SDL2::SDL2
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
	list(APPEND SOURCES fixmath/fixmath.natvis FileClasses/INIFile.natvis)
	list(APPEND EXE_SOURCES ../Windows/DuneLegacy.exe.manifest ../resource.rc)
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

add_library(dune STATIC ${SOURCES} ${HEADERS})
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

if(DUNE_PRECOMPILED_HEADERS)
	target_precompile_headers(dune PRIVATE stdafx.h)
endif()

add_executable(dunelegacy ${EXE_SOURCES} ${EXE_HEADERS})
target_compile_options(dunelegacy PRIVATE ${dune_flags})
target_link_libraries(dunelegacy PRIVATE dune SDL2::SDL2main harden_interface)

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

set(CLANGFORMAT_SOURCES ${SOURCES} ${EXE_SOURCES} ${HEADERS} ${EXE_HEADERS} stdafx.h)

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

