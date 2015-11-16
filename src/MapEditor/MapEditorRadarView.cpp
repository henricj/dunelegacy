/*
 *  This file is part of Dune Legacy.
 *
 *  Dune Legacy is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  Dune Legacy is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with Dune Legacy.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <MapEditor/MapEditorRadarView.h>

#include <MapEditor/MapEditor.h>

#include <globals.h>

#include <sand.h>

#include <ScreenBorder.h>
#include <Tile.h>

#include <misc/draw_util.h>


MapEditorRadarView::MapEditorRadarView(MapEditor* pMapEditor)
 : RadarViewBase(), pMapEditor(pMapEditor)
{
}


MapEditorRadarView::~MapEditorRadarView()
{

}

int MapEditorRadarView::getMapSizeX() const {
    return pMapEditor->getMap().getSizeX();
}

int MapEditorRadarView::getMapSizeY() const {
    return pMapEditor->getMap().getSizeY();
}

void MapEditorRadarView::draw(SDL_Surface* screen, Point position)
{
    SDL_Rect radarPosition = { static_cast<Sint16>(position.x + RADARVIEW_BORDERTHICKNESS), static_cast<Sint16>(position.y + RADARVIEW_BORDERTHICKNESS), RADARWIDTH, RADARHEIGHT};

    const MapData& map = pMapEditor->getMap();

    int scale = 1;
    int offsetX = 0;
    int offsetY = 0;

    calculateScaleAndOffsets(map.getSizeX(), map.getSizeY(), scale, offsetX, offsetY);

    // Lock the screen for direct access to the pixels
    if(!SDL_MUSTLOCK(screen) || (SDL_LockSurface(screen) == 0)) {
        for(int y = 0; y <  map.getSizeY(); y++) {
            for(int x = 0; x <  map.getSizeX(); x++) {

                Uint32 color;

				switch(map(x,y)) {
					case Terrain_Dunes:         color = COLOR_DESERTSAND;   break;
					case Terrain_Mountain:      color = COLOR_MOUNTAIN;		break;
					case Terrain_Rock:		    color = COLOR_DARKGREY;		break;
					case Terrain_Sand:		    color = COLOR_DESERTSAND;	break;
					case Terrain_Spice:		    color = COLOR_SPICE;		break;
					case Terrain_ThickSpice:    color = COLOR_THICKSPICE;	break;
					case Terrain_SpiceBloom:    color = COLOR_RED;          break;
					case Terrain_SpecialBloom:	color = COLOR_RED;			break;
                    case Terrain_Slab:			color = COLOR_DARKGREY;     break;
                    default:                    color = COLOR_DARKGREY;     break;
				};


                if(map(x,y) == Terrain_Sand) {
                    std::vector<Coord>& spiceFields = pMapEditor->getSpiceFields();

                    for(size_t i = 0; i < spiceFields.size(); i++) {
                        if(spiceFields[i].x == x && spiceFields[i].y == y) {
                            color = COLOR_THICKSPICE;
                            break;
                        } else if(distanceFrom(spiceFields[i], Coord(x,y)) <= 5) {
                            color = COLOR_SPICE;
                            break;
                        }
                    }
                }

                // check for classic map items (spice blooms, special blooms)
                std::vector<Coord>& spiceBlooms = pMapEditor->getSpiceBlooms();
                for(size_t i = 0; i < spiceBlooms.size(); i++) {
                    if(spiceBlooms[i].x == x && spiceBlooms[i].y == y) {
                        color = COLOR_RED;
                        break;
                    }
                }



                std::vector<Coord>& specialBlooms = pMapEditor->getSpecialBlooms();
                for(size_t i = 0; i < specialBlooms.size(); i++) {
                    if(specialBlooms[i].x == x && specialBlooms[i].y == y) {
                        color = COLOR_RED;
                        break;
                    }
                }

                for(int j = 0; j < scale; j++) {
                    Uint8* p = (Uint8 *) screen->pixels + (radarPosition.y + offsetY + scale*y + j) * screen->pitch + (radarPosition.x + offsetX + scale*x);

                    for(int i = 0; i < scale; i++, p++) {
                        // Do not use putPixel here to avoid overhead
                        *p = color;
                    }
                }
            }
        }

        std::vector<MapEditor::Unit>& units = pMapEditor->getUnitList();
        std::vector<MapEditor::Unit>::const_iterator uIter;
        for(uIter = units.begin(); uIter != units.end(); ++uIter) {

            if(uIter->position.x >= 0 && uIter->position.x < map.getSizeX()
                && uIter->position.y >= 0 && uIter->position.y < map.getSizeY()) {

                for(int i = 0; i < scale; i++) {
                    for(int j = 0; j < scale; j++) {
                        putPixel(   screen,
                                    radarPosition.x + offsetX + scale*uIter->position.x + i,
                                    radarPosition.y + offsetY + scale*uIter->position.y + j,
                                    houseColor[uIter->house]);
                    }
                }
            }
        }

        std::vector<MapEditor::Structure>& structures = pMapEditor->getStructureList();
        std::vector<MapEditor::Structure>::const_iterator sIter;
        for(sIter = structures.begin(); sIter != structures.end(); ++sIter) {
            Coord structureSize = getStructureSize(sIter->itemID);

            for(int y = 0; y < structureSize.y; y++) {
                for(int x = 0; x < structureSize.x; x++) {

                    if(x >= 0 && x < map.getSizeX()
                        && y >= 0 && y < map.getSizeY()) {

                        for(int i = 0; i < scale; i++) {
                            for(int j = 0; j < scale; j++) {
                                putPixel(   screen,
                                            radarPosition.x + offsetX + scale*(sIter->position.x+x) + i,
                                            radarPosition.y + offsetY + scale*(sIter->position.y+y) + j,
                                            houseColor[sIter->house]);
                            }
                        }
                    }
                }
            }
        }



        if (SDL_MUSTLOCK(screen))
            SDL_UnlockSurface(screen);
    }



    SDL_Rect RadarRect;
    RadarRect.x = (screenborder->getLeft() * map.getSizeX()*scale) / (map.getSizeX()*TILESIZE) + offsetX;
    RadarRect.y = (screenborder->getTop() * map.getSizeY()*scale) / (map.getSizeY()*TILESIZE) + offsetY;
    RadarRect.w = ((screenborder->getRight() - screenborder->getLeft()) * map.getSizeX()*scale) / (map.getSizeX()*TILESIZE);
    RadarRect.h = ((screenborder->getBottom() - screenborder->getTop()) * map.getSizeY()*scale) / (map.getSizeY()*TILESIZE);

    if(RadarRect.x < offsetX) {
        RadarRect.w -= RadarRect.x;
        RadarRect.x = offsetX;
    }

    if(RadarRect.y < offsetY) {
        RadarRect.h -= RadarRect.y;
        RadarRect.y = offsetY;
    }

    int offsetFromRightX = 128 - map.getSizeX()*scale - offsetX;
    if(RadarRect.x + RadarRect.w > radarPosition.w - offsetFromRightX) {
        RadarRect.w  = radarPosition.w - offsetFromRightX - RadarRect.x - 1;
    }

    int offsetFromBottomY = 128 - map.getSizeY()*scale - offsetY;
    if(RadarRect.y + RadarRect.h > radarPosition.h - offsetFromBottomY) {
        RadarRect.h = radarPosition.h - offsetFromBottomY - RadarRect.y - 1;
    }

    drawRect(   screen,
                radarPosition.x + RadarRect.x,
                radarPosition.y + RadarRect.y,
                radarPosition.x + (RadarRect.x + RadarRect.w),
                radarPosition.y + (RadarRect.y + RadarRect.h),
                COLOR_WHITE);

}
