#include <INIMap/INIMapPreviewCreator.h>

#include <FileClasses/FileManager.h>

#include <MapSeed.h>
#include <RadarView.h>

#include <fmt/printf.h>
#include <misc/draw_util.h>
#include <misc/exceptions.h>

#include <globals.h>
#include <mmath.h>
#include <sand.h>

#include <algorithm>

using namespace std::literals;

INIMapPreviewCreator::INIMapPreviewCreator(INIMap::inifile_ptr pINIFile) : INIMap(std::move(pINIFile)) { }

INIMapPreviewCreator::~INIMapPreviewCreator() = default;

/**
    This method is used to create a mini map of a map file before the map is being played (e.g. in the map selection
   menu). The surface is 128x128 pixels plus 2*borderWidth and the map is scaled appropriately. \param  borderWidth the
   width of the border \param  borderColor the color of the border \return the minimap of size
   (128+2*borderWidth)x(128+2*borderWidth)
*/
sdl2::surface_ptr INIMapPreviewCreator::createMinimapImageOfMap(int borderWidth, uint32_t borderColor) {
    checkFeatures();

    auto pMinimap = sdl2::surface_ptr{
        SDL_CreateRGBSurface(0, 128 + 2 * borderWidth, 128 + 2 * borderWidth, SCREEN_BPP, RMASK, GMASK, BMASK, AMASK)};
    if (pMinimap == nullptr) {
        return nullptr;
    }
    SDL_FillRect(pMinimap.get(), nullptr, borderColor);
    SDL_Rect dest = {borderWidth, borderWidth, pMinimap->w - 2 * borderWidth, pMinimap->h - 2 * borderWidth};
    SDL_FillRect(pMinimap.get(), &dest, COLOR_BLACK);

    int version = inifile_->getIntValue("BASIC", "Version", 1);

    int offsetX        = 0;
    int offsetY        = 0;
    int scale          = 1;
    int sizeX          = 64;
    int sizeY          = 64;
    int logicalSizeX   = 64;
    int logicalSizeY   = 64;
    int logicalOffsetX = 0;
    int logicalOffsetY = 0;

    if (version < 2) {
        // old map format with seed value
        int SeedNum = inifile_->getIntValue("MAP", "Seed", -1);

        if (SeedNum == -1) {
            logError("Cannot read Seed in this map!");
        }

        int mapscale = inifile_->getIntValue("BASIC", "MapScale", 0);

        switch (mapscale) {
            case 0: {
                scale          = 2;
                sizeX          = 62;
                sizeY          = 62;
                offsetX        = 2;
                offsetY        = 2;
                logicalOffsetX = 1;
                logicalOffsetY = 1;
            } break;

            case 1: {
                scale          = 4;
                sizeX          = 32;
                sizeY          = 32;
                logicalOffsetX = 16;
                logicalOffsetY = 16;
            } break;

            case 2: {
                scale          = 5;
                sizeX          = 21;
                sizeY          = 21;
                offsetX        = 11;
                offsetY        = 11;
                logicalOffsetX = 11;
                logicalOffsetY = 11;
            } break;

            default: {
                logError("Unknown MapScale!");
            } break;
        }

        logicalSizeX = 64;
        logicalSizeY = 64;

        offsetX += borderWidth;
        offsetY += borderWidth;

        uint16_t SeedMap[64 * 64];
        createMapWithSeed(SeedNum, SeedMap);

        // "draw" spice fields into SeedMap
        std::string FieldString = inifile_->getStringValue("MAP", "Field");
        if (!FieldString.empty()) {
            std::vector<std::string> FieldPositions = splitStringToStringVector(FieldString);

            for (auto& FieldPosition : FieldPositions) {
                // set bloom
                int FieldPos = 0;
                if (parseString(FieldPosition, FieldPos)) {
                    int xpos = FieldPos % logicalSizeX;
                    int ypos = FieldPos / logicalSizeX;

                    for (int x = -5; x <= 5; x++) {
                        for (int y = -5; y <= 5; y++) {

                            if (xpos + x >= 0 && xpos + x < logicalSizeX && ypos + y >= 0 && ypos + y < logicalSizeY) {
                                if (((SeedMap[64 * (ypos + y) + (xpos + x)] >> 4) == 0x7)
                                    && (distanceFrom(xpos, ypos, xpos + x, ypos + y) <= 5)) {

                                    SeedMap[64 * (ypos + y) + (xpos + x)] = (x == 0 && y == 0) ? 0xC0 : 0xB0;
                                }
                            }
                        }
                    }

                } else {
                    logError(inifile_->getLineNumber("MAP", "Field"), "Invalid value for key Field: " + FieldString);
                }
            }
        }

        for (int y = 0; y < sizeY; y++) {
            for (int x = 0; x < sizeX; x++) {
                Uint32 color = COLOR_BLACK;
                const auto seedmaptype =
                    static_cast<uint8_t>(SeedMap[(y + logicalOffsetY) * 64 + x + logicalOffsetX] >> 4u);
                switch (seedmaptype) {

                    case 0x7: {
                        // Normal sand
                        color = COLOR_DESERTSAND;
                    } break;

                    case 0x2:
                    case 0x8: {
                        // Rock or building
                        color = COLOR_ROCK;
                    } break;

                    case 0x9: {
                        // Sand dunes
                        color = COLOR_DESERTSAND;
                    } break;

                    case 0xa: {
                        // Mountain
                        color = COLOR_MOUNTAIN;
                    } break;

                    case 0xb: {
                        // Spice
                        color = COLOR_SPICE;
                    } break;

                    case 0xc: {
                        // Thick spice
                        color = COLOR_THICKSPICE;
                    } break;
                }

                for (int i = 0; i < scale; i++) {
                    for (int j = 0; j < scale; j++) {
                        putPixel(pMinimap.get(), x * scale + i + offsetX, y * scale + j + offsetY, color);
                    }
                }
            }
        }

        // draw spice blooms
        std::string BloomString = inifile_->getStringValue("MAP", "Bloom");
        if (!BloomString.empty()) {
            std::vector<std::string> BloomPositions = splitStringToStringVector(BloomString);

            for (const std::string& strBloomPos : BloomPositions) {
                // set bloom
                int BloomPos = 0;
                if (parseString(strBloomPos, BloomPos)) {
                    int xpos = BloomPos % logicalSizeX - logicalOffsetX;
                    int ypos = BloomPos / logicalSizeX - logicalOffsetY;
                    if (xpos >= 0 && xpos < sizeX && ypos >= 0 && ypos < sizeY) {
                        for (int i = 0; i < scale; i++) {
                            for (int j = 0; j < scale; j++) {
                                putPixel(pMinimap.get(), xpos * scale + i + offsetX, ypos * scale + j + offsetY,
                                         COLOR_BLOOM);
                            }
                        }
                    }
                } else {
                    logError(inifile_->getLineNumber("MAP", "Bloom"), "Invalid value for key Bloom: " + BloomString);
                }
            }
        }

        // draw special blooms
        const auto SpecialString = inifile_->getStringValue("MAP", "Special");
        if (!SpecialString.empty()) {
            const auto SpecialPositions = splitStringToStringVector(SpecialString);

            for (const auto& strSpecialPos : SpecialPositions) {
                // set bloom
                auto SpecialPos = 0;
                if (parseString(strSpecialPos, SpecialPos)) {
                    int xpos = SpecialPos % logicalSizeX - logicalOffsetX;
                    int ypos = SpecialPos / logicalSizeX - logicalOffsetY;
                    if (xpos >= 0 && xpos < sizeX && ypos >= 0 && ypos < sizeY) {
                        for (int i = 0; i < scale; i++) {
                            for (int j = 0; j < scale; j++) {
                                putPixel(pMinimap.get(), xpos * scale + i + offsetX, ypos * scale + j + offsetY,
                                         COLOR_BLOOM);
                            }
                        }
                    }
                } else {
                    logError(inifile_->getLineNumber("MAP", "Special"),
                             "Invalid value for key Special: " + SpecialString);
                }
            }
        }

    } else {
        // new map format with saved map

        if ((!inifile_->hasKey("MAP", "SizeX")) || (!inifile_->hasKey("MAP", "SizeY"))) {
            logError("SizeX and SizeY must be specified!");
        }

        sizeX = inifile_->getIntValue("MAP", "SizeX", 0);
        sizeY = inifile_->getIntValue("MAP", "SizeY", 0);

        logicalSizeX = sizeX;
        logicalSizeY = sizeY;

        RadarView::calculateScaleAndOffsets(sizeX, sizeY, scale, offsetX, offsetY);

        offsetX += borderWidth;
        offsetY += borderWidth;

        for (int y = 0; y < sizeY; y++) {
            const auto rowKey = fmt::sprintf("%.3d", y);

            if (!inifile_->hasKey("MAP", rowKey)) {
                logError(inifile_->getLineNumber("MAP"), "Map row " + std::to_string(y) + " does not exist!");
            }

            const auto rowString = inifile_->getStringValue("MAP", rowKey);
            for (int x = 0; x < sizeX; x++) {
                Uint32 color = COLOR_BLACK;
                switch (rowString.at(x)) {
                    case '-': {
                        // Normal sand
                        color = COLOR_DESERTSAND;
                    } break;

                    case '^': {
                        // Sand dunes
                        color = COLOR_DESERTSAND;
                    } break;

                    case '~': {
                        // Spice
                        color = COLOR_SPICE;
                    } break;

                    case '+': {
                        // Thick spice
                        color = COLOR_THICKSPICE;
                    } break;

                    case '%': {
                        // Rock
                        color = COLOR_ROCK;
                    } break;

                    case '@': {
                        // Mountain
                        color = COLOR_MOUNTAIN;
                    } break;

                    case 'O':
                    case 'Q': {
                        // Spice Bloom and Special Bloom
                        color = COLOR_BLOOM;
                    } break;

                    default: {
                        logError(inifile_->getLineNumber("MAP", rowKey),
                                 std::string("Unknown map tile type '") + rowString.at(x) + "' in map tile ("
                                     + std::to_string(x) + ", " + std::to_string(y) + ")!");
                    } break;
                }

                for (int i = 0; i < scale; i++) {
                    for (int j = 0; j < scale; j++) {
                        putPixel(pMinimap.get(), x * scale + i + offsetX, y * scale + j + offsetY, color);
                    }
                }
            }
        }
    }

    static constexpr auto sectionname = "STRUCTURES"sv;

    // draw structures
    if (inifile_->hasSection(sectionname)) {
        for (const INIFile::Key& key : inifile_->keys(sectionname)) {
            const auto tmpkey = key.getKeyName();
            const auto tmp    = key.getStringView();

            if (tmpkey.compare(0, 3, "GEN") == 0) {
                // Gen Object/Structure

                const auto PosStr = tmpkey.substr(3, tmpkey.size() - 3);
                int pos           = 0;
                if (!parseString(PosStr, pos)) {
                    continue;
                }

                std::string HouseStr, BuildingStr;
                splitString(tmp, HouseStr, BuildingStr);

                const auto house = getHouseByName(HouseStr);
                Uint32 color     = COLOR_WHITE;
                if (house != HOUSETYPE::HOUSE_INVALID) {
                    const auto house_id = static_cast<int>(house);
                    color               = SDL2RGB(dune::globals::palette[dune::globals::houseToPaletteIndex[house_id]]);
                } else {
                    convertToLower(HouseStr);
                    if (HouseStr.length() == 7 && HouseStr.substr(0, 6) == "player") {
                        const auto playernum = HouseStr.at(6) - '0';

                        if (playernum >= 1 && playernum <= 6) {
                            int val = 32 * (playernum - 1) + 32;
                            color   = COLOR_RGB(val, val, val);
                        }
                    } else {
                        logError(inifile_->getLineNumber(sectionname, key.getKeyName()),
                                 "Invalid house string: '" + HouseStr + "'!");
                    }
                }

                if (BuildingStr == "Concrete") {
                    // nothing
                } else if (BuildingStr == "Wall") {
                    int x = pos % logicalSizeX - logicalOffsetX;
                    int y = pos / logicalSizeX - logicalOffsetY;

                    if (x >= 0 && x < sizeX && y >= 0 && y < sizeY) {
                        for (int i = 0; i < scale; i++) {
                            for (int j = 0; j < scale; j++) {
                                putPixel(pMinimap.get(), x * scale + i + offsetX, y * scale + j + offsetY, color);
                            }
                        }
                    }
                }
            } else if (key.getKeyName().find("ID") == 0) {
                // other structure
                std::string HouseStr, BuildingStr, health, PosStr;
                splitString(tmp, HouseStr, BuildingStr, health, PosStr);

                int pos = 0;
                if (!parseString(PosStr, pos)) {
                    continue;
                }

                const auto house = getHouseByName(HouseStr);
                auto color       = COLOR_WHITE;
                if (house != HOUSETYPE::HOUSE_INVALID) {
                    const auto house_id = static_cast<int>(house);
                    color               = SDL2RGB(dune::globals::palette[dune::globals::houseToPaletteIndex[house_id]]);
                } else {
                    convertToLower(HouseStr);
                    if (HouseStr.length() == 7 && HouseStr.substr(0, 6) == "player") {
                        int playernum = HouseStr.at(6) - '0';

                        if (playernum >= 1 && playernum <= 6) {
                            int val = 32 * (playernum - 1) + 32;
                            color   = COLOR_RGB(val, val, val);
                        }
                    } else {
                        logError(inifile_->getLineNumber(sectionname, key.getKeyName()),
                                 "Invalid house string: '" + HouseStr + "'!");
                    }
                }

                Coord size = getStructureSize(getItemIDByName(BuildingStr));

                int posX = pos % logicalSizeX - logicalOffsetX;
                int posY = pos / logicalSizeX - logicalOffsetY;
                for (int x = posX; x < posX + size.x; x++) {
                    for (int y = posY; y < posY + size.y; y++) {
                        if (x >= 0 && x < sizeX && y >= 0 && y < sizeY) {
                            for (int i = 0; i < scale; i++) {
                                for (int j = 0; j < scale; j++) {
                                    putPixel(pMinimap.get(), x * scale + i + offsetX, y * scale + j + offsetY, color);
                                }
                            }
                        }
                    }
                }
            }
        }
    }

    return pMinimap;
}
