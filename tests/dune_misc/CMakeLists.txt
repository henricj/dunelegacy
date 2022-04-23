
add_executable(dune_misc_test string_util_test.cpp)
target_include_directories(dune_misc_test PRIVATE ../../include)
target_link_libraries(dune_misc_test PRIVATE dune GTest::gtest GTest::gtest_main)

target_precompile_headers(dune_misc_test REUSE_FROM dune)

add_test(NAME dune_misc COMMAND dune_misc_test)
