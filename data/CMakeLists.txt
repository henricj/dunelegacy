cmake_minimum_required(VERSION 3.16)

set(PakFiles
	GFXHD.PAK
	LEGACY.PAK
	OPENSD2.PAK
)

add_subdirectory(locale)
add_subdirectory(maps)

install(FILES ${PakFiles} DESTINATION .)
