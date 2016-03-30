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

#ifndef MAPCHOICE_H
#define MAPCHOICE_H

#include "MenuBase.h"
#include <DataTypes.h>
#include <GUI/dune/MessageTicker.h>
#include <misc/draw_util.h>
#include <misc/BlendBlitter.h>
#include <mmath.h>
#include <vector>
#include <SDL.h>

#define MAPCHOICESTATE_FADEINPLANET 0
#define MAPCHOICESTATE_SHOWPLANET   1
#define MAPCHOICESTATE_BLENDPLANET  2
#define MAPCHOICESTATE_SHOWMAPONLY  3
#define MAPCHOICESTATE_BLENDMAP     4
#define MAPCHOICESTATE_BLENDING		5
#define MAPCHOICESTATE_ARROWS		6
#define MAPCHOICESTATE_BLINKING		7

class MapChoice : public MenuBase
{
public:
	MapChoice(int newHouse, unsigned int LastMission);
	virtual ~MapChoice();

	virtual int showMenu();

	void drawSpecificStuff();
	bool doInput(SDL_Event &event);

private:
	void createMapSurfaceWithPieces();
	void loadINI();

private:
	struct TGroup {
		std::vector<int> newRegion[NUM_HOUSES];

		struct TAttackRegion {
			int regionNum;
			int arrowNum;
			Coord arrowPosition;
		} attackRegion[4];

		struct TText {
			std::string message;
			int region;		///< when this region is changed, this message will appear.
		};

		std::vector<TText> text;
	} group[9];

	int house;
	unsigned int lastScenario;
	SDL_Surface* mapSurface;
	SDL_Texture* mapTexture;
	Coord piecePosition[28];
	BlendBlitter* curBlendBlitter;
	unsigned int curHouse2Blit;
	unsigned int curRegion2Blit;
	bool bFastBlending;
	int mapChoiceState;
	int selectedRegion;
	Uint32	selectionTime;
	Uint32  stateSwitchTime;
	MessageTicker  msgticker;

	SDL_Rect centerAreaRect;
};

#endif //MAPCHOICE_H
