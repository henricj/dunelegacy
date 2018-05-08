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

#ifndef GFXMANAGER_H
#define GFXMANAGER_H

#include "Animation.h"
#include "Shpfile.h"
#include "Wsafile.h"
#include <DataTypes.h>

#include <misc/SDL2pp.h>

#include <string>
#include <array>
#include <memory>

#define NUM_TERRAIN_TILES_X 11
#define NUM_TERRAIN_TILES_Y 8
#define NUM_MAPCHOICEPIECES 28
#define NUM_WINDTRAP_ANIMATIONS (2*STRUCTURE_ANIMATIONTIMER+4)
#define NUM_WINDTRAP_ANIMATIONS_PER_ROW 10
#define NUM_STATIC_ANIMATIONS_PER_ROW 7

// ObjPics
typedef enum {
    ObjPic_Tank_Base,
    ObjPic_Tank_Gun,
    ObjPic_Siegetank_Base,
    ObjPic_Siegetank_Gun,
    ObjPic_Devastator_Base,
    ObjPic_Devastator_Gun,
    ObjPic_Sonictank_Gun,
    ObjPic_Launcher_Gun,
    ObjPic_Quad,
    ObjPic_Trike,
    ObjPic_Harvester,
    ObjPic_Harvester_Sand,
    ObjPic_MCV,
    ObjPic_Carryall,
    ObjPic_CarryallShadow,
    ObjPic_Frigate,
    ObjPic_FrigateShadow,
    ObjPic_Ornithopter,
    ObjPic_OrnithopterShadow,
    ObjPic_Trooper,
    ObjPic_Troopers,
    ObjPic_Soldier,
    ObjPic_Infantry,
    ObjPic_Saboteur,
    ObjPic_Sandworm,
    ObjPic_ConstructionYard,
    ObjPic_Windtrap,
    ObjPic_Refinery,
    ObjPic_Barracks,
    ObjPic_WOR,
    ObjPic_Radar,
    ObjPic_LightFactory,
    ObjPic_Silo,
    ObjPic_HeavyFactory,
    ObjPic_HighTechFactory,
    ObjPic_IX,
    ObjPic_Palace,
    ObjPic_RepairYard,
    ObjPic_Starport,
    ObjPic_GunTurret,
    ObjPic_RocketTurret,
    ObjPic_Wall,
    ObjPic_Bullet_SmallRocket,
    ObjPic_Bullet_MediumRocket,
    ObjPic_Bullet_LargeRocket,
    ObjPic_Bullet_Small,
    ObjPic_Bullet_Medium,
    ObjPic_Bullet_Large,
    ObjPic_Bullet_Sonic,
    ObjPic_Bullet_SonicTemp,
    ObjPic_Hit_Gas,
    ObjPic_Hit_ShellSmall,
    ObjPic_Hit_ShellMedium,
    ObjPic_Hit_ShellLarge,
    ObjPic_ExplosionSmall,
    ObjPic_ExplosionMedium1,
    ObjPic_ExplosionMedium2,
    ObjPic_ExplosionLarge1,
    ObjPic_ExplosionLarge2,
    ObjPic_ExplosionSmallUnit,
    ObjPic_ExplosionFlames,
    ObjPic_ExplosionSpiceBloom,
    ObjPic_DeadInfantry,
    ObjPic_DeadAirUnit,
    ObjPic_Smoke,
    ObjPic_SandwormShimmerMask,
    ObjPic_SandwormShimmerTemp,
    ObjPic_Terrain,
    ObjPic_DestroyedStructure,
    ObjPic_RockDamage,
    ObjPic_SandDamage,
    ObjPic_Terrain_Hidden,
    ObjPic_Terrain_HiddenFog,
    ObjPic_Terrain_Tracks,
    ObjPic_Star,
    NUM_OBJPICS
} ObjPic_enum;

static const std::array<std::string, NUM_OBJPICS> ObjPicNames =  { { "Tank_Base", "Tank_Gun", "Siegetank_Base", "Siegetank_Gun", "Devastator_Base",
    "Devastator_Gun", "Sonictank_Gun", "Launcher_Gun", "Quad", "Trike", "Harvester", "Harvester_Sand", "MCV", "Carryall", "CarryallShadow",
    "Frigate", "FrigateShadow", "Ornithopter", "OrnithopterShadow", "Trooper", "Troopers", "Soldier", "Infantry", "Saboteur", "Sandworm",
    "ConstructionYard", "Windtrap", "Refinery", "Barracks", "WOR", "Radar", "LightFactory", "Silo", "HeavyFactory", "HighTechFactory",
    "IX", "Palace", "RepairYard", "Starport", "GunTurret", "RocketTurret", "Wall",
    "Bullet_SmallRocket", "Bullet_MediumRocket", "Bullet_LargeRocket", "Bullet_Small", "Bullet_Medium", "Bullet_Large", "Bullet_Sonic",
    "Bullet_SonicTemp", "Hit_Gas", "Hit_ShellSmall", "Hit_ShellMedium", "Hit_ShellLarge", "ExplosionSmall", "ExplosionMedium1",
    "ExplosionMedium2", "ExplosionLarge1", "ExplosionLarge2", "ExplosionSmallUnit", "ExplosionFlames", "ExplosionSpiceBloom",
    "DeadInfantry", "DeadAirUnit", "Smoke", "SandwormShimmerMask", "SandwormShimmerTemp", "Terrain", "DestroyedStructure", "RockDamage",
    "SandDamage", "Terrain_Hidden", "Terrain_HiddenFog", "Terrain_Tracks", "Star" } };

#define GROUNDUNIT_ROW(i) (i+2)|TILE_NORMAL,(i+1)|TILE_NORMAL,i|TILE_NORMAL,(i+1)|TILE_FLIPV,(i+2)|TILE_FLIPV,(i+3)|TILE_FLIPV, (i+4)|TILE_NORMAL,(i+3)|TILE_NORMAL
#define AIRUNIT_ROW(i) (i+2)|TILE_NORMAL,(i+1)|TILE_NORMAL,i|TILE_NORMAL,(i+1)|TILE_FLIPV,(i+2)|TILE_FLIPV,(i+1)|TILE_ROTATE, i|TILE_FLIPH,(i+1)|TILE_FLIPH
#define ORNITHOPTER_ROW(i) (i+6)|TILE_NORMAL,(i+3)|TILE_NORMAL,i|TILE_NORMAL,(i+3)|TILE_FLIPV,(i+6)|TILE_FLIPV,(i+3)|TILE_ROTATE, i|TILE_FLIPH,(i+3)|TILE_FLIPH
#define INFANTRY_ROW(i) (i+3)|TILE_NORMAL,i|TILE_NORMAL,(i+3)|TILE_FLIPV,(i+6)|TILE_NORMAL
#define MULTIINFANTRY_ROW(i) (i+4)|TILE_NORMAL,i|TILE_NORMAL,(i+4)|TILE_FLIPV,(i+8)|TILE_NORMAL
#define HARVESTERSAND_ROW(i) (i+6)|TILE_NORMAL,(i+3)|TILE_NORMAL,i|TILE_NORMAL,(i+3)|TILE_FLIPV,(i+6)|TILE_FLIPV,(i+9)|TILE_FLIPV,(i+12)|TILE_NORMAL,(i+9)|TILE_NORMAL
#define ROCKET_ROW(i)   (i+4)|TILE_NORMAL,(i+3)|TILE_NORMAL,(i+2)|TILE_NORMAL,(i+1)|TILE_NORMAL,i|TILE_NORMAL,(i+1)|TILE_FLIPV,(i+2)|TILE_FLIPV,(i+3)|TILE_FLIPV, \
                        (i+4)|TILE_FLIPV,(i+3)|TILE_ROTATE,(i+2)|TILE_ROTATE, (i+1)|TILE_ROTATE,i|TILE_FLIPH,(i+1)|TILE_FLIPH,(i+2)|TILE_FLIPH,(i+3)|TILE_FLIPH


// SmallDetailPics
typedef enum {
    Picture_Barracks,
    Picture_ConstructionYard,
    Picture_Carryall,
    Picture_Devastator,
    Picture_Deviator,
    Picture_DeathHand,
    Picture_Fremen,
    Picture_Frigate,
    Picture_GunTurret,
    Picture_Harvester,
    Picture_HeavyFactory,
    Picture_HighTechFactory,
    Picture_Soldier,
    Picture_IX,
    Picture_Launcher,
    Picture_LightFactory,
    Picture_MCV,
    Picture_Ornithopter,
    Picture_Palace,
    Picture_Quad,
    Picture_Radar,
    Picture_RaiderTrike,
    Picture_Refinery,
    Picture_RepairYard,
    Picture_RocketTurret,
    Picture_Saboteur,
    Picture_Sandworm,
    Picture_Sardaukar,
    Picture_SiegeTank,
    Picture_Silo,
    Picture_Slab1,
    Picture_Slab4,
    Picture_SonicTank,
    Picture_Special,
    Picture_StarPort,
    Picture_Tank,
    Picture_Trike,
    Picture_Trooper,
    Picture_Wall,
    Picture_WindTrap,
    Picture_WOR,
    NUM_SMALLDETAILPICS
} SmallDetailPics_Enum;

// tiny pictures used for tutorial hints (has the same order as ItemID_enum, except the first entry)
typedef enum {
    TinyPicture_Spice = 0,
    TinyPicture_Barracks = 1,
    TinyPicture_ConstructionYard = 2,
    TinyPicture_GunTurret = 3,
    TinyPicture_HeavyFactory = 4,
    TinyPicture_HighTechFactory = 5,
    TinyPicture_IX = 6,
    TinyPicture_LightFactory = 7,
    TinyPicture_Palace = 8,
    TinyPicture_Radar = 9,
    TinyPicture_Refinery = 10,
    TinyPicture_RepairYard = 11,
    TinyPicture_RocketTurret = 12,
    TinyPicture_Silo = 13,
    TinyPicture_Slab1 = 14,
    TinyPicture_Slab4 = 15,
    TinyPicture_StarPort = 16,
    TinyPicture_Wall = 17,
    TinyPicture_WindTrap = 18,
    TinyPicture_WOR = 19,
    TinyPicture_Carryall = 20,
    TinyPicture_Devastator = 21,
    TinyPicture_Deviator = 22,
    TinyPicture_Frigate = 23,
    TinyPicture_Harvester = 24,
    TinyPicture_Soldier = 25,
    TinyPicture_Launcher = 26,
    TinyPicture_MCV = 27,
    TinyPicture_Ornithopter = 28,
    TinyPicture_Quad = 29,
    TinyPicture_Saboteur = 30,
    TinyPicture_Sandworm = 31,
    TinyPicture_SiegeTank = 32,
    TinyPicture_SonicTank = 33,
    TinyPicture_Tank = 34,
    TinyPicture_Trike = 35,
    TinyPicture_RaiderTrike = 36,
    TinyPicture_Trooper = 37,
    TinyPicture_Special = 38,
    TinyPicture_Infantry = 39,
    TinyPicture_Troopers = 40,
    NUM_TINYPICTURE
} TinyPicture_Enum;

// UI Graphics
typedef enum {
    UI_RadarAnimation,
    UI_CursorNormal,
    UI_CursorUp,
    UI_CursorRight,
    UI_CursorDown,
    UI_CursorLeft,
    UI_CursorMove_Zoomlevel0,
    UI_CursorMove_Zoomlevel1,
    UI_CursorMove_Zoomlevel2,
    UI_CursorAttack_Zoomlevel0,
    UI_CursorAttack_Zoomlevel1,
    UI_CursorAttack_Zoomlevel2,
    UI_CursorCapture_Zoomlevel0,
    UI_CursorCapture_Zoomlevel1,
    UI_CursorCapture_Zoomlevel2,
    UI_CursorCarryallDrop_Zoomlevel0,
    UI_CursorCarryallDrop_Zoomlevel1,
    UI_CursorCarryallDrop_Zoomlevel2,
    UI_SendToRepairIcon,
    UI_ReturnIcon,
    UI_DeployIcon,
    UI_DestructIcon,
    UI_CreditsDigits,
    UI_SideBar,
    UI_Indicator,
    UI_InvalidPlace_Zoomlevel0,
    UI_InvalidPlace_Zoomlevel1,
    UI_InvalidPlace_Zoomlevel2,
    UI_ValidPlace_Zoomlevel0,
    UI_ValidPlace_Zoomlevel1,
    UI_ValidPlace_Zoomlevel2,
    UI_GreyPlace_Zoomlevel0,
    UI_GreyPlace_Zoomlevel1,
    UI_GreyPlace_Zoomlevel2,
    UI_MenuBackground,
    UI_GameStatsBackground,
    UI_SelectionBox_Zoomlevel0,
    UI_SelectionBox_Zoomlevel1,
    UI_SelectionBox_Zoomlevel2,
    UI_OtherPlayerSelectionBox_Zoomlevel0,
    UI_OtherPlayerSelectionBox_Zoomlevel1,
    UI_OtherPlayerSelectionBox_Zoomlevel2,
    UI_TopBar,
    UI_ButtonUp,
    UI_ButtonUp_Pressed,
    UI_ButtonDown,
    UI_ButtonDown_Pressed,
    UI_BuilderListUpperCap,
    UI_BuilderListLowerCap,
    UI_CustomGamePlayersArrow,
    UI_CustomGamePlayersArrowNeutral,
    UI_MessageBox,
    UI_Mentat,
    UI_Mentat_Pressed,
    UI_Options,
    UI_Options_Pressed,
    UI_Upgrade,
    UI_Upgrade_Pressed,
    UI_Repair,
    UI_Repair_Pressed,
    UI_HouseSelect,
    UI_SelectYourHouseLarge,
    UI_Herald_Colored,
    UI_Herald_ColoredLarge,
    UI_Herald_Grey,
    UI_Herald_ArrowLeft,
    UI_Herald_ArrowLeftLarge,
    UI_Herald_ArrowLeftHighlight,
    UI_Herald_ArrowLeftHighlightLarge,
    UI_Herald_ArrowRight,
    UI_Herald_ArrowRightLarge,
    UI_Herald_ArrowRightHighlight,
    UI_Herald_ArrowRightHighlightLarge,
    UI_Minus,
    UI_Minus_Active,
    UI_Minus_Pressed,
    UI_Plus,
    UI_Plus_Active,
    UI_Plus_Pressed,
    UI_MissionSelect,
    UI_OptionsMenu,
    UI_LoadSaveWindow,
    UI_NewMapWindow,
    UI_GameMenu,
    UI_MentatBackground,
    UI_MentatBackgroundBene,
    UI_MentatHouseChoiceInfoQuestion,
    UI_MentatYes,
    UI_MentatYes_Pressed,
    UI_MentatNo,
    UI_MentatNo_Pressed,
    UI_MentatExit,
    UI_MentatExit_Pressed,
    UI_MentatProcced,
    UI_MentatProcced_Pressed,
    UI_MentatRepeat,
    UI_MentatRepeat_Pressed,
    UI_PlanetBackground,
    UI_MenuButtonBorder,
    UI_DuneLegacy,
    UI_MapChoiceScreen,
    UI_MapChoicePlanet,
    UI_MapChoiceMapOnly,
    UI_MapChoiceMap,
    UI_MapChoiceClickMap,
    UI_MapChoiceArrow_None,
    UI_MapChoiceArrow_LeftUp,
    UI_MapChoiceArrow_Up,
    UI_MapChoiceArrow_RightUp,
    UI_MapChoiceArrow_Right,
    UI_MapChoiceArrow_RightDown,
    UI_MapChoiceArrow_Down,
    UI_MapChoiceArrow_LeftDown,
    UI_MapChoiceArrow_Left,
    UI_StructureSizeLattice,
    UI_StructureSizeConcrete,
    UI_MapEditor_SideBar,
    UI_MapEditor_BottomBar,
    UI_MapEditor_ExitIcon,
    UI_MapEditor_NewIcon,
    UI_MapEditor_LoadIcon,
    UI_MapEditor_SaveIcon,
    UI_MapEditor_UndoIcon,
    UI_MapEditor_RedoIcon,
    UI_MapEditor_PlayerIcon,
    UI_MapEditor_MapSettingsIcon,
    UI_MapEditor_ChoamIcon,
    UI_MapEditor_ReinforcementsIcon,
    UI_MapEditor_TeamsIcon,
    UI_MapEditor_MirrorNoneIcon,
    UI_MapEditor_MirrorHorizontalIcon,
    UI_MapEditor_MirrorVerticalIcon,
    UI_MapEditor_MirrorBothIcon,
    UI_MapEditor_MirrorPointIcon,
    UI_MapEditor_ArrowUp,
    UI_MapEditor_ArrowUp_Active,
    UI_MapEditor_ArrowDown,
    UI_MapEditor_ArrowDown_Active,
    UI_MapEditor_Plus,
    UI_MapEditor_Plus_Active,
    UI_MapEditor_Minus,
    UI_MapEditor_Minus_Active,
    UI_MapEditor_RotateLeftIcon,
    UI_MapEditor_RotateLeftHighlightIcon,
    UI_MapEditor_RotateRightIcon,
    UI_MapEditor_RotateRightHighlightIcon,
    UI_MapEditor_Sand,
    UI_MapEditor_Dunes,
    UI_MapEditor_SpecialBloom,
    UI_MapEditor_Spice,
    UI_MapEditor_ThickSpice,
    UI_MapEditor_SpiceBloom,
    UI_MapEditor_Slab,
    UI_MapEditor_Rock,
    UI_MapEditor_Mountain,
    UI_MapEditor_Slab1,
    UI_MapEditor_Wall,
    UI_MapEditor_GunTurret,
    UI_MapEditor_RocketTurret,
    UI_MapEditor_ConstructionYard,
    UI_MapEditor_Windtrap,
    UI_MapEditor_Radar,
    UI_MapEditor_Silo,
    UI_MapEditor_IX,
    UI_MapEditor_Barracks,
    UI_MapEditor_WOR,
    UI_MapEditor_LightFactory,
    UI_MapEditor_Refinery,
    UI_MapEditor_HighTechFactory,
    UI_MapEditor_HeavyFactory,
    UI_MapEditor_RepairYard,
    UI_MapEditor_Starport,
    UI_MapEditor_Palace,
    UI_MapEditor_Soldier,
    UI_MapEditor_Trooper,
    UI_MapEditor_Harvester,
    UI_MapEditor_Infantry,
    UI_MapEditor_Troopers,
    UI_MapEditor_MCV,
    UI_MapEditor_Trike,
    UI_MapEditor_Raider,
    UI_MapEditor_Quad,
    UI_MapEditor_Tank,
    UI_MapEditor_SiegeTank,
    UI_MapEditor_Launcher,
    UI_MapEditor_Devastator,
    UI_MapEditor_SonicTank,
    UI_MapEditor_Deviator,
    UI_MapEditor_Saboteur,
    UI_MapEditor_Sandworm,
    UI_MapEditor_SpecialUnit,
    UI_MapEditor_Carryall,
    UI_MapEditor_Ornithopter,
    UI_MapEditor_Pen1x1,
    UI_MapEditor_Pen3x3,
    UI_MapEditor_Pen5x5,
    NUM_UIGRAPHICS
} UIGraphics_Enum;

//Animation
typedef enum {
    Anim_HarkonnenEyes,
    Anim_HarkonnenMouth,
    Anim_HarkonnenShoulder,
    Anim_AtreidesEyes,
    Anim_AtreidesMouth,
    Anim_AtreidesShoulder,
    Anim_AtreidesBook,
    Anim_OrdosEyes,
    Anim_OrdosMouth,
    Anim_OrdosShoulder,
    Anim_OrdosRing,
    Anim_FremenEyes,
    Anim_FremenMouth,
    Anim_FremenShoulder,
    Anim_FremenBook,
    Anim_SardaukarEyes,
    Anim_SardaukarMouth,
    Anim_SardaukarShoulder,
    Anim_MercenaryEyes,
    Anim_MercenaryMouth,
    Anim_MercenaryShoulder,
    Anim_MercenaryRing,
    Anim_BeneEyes,
    Anim_BeneMouth,
    Anim_HarkonnenPlanet,
    Anim_AtreidesPlanet,
    Anim_OrdosPlanet,
    Anim_FremenPlanet,
    Anim_SardaukarPlanet,
    Anim_MercenaryPlanet,
    Anim_Win1,
    Anim_Win2,
    Anim_Lose1,
    Anim_Lose2,
    Anim_Barracks,
    Anim_Carryall,
    Anim_ConstructionYard,
    Anim_Fremen,
    Anim_DeathHand,
    Anim_Devastator,
    Anim_Harvester,
    Anim_Radar,
    Anim_HighTechFactory,
    Anim_SiegeTank,
    Anim_HeavyFactory,
    Anim_Trooper,
    Anim_Infantry,
    Anim_IX,
    Anim_LightFactory,
    Anim_Tank,
    Anim_MCV,
    Anim_Deviator,
    Anim_Ornithopter,
    Anim_Raider,
    Anim_Palace,
    Anim_Quad,
    Anim_Refinery,
    Anim_RepairYard,
    Anim_Launcher,
    Anim_RocketTurret,
    Anim_Saboteur,
    Anim_Slab1,
    Anim_SonicTank,
    Anim_StarPort,
    Anim_Silo,
    Anim_Trike,
    Anim_GunTurret,
    Anim_Wall,
    Anim_WindTrap,
    Anim_WOR,
    Anim_Sandworm,
    Anim_Sardaukar,
    Anim_Frigate,
    Anim_Slab4,
    NUM_ANIMATION
} Animation_enum;


class GFXManager {
public:
    GFXManager();
    ~GFXManager();

    GFXManager(const GFXManager &) = delete;
    GFXManager(GFXManager &&) = default;
    GFXManager& operator=(const GFXManager &) = default;
    GFXManager& operator=(GFXManager &&) = default;

    SDL_Texture*     getZoomedObjPic(unsigned int id, int house, unsigned int z);
    SDL_Texture*     getZoomedObjPic(unsigned int id, unsigned int z) { return getZoomedObjPic(id, HOUSE_HARKONNEN, z); };
    zoomable_texture getObjPic(unsigned int id, int house=HOUSE_HARKONNEN);

    SDL_Texture*     getSmallDetailPic(unsigned int id);
    SDL_Texture*     getTinyPicture(unsigned int id);
    SDL_Texture*     getUIGraphic(unsigned int id, int house=HOUSE_HARKONNEN);
    SDL_Texture*     getMapChoicePiece(unsigned int num, int house);

    SDL_Surface*     getUIGraphicSurface(unsigned int id, int house=HOUSE_HARKONNEN);
    SDL_Surface*     getMapChoicePieceSurface(unsigned int num, int house);

    SDL_Surface*     getBackgroundSurface() { return pBackgroundSurface.get(); };

    Animation*       getAnimation(unsigned int id);

private:
    std::unique_ptr<Animation>  loadAnimationFromWsa(const std::string& filename) const;
    sdl2::surface_ptr           generateWindtrapAnimationFrames(SDL_Surface* windtrapPic) const;
    sdl2::surface_ptr           generateMapChoiceArrowFrames(SDL_Surface* arrowPic, int house=HOUSE_HARKONNEN) const;

    std::unique_ptr<Shpfile>  loadShpfile(const std::string& filename) const;
    std::unique_ptr<Wsafile>  loadWsafile(const std::string& filename) const;

    sdl2::texture_ptr   extractSmallDetailPic(const std::string& filename) const;


    sdl2::surface_ptr   generateDoubledObjPic(unsigned int id, int h) const;
    sdl2::surface_ptr   generateTripledObjPic(unsigned int id, int h) const;

    // 8-bit surfaces kept in main memory for processing as needed, e.g. color remapping
    std::array<std::array<std::array<sdl2::surface_ptr, NUM_ZOOMLEVEL>, NUM_HOUSES>, NUM_OBJPICS> objPic;
    std::array<std::array<sdl2::surface_ptr, NUM_HOUSES>, NUM_UIGRAPHICS> uiGraphic;
    std::array<std::array<sdl2::surface_ptr, NUM_HOUSES>, NUM_MAPCHOICEPIECES> mapChoicePieces;
    std::array<std::unique_ptr<Animation>, NUM_ANIMATION> animation{};

    // 32-bit surfaces
    sdl2::surface_ptr    pBackgroundSurface;

    // Textures
    std::array<std::array<std::array<sdl2::texture_ptr, NUM_ZOOMLEVEL>, NUM_HOUSES>, NUM_OBJPICS> objPicTex;
    std::array<sdl2::texture_ptr, NUM_SMALLDETAILPICS> smallDetailPicTex;
    std::array<sdl2::texture_ptr, NUM_TINYPICTURE> tinyPictureTex;
    std::array<std::array<sdl2::texture_ptr, NUM_HOUSES>, NUM_UIGRAPHICS> uiGraphicTex;
    std::array<std::array<sdl2::texture_ptr, NUM_HOUSES>, NUM_MAPCHOICEPIECES> mapChoicePiecesTex;
};

#endif // GFXMANAGER_H
