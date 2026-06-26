
add_executable(dune_misc_test string_util_test.cpp md5_test.cpp)
target_include_directories(dune_misc_test PRIVATE ../../include)
dune_link_dune_and_core(dune_misc_test)
target_link_libraries(dune_misc_test PRIVATE GTest::gtest GTest::gtest_main)

if(DUNE_PRECOMPILED_HEADERS)
	target_precompile_headers(dune_misc_test PRIVATE ../../src/stdafx.h)
endif()

add_test(NAME dune_misc COMMAND dune_misc_test)
