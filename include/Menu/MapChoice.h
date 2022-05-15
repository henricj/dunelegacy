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

#include "FileClasses/GFXConstants.h"
#include "GUI/StaticContainer.h"
#include "MenuBase.h"
#include <DataTypes.h>
#include <GUI/dune/MessageTicker.h>
#include <misc/BlendBlitter.h>
#include <misc/SDL2pp.h>

#include <array>
#include <vector>

inline constexpr auto MAPCHOICESTATE_FADEINPLANET = 0;
inline constexpr auto MAPCHOICESTATE_SHOWPLANET   = 1;
inline constexpr auto MAPCHOICESTATE_BLENDPLANET  = 2;
inline constexpr auto MAPCHOICESTATE_SHOWMAPONLY  = 3;
inline constexpr auto MAPCHOICESTATE_BLENDMAP     = 4;
inline constexpr auto MAPCHOICESTATE_BLENDING     = 5;
inline constexpr auto MAPCHOICESTATE_ARROWS       = 6;
inline constexpr auto MAPCHOICESTATE_BLINKING     = 7;

class MapChoice final : public MenuBase {
    using parent = MenuBase;

public:
    MapChoice(HOUSETYPE newHouse, unsigned int lastMission, uint32_t alreadyPlayedRegions);
    ~MapChoice() override;

    int showMenu() override;

    [[nodiscard]] int getSelectedMission() const;

    [[nodiscard]] uint32_t getAlreadyPlayedRegions() const { return alreadyPlayedRegions; }

    void drawSpecificStuff() override;
    bool doInput(SDL_Event& event) override;

private:
    void createMapSurfaceWithPieces(unsigned int scenario);
    void loadINI();

private:
    struct TGroup {
        std::array<std::vector<UIGraphics_Enum>, static_cast<int>(HOUSETYPE::NUM_HOUSES)> newRegion;

        struct TAttackRegion {
            int regionNum;
            int arrowNum;
            Coord arrowPosition;
        };

        std::array<TAttackRegion, 4> attackRegion;

        struct TText {
            std::string message;
            int region; ///< when this region is changed, this message will appear.
        };

        std::vector<TText> text;
    };

    std::array<TGroup, 9> group;

    HOUSETYPE house;
    unsigned int lastScenario;
    uint32_t alreadyPlayedRegions;
    sdl2::surface_ptr mapSurface;
    sdl2::texture_ptr mapTexture;
    std::array<Coord, 28> piecePosition;
    std::unique_ptr<BlendBlitter> curBlendBlitter;
    HOUSETYPE curHouse2Blit     = static_cast<HOUSETYPE>(0);
    unsigned int curRegion2Blit = 0;
    bool bFastBlending          = false;
    int mapChoiceState;
    UIGraphics_Enum selectedRegion = static_cast<UIGraphics_Enum>(-1);
    dune::dune_clock::time_point selectionTime{};
    dune::dune_clock::time_point stateSwitchTime{};

    StaticContainer container_;
    MessageTicker msgticker;

    SDL_Rect centerAreaRect{};
};

#endif // MAPCHOICE_H
