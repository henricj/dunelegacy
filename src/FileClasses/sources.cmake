add_sources(FILE_CLASSES_SOURCES
	Animation.cpp
	Cpsfile.cpp
	Decode.cpp
	DuneConfig.cpp
	FileManager.cpp
	Font.cpp
	FontManager.cpp
	GFXManager.cpp
	Icnfile.cpp
	IndexedTextFile.cpp
	INIFile.cpp
	LoadSavePNG.cpp
	MentatTextFile.cpp
	Pakfile.cpp
	Palette.cpp
	Palfile.cpp
	PictureFactory.cpp
	POFile.cpp
	SaveTextureAsBmp.cpp
	SFXManager.cpp
	Shpfile.cpp
	SurfaceLoader.cpp
	TextManager.cpp
	TTFFont.cpp
	Vocfile.cpp
	Wsafile.cpp
	adl/sound_adlib.cpp
	music/ADLPlayer.cpp
	music/DirectoryPlayer.cpp
	music/XMIPlayer.cpp
)

include(FileClasses/xmidi/sources.cmake)
