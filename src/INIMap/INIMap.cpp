#include "INIMap/INIMap.h"


INIMap::INIMap(GameType gameType, const std::filesystem::path& mapname, const std::string& mapdata)
    : mapname(mapname.u8string()) {
    if(gameType == GameType::Campaign || gameType == GameType::Skirmish) {
        // load from PAK-File
        inifile = std::make_unique<INIFile>(pFileManager->openFile(this->mapname).get());
    } else if(gameType == GameType::CustomGame || gameType == GameType::CustomMultiplayer) {
        auto RWops = sdl2::RWops_ptr{SDL_RWFromConstMem(mapdata.c_str(), mapdata.size())};

        inifile = std::make_unique<INIFile>(RWops.get());
    } else {
        inifile = std::make_unique<INIFile>(mapname);
    }
}

INIMap::~INIMap() = default;

void INIMap::checkFeatures() {
    if(!inifile->hasSection("FEATURES")) { return; }

    for(const auto& key : inifile->getSection("FEATURES")) {
        if(key.getBoolValue(true) == true) {
            logError(key.getLineNumber(), "Unsupported feature \"" + key.getKeyName() + "\"!");
            return;
        }
    }
}
