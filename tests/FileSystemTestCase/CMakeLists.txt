
find_package(GTest CONFIG REQUIRED)

add_executable(FileSystemTestCase FileSystemTestCase.cpp)
target_link_libraries(FileSystemTestCase PRIVATE dune GTest::gtest GTest::gtest_main)

target_precompile_headers(FileSystemTestCase REUSE_FROM dune)

if(MSVC)
	set_property(TARGET FileSystemTestCase PROPERTY VS_DEBUGGER_WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})
endif()

add_test(NAME FileSystem COMMAND FileSystemTestCase)
