message(STATUS "Enabling ASAN")

if(DUNE_ASAN_FLAGS)
    add_compile_definitions(${DUNE_ASAN_FLAGS})
endif()

if(MSVC)
    add_compile_options(/fsanitize=address)
else()
    set(asan_flags -fsanitize=address -fno-omit-frame-pointer)
    add_compile_options(${asan_flags})
    add_link_options(${asan_flags} -lpthread -ldl)
endif()
