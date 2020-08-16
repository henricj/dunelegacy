#include "INIMap/INIMap.h"


INIMap::INIMap(GameType gameType, const std::filesystem::path& mapname, const std::string& mapdata)
    : mapname(mapname.u8string()) {

    sdl2::RWops_ptr file;

    if(gameType == GameType::Campaign || gameType == GameType::Skirmish) {
        // load from PAK-File

        file = pFileManager->openFile(this->mapname);

    } else if(gameType == GameType::CustomGame || gameType == GameType::CustomMultiplayer) {
        file = sdl2::RWops_ptr{SDL_RWFromConstMem(mapdata.c_str(), mapdata.size())};
    } else {
        file = sdl2::RWops_ptr{SDL_RWFromFile(this->mapname.c_str(), "r")};
    }

    if(!file) THROW(std::invalid_argument, "Unable to open file %s", this->mapname);

    std::array<char, 1024> buffer;
    size_t                 count   = 0u;

    std::function<int()> getchar = [file = file.get(), &buffer, &count] {
        if(!count) {
            count = SDL_RWread(file, buffer.data(), 1, buffer.size());
            if(!count) return EOF;
        }

        return static_cast<int>(buffer[count--]);
    };

    inifile = std::make_unique<Dune::Engine::INIFile>(getchar);
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
