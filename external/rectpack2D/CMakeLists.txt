option(DUNE_FIND_rectpack2D "Use find_package() for rectpack2D" OFF)

if(DUNE_FIND_rectpack2D)
	find_package(rectpack2D REQUIRED)
	return()
endif()

get_filename_component(header_dir "include" ABSOLUTE BASE_DIR "${CMAKE_CURRENT_BINARY_DIR}")

add_library(rectpack2D INTERFACE)
target_include_directories(rectpack2D INTERFACE "${header_dir}")


list(APPEND headers
    best_bin_finder.h
    empty_space_allocators.h
    empty_spaces.h
    finders_interface.h
    insert_and_split.h
    rect_structs.h
)

foreach(header ${headers})
    get_filename_component(absolute_input_path "${header}" ABSOLUTE BASE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/rectpack2D/src")
    list(APPEND SOURCES ${absolute_input_path})
    get_filename_component(absolute_output_path "rectpack2D/${header}" ABSOLUTE BASE_DIR "${header_dir}")
    list(APPEND output_headers ${absolute_output_path})
endforeach()

add_custom_command(
    DEPENDS ${SOURCES}
    OUTPUT ${output_headers}
    COMMENT "Copying rectpack2D headers"
    COMMAND "${CMAKE_COMMAND}" -E make_directory "${header_dir}/rectpack2D"
    COMMAND "${CMAKE_COMMAND}" -E copy_if_different ${SOURCES} "${header_dir}/rectpack2D/"
)

add_custom_target(rectpack2D_copy DEPENDS ${output_headers})

add_dependencies(rectpack2D rectpack2D_copy)
