
find_package(GTest CONFIG REQUIRED)

add_executable(INIFileTestCase
    INIFileTestCase1.cpp
    INIFileTestCase2.cpp
    INIFileTestCase3.cpp
    INIFileTestSimple.cpp
    InitGameTestCase.cpp
)

target_link_libraries(INIFileTestCase PRIVATE dune GTest::gtest GTest::gtest_main)

set(INI_FILES
    ${CMAKE_CURRENT_SOURCE_DIR}/INIFileTestCase1.ini
    ${CMAKE_CURRENT_SOURCE_DIR}/INIFileTestCase2.ini
    ${CMAKE_CURRENT_SOURCE_DIR}/INIFileTestCase2.ini.ref1
    ${CMAKE_CURRENT_SOURCE_DIR}/INIFileTestCase2.ini.ref2
    ${CMAKE_CURRENT_SOURCE_DIR}/INIFileTestCase2.ini.ref3
    ${CMAKE_CURRENT_SOURCE_DIR}/INIFileTestCase3.ini
    ${CMAKE_CURRENT_SOURCE_DIR}/INIFileTestCase3.ini.ref1
    ${CMAKE_CURRENT_SOURCE_DIR}/INIFileTestCase3.ini.ref2
    ${CMAKE_CURRENT_SOURCE_DIR}/INIFileTestCase3.ini.ref3
    ${CMAKE_CURRENT_SOURCE_DIR}/INIFileTestCase3.ini.ref4
    "${CMAKE_CURRENT_SOURCE_DIR}/../../data/maps/multiplayer/2P - 32x128 - Gatekeeper.ini"
    "${CMAKE_CURRENT_SOURCE_DIR}/../../data/maps/multiplayer/2P - 32x32 - X-Factor.ini"
    "${CMAKE_CURRENT_SOURCE_DIR}/../../data/maps/multiplayer/2P - 64x32 - Cliffs Of Rene.ini"
    "${CMAKE_CURRENT_SOURCE_DIR}/../../data/maps/multiplayer/2P - 64x64 - Bottle Neck.ini"
    "${CMAKE_CURRENT_SOURCE_DIR}/../../data/maps/multiplayer/2P - 64x64 - Broken Mountains.ini"
    "${CMAKE_CURRENT_SOURCE_DIR}/../../data/maps/multiplayer/2P - 64x64 - David's Pass.ini"
    "${CMAKE_CURRENT_SOURCE_DIR}/../../data/maps/multiplayer/2P - 64x64 - Face Off.ini"
    "${CMAKE_CURRENT_SOURCE_DIR}/../../data/maps/multiplayer/2P - 64x64 - Great Divide.ini"
    "${CMAKE_CURRENT_SOURCE_DIR}/../../data/maps/multiplayer/2P - 64x64 - Sanctuarys.ini"
    "${CMAKE_CURRENT_SOURCE_DIR}/../../data/maps/multiplayer/4P - 128x128 - Deserted.ini"
    "${CMAKE_CURRENT_SOURCE_DIR}/../../data/maps/multiplayer/4P - 128x128 - Equilibrium.ini"
    "${CMAKE_CURRENT_SOURCE_DIR}/../../data/maps/multiplayer/4P - 128x128 - Four Cities.ini"
    "${CMAKE_CURRENT_SOURCE_DIR}/../../data/maps/multiplayer/4P - 128x128 - Hungry Hippos.ini"
    "${CMAKE_CURRENT_SOURCE_DIR}/../../data/maps/multiplayer/4P - 128x128 - Silicon Valley XL.ini"
    "${CMAKE_CURRENT_SOURCE_DIR}/../../data/maps/multiplayer/4P - 128x128 - Snake Pass.ini"
    "${CMAKE_CURRENT_SOURCE_DIR}/../../data/maps/multiplayer/4P - 128x128 - Spicestorm.ini"
    "${CMAKE_CURRENT_SOURCE_DIR}/../../data/maps/multiplayer/4P - 128x128 - The Sardaukar Outpost.ini"
    "${CMAKE_CURRENT_SOURCE_DIR}/../../data/maps/multiplayer/4P - 128x128 - Worm Investation.ini"
    "${CMAKE_CURRENT_SOURCE_DIR}/../../data/maps/multiplayer/4P - 128x128 - Wormhole.ini"
    "${CMAKE_CURRENT_SOURCE_DIR}/../../data/maps/multiplayer/4P - 128x64 - Gamma Sector.ini"
    "${CMAKE_CURRENT_SOURCE_DIR}/../../data/maps/multiplayer/4P - 64x64 - Channels.ini"
    "${CMAKE_CURRENT_SOURCE_DIR}/../../data/maps/multiplayer/4P - 64x64 - Clear Path.ini"
    "${CMAKE_CURRENT_SOURCE_DIR}/../../data/maps/multiplayer/4P - 64x64 - Combed.ini"
    "${CMAKE_CURRENT_SOURCE_DIR}/../../data/maps/multiplayer/4P - 64x64 - Four Chambers.ini"
    "${CMAKE_CURRENT_SOURCE_DIR}/../../data/maps/multiplayer/4P - 64x64 - Four Courners.ini"
    "${CMAKE_CURRENT_SOURCE_DIR}/../../data/maps/multiplayer/4P - 64x64 - Sietch Stefan.ini"
    "${CMAKE_CURRENT_SOURCE_DIR}/../../data/maps/multiplayer/4P - 64x64 - Silicon Valley.ini"
    "${CMAKE_CURRENT_SOURCE_DIR}/../../data/maps/multiplayer/4P - 64x64 - Stronghold.ini"
    "${CMAKE_CURRENT_SOURCE_DIR}/../../data/maps/multiplayer/4P - 64x64 - Vast Armies Have Arrived.ini"
    "${CMAKE_CURRENT_SOURCE_DIR}/../../data/maps/multiplayer/5P - 128x128 - Fortress.ini"
    "${CMAKE_CURRENT_SOURCE_DIR}/../../data/maps/multiplayer/5P - 128x128 - Gridlocked.ini"
    "${CMAKE_CURRENT_SOURCE_DIR}/../../data/maps/multiplayer/5P - 128x128 - Hellvetika.ini"
    "${CMAKE_CURRENT_SOURCE_DIR}/../../data/maps/multiplayer/5P - 128x128 - Kragetam.ini"
    "${CMAKE_CURRENT_SOURCE_DIR}/../../data/maps/multiplayer/5P - 128x128 - Meadow.ini"
    "${CMAKE_CURRENT_SOURCE_DIR}/../../data/maps/multiplayer/5P - 128x64 - Watch Your Track.ini"
    "${CMAKE_CURRENT_SOURCE_DIR}/../../data/maps/multiplayer/6P - 128x128 - Gargantuan Mountains.ini"
    "${CMAKE_CURRENT_SOURCE_DIR}/../../data/maps/multiplayer/6P - 64x128 - Rocking Fields.ini"
    "${CMAKE_CURRENT_SOURCE_DIR}/../../data/maps/multiplayer/6P - 64x64 - Fertile Basin.ini"
    "${CMAKE_CURRENT_SOURCE_DIR}/../../data/maps/singleplayer/2P - 32x128 - Canyon.ini"
    "${CMAKE_CURRENT_SOURCE_DIR}/../../data/maps/singleplayer/2P - 64x64 - Duality.ini"
    "${CMAKE_CURRENT_SOURCE_DIR}/../../data/maps/singleplayer/2P - 64x64 - North vs. South.ini"
    "${CMAKE_CURRENT_SOURCE_DIR}/../../data/maps/singleplayer/2P - 64x64 - Twin Fists.ini"
    "${CMAKE_CURRENT_SOURCE_DIR}/../../data/maps/singleplayer/3P - 64x32 - Middle Man.ini"
    "${CMAKE_CURRENT_SOURCE_DIR}/../../data/maps/singleplayer/4P - 64x64 - 3 vs 1.ini"
    "${CMAKE_CURRENT_SOURCE_DIR}/../../data/maps/singleplayer/5P - 128x128 - All against Atreides.ini"
    "${CMAKE_CURRENT_SOURCE_DIR}/../../data/maps/singleplayer/5P - 128x128 - Sardaukar Base Easy.ini"
    "${CMAKE_CURRENT_SOURCE_DIR}/../../data/maps/singleplayer/5P - 128x128 - Sardaukar Base.ini"
)

set(PAK_FILES
    "${CMAKE_CURRENT_SOURCE_DIR}/../../data/OPENSD2.PAK"
    "${CMAKE_CURRENT_SOURCE_DIR}/../../data/LEGACY.PAK"
)

set(INPUT_FILES ${INI_FILES} ${PAK_FILES})

foreach(f ${INPUT_FILES})
    get_filename_component(name ${f} NAME)
    list(APPEND OUTPUT_FILES ${name})
endforeach()

add_custom_target(copy_for_ini_tests ALL DEPENDS ${OUTPUT_FILES})

add_custom_command(
    DEPENDS ${INPUT_FILES}
    OUTPUT ${OUTPUT_FILES}
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
            ${INPUT_FILES}
            ${CMAKE_CURRENT_BINARY_DIR}
    VERBATIM
)

if(DUNE_PRECOMPILED_HEADERS)
    if(MSVC)
        target_precompile_headers(INIFileTestCase PRIVATE ../../src/stdafx.h)
    else()
	    target_precompile_headers(INIFileTestCase REUSE_FROM dune)
    endif()
endif()

add_test(NAME INIFile COMMAND INIFileTestCase)
