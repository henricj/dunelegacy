enable_testing()

find_package(GTest CONFIG REQUIRED)

add_compile_options(${DUNE_TARGET_COMPILE_FLAGS})

add_subdirectory(dune_misc)
add_subdirectory(random)
add_subdirectory(INIFileTestCase)
add_subdirectory(FileSystemTestCase)

