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
#include <misc/SDL2pp.h>

#include <mmath.h>
#include <vector>

#define MAPCHOICESTATE_FADEINPLANET 0
#define MAPCHOICESTATE_SHOWPLANET   1
#define MAPCHOICESTATE_BLENDPLANET  2
#define MAPCHOICESTATE_SHOWMAPONLY  3
#define MAPCHOICESTATE_BLENDMAP     4
#define MAPCHOICESTATE_BLENDING     5
#define MAPCHOICESTATE_ARROWS       6
#define MAPCHOICESTATE_BLINKING     7

class MapChoice : public MenuBase
{
public:
    MapChoice(int newHouse, unsigned int lastMission, Uint32 alreadyPlayedRegions);
    virtual ~MapChoice();

    int showMenu() override;

    inline int getSelectedMission() const {
        int regionIndex;
        for(regionIndex = 0; regionIndex < 4; regionIndex++) {
            if(group[lastScenario].attackRegion[regionIndex].regionNum == selectedRegion) {
                break;
            }
        }

        int newMission;
        if(lastScenario <= 7) {
            newMission = (lastScenario-1) * 3 + 2 + regionIndex;
        } else if(lastScenario == 8) {
            newMission = (lastScenario-1) * 3 - 1 + 2 + regionIndex;
        } else {
            THROW(std::runtime_error, "lastScenario = %u is no valid scenario number!", lastScenario);
        }
        return newMission;
    };

    inline Uint32 getAlreadyPlayedRegions() const { return alreadyPlayedRegions; }

    void drawSpecificStuff() override;
    bool doInput(SDL_Event &event) override;

private:
    void createMapSurfaceWithPieces(unsigned int scenario);
    void loadINI();

private:
    struct TGroup {
        std::array<std::vector<int>, NUM_HOUSES> newRegion;

        struct TAttackRegion {
            int regionNum;
            int arrowNum;
            Coord arrowPosition;
        };

        std::array<TAttackRegion, 4> attackRegion;

        struct TText {
            std::string message;
            int region;     ///< when this region is changed, this message will appear.
        };

        std::vector<TText> text;
    };

    std::array<TGroup, 9> group;

    int house;
    unsigned int lastScenario;
    Uint32 alreadyPlayedRegions;
    sdl2::surface_ptr mapSurface;
    sdl2::texture_ptr mapTexture;
    std::array<Coord, 28> piecePosition;
    std::unique_ptr<BlendBlitter> curBlendBlitter;
    unsigned int curHouse2Blit;
    unsigned int curRegion2Blit;
    bool bFastBlending;
    int mapChoiceState;
    int selectedRegion;
    Uint32  selectionTime;
    Uint32  stateSwitchTime;
    MessageTicker  msgticker;

    SDL_Rect centerAreaRect;
};

#endif //MAPCHOICE_H
