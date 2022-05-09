cmake_policy(SET CMP0115 NEW)

set(LIBBINIO_GENERATED_INCLUDE "${CMAKE_CURRENT_BINARY_DIR}/include/libbinio")
set(LIBBINIO_GENERATED_BINIO_H "${LIBBINIO_GENERATED_INCLUDE}/binio.h")
set(LIBBINIO_SOURCE_BINIO_H "${CMAKE_CURRENT_SOURCE_DIR}/binio.h")

file(MAKE_DIRECTORY "${LIBBINIO_GENERATED_INCLUDE}")

if (EXISTS "${LIBBINIO_SOURCE_BINIO_H}")
	file(REMOVE "${LIBBINIO_SOURCE_BINIO_H}")
endif()

set(TYPE_INT "long")
set(TYPE_FLOAT "double")
set(ENABLE_IOSTREAM 1)
set(ENABLE_STRING 1)
set(ISO_STDLIB 1)
set(WITH_MATH 1)

configure_file(libbinio/src/binio.h.in ${LIBBINIO_GENERATED_BINIO_H})

set(SOURCES
	libbinio/src/binfile.cpp
	libbinio/src/binio.cpp
	libbinio/src/binstr.cpp
	libbinio/src/binwrap.cpp
)

set(HEADERS
	libbinio/src/binfile.h
	"${LIBBINIO_GENERATED_BINIO_H}"
	libbinio/src/binstr.h
	libbinio/src/binwrap.h
)

add_library(libbinio ${SOURCES} ${HEADERS})

target_include_directories(libbinio
	INTERFACE libbinio/src
	PUBLIC "${LIBBINIO_GENERATED_INCLUDE}"
)

