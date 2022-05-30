if(WIN32)
	add_compile_definitions(_WINSOCK_DEPRECATED_NO_WARNINGS)
endif()

add_subdirectory(enet)
target_include_directories(enet INTERFACE enet/include)
