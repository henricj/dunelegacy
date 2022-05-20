add_sources(PLATFORMS_SOURCES
	logging_compiler.cpp
)

if(WIN32)
	include(platforms/windows/sources.cmake)
endif()

if(UNIX)
	include(platforms/posix/sources.cmake)
endif()
