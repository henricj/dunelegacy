vcpkg_from_github(
    OUT_SOURCE_PATH SOURCE_PATH
    REPO libsdl-org/SDL
    REF "release-${VERSION}"
    SHA512 df5a323af7ac366661a3c0e887969c72584d232f3cc211419d59b0487b620b6b2859d4549c9e8df002ee489290062e466fcfddf7edc0872a37b1f2845e81c0f3
    HEAD_REF main
    PATCHES
        fix-freebsd.patch
)

string(COMPARE EQUAL "${VCPKG_LIBRARY_LINKAGE}" "static" SDL_STATIC)
string(COMPARE EQUAL "${VCPKG_LIBRARY_LINKAGE}" "dynamic" SDL_SHARED)
string(COMPARE EQUAL "${VCPKG_CRT_LINKAGE}" "static" FORCE_STATIC_VCRT)

vcpkg_check_features(OUT_FEATURE_OPTIONS FEATURE_OPTIONS
    FEATURES
        alsa     SDL_ALSA
        dbus     SDL_DBUS
        ibus     SDL_IBUS
        vulkan   SDL_VULKAN
        wayland  SDL_WAYLAND
        x11      SDL_X11
        libusb   SDL_HIDAPI_LIBUSB
)

if (VCPKG_TARGET_IS_EMSCRIPTEN)
    vcpkg_check_features(OUT_FEATURE_OPTIONS EMSCRIPTEN_FEATURE_OPTIONS
        FEATURES
            emscripten-pthreads     SDL_PTHREADS
    )
    vcpkg_list(APPEND FEATURE_OPTIONS "${EMSCRIPTEN_FEATURE_OPTIONS}")
endif()

if ("x11" IN_LIST FEATURES)
    message(WARNING "You will need to install X11 desktop dependencies to use feature x11:\nsudo apt install libx11-dev libxext-dev libxrandr-dev libxcursor-dev libxfixes-dev libxi-dev libxss-dev libxtst-dev libgl1-mesa-dev\n")
endif()
if ("wayland" IN_LIST FEATURES)
    message(WARNING "You will need to install Wayland dependencies to use feature wayland:\nsudo apt install libwayland-dev libxkbcommon-dev libegl1-mesa-dev\n")
endif()
if ("ibus" IN_LIST FEATURES)
    message(WARNING "You will need to install ibus dependencies to use feature ibus:\nsudo apt install libibus-1.0-dev\n")
    list(APPEND FEATURE_OPTIONS -DSDL_DBUS=ON)
endif()

if ("libusb" IN_LIST FEATURES)
    if(VCPKG_LIBRARY_LINKAGE STREQUAL "dynamic")
        vcpkg_list(APPEND FEATURE_OPTIONS "-DSDL_HIDAPI_LIBUSB_SHARED=ON")
    else()
        vcpkg_list(APPEND FEATURE_OPTIONS "-DSDL_HIDAPI_LIBUSB_SHARED=OFF")
    endif()
endif()

set(SDL_EXPECTS_WINDOWING OFF)
if (VCPKG_TARGET_IS_LINUX AND ("x11" IN_LIST FEATURES OR "wayland" IN_LIST FEATURES))
    set(SDL_EXPECTS_WINDOWING ON)
    list(APPEND FEATURE_OPTIONS -DSDL_UNIX_CONSOLE_BUILD=OFF)
else()
    # Allow intentionally headless builds, such as analysis jobs using the config-sdl3 manifest.
    list(APPEND FEATURE_OPTIONS -DSDL_UNIX_CONSOLE_BUILD=ON)
endif()

if (VCPKG_TARGET_IS_LINUX AND NOT SDL_EXPECTS_WINDOWING)
    message(WARNING "The selected features don't allow sdl3 to create windows, which is usually unintentional. You can get windowing support by installing the x11 and/or wayland features.")
endif()

vcpkg_cmake_configure(
    SOURCE_PATH "${SOURCE_PATH}"
    OPTIONS
        ${FEATURE_OPTIONS}
        -DSDL_STATIC=${SDL_STATIC}
        -DSDL_SHARED=${SDL_SHARED}
        -DSDL_DYNAPI=OFF
        -DSDL_FORCE_STATIC_VCRT=${FORCE_STATIC_VCRT}
        -DSDL_LIBC=ON
        -DSDL_TEST_LIBRARY=OFF
        -DSDL_TESTS=OFF
        -DSDL_X11_XSCRNSAVER=OFF
        -DSDL_INSTALL_CMAKEDIR_ROOT=share/${PORT}
        # Specifying the revision skips the need to use git to determine a version
        -DSDL_REVISION=vcpkg
    MAYBE_UNUSED_VARIABLES
        SDL_FORCE_STATIC_VCRT
)

if (SDL_EXPECTS_WINDOWING)
    if (VCPKG_BUILD_TYPE STREQUAL "release")
        set(SDL_CACHE_SUFFIX "-rel")
    else()
        set(SDL_CACHE_SUFFIX "-dbg")
    endif()
    set(SDL_CACHE_FILE "${CURRENT_BUILDTREES_DIR}/${TARGET_TRIPLET}${SDL_CACHE_SUFFIX}/CMakeCache.txt")

    if (NOT EXISTS "${SDL_CACHE_FILE}")
        message(FATAL_ERROR "Expected SDL build cache at ${SDL_CACHE_FILE}, but it was not generated.")
    endif()

    file(STRINGS "${SDL_CACHE_FILE}" SDL_CACHE_LINES
        REGEX "^SDL_(X11|WAYLAND|IBUS):BOOL=")

    if ("x11" IN_LIST FEATURES)
        list(FILTER SDL_CACHE_LINES INCLUDE REGEX "^SDL_X11:BOOL=ON$")
        if (NOT SDL_CACHE_LINES)
            message(FATAL_ERROR "sdl3 was built with feature x11 requested, but SDL_X11 is OFF. Install the required X11 development packages or disable the x11 feature.")
        endif()
    endif()

    if ("wayland" IN_LIST FEATURES)
        file(STRINGS "${SDL_CACHE_FILE}" SDL_WAYLAND_CACHE_LINES REGEX "^SDL_WAYLAND:BOOL=ON$")
        if (NOT SDL_WAYLAND_CACHE_LINES)
            message(FATAL_ERROR "sdl3 was built with feature wayland requested, but SDL_WAYLAND is OFF. Install the required Wayland development packages or disable the wayland feature.")
        endif()
    endif()

    if ("ibus" IN_LIST FEATURES)
        file(STRINGS "${SDL_CACHE_FILE}" SDL_IBUS_CACHE_LINES REGEX "^SDL_IBUS:BOOL=ON$")
        if (NOT SDL_IBUS_CACHE_LINES)
            message(FATAL_ERROR "sdl3 was built with feature ibus requested, but SDL_IBUS is OFF. Install the required ibus development packages or disable the ibus feature.")
        endif()

        file(STRINGS "${SDL_CACHE_FILE}" SDL_DBUS_CACHE_LINES REGEX "^SDL_DBUS:BOOL=ON$")
        if (NOT SDL_DBUS_CACHE_LINES)
            message(FATAL_ERROR "sdl3 was built with feature ibus requested, but SDL_DBUS is OFF. SDL's ibus backend requires D-Bus support as well.")
        endif()
    endif()
endif()

vcpkg_cmake_install()
vcpkg_cmake_config_fixup()

file(REMOVE_RECURSE
    "${CURRENT_PACKAGES_DIR}/debug/include"
    "${CURRENT_PACKAGES_DIR}/debug/share"
)

vcpkg_copy_pdbs()
vcpkg_fixup_pkgconfig()

file(INSTALL "${CMAKE_CURRENT_LIST_DIR}/usage" DESTINATION "${CURRENT_PACKAGES_DIR}/share/${PORT}")
vcpkg_install_copyright(FILE_LIST "${SOURCE_PATH}/LICENSE.txt"
    COMMENT "Some configurations may use code licensed under the MIT and Apache-2.0 licenses."
)
