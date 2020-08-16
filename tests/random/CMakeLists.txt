
add_executable(lemire_test lemire_test.cpp)
target_include_directories(lemire_test PRIVATE ../../include/utility)
target_link_libraries(lemire_test PRIVATE GTest::gtest GTest::gtest_main)

add_test(NAME lemire COMMAND lemire_test)

return()

add_executable(serialization_test serialization_test.cpp)
target_include_directories(serialization_test PRIVATE ../../include)
target_link_libraries(serialization_test PRIVATE dune GTest::gtest GTest::gtest_main)

target_precompile_headers(serialization_test REUSE_FROM dune)

add_test(NAME random_serialization COMMAND serialization_test)
