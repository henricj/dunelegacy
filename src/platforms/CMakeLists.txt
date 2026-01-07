target_sources(dune PRIVATE
	logging_compiler.cpp
)

if(WIN32)
	add_subdirectory(windows)
endif()

if(UNIX)
	add_subdirectory(posix)
endif()
