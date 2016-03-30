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

#ifndef SAND_H
#define SAND_H

#include <SDL.h>

#include <fixmath/FixPoint.h>
#include <DataTypes.h>
#include <string>

// forward declarations
class GameInitSettings;
extern SDL_Window* window;
extern SDL_Renderer* renderer;

void drawCursor();

inline void adjustMouseCoords(int& x, int& y) {
//    int old_x = x, oldy = y;
    int win_w, win_h;
    SDL_Rect vp;
    SDL_GetWindowSize(window, &win_w, &win_h);
    SDL_RenderGetViewport(renderer, &vp);
//    fprintf(stderr, "viewport is %dx%d at (%d,%d), window size %dx%d\n", vp.w, vp.h, vp.x, vp.y, win_w, win_h);
    x = x * (vp.w+2*vp.x) < vp.x * win_w ? 0 : x * (vp.w+2*vp.x) > (vp.x + vp.w) * win_w ? vp.w : (x*(vp.w+2*vp.x))/win_w-vp.x;
    y = y * (vp.h+2*vp.y) < vp.y * win_h ? 0 : y * (vp.h+2*vp.y) > (vp.y + vp.h) * win_h ? vp.h : (y*(vp.h+2*vp.y))/win_h-vp.y;
//    fprintf(stderr, "adjusting %d,%d to %d,%d\n", old_x, oldy, x, y);
}

inline void resetMouseCoords(int& x, int& y) {
//    int old_x = x, oldy = y;
    int win_w, win_h;
    SDL_Rect vp;
    SDL_GetWindowSize(window, &win_w, &win_h);
    SDL_RenderGetViewport(renderer, &vp);
//    fprintf(stderr, "viewport is %dx%d at (%d,%d)\n", vp.w, vp.h, vp.x, vp.y);
    x = (x + vp.x)*win_w/(vp.w+2*vp.x);
    y = (y + vp.y)*win_h/(vp.h+2*vp.y);
//    fprintf(stderr, "reseting %d,%d to %d,%d\n", old_x, oldy, x, y);
}


std::string     resolveItemName(int itemID);

int             getAnimByFilename(std::string filename);

Coord	        getStructureSize(int itemID);

Uint32          getItemIDByName(std::string name);
std::string     getItemNameByID(Uint32 itemID);
SDL_Texture*    resolveItemPicture(int itemID, HOUSETYPE house = HOUSE_HARKONNEN);

HOUSETYPE       getHouseByName(std::string name);
std::string     getHouseNameByNumber(HOUSETYPE house);

ATTACKMODE      getAttackModeByName(std::string name);
std::string     getAttackModeNameByMode(ATTACKMODE attackMode);

DropLocation    getDropLocationByName(std::string name);
std::string     getDropLocationNameByID(DropLocation dropLocation);
std::string     resolveDropLocationName(DropLocation dropLocation);


TeamBehavior    getTeamBehaviorByName(std::string name);
std::string     getTeamBehaviorNameByID(TeamBehavior teamBehavior);

TeamType        getTeamTypeByName(std::string name);
std::string     getTeamTypeNameByID(TeamType teamType);


Uint32          getColorByTerrainType(int terrainType);

FixPoint        getDeviateWeakness(HOUSETYPE house);

inline int missionNumberToLevelNumber(int missionNumber) {
    if(missionNumber != 22) {
        return ((missionNumber+1)/3)+1;
    } else {
        return 9;
    }
}

void startReplay(std::string filename);
void startSinglePlayerGame(const GameInitSettings& init);
void startMultiPlayerGame(const GameInitSettings& init);


#endif //SAND_H
