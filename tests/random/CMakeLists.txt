
add_executable(lemire_test lemire_test.cpp)
target_include_directories(lemire_test PRIVATE ../../include)
target_link_libraries(lemire_test PRIVATE GTest::gtest GTest::gtest_main)

add_test(NAME lemire COMMAND lemire_test)

add_executable(serialization_test serialization_test.cpp)
target_include_directories(serialization_test PRIVATE ../../include)
target_link_libraries(serialization_test PRIVATE dune GTest::gtest GTest::gtest_main)

target_precompile_headers(serialization_test REUSE_FROM dune)

add_test(NAME random_serialization COMMAND serialization_test)

add_executable(random_test random_test.cpp)
target_include_directories(random_test PRIVATE ../../include)
target_link_libraries(random_test PRIVATE dune GTest::gtest GTest::gtest_main)

target_precompile_headers(random_test REUSE_FROM dune)

add_test(NAME random COMMAND random_test)
