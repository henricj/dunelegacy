#include <INIMap/INIMapPreviewCreator.h>

#include <FileClasses/FileManager.h>

#include <MapSeed.h>
#include <RadarView.h>

#include <misc/draw_util.h>
#include <misc/format.h>
#include <misc/exceptions.h>

#include <mmath.h>
#include <sand.h>
#include <globals.h>

#include <algorithm>

INIMapPreviewCreator::INIMapPreviewCreator(INIMap::inifile_ptr pINIFile)
 : INIMap(std::move(pINIFile))
{

}

INIMapPreviewCreator::~INIMapPreviewCreator() = default;

/**
    This method is used to create a mini map of a map file before the map is being played (e.g. in the map selection menu).
    The surface is 128x128 pixels plus 2*borderWidth and the map is scaled appropriately.
    \param  borderWidth the width of the border
    \param  borderColor the color of the border
    \return the minimap of size (128+2*borderWidth)x(128+2*borderWidth)
*/
sdl2::surface_ptr INIMapPreviewCreator::createMinimapImageOfMap(int borderWidth, Uint32 borderColor) {
    checkFeatures();

    auto pMinimap = sdl2::surface_ptr{ SDL_CreateRGBSurface(0, 128+2*borderWidth, 128+2*borderWidth, SCREEN_BPP, RMASK, GMASK, BMASK, AMASK) };
    if(pMinimap == nullptr) {
        return nullptr;
    }
    SDL_FillRect(pMinimap.get(), nullptr, borderColor);
    SDL_Rect dest = { borderWidth, borderWidth, pMinimap->w - 2*borderWidth, pMinimap->h - 2*borderWidth};
    SDL_FillRect(pMinimap.get(), &dest, COLOR_BLACK);

    int version = inifile->getIntValue("BASIC", "Version", 1);

    int offsetX = 0;
    int offsetY = 0;
    int scale = 1;
    int sizeX = 64;
    int sizeY = 64;
    int logicalSizeX = 64;
    int logicalSizeY = 64;
    int logicalOffsetX = 0;
    int logicalOffsetY = 0;

    if(version < 2) {
        // old map format with seed value
        int SeedNum = inifile->getIntValue("MAP","Seed",-1);

        if(SeedNum == -1) {
            logError("Cannot read Seed in this map!");
        }

        int mapscale = inifile->getIntValue("BASIC","MapScale",0);

        switch(mapscale) {
            case 0: {
                scale = 2;
                sizeX = 62;
                sizeY = 62;
                offsetX = 2;
                offsetY = 2;
                logicalOffsetX = 1;
                logicalOffsetY = 1;
            } break;

            case 1: {
                scale = 4;
                sizeX = 32;
                sizeY = 32;
                logicalOffsetX = 16;
                logicalOffsetY = 16;
            } break;

            case 2: {
                scale = 5;
                sizeX = 21;
                sizeY = 21;
                offsetX = 11;
                offsetY = 11;
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

        Uint16 SeedMap[64*64];
        createMapWithSeed(SeedNum,SeedMap);

        // "draw" spice fields into SeedMap
        std::string FieldString = inifile->getStringValue("MAP","Field");
        if(FieldString != "") {
            std::vector<std::string> FieldPositions  = splitStringToStringVector(FieldString);

            for(unsigned int i=0; i < FieldPositions.size();i++) {
                // set bloom
                int FieldPos;
                if(parseString(FieldPositions[i], FieldPos)) {
                    int xpos = FieldPos % logicalSizeX;
                    int ypos = FieldPos / logicalSizeX;

                    for(int x = -5; x <= 5; x++) {
                        for(int y = -5; y <= 5; y++) {

                            if(xpos+x >= 0 && xpos+x < logicalSizeX && ypos+y >= 0 && ypos+y < logicalSizeY) {
                                if(((SeedMap[64*(ypos+y) + (xpos+x)] >> 4) == 0x7)
                                    && (distanceFrom(xpos, ypos, xpos + x, ypos + y) <= 5)) {

                                    SeedMap[64*(ypos+y) + (xpos+x)] = (x==0 && y==0) ? 0xC0 : 0xB0;
                                }
                            }
                        }
                    }

                } else {
                    logError(inifile->getKey("MAP", "Field")->getLineNumber(), "Invalid value for key Field: " + FieldString);
                }
            }
        }

        for(int y = 0; y < sizeY; y++) {
            for(int x = 0; x < sizeX; x++) {
                Uint32 color = COLOR_BLACK;
                unsigned char seedmaptype = SeedMap[(y+logicalOffsetY)*64+x+logicalOffsetX] >> 4;
                switch(seedmaptype) {

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

                for(int i=0;i<scale;i++) {
                    for(int j=0;j<scale;j++) {
                        putPixel(pMinimap.get(), x*scale + i + offsetX, y*scale + j + offsetY, color);
                    }
                }
            }
        }

        // draw spice blooms
        std::string BloomString = inifile->getStringValue("MAP","Bloom");
        if(BloomString != "") {
            std::vector<std::string> BloomPositions  = splitStringToStringVector(BloomString);

            for(const std::string& strBloomPos : BloomPositions) {
                // set bloom
                int BloomPos;
                if(parseString(strBloomPos, BloomPos)) {
                    int xpos = BloomPos % logicalSizeX - logicalOffsetX;
                    int ypos = BloomPos / logicalSizeX - logicalOffsetY;
                    if(xpos >= 0 && xpos < sizeX && ypos >= 0 && ypos < sizeY) {
                        for(int i=0;i<scale;i++) {
                            for(int j=0;j<scale;j++) {
                                putPixel(pMinimap.get(), xpos*scale + i + offsetX, ypos*scale + j + offsetY, COLOR_BLOOM);
                            }
                        }
                    }
                } else {
                    logError(inifile->getKey("MAP", "Bloom")->getLineNumber(), "Invalid value for key Bloom: " + BloomString);
                }
            }
        }

        // draw special blooms
        std::string SpecialString = inifile->getStringValue("MAP","Special");
        if(SpecialString != "") {
            std::vector<std::string> SpecialPositions  = splitStringToStringVector(SpecialString);

            for(const std::string& strSpecialPos : SpecialPositions) {
                // set bloom
                int SpecialPos;
                if(parseString(strSpecialPos, SpecialPos)) {
                    int xpos = SpecialPos % logicalSizeX - logicalOffsetX;
                    int ypos = SpecialPos / logicalSizeX - logicalOffsetY;
                    if(xpos >= 0 && xpos < sizeX && ypos >= 0 && ypos < sizeY) {
                        for(int i=0;i<scale;i++) {
                            for(int j=0;j<scale;j++) {
                                putPixel(pMinimap.get(), xpos*scale + i + offsetX, ypos*scale + j + offsetY, COLOR_BLOOM);
                            }
                        }
                    }
                } else {
                    logError(inifile->getKey("MAP", "Special")->getLineNumber(), "Invalid value for key Special: " + SpecialString);
                }
            }
        }


    } else {
        // new map format with saved map

        if((inifile->hasKey("MAP","SizeX") == false) || (inifile->hasKey("MAP","SizeY") == false)) {
            logError("SizeX and SizeY must be specified!");
        }

        sizeX = inifile->getIntValue("MAP","SizeX", 0);
        sizeY = inifile->getIntValue("MAP","SizeY", 0);

        logicalSizeX = sizeX;
        logicalSizeY = sizeY;

        RadarView::calculateScaleAndOffsets(sizeX, sizeY, scale, offsetX, offsetY);

        offsetX += borderWidth;
        offsetY += borderWidth;

        for(int y=0;y<sizeY;y++) {
            std::string rowKey = fmt::sprintf("%.3d", y);

            if(inifile->hasKey("MAP", rowKey) == false) {
                logError(inifile->getSection("MAP").getLineNumber(), "Map row " + std::to_string(y) + " does not exist!");
            }

            std::string rowString = inifile->getStringValue("MAP",rowKey);
            for(int x=0;x<sizeX;x++) {
                Uint32 color = COLOR_BLACK;
                switch(rowString.at(x)) {
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
                        logError(inifile->getKey("MAP", rowKey)->getLineNumber(), std::string("Unknown map tile type '") + rowString.at(x) + "' in map tile (" + std::to_string(x) + ", " + std::to_string(y) + ")!");
                    } break;
                }

                for(int i=0;i<scale;i++) {
                    for(int j=0;j<scale;j++) {
                        putPixel(pMinimap.get(), x*scale + i + offsetX, y*scale + j + offsetY, color);
                    }
                }
            }
        }
    }

    // draw structures
    if(inifile->hasSection("STRUCTURES")) {
        for(const INIFile::Key& key : inifile->getSection("STRUCTURES")) {
            std::string tmpkey = key.getKeyName();
            std::string tmp = key.getStringValue();

            if(tmpkey.compare(0,3,"GEN") == 0) {
                // Gen Object/Structure

                std::string PosStr = tmpkey.substr(3,tmpkey.size()-3);
                int pos;
                if(!parseString(PosStr, pos)) {
                    continue;
                }

                std::string HouseStr, BuildingStr;
                splitString(tmp, HouseStr, BuildingStr);

                int house = getHouseByName(HouseStr);
                Uint32 color = COLOR_WHITE;
                if(house != HOUSE_INVALID) {
                    color = SDL2RGB(palette[houseToPaletteIndex[house]]);
                } else {
                    convertToLower(HouseStr);
                    if(HouseStr.length() == 7 && HouseStr.substr(0,6) == "player") {
                        int playernum = HouseStr.at(6)-'0';

                        if(playernum >= 1 && playernum <= 6) {
                            int val = 32*(playernum - 1) + 32;
                            color = COLOR_RGB(val, val, val);
                        }
                    } else {
                        logError(key.getLineNumber(), "Invalid house string: '" + HouseStr + "'!");
                    }
                }

                if(BuildingStr == "Concrete") {
                    // nothing
                } else if(BuildingStr == "Wall") {
                    int x = pos % logicalSizeX - logicalOffsetX;
                    int y = pos / logicalSizeX - logicalOffsetY;

                    if(x >= 0 && x < sizeX && y >= 0 && y < sizeY) {
                        for(int i=0;i<scale;i++) {
                            for(int j=0;j<scale;j++) {
                                putPixel(pMinimap.get(), x*scale + i + offsetX, y*scale + j + offsetY, color);
                            }
                        }
                    }
                }
            } else if(key.getKeyName().find("ID") == 0) {
                // other structure
                std::string HouseStr, BuildingStr, health, PosStr;
                splitString(tmp, HouseStr, BuildingStr, health, PosStr);

                int pos;
                if(!parseString(PosStr, pos)) {
                    continue;
                }

                int house = getHouseByName(HouseStr);
                Uint32 color = COLOR_WHITE;
                if(house != HOUSE_INVALID) {
                    color = SDL2RGB(palette[houseToPaletteIndex[house]]);
                } else {
                    convertToLower(HouseStr);
                    if(HouseStr.length() == 7 && HouseStr.substr(0,6) == "player") {
                        int playernum = HouseStr.at(6)-'0';

                        if(playernum >= 1 && playernum <= 6) {
                            int val = 32*(playernum - 1) + 32;
                            color = COLOR_RGB(val, val, val);
                        }
                    } else {
                        logError(key.getLineNumber(), "Invalid house string: '" + HouseStr + "'!");
                    }
                }

                Coord size = getStructureSize(getItemIDByName(BuildingStr));

                int posX = pos % logicalSizeX - logicalOffsetX;
                int posY = pos / logicalSizeX - logicalOffsetY;
                for(int x = posX; x < posX + size.x; x++) {
                    for(int y = posY; y < posY + size.y; y++) {
                        if(x >= 0 && x < sizeX && y >= 0 && y < sizeY) {
                            for(int i=0;i<scale;i++) {
                                for(int j=0;j<scale;j++) {
                                    putPixel(pMinimap.get(), x*scale + i + offsetX, y*scale + j + offsetY, color);
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
