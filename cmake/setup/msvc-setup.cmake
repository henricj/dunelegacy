message(STATUS "Configuring MSVC")

add_compile_options(/diagnostics:caret /GA /GS /utf-8 /volatile:iso /permissive- /Zc:__cplusplus /Zc:inline /fp:fast)
add_compile_options(/wd4267)

# C4018 'token' : signed/unsigned mismatch
# C4100 'identifier' : unreferenced formal parameter
# C4389 'equality-operator' : signed/unsigned mismatch
# C4456 declaration of 'identifier' hides previous local declaration
# C4458 declaration of 'identifier' hides class member
set(DUNE_TARGET_COMPILE_FLAGS "/W4 /we4018 /we4100 /we4389 /we4456 /we4458" CACHE STRING "Dune compiler flags (not applied to external/*)")

set(DUNE_MSVC_DEBUG_FLAGS "/ZI /Ob0 /Od /RTC1 /RTCs /JMC" CACHE STRING "Debug compiler flags")
set(DUNE_MSVC_RELEASE_FLAGS "/EHsc /GF /Gy /Gw /Zi /O2 /Ob3 /Oi /DNDEBUG" CACHE STRING "Release compiler flags")

set(DUNE_TARGET_ARCHITECTURE ${CMAKE_SYSTEM_PROCESSOR} CACHE STRING "Target processor architecture")
set_property(CACHE DUNE_TARGET_ARCHITECTURE PROPERTY STRINGS x64 x86 arm64)

if(${DUNE_TARGET_ARCHITECTURE} MATCHES "^(x86|x64)$")
    set(DUNE_TARGET_ARCHITECTURE_EXTENSION "" CACHE STRING "Enable the avx, avx2, or avx512 exensions (avx2 implies avx, avx512 implies avx and avx2")
    set_property(CACHE DUNE_TARGET_ARCHITECTURE_EXTENSION PROPERTY STRINGS "" avx avx2 avx512)

    if("${DUNE_TARGET_ARCHITECTURE_EXTENSION}" STREQUAL "avx")
        set(HAVE_AVX ON)
        message(STATUS "Enabling AVX instructions")
        add_compile_options(/arch:AVX)
    elseif("${DUNE_TARGET_ARCHITECTURE_EXTENSION}" STREQUAL "avx2")
        set(HAVE_AVX2 ON)
        message(STATUS "Enabling AVX and AVX2 instructions")
        add_compile_options(/arch:AVX2)
    elseif("${DUNE_TARGET_ARCHITECTURE_EXTENSION}" STREQUAL "avx512")
        set(HAVE_AVX512 ON)
        message(STATUS "Enabling AVX, AVX2, and AVX512 instructions")
        add_compile_options(/arch:AVX512)
    elseif (NOT "${DUNE_TARGET_ARCHITECTURE_EXTENSION}" STREQUAL "")
        message (FATAL_ERROR "Unknown architecture extensions: ${DUNE_TARGET_ARCHITECTURE_EXTENSION}")
    else()
        message(STATUS "Using default instructions (SSE2)")
    endif()
endif()

if(${DUNE_TARGET_ARCHITECTURE} STREQUAL "x86")
    add_link_options(/LARGEADDRESSAWARE)
endif()

set(relase_flags ${DUNE_MSVC_RELEASE_FLAGS})
separate_arguments(release_flags)
set(debug_flags ${DUNE_MSVC_DEBUG_FLAGS})
separate_arguments(debug_flags)

foreach(config ${build_list})
    string(TOUPPER "${config}" config)

    if(config MATCHES "DEBUG")
        string(REGEX REPLACE "/Zi" "" CMAKE_CXX_FLAGS_${config} "${CMAKE_CXX_FLAGS_${config}}")
        string(REGEX REPLACE "/Zi" "" CMAKE_C_FLAGS_${config} "${CMAKE_C_FLAGS_${config}}")
        add_compile_options("$<$<CONFIG:${config}>:${debug_flags}>")
        continue()
    endif()

    add_compile_options("$<$<CONFIG:${config}>:${release_flags}>")
    add_link_options("$<$<CONFIG:${config}>:/LTCG /OPT:REF,ICF=3>")
endforeach()
