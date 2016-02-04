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
	radarSurface = SDL_CreateRGBSurface(SDL_HWSURFACE, RADARWIDTH, RADARHEIGHT, SCREEN_BPP, RMASK, GMASK, BMASK, AMASK);
	SDL_FillRect(radarSurface, NULL, COLOR_BLACK);
}


MapEditorRadarView::~MapEditorRadarView()
{
    if(radarSurface != NULL) {
        SDL_FreeSurface(radarSurface);
    }
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

    updateRadarSurface(map, scale, offsetX, offsetY);

    SDL_BlitSurface(radarSurface, NULL, screen, &radarPosition);

    // draw viewport rect on radar
    SDL_Rect radarRect;
    radarRect.x = (screenborder->getLeft() * map.getSizeX()*scale) / (map.getSizeX()*TILESIZE) + offsetX;
    radarRect.y = (screenborder->getTop() * map.getSizeY()*scale) / (map.getSizeY()*TILESIZE) + offsetY;
    radarRect.w = ((screenborder->getRight() - screenborder->getLeft()) * map.getSizeX()*scale) / (map.getSizeX()*TILESIZE);
    radarRect.h = ((screenborder->getBottom() - screenborder->getTop()) * map.getSizeY()*scale) / (map.getSizeY()*TILESIZE);

    if(radarRect.x < offsetX) {
        radarRect.w -= radarRect.x;
        radarRect.x = offsetX;
    }

    if(radarRect.y < offsetY) {
        radarRect.h -= radarRect.y;
        radarRect.y = offsetY;
    }

    int offsetFromRightX = 128 - map.getSizeX()*scale - offsetX;
    if(radarRect.x + radarRect.w > radarPosition.w - offsetFromRightX) {
        radarRect.w  = radarPosition.w - offsetFromRightX - radarRect.x - 1;
    }

    int offsetFromBottomY = 128 - map.getSizeY()*scale - offsetY;
    if(radarRect.y + radarRect.h > radarPosition.h - offsetFromBottomY) {
        radarRect.h = radarPosition.h - offsetFromBottomY - radarRect.y - 1;
    }

    drawRect(   screen,
                radarPosition.x + radarRect.x,
                radarPosition.y + radarRect.y,
                radarPosition.x + (radarRect.x + radarRect.w),
                radarPosition.y + (radarRect.y + radarRect.h),
                COLOR_WHITE);

}

void MapEditorRadarView::updateRadarSurface(const MapData& map, int scale, int offsetX, int offsetY) {

    SDL_FillRect(radarSurface, NULL, COLOR_BLACK);

    // Lock the surface for direct access to the pixels
    if(!SDL_MUSTLOCK(radarSurface) || (SDL_LockSurface(radarSurface) == 0)) {
        for(int y = 0; y <  map.getSizeY(); y++) {
            for(int x = 0; x <  map.getSizeX(); x++) {

                Uint32 color = getColorByTerrainType(map(x,y));

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
                        color = COLOR_BLOOM;
                        break;
                    }
                }



                std::vector<Coord>& specialBlooms = pMapEditor->getSpecialBlooms();
                for(size_t i = 0; i < specialBlooms.size(); i++) {
                    if(specialBlooms[i].x == x && specialBlooms[i].y == y) {
                        color = COLOR_BLOOM;
                        break;
                    }
                }

                color = MapRGBA(radarSurface->format, color);

                for(int j = 0; j < scale; j++) {
                    Uint32* p = ((Uint32*) ((Uint8 *) radarSurface->pixels + (offsetY + scale*y + j) * radarSurface->pitch)) + (offsetX + scale*x);

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
                        putPixel(   radarSurface,
                                    offsetX + scale*uIter->position.x + i,
                                    offsetY + scale*uIter->position.y + j,
                                    SDL2RGB(palette[houseToPaletteIndex[uIter->house]]));
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
                                putPixel(   radarSurface,
                                            offsetX + scale*(sIter->position.x+x) + i,
                                            offsetY + scale*(sIter->position.y+y) + j,
                                            SDL2RGB(palette[houseToPaletteIndex[sIter->house]]));
                            }
                        }
                    }
                }
            }
        }

        if (SDL_MUSTLOCK(radarSurface))
            SDL_UnlockSurface(radarSurface);
    }
}
