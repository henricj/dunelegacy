#include <INIMap/INIMapPreviewCreator.h>

#include <FileClasses/FileManager.h>

#include <MapSeed.h>
#include <RadarView.h>

#include <misc/draw_util.h>
#include <misc/string_util.h>

#include <mmath.h>
#include <sand.h>
#include <globals.h>

#include <stdio.h>
#include <stdexcept>
#include <algorithm>

INIMapPreviewCreator::INIMapPreviewCreator(std::shared_ptr<INIFile>& pINIFile)
 : INIMap(pINIFile)
{

}

INIMapPreviewCreator::~INIMapPreviewCreator() {
}

/**
    This method is used to create a mini map of a map file before the map is being played (e.g. in the map selection menu).
    The surface is always 128x128 pixels and the map is scaled appropriately.
    \return the minimap (128x128)
*/
SDL_Surface* INIMapPreviewCreator::createMinimapImageOfMap() {
    checkFeatures();

    SDL_Surface* pMinimap;
    // create surface
	if((pMinimap = SDL_CreateRGBSurface(SDL_HWSURFACE,128,128,8,0,0,0,0))== NULL) {
		return NULL;
	}
	palette.applyToSurface(pMinimap);

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
            SDL_FreeSurface(pMinimap);
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
                SDL_FreeSurface(pMinimap);
                logError("Unknown MapScale!");
            } break;
	    }

	    logicalSizeX = 64;
	    logicalSizeY = 64;


        Uint16 SeedMap[64*64];
        createMapWithSeed(SeedNum,SeedMap);

        // "draw" spice fields into SeedMap
        std::string FieldString = inifile->getStringValue("MAP","Field");
        if(FieldString != "") {
            std::vector<std::string> FieldPositions  = splitString(FieldString);

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
                    SDL_FreeSurface(pMinimap);
                    logError(inifile->getKey("MAP", "Field")->getLineNumber(), "Invalid value for key Field: " + FieldString);
                }
            }
        }

        for(int y = 0; y < sizeY; y++) {
            for(int x = 0; x < sizeX; x++) {
                int color = PALCOLOR_BLACK;
                unsigned char seedmaptype = SeedMap[(y+logicalOffsetY)*64+x+logicalOffsetX] >> 4;
                switch(seedmaptype) {

                    case 0x7: {
                        // Normal sand
                        color = PALCOLOR_DESERTSAND;
                    } break;

                    case 0x2:
                    case 0x8: {
                        // Rock or building
                        color = PALCOLOR_DARKGREY;
                    } break;

                    case 0x9: {
                        // Sand dunes
                        color = PALCOLOR_DESERTSAND;
                    } break;

                    case 0xa: {
                        // Mountain
                        color = PALCOLOR_MOUNTAIN;
                    } break;

                    case 0xb: {
                        // Spice
                        color = PALCOLOR_SPICE;
                    } break;

                    case 0xc: {
                        // Thick spice
                        color = PALCOLOR_THICKSPICE;
                    } break;
                }

                for(int i=0;i<scale;i++) {
                    for(int j=0;j<scale;j++) {
                        putPixel(pMinimap, x*scale + i + offsetX, y*scale + j + offsetY, color);
                    }
                }
            }
        }

        // draw spice blooms
        std::string BloomString = inifile->getStringValue("MAP","Bloom");
        if(BloomString != "") {
            std::vector<std::string> BloomPositions  = splitString(BloomString);

            for(unsigned int i=0; i < BloomPositions.size();i++) {
                // set bloom
                int BloomPos;
                if(parseString(BloomPositions[i], BloomPos)) {
                    int xpos = BloomPos % logicalSizeX - logicalOffsetX;
                    int ypos = BloomPos / logicalSizeX - logicalOffsetY;
                    if(xpos >= 0 && xpos < sizeX && ypos >= 0 && ypos < sizeY) {
                        for(int i=0;i<scale;i++) {
                            for(int j=0;j<scale;j++) {
                                putPixel(pMinimap, xpos*scale + i + offsetX, ypos*scale + j + offsetY, PALCOLOR_RED);
                            }
                        }
                    }
                } else {
                    SDL_FreeSurface(pMinimap);
                    logError(inifile->getKey("MAP", "Bloom")->getLineNumber(), "Invalid value for key Bloom: " + BloomString);
                }
            }
        }

        // draw special blooms
        std::string SpecialString = inifile->getStringValue("MAP","Special");
        if(SpecialString != "") {
            std::vector<std::string> SpecialPositions  = splitString(SpecialString);

            for(unsigned int i=0; i < SpecialPositions.size();i++) {
                // set bloom
                int SpecialPos;
                if(parseString(SpecialPositions[i], SpecialPos)) {
                    int xpos = SpecialPos % logicalSizeX - logicalOffsetX;
                    int ypos = SpecialPos / logicalSizeX - logicalOffsetY;
                    if(xpos >= 0 && xpos < sizeX && ypos >= 0 && ypos < sizeY) {
                        for(int i=0;i<scale;i++) {
                            for(int j=0;j<scale;j++) {
                                putPixel(pMinimap, xpos*scale + i + offsetX, ypos*scale + j + offsetY, PALCOLOR_RED);
                            }
                        }
                    }
                } else {
                    SDL_FreeSurface(pMinimap);
                    logError(inifile->getKey("MAP", "Special")->getLineNumber(), "Invalid value for key Special: " + SpecialString);
                }
            }
        }


    } else {
        // new map format with saved map

        if((inifile->hasKey("MAP","SizeX") == false) || (inifile->hasKey("MAP","SizeY") == false)) {
            SDL_FreeSurface(pMinimap);
            logError("SizeX and SizeY must be specified!");
        }

        sizeX = inifile->getIntValue("MAP","SizeX", 0);
        sizeY = inifile->getIntValue("MAP","SizeY", 0);

        logicalSizeX = sizeX;
        logicalSizeY = sizeY;

        RadarView::calculateScaleAndOffsets(sizeX, sizeY, scale, offsetX, offsetY);

        for(int y=0;y<sizeY;y++) {
            std::string rowKey = strprintf("%.3d", y);

            if(inifile->hasKey("MAP", rowKey) == false) {
                SDL_FreeSurface(pMinimap);
                logError(inifile->getSection("MAP")->getLineNumber(), "Map row " + stringify(y) + " does not exist!");
            }

            std::string rowString = inifile->getStringValue("MAP",rowKey);
            for(int x=0;x<sizeX;x++) {
                int color = PALCOLOR_BLACK;
                switch(rowString.at(x)) {
                    case '-': {
                        // Normal sand
                        color = PALCOLOR_DESERTSAND;
                    } break;

                    case '^': {
                        // Sand dunes
                        color = PALCOLOR_DESERTSAND;
                    } break;

                    case '~': {
                        // Spice
                        color = PALCOLOR_SPICE;
                    } break;

                    case '+': {
                        // Thick spice
                        color = PALCOLOR_THICKSPICE;
                    } break;

                    case '%': {
                        // Rock
                        color = PALCOLOR_DARKGREY;
                    } break;

                    case '@': {
                        // Mountain
                        color = PALCOLOR_MOUNTAIN;
                    } break;

                    case 'O':
                    case 'Q': {
                        // Spice Bloom and Special Bloom
                        color = PALCOLOR_RED;
                    } break;

                    default: {
                        SDL_FreeSurface(pMinimap);
                        logError(inifile->getKey("MAP", rowKey)->getLineNumber(), std::string("Unknown map tile type '") + rowString.at(x) + "' in map tile (" + stringify(x) + ", " + stringify(y) + ")!");
                    } break;
                }

                for(int i=0;i<scale;i++) {
                    for(int j=0;j<scale;j++) {
                        putPixel(pMinimap, x*scale + i + offsetX, y*scale + j + offsetY, color);
                    }
                }
            }
        }
    }

    // draw structures
    INIFile::KeyIterator iter;

    for(iter = inifile->begin("STRUCTURES"); iter != inifile->end("STRUCTURES"); ++iter) {
		std::string tmpkey = iter->getKeyName();
		std::string tmp = iter->getStringValue();

		if(tmpkey.find("GEN") == 0) {
			// Gen Object/Structure

			std::string PosStr = tmpkey.substr(3,tmpkey.size()-3);
			int pos;
			if(!parseString(PosStr, pos)) {
                continue;
			}

			std::string HouseStr, BuildingStr;
			splitString(tmp,2,&HouseStr,&BuildingStr);

			int house = getHouseByName(HouseStr);
			int color = PALCOLOR_WHITE;
			if(house != HOUSE_INVALID) {
				color = houseColor[house];
			} else {
                convertToLower(HouseStr);
			    if(HouseStr.length() == 7 && HouseStr.substr(0,6) == "player") {
			        int playernum = HouseStr.at(6)-'0';

			        if(playernum >= 1 && playernum <= 6) {
			            color = 128 + playernum - 1;
			        }
			    } else {
			        SDL_FreeSurface(pMinimap);
                    logError(iter->getLineNumber(), "Invalid house string: '" + HouseStr + "'!");
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
                            putPixel(pMinimap, x*scale + i + offsetX, y*scale + j + offsetY, color);
                        }
                    }
                }
			}
		} else if(iter->getKeyName().find("ID") == 0) {
			// other structure
			std::string HouseStr, BuildingStr, health, PosStr;
			splitString(tmp,6,&HouseStr,&BuildingStr,&health,&PosStr);

			int pos;
			if(!parseString(PosStr, pos)) {
                continue;
			}

			int house = getHouseByName(HouseStr);
			int color = PALCOLOR_WHITE;
			if(house != HOUSE_INVALID) {
				color = houseColor[house];
			} else {
			    convertToLower(HouseStr);
			    if(HouseStr.length() == 7 && HouseStr.substr(0,6) == "player") {
			        int playernum = HouseStr.at(6)-'0';

			        if(playernum >= 1 && playernum <= 6) {
			            color = 128 + playernum - 1;
			        }
			    } else {
			        SDL_FreeSurface(pMinimap);
                    logError(iter->getLineNumber(), "Invalid house string: '" + HouseStr + "'!");
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
                                putPixel(pMinimap, x*scale + i + offsetX, y*scale + j + offsetY, color);
                            }
                        }
                    }
                }
			}
		}
	}

    return pMinimap;
}
