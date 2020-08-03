
add_executable(lemire_test lemire_test.cpp)
target_include_directories(lemire_test PRIVATE ../../include)
target_link_libraries(lemire_test PRIVATE GTest::gtest GTest::gtest_main)

add_test(NAME lemire COMMAND lemire_test)
