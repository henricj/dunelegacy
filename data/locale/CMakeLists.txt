cmake_minimum_required(VERSION 3.16)

set(OUTPUT
	English.en.po
	French.fr.po
	German.de.po
	Spanish.es.po
)

install(FILES ${OUTPUT} DESTINATION locale)
