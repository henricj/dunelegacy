option(DUNE_FIND_libbinio "Use find_package() for libbinio" OFF)

if(DUNE_FIND_libbinio)
	find_package(libbinio REQUIRED)
else()
	add_subdirectory(libbinio)
endif()
