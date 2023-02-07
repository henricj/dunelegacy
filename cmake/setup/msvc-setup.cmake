message(STATUS "Configuring MSVC")

include(CheckCXXCompilerFlag)

add_compile_options(/diagnostics:caret /utf-8 /volatile:iso
    /permissive- /Zc:__cplusplus /Zc:inline)
add_compile_options(/fp:fast)
add_compile_options(/wd4267)

set(SETUP_MSVC_ORIGINAL_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /WX")

check_cxx_compiler_flag(/Zc:preprocessor HAS_ZC_PREPROCESSOR_FLAG)

set(CMAKE_CXX_FLAGS "${SETUP_MSVC_ORIGINAL_CXX_FLAGS}")

if(HAS_ZC_PREPROCESSOR_FLAG)
    add_compile_options(/Zc:preprocessor)
endif()

# Optimize for Windows Application (i.e., not a DLL)
# https://learn.microsoft.com/en-us/cpp/build/reference/ga-optimize-for-windows-application
add_compile_options(/GA)

# C4018 'token' : signed/unsigned mismatch
# C4100 'identifier' : unreferenced formal parameter
# C4127 conditional expression is constant
# C4389 'equality-operator' : signed/unsigned mismatch
# C4456 declaration of 'identifier' hides previous local declaration
# C4702 unreachable code
# C4458 declaration of 'identifier' hides class member
# C5222 'attribute-name': all unscoped attribute names are reserved for future standardization
set(DUNE_TARGET_COMPILE_FLAGS "/W4 /we4018 /we4100 /we4127 /we4389 /we4456 /we4458 /we4702 /we5222" CACHE STRING "Dune compiler flags (not applied to external/*)")

set(DUNE_MSVC_DEBUG_FLAGS_DEFAULT "/Ob0 /Od /RTC1 /RTCs /JMC")

if(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    string(PREPEND DUNE_MSVC_DEBUG_FLAGS_DEFAULT "/Zi ")
else()
    string(PREPEND DUNE_MSVC_DEBUG_FLAGS_DEFAULT "/ZI ")
endif()

set(DUNE_MSVC_DEBUG_FLAGS "${DUNE_MSVC_DEBUG_FLAGS_DEFAULT}" CACHE STRING "Debug compiler flags")
set(DUNE_MSVC_RELEASE_FLAGS "/Zi /EHsc /O2 /Ob3 /Gw" CACHE STRING "Release compiler flags")

set(DUNE_TARGET_ARCHITECTURE ${CMAKE_SYSTEM_PROCESSOR} CACHE STRING "Target processor architecture")
set_property(CACHE DUNE_TARGET_ARCHITECTURE PROPERTY STRINGS x64 x86 arm64)

if(DUNE_TARGET_ARCHITECTURE MATCHES "^(x86|x64)$")
    set(DUNE_TARGET_ARCHITECTURE_EXTENSION "" CACHE STRING "Enable the avx, avx2, or avx512 exensions (avx2 implies avx, avx512 implies avx and avx2")
    set_property(CACHE DUNE_TARGET_ARCHITECTURE_EXTENSION PROPERTY STRINGS "" avx avx2 avx512)

    if(DUNE_TARGET_ARCHITECTURE_EXTENSION STREQUAL "avx")
        set(HAVE_AVX ON)
        message(STATUS "Enabling AVX instructions")
        add_compile_options(/arch:AVX)
    elseif(DUNE_TARGET_ARCHITECTURE_EXTENSION STREQUAL "avx2")
        set(HAVE_AVX2 ON)
        message(STATUS "Enabling AVX and AVX2 instructions")
        add_compile_options(/arch:AVX2)
    elseif(DUNE_TARGET_ARCHITECTURE_EXTENSION STREQUAL "avx512")
        set(HAVE_AVX512 ON)
        message(STATUS "Enabling AVX, AVX2, and AVX512 instructions")
        add_compile_options(/arch:AVX512)
    elseif (NOT DUNE_TARGET_ARCHITECTURE_EXTENSION STREQUAL "")
        message (FATAL_ERROR "Unknown architecture extensions: ${DUNE_TARGET_ARCHITECTURE_EXTENSION}")
    else()
        message(STATUS "Using default instructions (SSE2)")
    endif()
endif()

if(DUNE_TARGET_ARCHITECTURE STREQUAL "x86")
    add_link_options(/LARGEADDRESSAWARE)
endif()

set(release_flags "${DUNE_MSVC_RELEASE_FLAGS}")
separate_arguments(release_flags)

set(debug_flags ${DUNE_MSVC_DEBUG_FLAGS})
separate_arguments(debug_flags)

add_compile_options("$<$<CONFIG:DEBUG>:${debug_flags}>")
add_compile_options("$<$<NOT:$<CONFIG:DEBUG>>:${release_flags}>")
add_link_options("$<$<NOT:$<CONFIG:DEBUG>>:/OPT:REF,ICF=3>")

if(DUNE_TARGET_ARCHITECTURE STREQUAL "arm64")
    add_link_options("$<$<NOT:$<CONFIG:DEBUG>>:/OPT:LBR>")
endif()

# Strip out options we want to set ourselves
function(strip_msvc_debug_compiler_options OPTIONS_VAR)
    string(REGEX REPLACE "/Zi" "" local_options_var "${${OPTIONS_VAR}}")
    set(${OPTIONS_VAR} "${local_options_var}" PARENT_SCOPE)
endfunction()

function(strip_msvc_release_compiler_options OPTIONS_VAR)
    string(REGEX REPLACE "/O2" "" local_options_var "${${OPTIONS_VAR}}")
    string(REGEX REPLACE "/Ob1" "" local_options_var "${local_options_var}")
    string(REGEX REPLACE "/Ob2" "" local_options_var "${local_options_var}")
    set(${OPTIONS_VAR} "${local_options_var}" PARENT_SCOPE)
endfunction()

foreach(config ${build_list})
    string(TOUPPER "${config}" upper_config)

    if(upper_config STREQUAL "DEBUG")
        strip_msvc_debug_compiler_options(CMAKE_CXX_FLAGS)
        strip_msvc_debug_compiler_options(CMAKE_C_FLAGS)

        strip_msvc_debug_compiler_options(CMAKE_CXX_FLAGS_${upper_config})
        strip_msvc_debug_compiler_options(CMAKE_C_FLAGS_${upper_config})
    else()
        strip_msvc_release_compiler_options(CMAKE_CXX_FLAGS)
        strip_msvc_release_compiler_options(CMAKE_C_FLAGS)

        strip_msvc_release_compiler_options(CMAKE_CXX_FLAGS_${upper_config})
        strip_msvc_release_compiler_options(CMAKE_C_FLAGS_${upper_config})

    endif()
endforeach()
