enable_testing()

find_package(GTest CONFIG REQUIRED)

add_subdirectory(random)
add_subdirectory(INIFileTestCase)
add_subdirectory(FileSystemTestCase)

