message(STATUS "Configuring WIN32")

add_compile_definitions(WIN32_LEAN_AND_MEAN NOMINMAX)
add_compile_definitions(NTDDI_VERSION=NTDDI_VISTASP2 _WIN32_WINNT=_WIN32_WINNT_VISTA)

set(DUNE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:DebugDLL>" CACHE STRING "MSVC Runtime")
set_property(CACHE DUNE_MSVC_RUNTIME_LIBRARY PROPERTY STRINGS MultiThreaded MultiThreadedDLL MultiThreadedDebug MultiThreadedDLL)

set(CMAKE_MSVC_RUNTIME_LIBRARY ${DUNE_MSVC_RUNTIME_LIBRARY})

