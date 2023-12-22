add_library(dunelegacyObjc STATIC)

target_sources(dunelegacyObjc
  PRIVATE
    MacFunctions.h
    MacFunctions.m
)

target_include_directories(dunelegacyObjc
  INTERFACE "${CMAKE_CURRENT_SOURCE_DIR}")

target_link_libraries(dunelegacyObjc
    PRIVATE "-framework Cocoa"
    PRIVATE "-framework Foundation"
    PRIVATE "-framework AppKit"
)

set_target_properties(dunelegacyObjc
  PROPERTIES
    LINKER_LANGUAGE "CXX"
    XCODE_ATTRIBUTE_CLANG_ENABLE_OBJC_ARC YES
    XCODE_ATTRIBUTE_CLANG_ENABLE_MODULES YES
    XCODE_ATTRIBUTE_CLANG_ENABLE_OBJC_WEAK YES
)
