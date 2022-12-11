set(ADPLUG_GENERATED_INCLUDE ${CMAKE_CURRENT_BINARY_DIR}/include/adplug)

file(MAKE_DIRECTORY ${ADPLUG_GENERATED_INCLUDE})

configure_file(adplug/src/version.h.in ${ADPLUG_GENERATED_INCLUDE}/version.h)

set(SOURCES
	adplug/src/adlibemu.c
	adplug/src/debug.c
	adplug/src/fmopl.c
	adplug/src/nukedopl.c
	adplug/src/a2m.cpp
	adplug/src/adl.cpp
	adplug/src/adplug.cpp
	adplug/src/adtrack.cpp
	adplug/src/amd.cpp
	adplug/src/analopl.cpp
	adplug/src/bam.cpp
	adplug/src/bmf.cpp
	adplug/src/cff.cpp
	adplug/src/cmf.cpp
	adplug/src/cmfmcsop.cpp
	adplug/src/coktel.cpp
	adplug/src/composer.cpp
	adplug/src/d00.cpp
	adplug/src/database.cpp
	adplug/src/dfm.cpp
	adplug/src/diskopl.cpp
	adplug/src/dmo.cpp
	adplug/src/dro2.cpp
	adplug/src/dro.cpp
	adplug/src/dtm.cpp
	adplug/src/emuopl.cpp
	adplug/src/flash.cpp
	adplug/src/fmc.cpp
	adplug/src/fprovide.cpp
	adplug/src/got.cpp
	adplug/src/herad.cpp
	adplug/src/hsc.cpp
	adplug/src/hsp.cpp
	adplug/src/hybrid.cpp
	adplug/src/hyp.cpp
	adplug/src/imf.cpp
	adplug/src/jbm.cpp
	adplug/src/ksm.cpp
	adplug/src/lds.cpp
	adplug/src/mad.cpp
	adplug/src/mdi.cpp
	adplug/src/mid.cpp
	adplug/src/mkj.cpp
	adplug/src/msc.cpp
	adplug/src/mtk.cpp
	adplug/src/mus.cpp
	adplug/src/nemuopl.cpp
	adplug/src/player.cpp
	adplug/src/players.cpp
	adplug/src/protrack.cpp
	adplug/src/psi.cpp
	adplug/src/rad2.cpp
	adplug/src/rat.cpp
	adplug/src/raw.cpp
	adplug/src/realopl.cpp
	adplug/src/rix.cpp
	adplug/src/rol.cpp
	adplug/src/s3m.cpp
	adplug/src/sa2.cpp
	adplug/src/sng.cpp
	adplug/src/sop.cpp
	adplug/src/surroundopl.cpp
	adplug/src/temuopl.cpp
	adplug/src/u6m.cpp
	adplug/src/vgm.cpp
	adplug/src/woodyopl.cpp
	adplug/src/xad.cpp
	adplug/src/xsm.cpp
)

set(HEADERS
	adlib.h
	${ADPLUG_GENERATED_INCLUDE}/version.h
	adplug/src/adplug.h
	adplug/src/emuopl.h
	adplug/src/fmopl.h
	adplug/src/silentopl.h
	adplug/src/opl.h
	adplug/src/diskopl.h
	adplug/src/a2m.h
	adplug/src/amd.h
	adplug/src/bam.h
	adplug/src/d00.h
	adplug/src/dfm.h
	adplug/src/hsc.h
	adplug/src/hsp.h
	adplug/src/imf.h
	adplug/src/ksm.h
	adplug/src/lds.h
	adplug/src/mid.h
	adplug/src/mkj.h
	adplug/src/mtk.h
	adplug/src/protrack.h
	adplug/src/rad2.h
	adplug/src/raw.h
	adplug/src/s3m.h
	adplug/src/sa2.h
	adplug/src/sng.h
	adplug/src/u6m.h
	adplug/src/player.h
	adplug/src/fmc.h
	adplug/src/mad.h
	adplug/src/xad.h
	adplug/src/bmf.h
	adplug/src/flash.h
	adplug/src/hyp.h
	adplug/src/psi.h
	adplug/src/rat.h
	adplug/src/hybrid.h
	adplug/src/rol.h
	adplug/src/adtrack.h
	adplug/src/cff.h
	adplug/src/dtm.h
	adplug/src/dmo.h
	adplug/src/fprovide.h
	adplug/src/database.h
	adplug/src/players.h
	adplug/src/xsm.h
	adplug/src/adlibemu.h
	adplug/src/kemuopl.h
	adplug/src/dro.h
	adplug/src/realopl.h
	adplug/src/analopl.h
	adplug/src/temuopl.h
	adplug/src/msc.h
	adplug/src/rix.h
	adplug/src/adl.h
	adplug/src/jbm.h
	adplug/src/cmf.h
	adplug/src/surroundopl.h
	adplug/src/dro2.h
	adplug/src/got.h
	adplug/src/version.h
	adplug/src/wemuopl.h
	adplug/src/woodyopl.h
	adplug/src/nemuopl.h
	adplug/src/nukedopl.h
	adplug/src/mus.h
	adplug/src/mdi.h
	adplug/src/cmfmcsop.h
	adplug/src/coktel.h
	adplug/src/composer.h
	adplug/src/vgm.h
	adplug/src/sop.h
	adplug/src/herad.h
	adplug/src/strnlen.h
	adplug/src/load_helper.h
)

check_symbol_exists(_stricmp "string.h" HAVE_STRICMP)
check_symbol_exists(strcasecmp "strings.h" HAVE_STRCASECMP)

add_library(adplug ${SOURCES} ${HEADERS})
target_link_libraries(adplug PUBLIC libbinio)

if(HAVE_STRICMP)
	target_compile_definitions(adplug PRIVATE stricmp=_stricmp)
elseif(HAVE_STRCASECMP)
	target_compile_definitions(adplug PRIVATE stricmp=strcasecmp)
endif()

target_include_directories(adplug
	INTERFACE ${CMAKE_CURRENT_BINARY_DIR}/include ${CMAKE_CURRENT_SOURCE_DIR}
	PRIVATE ${ADPLUG_GENERATED_INCLUDE}
)
