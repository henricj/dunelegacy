
add_executable(dune_misc_test string_util_test.cpp)
target_include_directories(dune_misc_test PRIVATE ../../include)
target_link_libraries(dune_misc_test PRIVATE dune GTest::gtest GTest::gtest_main)

if(DUNE_PRECOMPILED_HEADERS)
	if(MSVC)
		target_precompile_headers(dune_misc_test PRIVATE ../../src/stdafx.h)
	else()
		target_precompile_headers(dune_misc_test REUSE_FROM dune)
	endif()
endif()

add_test(NAME dune_misc COMMAND dune_misc_test)
