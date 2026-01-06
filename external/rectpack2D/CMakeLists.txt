option(DUNE_FIND_rectpack2D "Use find_package() for rectpack2D" OFF)

if(DUNE_FIND_rectpack2D)
	find_package(rectpack2D REQUIRED)
	return()
endif()

add_library(rectpack2D INTERFACE)
target_include_directories(rectpack2D INTERFACE "${CMAKE_CURRENT_SOURCE_DIR}/rectpack2D/src")
