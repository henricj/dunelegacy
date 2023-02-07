#include "INIMap/INIMap.h"

#include <gsl/gsl>

INIMap::INIMap(GameType gameType, const std::filesystem::path& mapname, const std::string& mapdata)
    : mapname_(reinterpret_cast<const char*>(mapname.u8string().c_str())) {
    if (gameType == GameType::Campaign || gameType == GameType::Skirmish) {
        // load from PAK-File
        inifile_ = std::make_unique<INIFile>(dune::globals::pFileManager->openFile(this->mapname_).get());
    } else if (!mapdata.empty() && (gameType == GameType::CustomGame || gameType == GameType::CustomMultiplayer)) {
        const auto RWops = sdl2::RWops_ptr{SDL_RWFromConstMem(mapdata.c_str(), gsl::narrow<int>(mapdata.size()))};

        inifile_ = std::make_unique<INIFile>(RWops.get());
    } else {
        inifile_ = std::make_unique<INIFile>(mapname);
    }
}

INIMap::~INIMap() = default;

void INIMap::checkFeatures() const {
    if (!inifile_->hasSection("FEATURES")) {
        return;
    }

    for (const auto& key : inifile_->keys("FEATURES")) {
        if (key.getBoolValue(true)) {
            logError(inifile_->getLineNumber("FEATURES", key.getKeyName()), "Unsupported feature \"{}\"!",
                     key.getKeyName());
            return;
        }
    }
}
