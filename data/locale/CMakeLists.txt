cmake_minimum_required(VERSION 3.16)

find_package(Gettext)

set(PO_SOURCES
	English.en.po
	French.fr.po
	German.de.po
	Spanish.es.po
)

if(FOUND_GETTEXT)
	GETTEXT_CREATE_TRANSLATIONS(dunelegacy.pot ${PO_SOURCES})
endif()

install(FILES ${PO_SOURCES} DESTINATION locale)
