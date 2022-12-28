option(DUNE_FIND_adplug "Use find_package() for adplug" OFF)

if(DUNE_FIND_adplug)
	find_package(adplug REQUIRED)
else()
	add_subdirectory(adplug)
endif()
