cmake_minimum_required(VERSION 3.16)

include(sources.cmake)

install(FILES ${ENGLISH_PO} ${PO_SOURCES} DESTINATION locale)
