option(DUNE_FIND_enet "Use find_package() for enet" OFF)

if(DUNE_FIND_enet)
	find_package(enet REQUIRED)
	return()
endif()

if(WIN32)
	add_compile_definitions(_WINSOCK_DEPRECATED_NO_WARNINGS)
endif()

add_subdirectory(enet)
target_include_directories(enet INTERFACE enet/include)
