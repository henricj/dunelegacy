enable_testing()

find_package(GTest CONFIG REQUIRED)

set(dune_flags ${DUNE_TARGET_COMPILE_FLAGS})
separate_arguments(dune_flags)
add_compile_options(${dune_flags})

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_subdirectory(dune_misc)
add_subdirectory(random)
add_subdirectory(INIFileTestCase)
add_subdirectory(FileSystemTestCase)

