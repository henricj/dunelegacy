target_sources(dune_core PRIVATE
	logging_compiler.cpp
)

if(WIN32)
	add_subdirectory(windows)
endif()

if(UNIX)
	add_subdirectory(posix)
endif()
