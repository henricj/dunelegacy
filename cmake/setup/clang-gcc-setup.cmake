message(STATUS "Configuring Clang/GCC")

add_compile_options(-Wall)

set(DUNE_TARGET_ARCHITECTURE ${CMAKE_SYSTEM_PROCESSOR} CACHE STRING "Target processor architecture")
set_property(CACHE DUNE_TARGET_ARCHITECTURE PROPERTY STRINGS x64 x86 arm64)

set(DUNE_TARGET_ARCHITECTURE_EXTENSION "" CACHE STRING "Specify the specific target architecture default native")
set_property(CACHE DUNE_TARGET_ARCHITECTURE_EXTENSION PROPERTY STRINGS "" default)

if(DUNE_TARGET_ARCHITECTURE_EXTENSION)
	if(DUNE_TARGET_ARCHITECTURE_EXTENSION STREQUAL "default")
	else()
		add_compile_options("-march=${DUNE_TARGET_ARCHITECTURE_EXTENSION}")
	endif()
endif()
