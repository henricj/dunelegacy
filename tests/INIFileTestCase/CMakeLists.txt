
find_package(GTest CONFIG REQUIRED)

add_executable(INIFileTestCase INIFileTestCase1.cpp INIFileTestCase2.cpp INIFileTestCase3.cpp)
target_link_libraries(INIFileTestCase PRIVATE engine GTest::gtest GTest::gtest_main)

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
)

add_custom_command(
        TARGET INIFileTestCase POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy
                ${INI_FILES}
                ${CMAKE_CURRENT_BINARY_DIR})

target_precompile_headers(INIFileTestCase REUSE_FROM dune)

add_test(NAME INIFile COMMAND INIFileTestCase)
