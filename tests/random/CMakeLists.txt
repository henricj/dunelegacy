
add_executable(lemier_test lemier_test.cpp)
target_include_directories(lemier_test PRIVATE ../../include)
target_link_libraries(lemier_test PRIVATE GTest::gtest GTest::gtest_main)

add_test(NAME lemier COMMAND lemier_test)
