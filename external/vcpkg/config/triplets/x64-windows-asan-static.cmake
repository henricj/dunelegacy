set(VCPKG_TARGET_ARCHITECTURE x64)
set(VCPKG_CRT_LINKAGE static)
set(VCPKG_LIBRARY_LINKAGE static)
set(VCPKG_C_FLAGS "${VCPKG_C_FLAGS} /fsanitize=address")
set(VCPKG_CXX_FLAGS "${VCPKG_CXX_FLAGS} /fsanitize=address")
set(VCPKG_LINKER_FLAGS "${VCPKG_LINKER_FLAGS} /fsanitize=address")
