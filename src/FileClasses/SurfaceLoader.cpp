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

#include <FileClasses/SurfaceLoader.h>

#include <globals.h>

#include <FileClasses/GFXManager.h>

#include <FileClasses/Cpsfile.h>
#include <FileClasses/FileManager.h>
#include <FileClasses/Icnfile.h>
#include <FileClasses/LoadSavePNG.h>
#include <FileClasses/Palfile.h>
#include <FileClasses/PictureFactory.h>
#include <FileClasses/Shpfile.h>
#include <FileClasses/TextManager.h>
#include <FileClasses/Wsafile.h>

#include <misc/Scaler.h>
#include <misc/draw_util.h>
#include <misc/exceptions.h>

#define GROUNDUNIT_ROW(i)                                                                                              \
    (i + 2) | TILE_NORMAL, (i + 1) | TILE_NORMAL, i | TILE_NORMAL, (i + 1) | TILE_FLIPV, (i + 2) | TILE_FLIPV,         \
        (i + 3) | TILE_FLIPV, (i + 4) | TILE_NORMAL, (i + 3) | TILE_NORMAL
#define AIRUNIT_ROW(i)                                                                                                 \
    (i + 2) | TILE_NORMAL, (i + 1) | TILE_NORMAL, i | TILE_NORMAL, (i + 1) | TILE_FLIPV, (i + 2) | TILE_FLIPV,         \
        (i + 1) | TILE_ROTATE, i | TILE_FLIPH, (i + 1) | TILE_FLIPH
#define ORNITHOPTER_ROW(i)                                                                                             \
    (i + 6) | TILE_NORMAL, (i + 3) | TILE_NORMAL, i | TILE_NORMAL, (i + 3) | TILE_FLIPV, (i + 6) | TILE_FLIPV,         \
        (i + 3) | TILE_ROTATE, i | TILE_FLIPH, (i + 3) | TILE_FLIPH
#define INFANTRY_ROW(i)      (i + 3) | TILE_NORMAL, i | TILE_NORMAL, (i + 3) | TILE_FLIPV, (i + 6) | TILE_NORMAL
#define MULTIINFANTRY_ROW(i) (i + 4) | TILE_NORMAL, i | TILE_NORMAL, (i + 4) | TILE_FLIPV, (i + 8) | TILE_NORMAL
#define HARVESTERSAND_ROW(i)                                                                                           \
    (i + 6) | TILE_NORMAL, (i + 3) | TILE_NORMAL, i | TILE_NORMAL, (i + 3) | TILE_FLIPV, (i + 6) | TILE_FLIPV,         \
        (i + 9) | TILE_FLIPV, (i + 12) | TILE_NORMAL, (i + 9) | TILE_NORMAL
#define ROCKET_ROW(i)                                                                                                  \
    (i + 4) | TILE_NORMAL, (i + 3) | TILE_NORMAL, (i + 2) | TILE_NORMAL, (i + 1) | TILE_NORMAL, i | TILE_NORMAL,       \
        (i + 1) | TILE_FLIPV, (i + 2) | TILE_FLIPV, (i + 3) | TILE_FLIPV, (i + 4) | TILE_FLIPV, (i + 3) | TILE_ROTATE, \
        (i + 2) | TILE_ROTATE, (i + 1) | TILE_ROTATE, i | TILE_FLIPH, (i + 1) | TILE_FLIPH, (i + 2) | TILE_FLIPH,      \
        (i + 3) | TILE_FLIPH

namespace {
constexpr auto ObjPicNames = std::to_array<std::string_view>({"Tank_Base",
                                                              "Tank_Gun",
                                                              "Siegetank_Base",
                                                              "Siegetank_Gun",
                                                              "Devastator_Base",
                                                              "Devastator_Gun",
                                                              "Sonictank_Gun",
                                                              "Launcher_Gun",
                                                              "Quad",
                                                              "Trike",
                                                              "Harvester",
                                                              "Harvester_Sand",
                                                              "MCV",
                                                              "Carryall",
                                                              "CarryallShadow",
                                                              "Frigate",
                                                              "FrigateShadow",
                                                              "Ornithopter",
                                                              "OrnithopterShadow",
                                                              "Trooper",
                                                              "Troopers",
                                                              "Soldier",
                                                              "Infantry",
                                                              "Saboteur",
                                                              "Sandworm",
                                                              "ConstructionYard",
                                                              "Windtrap",
                                                              "Refinery",
                                                              "Barracks",
                                                              "WOR",
                                                              "Radar",
                                                              "LightFactory",
                                                              "Silo",
                                                              "HeavyFactory",
                                                              "HighTechFactory",
                                                              "IX",
                                                              "Palace",
                                                              "RepairYard",
                                                              "Starport",
                                                              "GunTurret",
                                                              "RocketTurret",
                                                              "Wall",
                                                              "Bullet_SmallRocket",
                                                              "Bullet_MediumRocket",
                                                              "Bullet_LargeRocket",
                                                              "Bullet_Small",
                                                              "Bullet_Medium",
                                                              "Bullet_Large",
                                                              "Bullet_Sonic",
                                                              "Bullet_SonicTemp",
                                                              "Hit_Gas",
                                                              "Hit_ShellSmall",
                                                              "Hit_ShellMedium",
                                                              "Hit_ShellLarge",
                                                              "ExplosionSmall",
                                                              "ExplosionMedium1",
                                                              "ExplosionMedium2",
                                                              "ExplosionLarge1",
                                                              "ExplosionLarge2",
                                                              "ExplosionSmallUnit",
                                                              "ExplosionFlames",
                                                              "ExplosionSpiceBloom",
                                                              "DeadInfantry",
                                                              "DeadAirUnit",
                                                              "Smoke",
                                                              "SandwormShimmerMask",
                                                              "SandwormShimmerTemp",
                                                              "Terrain",
                                                              "DestroyedStructure",
                                                              "RockDamage",
                                                              "SandDamage",
                                                              "Terrain_Hidden",
                                                              "Terrain_HiddenFog",
                                                              "Terrain_Tracks",
                                                              "Star"});

static_assert(std::tuple_size_v<decltype(ObjPicNames)> == NUM_OBJPICS);

/**
    Number of columns and rows each obj pic has
*/
constexpr auto objPicTiles = std::to_array<Coord>({
    {8, 1},                                     // ObjPic_Tank_Base
    {8, 1},                                     // ObjPic_Tank_Gun
    {8, 1},                                     // ObjPic_Siegetank_Base
    {8, 1},                                     // ObjPic_Siegetank_Gun
    {8, 1},                                     // ObjPic_Devastator_Base
    {8, 1},                                     // ObjPic_Devastator_Gun
    {8, 1},                                     // ObjPic_Sonictank_Gun
    {8, 1},                                     // ObjPic_Launcher_Gun
    {8, 1},                                     // ObjPic_Quad
    {8, 1},                                     // ObjPic_Trike
    {8, 1},                                     // ObjPic_Harvester
    {8, 3},                                     // ObjPic_Harvester_Sand
    {8, 1},                                     // ObjPic_MCV
    {8, 2},                                     // ObjPic_Carryall
    {8, 2},                                     // ObjPic_CarryallShadow
    {8, 1},                                     // ObjPic_Frigate
    {8, 1},                                     // ObjPic_FrigateShadow
    {8, 3},                                     // ObjPic_Ornithopter
    {8, 3},                                     // ObjPic_OrnithopterShadow
    {4, 3},                                     // ObjPic_Trooper
    {4, 3},                                     // ObjPic_Troopers
    {4, 3},                                     // ObjPic_Soldier
    {4, 3},                                     // ObjPic_Infantry
    {4, 3},                                     // ObjPic_Saboteur
    {1, 9},                                     // ObjPic_Sandworm
    {4, 1},                                     // ObjPic_ConstructionYard
    {4, 1},                                     // ObjPic_Windtrap
    {10, 1},                                    // ObjPic_Refinery
    {4, 1},                                     // ObjPic_Barracks
    {4, 1},                                     // ObjPic_WOR
    {4, 1},                                     // ObjPic_Radar
    {6, 1},                                     // ObjPic_LightFactory
    {4, 1},                                     // ObjPic_Silo
    {8, 1},                                     // ObjPic_HeavyFactory
    {8, 1},                                     // ObjPic_HighTechFactory
    {4, 1},                                     // ObjPic_IX
    {4, 1},                                     // ObjPic_Palace
    {10, 1},                                    // ObjPic_RepairYard
    {10, 1},                                    // ObjPic_Starport
    {10, 1},                                    // ObjPic_GunTurret
    {10, 1},                                    // ObjPic_RocketTurret
    {25, 3},                                    // ObjPic_Wall
    {16, 1},                                    // ObjPic_Bullet_SmallRocket
    {16, 1},                                    // ObjPic_Bullet_MediumRocket
    {16, 1},                                    // ObjPic_Bullet_LargeRocket
    {1, 1},                                     // ObjPic_Bullet_Small
    {1, 1},                                     // ObjPic_Bullet_Medium
    {1, 1},                                     // ObjPic_Bullet_Large
    {1, 1},                                     // ObjPic_Bullet_Sonic
    {1, 1},                                     // ObjPic_Bullet_SonicTemp
    {5, 1},                                     // ObjPic_Hit_Gas
    {1, 1},                                     // ObjPic_Hit_ShellSmall
    {1, 1},                                     // ObjPic_Hit_ShellMedium
    {1, 1},                                     // ObjPic_Hit_ShellLarge
    {5, 1},                                     // ObjPic_ExplosionSmall
    {5, 1},                                     // ObjPic_ExplosionMedium1
    {5, 1},                                     // ObjPic_ExplosionMedium2
    {5, 1},                                     // ObjPic_ExplosionLarge1
    {5, 1},                                     // ObjPic_ExplosionLarge2
    {2, 1},                                     // ObjPic_ExplosionSmallUnit
    {21, 1},                                    // ObjPic_ExplosionFlames
    {3, 1},                                     // ObjPic_ExplosionSpiceBloom
    {6, 1},                                     // ObjPic_DeadInfantry
    {6, 1},                                     // ObjPic_DeadAirUnit
    {3, 1},                                     // ObjPic_Smoke
    {1, 1},                                     // ObjPic_SandwormShimmerMask
    {1, 1},                                     // ObjPic_SandwormShimmerTemp
    {NUM_TERRAIN_TILES_X, NUM_TERRAIN_TILES_Y}, // ObjPic_Terrain
    {14, 1},                                    // ObjPic_DestroyedStructure
    {6, 1},                                     // ObjPic_RockDamage
    {3, 1},                                     // ObjPic_SandDamage
    {16, 1},                                    // ObjPic_Terrain_Hidden
    {16, 1},                                    // ObjPic_Terrain_HiddenFog
    {8, 1},                                     // ObjPic_Terrain_Tracks
    {1, 1},                                     // ObjPic_Star
});

static_assert(std::tuple_size_v<decltype(objPicTiles)> == NUM_OBJPICS);

} // namespace

SurfaceLoader::SurfaceLoader() {

    const auto start = std::chrono::steady_clock::now();

    auto* const file_manager = dune::globals::pFileManager.get();

    // open all shp files
    auto units   = loadShpfile("UNITS.SHP");
    auto units1  = loadShpfile("UNITS1.SHP");
    auto units2  = loadShpfile("UNITS2.SHP");
    auto mouse   = loadShpfile("MOUSE.SHP");
    auto shapes  = loadShpfile("SHAPES.SHP");
    auto menshpa = loadShpfile("MENSHPA.SHP");
    auto menshph = loadShpfile("MENSHPH.SHP");
    auto menshpo = loadShpfile("MENSHPO.SHP");
    auto menshpm = loadShpfile("MENSHPM.SHP");

    std::unique_ptr<Shpfile> choam;

    if (file_manager->exists("CHOAM." + _("LanguageFileExtension"))) {
        choam = loadShpfile("CHOAM." + _("LanguageFileExtension"));
    } else if (file_manager->exists("CHOAMSHP.SHP")) {
        choam = loadShpfile("CHOAMSHP.SHP");
    } else {
        THROW(std::runtime_error,
              "SurfaceLoader::SurfaceLoader(): Cannot open CHOAMSHP.SHP or CHOAM." + _("LanguageFileExtension") + "!");
    }

    std::unique_ptr<Shpfile> bttn;
    if (file_manager->exists("BTTN." + _("LanguageFileExtension"))) {
        bttn = loadShpfile("BTTN." + _("LanguageFileExtension"));
    } else {
        // The US-Version has the buttons in SHAPES.SHP
        // => bttn == nullptr
    }

    std::unique_ptr<Shpfile> mentat;
    if (file_manager->exists("MENTAT." + _("LanguageFileExtension"))) {
        mentat = loadShpfile("MENTAT." + _("LanguageFileExtension"));
    } else {
        mentat = loadShpfile("MENTAT.SHP");
    }

    auto pieces = loadShpfile("PIECES.SHP");
    auto arrows = loadShpfile("ARROWS.SHP");

    // Load icon file
    auto icon =
        std::make_unique<Icnfile>(file_manager->openFile("ICON.ICN").get(), file_manager->openFile("ICON.MAP").get());

    // Load radar static
    auto radar = loadWsafile("STATIC.WSA");

    // open bene palette
    Palette benePalette = LoadPalette_RW(file_manager->openFile("BENE.PAL").get());

    const auto elapsed = std::chrono::steady_clock::now() - start;
    sdl2::log_info("SurfaceLoader load time: %s",
                   std::to_string(std::chrono::duration<double>(elapsed).count()).c_str());

    constexpr auto harkIdx = static_cast<int>(HOUSETYPE::HOUSE_HARKONNEN);

    // load object pics in the original resolution
    objPic[ObjPic_Tank_Base][static_cast<int>(harkIdx)][0] = units2->getPictureArray(8, 1, GROUNDUNIT_ROW(0));
    objPic[ObjPic_Tank_Gun][harkIdx][0]                    = units2->getPictureArray(8, 1, GROUNDUNIT_ROW(5));
    objPic[ObjPic_Siegetank_Base][harkIdx][0]              = units2->getPictureArray(8, 1, GROUNDUNIT_ROW(10));
    objPic[ObjPic_Siegetank_Gun][harkIdx][0]               = units2->getPictureArray(8, 1, GROUNDUNIT_ROW(15));
    objPic[ObjPic_Devastator_Base][harkIdx][0]             = units2->getPictureArray(8, 1, GROUNDUNIT_ROW(20));
    objPic[ObjPic_Devastator_Gun][harkIdx][0]              = units2->getPictureArray(8, 1, GROUNDUNIT_ROW(25));
    objPic[ObjPic_Sonictank_Gun][harkIdx][0]               = units2->getPictureArray(8, 1, GROUNDUNIT_ROW(30));
    objPic[ObjPic_Launcher_Gun][harkIdx][0]                = units2->getPictureArray(8, 1, GROUNDUNIT_ROW(35));
    objPic[ObjPic_Quad][harkIdx][0]                        = units->getPictureArray(8, 1, GROUNDUNIT_ROW(0));
    objPic[ObjPic_Trike][harkIdx][0]                       = units->getPictureArray(8, 1, GROUNDUNIT_ROW(5));
    objPic[ObjPic_Harvester][harkIdx][0]                   = units->getPictureArray(8, 1, GROUNDUNIT_ROW(10));
    { // Scope
        SDL_Color shadowTransparent = {0, 0, 0, 96};
        SDL_SetPaletteColors(objPic[ObjPic_Harvester][harkIdx][0]->format->palette, &shadowTransparent, PALCOLOR_SHADOW,
                             1);
        SDL_SetSurfaceBlendMode(objPic[ObjPic_Harvester][harkIdx][0].get(), SDL_BlendMode::SDL_BLENDMODE_BLEND);
    }

    objPic[ObjPic_Harvester_Sand][harkIdx][0] =
        units1->getPictureArray(8, 3, HARVESTERSAND_ROW(72), HARVESTERSAND_ROW(73), HARVESTERSAND_ROW(74));
    objPic[ObjPic_MCV][harkIdx][0] = units->getPictureArray(8, 1, GROUNDUNIT_ROW(15));
    { // Scope
        SDL_Color shadowTransparent = {0, 0, 0, 96};
        SDL_SetPaletteColors(objPic[ObjPic_MCV][harkIdx][0]->format->palette, &shadowTransparent, PALCOLOR_SHADOW, 1);
        SDL_SetSurfaceBlendMode(objPic[ObjPic_MCV][harkIdx][0].get(), SDL_BlendMode::SDL_BLENDMODE_BLEND);
    }
    objPic[ObjPic_Carryall][harkIdx][0]       = units->getPictureArray(8, 2, AIRUNIT_ROW(45), AIRUNIT_ROW(48));
    objPic[ObjPic_CarryallShadow][harkIdx][0] = nullptr; // create shadow after scaling
    objPic[ObjPic_Frigate][harkIdx][0]        = units->getPictureArray(8, 1, AIRUNIT_ROW(60));
    objPic[ObjPic_FrigateShadow][harkIdx][0]  = nullptr; // create shadow after scaling
    objPic[ObjPic_Ornithopter][harkIdx][0] =
        units->getPictureArray(8, 3, ORNITHOPTER_ROW(51), ORNITHOPTER_ROW(52), ORNITHOPTER_ROW(53));
    objPic[ObjPic_OrnithopterShadow][harkIdx][0] = nullptr; // create shadow after scaling
    objPic[ObjPic_Trooper][harkIdx][0] =
        units->getPictureArray(4, 3, INFANTRY_ROW(82), INFANTRY_ROW(83), INFANTRY_ROW(84));
    objPic[ObjPic_Troopers][harkIdx][0] = units->getPictureArray(4, 4, MULTIINFANTRY_ROW(103), MULTIINFANTRY_ROW(104),
                                                                 MULTIINFANTRY_ROW(105), MULTIINFANTRY_ROW(106));
    objPic[ObjPic_Soldier][harkIdx][0] =
        units->getPictureArray(4, 3, INFANTRY_ROW(73), INFANTRY_ROW(74), INFANTRY_ROW(75));
    objPic[ObjPic_Infantry][harkIdx][0] = units->getPictureArray(4, 4, MULTIINFANTRY_ROW(91), MULTIINFANTRY_ROW(92),
                                                                 MULTIINFANTRY_ROW(93), MULTIINFANTRY_ROW(94));
    objPic[ObjPic_Saboteur][harkIdx][0] =
        units->getPictureArray(4, 3, INFANTRY_ROW(63), INFANTRY_ROW(64), INFANTRY_ROW(65));
    objPic[ObjPic_Sandworm][harkIdx][0] = units1->getPictureArray(
        1, 9, 71 | TILE_NORMAL, 70 | TILE_NORMAL, 69 | TILE_NORMAL, 68 | TILE_NORMAL, 67 | TILE_NORMAL,
        68 | TILE_NORMAL, 69 | TILE_NORMAL, 70 | TILE_NORMAL, 71 | TILE_NORMAL);
    objPic[ObjPic_ConstructionYard][harkIdx][0]    = icon->getPictureArray(17);
    objPic[ObjPic_Windtrap][harkIdx][0]            = icon->getPictureArray(19);
    objPic[ObjPic_Refinery][harkIdx][0]            = icon->getPictureArray(21);
    objPic[ObjPic_Barracks][harkIdx][0]            = icon->getPictureArray(18);
    objPic[ObjPic_WOR][harkIdx][0]                 = icon->getPictureArray(16);
    objPic[ObjPic_Radar][harkIdx][0]               = icon->getPictureArray(26);
    objPic[ObjPic_LightFactory][harkIdx][0]        = icon->getPictureArray(12);
    objPic[ObjPic_Silo][harkIdx][0]                = icon->getPictureArray(25);
    objPic[ObjPic_HeavyFactory][harkIdx][0]        = icon->getPictureArray(13);
    objPic[ObjPic_HighTechFactory][harkIdx][0]     = icon->getPictureArray(14);
    objPic[ObjPic_IX][harkIdx][0]                  = icon->getPictureArray(15);
    objPic[ObjPic_Palace][harkIdx][0]              = icon->getPictureArray(11);
    objPic[ObjPic_RepairYard][harkIdx][0]          = icon->getPictureArray(22);
    objPic[ObjPic_Starport][harkIdx][0]            = icon->getPictureArray(20);
    objPic[ObjPic_GunTurret][harkIdx][0]           = icon->getPictureArray(23);
    objPic[ObjPic_RocketTurret][harkIdx][0]        = icon->getPictureArray(24);
    objPic[ObjPic_Wall][harkIdx][0]                = icon->getPictureArray(6, 25, 3, 1);
    objPic[ObjPic_Bullet_SmallRocket][harkIdx][0]  = units->getPictureArray(16, 1, ROCKET_ROW(35));
    objPic[ObjPic_Bullet_MediumRocket][harkIdx][0] = units->getPictureArray(16, 1, ROCKET_ROW(20));
    objPic[ObjPic_Bullet_LargeRocket][harkIdx][0]  = units->getPictureArray(16, 1, ROCKET_ROW(40));
    objPic[ObjPic_Bullet_Small][harkIdx][0]        = units1->getPicture(23);
    objPic[ObjPic_Bullet_Medium][harkIdx][0]       = units1->getPicture(24);
    objPic[ObjPic_Bullet_Large][harkIdx][0]        = units1->getPicture(25);
    objPic[ObjPic_Bullet_Sonic][harkIdx][0]        = units1->getPicture(10);
    replaceColor(objPic[ObjPic_Bullet_Sonic][harkIdx][0].get(), PALCOLOR_WHITE, PALCOLOR_BLACK);
    objPic[ObjPic_Bullet_SonicTemp][harkIdx][0]                         = units1->getPicture(10);
    objPic[ObjPic_Hit_Gas][static_cast<int>(HOUSETYPE::HOUSE_ORDOS)][0] = units1->getPictureArray(
        5, 1, 57 | TILE_NORMAL, 58 | TILE_NORMAL, 59 | TILE_NORMAL, 60 | TILE_NORMAL, 61 | TILE_NORMAL);
    objPic[ObjPic_Hit_Gas][harkIdx][0] = mapSurfaceColorRange(
        objPic[ObjPic_Hit_Gas][static_cast<int>(HOUSETYPE::HOUSE_ORDOS)][0].get(), PALCOLOR_ORDOS, PALCOLOR_HARKONNEN);
    objPic[ObjPic_Hit_ShellSmall][harkIdx][0]  = units1->getPicture(2);
    objPic[ObjPic_Hit_ShellMedium][harkIdx][0] = units1->getPicture(3);
    objPic[ObjPic_Hit_ShellLarge][harkIdx][0]  = units1->getPicture(4);
    objPic[ObjPic_ExplosionSmall][harkIdx][0]  = units1->getPictureArray(
         5, 1, 32 | TILE_NORMAL, 33 | TILE_NORMAL, 34 | TILE_NORMAL, 35 | TILE_NORMAL, 36 | TILE_NORMAL);
    objPic[ObjPic_ExplosionMedium1][harkIdx][0] = units1->getPictureArray(
        5, 1, 47 | TILE_NORMAL, 48 | TILE_NORMAL, 49 | TILE_NORMAL, 50 | TILE_NORMAL, 51 | TILE_NORMAL);
    objPic[ObjPic_ExplosionMedium2][harkIdx][0] = units1->getPictureArray(
        5, 1, 52 | TILE_NORMAL, 53 | TILE_NORMAL, 54 | TILE_NORMAL, 55 | TILE_NORMAL, 56 | TILE_NORMAL);
    objPic[ObjPic_ExplosionLarge1][harkIdx][0] = units1->getPictureArray(
        5, 1, 37 | TILE_NORMAL, 38 | TILE_NORMAL, 39 | TILE_NORMAL, 40 | TILE_NORMAL, 41 | TILE_NORMAL);
    objPic[ObjPic_ExplosionLarge2][harkIdx][0] = units1->getPictureArray(
        5, 1, 42 | TILE_NORMAL, 43 | TILE_NORMAL, 44 | TILE_NORMAL, 45 | TILE_NORMAL, 46 | TILE_NORMAL);
    objPic[ObjPic_ExplosionSmallUnit][harkIdx][0] = units1->getPictureArray(2, 1, 0 | TILE_NORMAL, 1 | TILE_NORMAL);
    objPic[ObjPic_ExplosionFlames][harkIdx][0]    = units1->getPictureArray(
           21, 1, 11 | TILE_NORMAL, 12 | TILE_NORMAL, 13 | TILE_NORMAL, 17 | TILE_NORMAL, 18 | TILE_NORMAL,
           19 | TILE_NORMAL, 17 | TILE_NORMAL, 18 | TILE_NORMAL, 19 | TILE_NORMAL, 17 | TILE_NORMAL, 18 | TILE_NORMAL,
           19 | TILE_NORMAL, 17 | TILE_NORMAL, 18 | TILE_NORMAL, 19 | TILE_NORMAL, 17 | TILE_NORMAL, 18 | TILE_NORMAL,
           19 | TILE_NORMAL, 20 | TILE_NORMAL, 21 | TILE_NORMAL, 22 | TILE_NORMAL);
    objPic[ObjPic_ExplosionSpiceBloom][harkIdx][0] =
        units1->getPictureArray(3, 1, 7 | TILE_NORMAL, 6 | TILE_NORMAL, 5 | TILE_NORMAL);
    objPic[ObjPic_DeadInfantry][harkIdx][0] = icon->getPictureArray(4, 1, 1, 6);
    objPic[ObjPic_DeadAirUnit][harkIdx][0]  = icon->getPictureArray(3, 1, 1, 6);
    objPic[ObjPic_Smoke][harkIdx][0] =
        units1->getPictureArray(3, 1, 29 | TILE_NORMAL, 30 | TILE_NORMAL, 31 | TILE_NORMAL);
    objPic[ObjPic_SandwormShimmerMask][harkIdx][0] = units1->getPicture(10);
    replaceColor(objPic[ObjPic_SandwormShimmerMask][harkIdx][0].get(), PALCOLOR_WHITE, PALCOLOR_BLACK);
    objPic[ObjPic_SandwormShimmerTemp][harkIdx][0] = units1->getPicture(10);
    objPic[ObjPic_Terrain][harkIdx][0]             = icon->getPictureRow(124, 209, NUM_TERRAIN_TILES_X);
    objPic[ObjPic_DestroyedStructure][harkIdx][0] =
        icon->getPictureRow2(14, 33, 125, 213, 214, 215, 223, 224, 225, 232, 233, 234, 240, 246, 247);
    objPic[ObjPic_RockDamage][harkIdx][0]        = icon->getPictureRow(1, 6);
    objPic[ObjPic_SandDamage][harkIdx][0]        = icon->getPictureRow(7, 12);
    objPic[ObjPic_Terrain_Hidden][harkIdx][0]    = icon->getPictureRow(108, 123);
    objPic[ObjPic_Terrain_HiddenFog][harkIdx][0] = icon->getPictureRow(108, 123);
    objPic[ObjPic_Terrain_Tracks][harkIdx][0]    = icon->getPictureRow(25, 32);
    objPic[ObjPic_Star][harkIdx][0]              = LoadPNG_RW(file_manager->openFile("Star5x5.png").get());
    objPic[ObjPic_Star][harkIdx][1]              = LoadPNG_RW(file_manager->openFile("Star7x7.png").get());
    objPic[ObjPic_Star][harkIdx][2]              = LoadPNG_RW(file_manager->openFile("Star11x11.png").get());

    SDL_Color fogTransparent = {0, 0, 0, 96};
    SDL_SetPaletteColors(objPic[ObjPic_Terrain_HiddenFog][harkIdx][0]->format->palette, &fogTransparent, PALCOLOR_BLACK,
                         1);

    // scale obj pics and apply color key
    for (int id = 0; id < NUM_OBJPICS; id++) {
        for (int h = 0; h < static_cast<int>(HOUSETYPE::NUM_HOUSES); h++) {
            if (objPic[id][h][0] != nullptr) {
                if (objPic[id][h][1] == nullptr) {
                    objPic[id][h][1] = generateDoubledObjPic(id, h);
                }
                SDL_SetColorKey(objPic[id][h][1].get(), SDL_TRUE, PALCOLOR_TRANSPARENT);

                if (objPic[id][h][2] == nullptr) {
                    objPic[id][h][2] = generateTripledObjPic(id, h);
                }
                SDL_SetColorKey(objPic[id][h][2].get(), SDL_TRUE, PALCOLOR_TRANSPARENT);

                SDL_SetColorKey(objPic[id][h][0].get(), SDL_TRUE, PALCOLOR_TRANSPARENT);
            }
        }
    }

    objPic[ObjPic_CarryallShadow][harkIdx][0]    = createShadowSurface(objPic[ObjPic_Carryall][harkIdx][0].get());
    objPic[ObjPic_CarryallShadow][harkIdx][1]    = createShadowSurface(objPic[ObjPic_Carryall][harkIdx][1].get());
    objPic[ObjPic_CarryallShadow][harkIdx][2]    = createShadowSurface(objPic[ObjPic_Carryall][harkIdx][2].get());
    objPic[ObjPic_FrigateShadow][harkIdx][0]     = createShadowSurface(objPic[ObjPic_Frigate][harkIdx][0].get());
    objPic[ObjPic_FrigateShadow][harkIdx][1]     = createShadowSurface(objPic[ObjPic_Frigate][harkIdx][1].get());
    objPic[ObjPic_FrigateShadow][harkIdx][2]     = createShadowSurface(objPic[ObjPic_Frigate][harkIdx][2].get());
    objPic[ObjPic_OrnithopterShadow][harkIdx][0] = createShadowSurface(objPic[ObjPic_Ornithopter][harkIdx][0].get());
    objPic[ObjPic_OrnithopterShadow][harkIdx][1] = createShadowSurface(objPic[ObjPic_Ornithopter][harkIdx][1].get());
    objPic[ObjPic_OrnithopterShadow][harkIdx][2] = createShadowSurface(objPic[ObjPic_Ornithopter][harkIdx][2].get());

    // load small detail pics
    smallDetailPic[Picture_Barracks]         = extractSmallDetailPic("BARRAC.WSA");
    smallDetailPic[Picture_ConstructionYard] = extractSmallDetailPic("CONSTRUC.WSA");
    smallDetailPic[Picture_Carryall]         = extractSmallDetailPic("CARRYALL.WSA");
    smallDetailPic[Picture_Devastator]       = extractSmallDetailPic("HARKTANK.WSA");
    smallDetailPic[Picture_Deviator]         = extractSmallDetailPic("ORDRTANK.WSA");
    smallDetailPic[Picture_DeathHand]        = extractSmallDetailPic("GOLD-BB.WSA");
    smallDetailPic[Picture_Fremen]           = extractSmallDetailPic("FREMEN.WSA");
    if (file_manager->exists("FRIGATE.WSA")) {
        smallDetailPic[Picture_Frigate] = extractSmallDetailPic("FRIGATE.WSA");
    } else {
        // US-Version 1.07 does not contain FRIGATE.WSA
        // We replace it with the starport
        smallDetailPic[Picture_Frigate] = extractSmallDetailPic("STARPORT.WSA");
    }
    smallDetailPic[Picture_GunTurret]       = extractSmallDetailPic("TURRET.WSA");
    smallDetailPic[Picture_Harvester]       = extractSmallDetailPic("HARVEST.WSA");
    smallDetailPic[Picture_HeavyFactory]    = extractSmallDetailPic("HVYFTRY.WSA");
    smallDetailPic[Picture_HighTechFactory] = extractSmallDetailPic("HITCFTRY.WSA");
    smallDetailPic[Picture_Soldier]         = extractSmallDetailPic("INFANTRY.WSA");
    smallDetailPic[Picture_IX]              = extractSmallDetailPic("IX.WSA");
    smallDetailPic[Picture_Launcher]        = extractSmallDetailPic("RTANK.WSA");
    smallDetailPic[Picture_LightFactory]    = extractSmallDetailPic("LITEFTRY.WSA");
    smallDetailPic[Picture_MCV]             = extractSmallDetailPic("MCV.WSA");
    smallDetailPic[Picture_Ornithopter]     = extractSmallDetailPic("ORNI.WSA");
    smallDetailPic[Picture_Palace]          = extractSmallDetailPic("PALACE.WSA");
    smallDetailPic[Picture_Quad]            = extractSmallDetailPic("QUAD.WSA");
    smallDetailPic[Picture_Radar]           = extractSmallDetailPic("HEADQRTS.WSA");
    smallDetailPic[Picture_RaiderTrike]     = extractSmallDetailPic("OTRIKE.WSA");
    smallDetailPic[Picture_Refinery]        = extractSmallDetailPic("REFINERY.WSA");
    smallDetailPic[Picture_RepairYard]      = extractSmallDetailPic("REPAIR.WSA");
    smallDetailPic[Picture_RocketTurret]    = extractSmallDetailPic("RTURRET.WSA");
    smallDetailPic[Picture_Saboteur]        = extractSmallDetailPic("SABOTURE.WSA");
    smallDetailPic[Picture_Sandworm]        = extractSmallDetailPic("WORM.WSA");
    smallDetailPic[Picture_Sardaukar]       = extractSmallDetailPic("SARDUKAR.WSA");
    smallDetailPic[Picture_SiegeTank]       = extractSmallDetailPic("HTANK.WSA");
    smallDetailPic[Picture_Silo]            = extractSmallDetailPic("STORAGE.WSA");
    smallDetailPic[Picture_Slab1]           = extractSmallDetailPic("SLAB.WSA");
    smallDetailPic[Picture_Slab4]           = extractSmallDetailPic("4SLAB.WSA");
    smallDetailPic[Picture_SonicTank]       = extractSmallDetailPic("STANK.WSA");
    smallDetailPic[Picture_Special]         = nullptr;
    smallDetailPic[Picture_StarPort]        = extractSmallDetailPic("STARPORT.WSA");
    smallDetailPic[Picture_Tank]            = extractSmallDetailPic("LTANK.WSA");
    smallDetailPic[Picture_Trike]           = extractSmallDetailPic("TRIKE.WSA");
    smallDetailPic[Picture_Trooper]         = extractSmallDetailPic("HYINFY.WSA");
    smallDetailPic[Picture_Wall]            = extractSmallDetailPic("WALL.WSA");
    smallDetailPic[Picture_WindTrap]        = extractSmallDetailPic("WINDTRAP.WSA");
    smallDetailPic[Picture_WOR]             = extractSmallDetailPic("WOR.WSA");
    // unused: FARTR.WSA, FHARK.WSA, FORDOS.WSA

    tinyPicture[TinyPicture_Spice]            = shapes->getPicture(94);
    tinyPicture[TinyPicture_Barracks]         = shapes->getPicture(62);
    tinyPicture[TinyPicture_ConstructionYard] = shapes->getPicture(60);
    tinyPicture[TinyPicture_GunTurret]        = shapes->getPicture(67);
    tinyPicture[TinyPicture_HeavyFactory]     = shapes->getPicture(56);
    tinyPicture[TinyPicture_HighTechFactory]  = shapes->getPicture(57);
    tinyPicture[TinyPicture_IX]               = shapes->getPicture(58);
    tinyPicture[TinyPicture_LightFactory]     = shapes->getPicture(55);
    tinyPicture[TinyPicture_Palace]           = shapes->getPicture(54);
    tinyPicture[TinyPicture_Radar]            = shapes->getPicture(70);
    tinyPicture[TinyPicture_Refinery]         = shapes->getPicture(64);
    tinyPicture[TinyPicture_RepairYard]       = shapes->getPicture(65);
    tinyPicture[TinyPicture_RocketTurret]     = shapes->getPicture(68);
    tinyPicture[TinyPicture_Silo]             = shapes->getPicture(69);
    tinyPicture[TinyPicture_Slab1]            = shapes->getPicture(53);
    tinyPicture[TinyPicture_Slab4]            = shapes->getPicture(71);
    tinyPicture[TinyPicture_StarPort]         = shapes->getPicture(63);
    tinyPicture[TinyPicture_Wall]             = shapes->getPicture(66);
    tinyPicture[TinyPicture_WindTrap]         = shapes->getPicture(61);
    tinyPicture[TinyPicture_WOR]              = shapes->getPicture(59);
    tinyPicture[TinyPicture_Carryall]         = shapes->getPicture(77);
    tinyPicture[TinyPicture_Devastator]       = shapes->getPicture(75);
    tinyPicture[TinyPicture_Deviator]         = shapes->getPicture(86);
    tinyPicture[TinyPicture_Frigate]          = shapes->getPicture(77); // use carryall picture
    tinyPicture[TinyPicture_Harvester]        = shapes->getPicture(88);
    tinyPicture[TinyPicture_Soldier]          = shapes->getPicture(90);
    tinyPicture[TinyPicture_Launcher]         = shapes->getPicture(73);
    tinyPicture[TinyPicture_MCV]              = shapes->getPicture(89);
    tinyPicture[TinyPicture_Ornithopter]      = shapes->getPicture(85);
    tinyPicture[TinyPicture_Quad]             = shapes->getPicture(74);
    tinyPicture[TinyPicture_Saboteur]         = shapes->getPicture(84);
    tinyPicture[TinyPicture_Sandworm]         = shapes->getPicture(93);
    tinyPicture[TinyPicture_SiegeTank]        = shapes->getPicture(72);
    tinyPicture[TinyPicture_SonicTank]        = shapes->getPicture(79);
    tinyPicture[TinyPicture_Tank]             = shapes->getPicture(78);
    tinyPicture[TinyPicture_Trike]            = shapes->getPicture(80);
    tinyPicture[TinyPicture_RaiderTrike]      = shapes->getPicture(87);
    tinyPicture[TinyPicture_Trooper]          = shapes->getPicture(76);
    tinyPicture[TinyPicture_Special]          = shapes->getPicture(75); // use devastator picture
    tinyPicture[TinyPicture_Infantry]         = shapes->getPicture(81);
    tinyPicture[TinyPicture_Troopers]         = shapes->getPicture(91);

    // load UI graphics
    uiGraphic[UI_RadarAnimation][harkIdx] =
        Scaler::doubleSurfaceNN(radar->getAnimationAsPictureRow(NUM_STATIC_ANIMATIONS_PER_ROW).get());

    uiGraphic[UI_CursorNormal][harkIdx] = mouse->getPicture(0);
    SDL_SetColorKey(uiGraphic[UI_CursorNormal][harkIdx].get(), SDL_TRUE, 0);
    uiGraphic[UI_CursorUp][harkIdx] = mouse->getPicture(1);
    SDL_SetColorKey(uiGraphic[UI_CursorUp][harkIdx].get(), SDL_TRUE, 0);
    uiGraphic[UI_CursorRight][harkIdx] = mouse->getPicture(2);
    SDL_SetColorKey(uiGraphic[UI_CursorRight][harkIdx].get(), SDL_TRUE, 0);
    uiGraphic[UI_CursorDown][harkIdx] = mouse->getPicture(3);
    SDL_SetColorKey(uiGraphic[UI_CursorDown][harkIdx].get(), SDL_TRUE, 0);
    uiGraphic[UI_CursorLeft][harkIdx] = mouse->getPicture(4);
    SDL_SetColorKey(uiGraphic[UI_CursorLeft][harkIdx].get(), SDL_TRUE, 0);

    uiGraphic[UI_CursorMove_Zoomlevel0][harkIdx] = mouse->getPicture(5);
    SDL_SetColorKey(uiGraphic[UI_CursorMove_Zoomlevel0][harkIdx].get(), SDL_TRUE, 0);
    uiGraphic[UI_CursorMove_Zoomlevel1][harkIdx] =
        Scaler::defaultDoubleTiledSurface(uiGraphic[UI_CursorMove_Zoomlevel0][harkIdx].get(), 1, 1);
    SDL_SetColorKey(uiGraphic[UI_CursorMove_Zoomlevel1][harkIdx].get(), SDL_TRUE, 0);
    uiGraphic[UI_CursorMove_Zoomlevel2][harkIdx] =
        Scaler::defaultTripleTiledSurface(uiGraphic[UI_CursorMove_Zoomlevel0][harkIdx].get(), 1, 1);
    SDL_SetColorKey(uiGraphic[UI_CursorMove_Zoomlevel2][harkIdx].get(), SDL_TRUE, 0);

    uiGraphic[UI_CursorAttack_Zoomlevel0][harkIdx] =
        mapSurfaceColorRange(uiGraphic[UI_CursorMove_Zoomlevel0][harkIdx].get(), 232, PALCOLOR_HARKONNEN);
    SDL_SetColorKey(uiGraphic[UI_CursorAttack_Zoomlevel0][harkIdx].get(), SDL_TRUE, 0);
    uiGraphic[UI_CursorAttack_Zoomlevel1][harkIdx] =
        Scaler::defaultDoubleTiledSurface(uiGraphic[UI_CursorAttack_Zoomlevel0][harkIdx].get(), 1, 1);
    SDL_SetColorKey(uiGraphic[UI_CursorAttack_Zoomlevel1][harkIdx].get(), SDL_TRUE, 0);
    uiGraphic[UI_CursorAttack_Zoomlevel2][harkIdx] =
        Scaler::defaultTripleTiledSurface(uiGraphic[UI_CursorAttack_Zoomlevel0][harkIdx].get(), 1, 1);
    SDL_SetColorKey(uiGraphic[UI_CursorAttack_Zoomlevel2][harkIdx].get(), SDL_TRUE, 0);

    uiGraphic[UI_CursorCapture_Zoomlevel0][harkIdx] = LoadPNG_RW(file_manager->openFile("Capture.png").get());
    SDL_SetColorKey(uiGraphic[UI_CursorCapture_Zoomlevel0][harkIdx].get(), SDL_TRUE, 0);
    uiGraphic[UI_CursorCapture_Zoomlevel1][harkIdx] =
        Scaler::defaultDoubleTiledSurface(uiGraphic[UI_CursorCapture_Zoomlevel0][harkIdx].get(), 1, 1);
    SDL_SetColorKey(uiGraphic[UI_CursorCapture_Zoomlevel1][harkIdx].get(), SDL_TRUE, 0);
    uiGraphic[UI_CursorCapture_Zoomlevel2][harkIdx] =
        Scaler::defaultTripleTiledSurface(uiGraphic[UI_CursorCapture_Zoomlevel0][harkIdx].get(), 1, 1);
    SDL_SetColorKey(uiGraphic[UI_CursorCapture_Zoomlevel2][harkIdx].get(), SDL_TRUE, 0);

    uiGraphic[UI_CursorCarryallDrop_Zoomlevel0][harkIdx] = LoadPNG_RW(file_manager->openFile("CarryallDrop.png").get());
    SDL_SetColorKey(uiGraphic[UI_CursorCarryallDrop_Zoomlevel0][harkIdx].get(), SDL_TRUE, 0);
    uiGraphic[UI_CursorCarryallDrop_Zoomlevel1][harkIdx] =
        Scaler::defaultDoubleTiledSurface(uiGraphic[UI_CursorCarryallDrop_Zoomlevel0][harkIdx].get(), 1, 1);
    SDL_SetColorKey(uiGraphic[UI_CursorCarryallDrop_Zoomlevel1][harkIdx].get(), SDL_TRUE, 0);
    uiGraphic[UI_CursorCarryallDrop_Zoomlevel2][harkIdx] =
        Scaler::defaultTripleTiledSurface(uiGraphic[UI_CursorCarryallDrop_Zoomlevel0][harkIdx].get(), 1, 1);
    SDL_SetColorKey(uiGraphic[UI_CursorCarryallDrop_Zoomlevel2][harkIdx].get(), SDL_TRUE, 0);

    uiGraphic[UI_ReturnIcon][harkIdx] = LoadPNG_RW(file_manager->openFile("Return.png").get());
    SDL_SetColorKey(uiGraphic[UI_ReturnIcon][harkIdx].get(), SDL_TRUE, 0);

    uiGraphic[UI_DeployIcon][harkIdx] = LoadPNG_RW(file_manager->openFile("Deploy.png").get());
    SDL_SetColorKey(uiGraphic[UI_DeployIcon][harkIdx].get(), SDL_TRUE, 0);

    uiGraphic[UI_DestructIcon][harkIdx] = LoadPNG_RW(file_manager->openFile("Destruct.png").get());
    SDL_SetColorKey(uiGraphic[UI_DestructIcon][harkIdx].get(), SDL_TRUE, 0);

    uiGraphic[UI_SendToRepairIcon][harkIdx] = LoadPNG_RW(file_manager->openFile("SendToRepair.png").get());
    SDL_SetColorKey(uiGraphic[UI_SendToRepairIcon][harkIdx].get(), SDL_TRUE, 0);

    uiGraphic[UI_CreditsDigits][harkIdx] = shapes->getPictureArray(
        10, 1, 2 | TILE_NORMAL, 3 | TILE_NORMAL, 4 | TILE_NORMAL, 5 | TILE_NORMAL, 6 | TILE_NORMAL, 7 | TILE_NORMAL,
        8 | TILE_NORMAL, 9 | TILE_NORMAL, 10 | TILE_NORMAL, 11 | TILE_NORMAL);
    uiGraphic[UI_SideBar][harkIdx] = picFactory_.createSideBar(false);
    uiGraphic[UI_Indicator][harkIdx] =
        units1->getPictureArray(3, 1, 8 | TILE_NORMAL, 9 | TILE_NORMAL, 10 | TILE_NORMAL);
    SDL_SetColorKey(uiGraphic[UI_Indicator][harkIdx].get(), SDL_TRUE, 0);
    SDL_Color indicatorTransparent = {255, 255, 255, 48};
    SDL_SetPaletteColors(uiGraphic[UI_Indicator][harkIdx]->format->palette, &indicatorTransparent, PALCOLOR_WHITE, 1);
    uiGraphic[UI_InvalidPlace_Zoomlevel0][harkIdx] = PictureFactory::createPlacingGrid(16, PALCOLOR_LIGHTRED);
    uiGraphic[UI_InvalidPlace_Zoomlevel1][harkIdx] = PictureFactory::createPlacingGrid(32, PALCOLOR_LIGHTRED);
    uiGraphic[UI_InvalidPlace_Zoomlevel2][harkIdx] = PictureFactory::createPlacingGrid(48, PALCOLOR_LIGHTRED);
    uiGraphic[UI_ValidPlace_Zoomlevel0][harkIdx]   = PictureFactory::createPlacingGrid(16, PALCOLOR_LIGHTGREEN);
    uiGraphic[UI_ValidPlace_Zoomlevel1][harkIdx]   = PictureFactory::createPlacingGrid(32, PALCOLOR_LIGHTGREEN);
    uiGraphic[UI_ValidPlace_Zoomlevel2][harkIdx]   = PictureFactory::createPlacingGrid(48, PALCOLOR_LIGHTGREEN);
    uiGraphic[UI_GreyPlace_Zoomlevel0][harkIdx]    = PictureFactory::createPlacingGrid(16, PALCOLOR_LIGHTGREY);
    uiGraphic[UI_GreyPlace_Zoomlevel1][harkIdx]    = PictureFactory::createPlacingGrid(32, PALCOLOR_LIGHTGREY);
    uiGraphic[UI_GreyPlace_Zoomlevel2][harkIdx]    = PictureFactory::createPlacingGrid(48, PALCOLOR_LIGHTGREY);
    uiGraphic[UI_MenuBackground][harkIdx]          = picFactory_.createMainBackground();
    uiGraphic[UI_GameStatsBackground][harkIdx]     = picFactory_.createGameStatsBackground(HOUSETYPE::HOUSE_HARKONNEN);
    uiGraphic[UI_GameStatsBackground][static_cast<int>(HOUSETYPE::HOUSE_ATREIDES)] =
        picFactory_.createGameStatsBackground(HOUSETYPE::HOUSE_ATREIDES);
    uiGraphic[UI_GameStatsBackground][static_cast<int>(HOUSETYPE::HOUSE_ORDOS)] =
        picFactory_.createGameStatsBackground(HOUSETYPE::HOUSE_ORDOS);
    uiGraphic[UI_GameStatsBackground][static_cast<int>(HOUSETYPE::HOUSE_FREMEN)] =
        picFactory_.createGameStatsBackground(HOUSETYPE::HOUSE_FREMEN);
    uiGraphic[UI_GameStatsBackground][static_cast<int>(HOUSETYPE::HOUSE_SARDAUKAR)] =
        picFactory_.createGameStatsBackground(HOUSETYPE::HOUSE_SARDAUKAR);
    uiGraphic[UI_GameStatsBackground][static_cast<int>(HOUSETYPE::HOUSE_MERCENARY)] =
        picFactory_.createGameStatsBackground(HOUSETYPE::HOUSE_MERCENARY);
    uiGraphic[UI_SelectionBox_Zoomlevel0][harkIdx] = LoadPNG_RW(file_manager->openFile("UI_SelectionBox.png").get());
    SDL_SetColorKey(uiGraphic[UI_SelectionBox_Zoomlevel0][harkIdx].get(), SDL_TRUE, 0);
    uiGraphic[UI_SelectionBox_Zoomlevel1][harkIdx] =
        Scaler::defaultDoubleTiledSurface(uiGraphic[UI_SelectionBox_Zoomlevel0][harkIdx].get(), 1, 1);
    SDL_SetColorKey(uiGraphic[UI_SelectionBox_Zoomlevel1][harkIdx].get(), SDL_TRUE, 0);
    uiGraphic[UI_SelectionBox_Zoomlevel2][harkIdx] =
        Scaler::defaultTripleTiledSurface(uiGraphic[UI_SelectionBox_Zoomlevel0][harkIdx].get(), 1, 1);
    SDL_SetColorKey(uiGraphic[UI_SelectionBox_Zoomlevel2][harkIdx].get(), SDL_TRUE, 0);
    uiGraphic[UI_OtherPlayerSelectionBox_Zoomlevel0][harkIdx] =
        LoadPNG_RW(file_manager->openFile("UI_OtherPlayerSelectionBox.png").get());
    SDL_SetColorKey(uiGraphic[UI_OtherPlayerSelectionBox_Zoomlevel0][harkIdx].get(), SDL_TRUE, 0);
    uiGraphic[UI_OtherPlayerSelectionBox_Zoomlevel1][harkIdx] =
        Scaler::defaultDoubleTiledSurface(uiGraphic[UI_OtherPlayerSelectionBox_Zoomlevel0][harkIdx].get(), 1, 1);
    SDL_SetColorKey(uiGraphic[UI_OtherPlayerSelectionBox_Zoomlevel1][harkIdx].get(), SDL_TRUE, 0);
    uiGraphic[UI_OtherPlayerSelectionBox_Zoomlevel2][harkIdx] =
        Scaler::defaultTripleTiledSurface(uiGraphic[UI_OtherPlayerSelectionBox_Zoomlevel0][harkIdx].get(), 1, 1);
    SDL_SetColorKey(uiGraphic[UI_OtherPlayerSelectionBox_Zoomlevel2][harkIdx].get(), SDL_TRUE, 0);
    uiGraphic[UI_TopBar][harkIdx]              = picFactory_.createTopBar();
    uiGraphic[UI_ButtonUp][harkIdx]            = choam->getPicture(0);
    uiGraphic[UI_ButtonUp_Pressed][harkIdx]    = choam->getPicture(1);
    uiGraphic[UI_ButtonDown][harkIdx]          = choam->getPicture(2);
    uiGraphic[UI_ButtonDown_Pressed][harkIdx]  = choam->getPicture(3);
    uiGraphic[UI_BuilderListUpperCap][harkIdx] = picFactory_.createBuilderListUpperCap();
    uiGraphic[UI_BuilderListLowerCap][harkIdx] = picFactory_.createBuilderListLowerCap();
    uiGraphic[UI_CustomGamePlayersArrow][harkIdx] =
        LoadPNG_RW(file_manager->openFile("CustomGamePlayers_Arrow.png").get());
    SDL_SetColorKey(uiGraphic[UI_CustomGamePlayersArrow][harkIdx].get(), SDL_TRUE, 0);
    uiGraphic[UI_CustomGamePlayersArrowNeutral][harkIdx] =
        LoadPNG_RW(file_manager->openFile("CustomGamePlayers_ArrowNeutral.png").get());
    SDL_SetColorKey(uiGraphic[UI_CustomGamePlayersArrowNeutral][harkIdx].get(), SDL_TRUE, 0);
    uiGraphic[UI_MessageBox][harkIdx] = picFactory_.createMessageBoxBorder();

    if (bttn != nullptr) {
        uiGraphic[UI_Mentat][harkIdx]          = bttn->getPicture(0);
        uiGraphic[UI_Mentat_Pressed][harkIdx]  = bttn->getPicture(1);
        uiGraphic[UI_Options][harkIdx]         = bttn->getPicture(2);
        uiGraphic[UI_Options_Pressed][harkIdx] = bttn->getPicture(3);
    } else {
        uiGraphic[UI_Mentat][harkIdx]          = shapes->getPicture(94);
        uiGraphic[UI_Mentat_Pressed][harkIdx]  = shapes->getPicture(95);
        uiGraphic[UI_Options][harkIdx]         = shapes->getPicture(96);
        uiGraphic[UI_Options_Pressed][harkIdx] = shapes->getPicture(97);
    }

    uiGraphic[UI_Upgrade][harkIdx] = choam->getPicture(4);
    SDL_SetColorKey(uiGraphic[UI_Upgrade][harkIdx].get(), SDL_TRUE, 0);
    uiGraphic[UI_Upgrade_Pressed][harkIdx] = choam->getPicture(5);
    SDL_SetColorKey(uiGraphic[UI_Upgrade_Pressed][harkIdx].get(), SDL_TRUE, 0);
    uiGraphic[UI_Repair][harkIdx]         = LoadPNG_RW(file_manager->openFile("Button_Repair.png").get());
    uiGraphic[UI_Repair_Pressed][harkIdx] = LoadPNG_RW(file_manager->openFile("Button_RepairPushed.png").get());
    uiGraphic[UI_Minus][harkIdx]          = LoadPNG_RW(file_manager->openFile("Button_Minus.png").get());
    uiGraphic[UI_Minus_Active][harkIdx] =
        mapSurfaceColorRange(uiGraphic[UI_Minus][harkIdx].get(), PALCOLOR_HARKONNEN, PALCOLOR_HARKONNEN - 2);
    uiGraphic[UI_Minus_Pressed][harkIdx] = LoadPNG_RW(file_manager->openFile("Button_MinusPushed.png").get());
    uiGraphic[UI_Plus][harkIdx]          = LoadPNG_RW(file_manager->openFile("Button_Plus.png").get());
    uiGraphic[UI_Plus_Active][harkIdx] =
        mapSurfaceColorRange(uiGraphic[UI_Plus][harkIdx].get(), PALCOLOR_HARKONNEN, PALCOLOR_HARKONNEN - 2);
    uiGraphic[UI_Plus_Pressed][harkIdx]  = LoadPNG_RW(file_manager->openFile("Button_PlusPushed.png").get());
    uiGraphic[UI_MissionSelect][harkIdx] = LoadPNG_RW(file_manager->openFile("Menu_MissionSelect.png").get());
    picFactory_.drawFrame(uiGraphic[UI_MissionSelect][harkIdx].get(), PictureFactory::SimpleFrame, nullptr);
    SDL_SetColorKey(uiGraphic[UI_MissionSelect][harkIdx].get(), SDL_TRUE, 0);
    uiGraphic[UI_OptionsMenu][harkIdx]    = picFactory_.createOptionsMenu();
    uiGraphic[UI_LoadSaveWindow][harkIdx] = picFactory_.createMenu(280, 228);
    uiGraphic[UI_NewMapWindow][harkIdx]   = picFactory_.createMenu(600, 440);
    uiGraphic[UI_DuneLegacy][harkIdx]     = LoadPNG_RW(file_manager->openFile("DuneLegacy.png").get());
    uiGraphic[UI_GameMenu][harkIdx]       = picFactory_.createMenu(uiGraphic[UI_DuneLegacy][harkIdx].get(), 158);
    picFactory_.drawFrame(uiGraphic[UI_DuneLegacy][harkIdx].get(), PictureFactory::SimpleFrame);

    uiGraphic[UI_PlanetBackground][harkIdx] = LoadCPS_RW(file_manager->openFile("BIGPLAN.CPS").get());
    picFactory_.drawFrame(uiGraphic[UI_PlanetBackground][harkIdx].get(), PictureFactory::SimpleFrame);
    uiGraphic[UI_MenuButtonBorder][harkIdx] =
        picFactory_.createFrame(PictureFactory::DecorationFrame1, 190, 123, false);

    picFactory_.drawFrame(uiGraphic[UI_DuneLegacy][harkIdx].get(), PictureFactory::SimpleFrame);

    uiGraphic[UI_MentatBackground][static_cast<int>(HOUSETYPE::HOUSE_HARKONNEN)] =
        Scaler::defaultDoubleSurface(LoadCPS_RW(file_manager->openFile("MENTATH.CPS").get()).get());
    uiGraphic[UI_MentatBackground][static_cast<int>(HOUSETYPE::HOUSE_ATREIDES)] =
        Scaler::defaultDoubleSurface(LoadCPS_RW(file_manager->openFile("MENTATA.CPS").get()).get());
    uiGraphic[UI_MentatBackground][static_cast<int>(HOUSETYPE::HOUSE_ORDOS)] =
        Scaler::defaultDoubleSurface(LoadCPS_RW(file_manager->openFile("MENTATO.CPS").get()).get());
    uiGraphic[UI_MentatBackground][static_cast<int>(HOUSETYPE::HOUSE_FREMEN)] =
        PictureFactory::mapMentatSurfaceToFremen(
            uiGraphic[UI_MentatBackground][static_cast<int>(HOUSETYPE::HOUSE_ATREIDES)].get());
    uiGraphic[UI_MentatBackground][static_cast<int>(HOUSETYPE::HOUSE_SARDAUKAR)] =
        PictureFactory::mapMentatSurfaceToSardaukar(
            uiGraphic[UI_MentatBackground][static_cast<int>(HOUSETYPE::HOUSE_HARKONNEN)].get());
    uiGraphic[UI_MentatBackground][static_cast<int>(HOUSETYPE::HOUSE_MERCENARY)] =
        PictureFactory::mapMentatSurfaceToMercenary(
            uiGraphic[UI_MentatBackground][static_cast<int>(HOUSETYPE::HOUSE_ORDOS)].get());

    uiGraphic[UI_MentatBackgroundBene][harkIdx] =
        Scaler::defaultDoubleSurface(LoadCPS_RW(file_manager->openFile("MENTATM.CPS").get()).get());
    if (uiGraphic[UI_MentatBackgroundBene][harkIdx] != nullptr) {
        benePalette.applyToSurface(uiGraphic[UI_MentatBackgroundBene][harkIdx].get());
    }

    uiGraphic[UI_MentatHouseChoiceInfoQuestion][static_cast<int>(HOUSETYPE::HOUSE_HARKONNEN)] =
        picFactory_.createMentatHouseChoiceQuestion(HOUSETYPE::HOUSE_HARKONNEN, benePalette);
    uiGraphic[UI_MentatHouseChoiceInfoQuestion][static_cast<int>(HOUSETYPE::HOUSE_ATREIDES)] =
        picFactory_.createMentatHouseChoiceQuestion(HOUSETYPE::HOUSE_ATREIDES, benePalette);
    uiGraphic[UI_MentatHouseChoiceInfoQuestion][static_cast<int>(HOUSETYPE::HOUSE_ORDOS)] =
        picFactory_.createMentatHouseChoiceQuestion(HOUSETYPE::HOUSE_ORDOS, benePalette);
    uiGraphic[UI_MentatHouseChoiceInfoQuestion][static_cast<int>(HOUSETYPE::HOUSE_SARDAUKAR)] =
        picFactory_.createMentatHouseChoiceQuestion(HOUSETYPE::HOUSE_SARDAUKAR, benePalette);
    uiGraphic[UI_MentatHouseChoiceInfoQuestion][static_cast<int>(HOUSETYPE::HOUSE_FREMEN)] =
        picFactory_.createMentatHouseChoiceQuestion(HOUSETYPE::HOUSE_FREMEN, benePalette);
    uiGraphic[UI_MentatHouseChoiceInfoQuestion][static_cast<int>(HOUSETYPE::HOUSE_MERCENARY)] =
        picFactory_.createMentatHouseChoiceQuestion(HOUSETYPE::HOUSE_MERCENARY, benePalette);

    uiGraphic[UI_MentatYes][harkIdx]             = Scaler::defaultDoubleSurface(mentat->getPicture(0).get());
    uiGraphic[UI_MentatYes_Pressed][harkIdx]     = Scaler::defaultDoubleSurface(mentat->getPicture(1).get());
    uiGraphic[UI_MentatNo][harkIdx]              = Scaler::defaultDoubleSurface(mentat->getPicture(2).get());
    uiGraphic[UI_MentatNo_Pressed][harkIdx]      = Scaler::defaultDoubleSurface(mentat->getPicture(3).get());
    uiGraphic[UI_MentatExit][harkIdx]            = Scaler::defaultDoubleSurface(mentat->getPicture(4).get());
    uiGraphic[UI_MentatExit_Pressed][harkIdx]    = Scaler::defaultDoubleSurface(mentat->getPicture(5).get());
    uiGraphic[UI_MentatProceed][harkIdx]         = Scaler::defaultDoubleSurface(mentat->getPicture(6).get());
    uiGraphic[UI_MentatProceed_Pressed][harkIdx] = Scaler::defaultDoubleSurface(mentat->getPicture(7).get());
    uiGraphic[UI_MentatRepeat][harkIdx]          = Scaler::defaultDoubleSurface(mentat->getPicture(8).get());
    uiGraphic[UI_MentatRepeat_Pressed][harkIdx]  = Scaler::defaultDoubleSurface(mentat->getPicture(9).get());

    { // Scope
        sdl2::surface_ptr pHouseChoiceBackground;
        if (file_manager->exists("HERALD." + _("LanguageFileExtension"))) {
            pHouseChoiceBackground = LoadCPS_RW(file_manager->openFile("HERALD." + _("LanguageFileExtension")).get());
        } else {
            pHouseChoiceBackground = LoadCPS_RW(file_manager->openFile("HERALD.CPS").get());
        }

        uiGraphic[UI_HouseSelect][static_cast<int>(HOUSETYPE::HOUSE_HARKONNEN)] =
            picFactory_.createHouseSelect(pHouseChoiceBackground.get());
        uiGraphic[UI_SelectYourHouseLarge][static_cast<int>(HOUSETYPE::HOUSE_HARKONNEN)] =
            Scaler::defaultDoubleSurface(getSubPicture(pHouseChoiceBackground.get(), 0, 0, 320, 50).get());
        uiGraphic[UI_Herald_Colored][static_cast<int>(HOUSETYPE::HOUSE_ATREIDES)] =
            getSubPicture(pHouseChoiceBackground.get(), 20, 54, 83, 91);
        uiGraphic[UI_Herald_ColoredLarge][static_cast<int>(HOUSETYPE::HOUSE_ATREIDES)] = Scaler::defaultDoubleSurface(
            uiGraphic[UI_Herald_Colored][static_cast<int>(HOUSETYPE::HOUSE_ATREIDES)].get());
        uiGraphic[UI_Herald_Colored][static_cast<int>(HOUSETYPE::HOUSE_ORDOS)] =
            getSubPicture(pHouseChoiceBackground.get(), 117, 54, 83, 91);
        uiGraphic[UI_Herald_ColoredLarge][static_cast<int>(HOUSETYPE::HOUSE_ORDOS)] =
            Scaler::defaultDoubleSurface(uiGraphic[UI_Herald_Colored][static_cast<int>(HOUSETYPE::HOUSE_ORDOS)].get());
        uiGraphic[UI_Herald_Colored][static_cast<int>(HOUSETYPE::HOUSE_HARKONNEN)] =
            getSubPicture(pHouseChoiceBackground.get(), 215, 54, 83, 91);
        uiGraphic[UI_Herald_ColoredLarge][static_cast<int>(HOUSETYPE::HOUSE_HARKONNEN)] = Scaler::defaultDoubleSurface(
            uiGraphic[UI_Herald_Colored][static_cast<int>(HOUSETYPE::HOUSE_HARKONNEN)].get());
        uiGraphic[UI_Herald_Colored][static_cast<int>(HOUSETYPE::HOUSE_FREMEN)] = PictureFactory::createHeraldFre(
            uiGraphic[UI_Herald_Colored][static_cast<int>(HOUSETYPE::HOUSE_HARKONNEN)].get());
        uiGraphic[UI_Herald_ColoredLarge][static_cast<int>(HOUSETYPE::HOUSE_FREMEN)] =
            Scaler::defaultDoubleSurface(uiGraphic[UI_Herald_Colored][static_cast<int>(HOUSETYPE::HOUSE_FREMEN)].get());
        uiGraphic[UI_Herald_Colored][static_cast<int>(HOUSETYPE::HOUSE_SARDAUKAR)] = PictureFactory::createHeraldSard(
            uiGraphic[UI_Herald_Colored][static_cast<int>(HOUSETYPE::HOUSE_ORDOS)].get(),
            uiGraphic[UI_Herald_Colored][static_cast<int>(HOUSETYPE::HOUSE_ATREIDES)].get());
        uiGraphic[UI_Herald_ColoredLarge][static_cast<int>(HOUSETYPE::HOUSE_SARDAUKAR)] = Scaler::defaultDoubleSurface(
            uiGraphic[UI_Herald_Colored][static_cast<int>(HOUSETYPE::HOUSE_SARDAUKAR)].get());
        uiGraphic[UI_Herald_Colored][static_cast<int>(HOUSETYPE::HOUSE_MERCENARY)] = PictureFactory::createHeraldMerc(
            uiGraphic[UI_Herald_Colored][static_cast<int>(HOUSETYPE::HOUSE_ATREIDES)].get(),
            uiGraphic[UI_Herald_Colored][static_cast<int>(HOUSETYPE::HOUSE_ORDOS)].get());
        uiGraphic[UI_Herald_ColoredLarge][static_cast<int>(HOUSETYPE::HOUSE_MERCENARY)] = Scaler::defaultDoubleSurface(
            uiGraphic[UI_Herald_Colored][static_cast<int>(HOUSETYPE::HOUSE_MERCENARY)].get());
    }

    uiGraphic[UI_Herald_Grey][static_cast<int>(HOUSETYPE::HOUSE_HARKONNEN)] = PictureFactory::createGreyHouseChoice(
        uiGraphic[UI_Herald_Colored][static_cast<int>(HOUSETYPE::HOUSE_HARKONNEN)].get());
    uiGraphic[UI_Herald_Grey][static_cast<int>(HOUSETYPE::HOUSE_ATREIDES)] = PictureFactory::createGreyHouseChoice(
        uiGraphic[UI_Herald_Colored][static_cast<int>(HOUSETYPE::HOUSE_ATREIDES)].get());
    uiGraphic[UI_Herald_Grey][static_cast<int>(HOUSETYPE::HOUSE_ORDOS)] = PictureFactory::createGreyHouseChoice(
        uiGraphic[UI_Herald_Colored][static_cast<int>(HOUSETYPE::HOUSE_ORDOS)].get());
    uiGraphic[UI_Herald_Grey][static_cast<int>(HOUSETYPE::HOUSE_FREMEN)] = PictureFactory::createGreyHouseChoice(
        uiGraphic[UI_Herald_Colored][static_cast<int>(HOUSETYPE::HOUSE_FREMEN)].get());
    uiGraphic[UI_Herald_Grey][static_cast<int>(HOUSETYPE::HOUSE_SARDAUKAR)] = PictureFactory::createGreyHouseChoice(
        uiGraphic[UI_Herald_Colored][static_cast<int>(HOUSETYPE::HOUSE_SARDAUKAR)].get());
    uiGraphic[UI_Herald_Grey][static_cast<int>(HOUSETYPE::HOUSE_MERCENARY)] = PictureFactory::createGreyHouseChoice(
        uiGraphic[UI_Herald_Colored][static_cast<int>(HOUSETYPE::HOUSE_MERCENARY)].get());

    uiGraphic[UI_Herald_ArrowLeft][static_cast<int>(HOUSETYPE::HOUSE_HARKONNEN)] =
        LoadPNG_RW(file_manager->openFile("ArrowLeft.png").get());
    uiGraphic[UI_Herald_ArrowLeftLarge][static_cast<int>(HOUSETYPE::HOUSE_HARKONNEN)] = Scaler::defaultDoubleSurface(
        uiGraphic[UI_Herald_ArrowLeft][static_cast<int>(HOUSETYPE::HOUSE_HARKONNEN)].get());
    uiGraphic[UI_Herald_ArrowLeftHighlight][static_cast<int>(HOUSETYPE::HOUSE_HARKONNEN)] =
        LoadPNG_RW(file_manager->openFile("ArrowLeftHighlight.png").get());
    uiGraphic[UI_Herald_ArrowLeftHighlightLarge][static_cast<int>(HOUSETYPE::HOUSE_HARKONNEN)] =
        Scaler::defaultDoubleSurface(
            uiGraphic[UI_Herald_ArrowLeftHighlight][static_cast<int>(HOUSETYPE::HOUSE_HARKONNEN)].get());
    uiGraphic[UI_Herald_ArrowRight][static_cast<int>(HOUSETYPE::HOUSE_HARKONNEN)] =
        LoadPNG_RW(file_manager->openFile("ArrowRight.png").get());
    uiGraphic[UI_Herald_ArrowRightLarge][static_cast<int>(HOUSETYPE::HOUSE_HARKONNEN)] = Scaler::defaultDoubleSurface(
        uiGraphic[UI_Herald_ArrowRight][static_cast<int>(HOUSETYPE::HOUSE_HARKONNEN)].get());
    uiGraphic[UI_Herald_ArrowRightHighlight][static_cast<int>(HOUSETYPE::HOUSE_HARKONNEN)] =
        LoadPNG_RW(file_manager->openFile("ArrowRightHighlight.png").get());
    uiGraphic[UI_Herald_ArrowRightHighlightLarge][static_cast<int>(HOUSETYPE::HOUSE_HARKONNEN)] =
        Scaler::defaultDoubleSurface(
            uiGraphic[UI_Herald_ArrowRightHighlight][static_cast<int>(HOUSETYPE::HOUSE_HARKONNEN)].get());

    uiGraphic[UI_MapChoiceScreen][static_cast<int>(HOUSETYPE::HOUSE_HARKONNEN)] =
        picFactory_.createMapChoiceScreen(HOUSETYPE::HOUSE_HARKONNEN);
    uiGraphic[UI_MapChoiceScreen][static_cast<int>(HOUSETYPE::HOUSE_ATREIDES)] =
        picFactory_.createMapChoiceScreen(HOUSETYPE::HOUSE_ATREIDES);
    uiGraphic[UI_MapChoiceScreen][static_cast<int>(HOUSETYPE::HOUSE_ORDOS)] =
        picFactory_.createMapChoiceScreen(HOUSETYPE::HOUSE_ORDOS);
    uiGraphic[UI_MapChoiceScreen][static_cast<int>(HOUSETYPE::HOUSE_FREMEN)] =
        picFactory_.createMapChoiceScreen(HOUSETYPE::HOUSE_FREMEN);
    uiGraphic[UI_MapChoiceScreen][static_cast<int>(HOUSETYPE::HOUSE_SARDAUKAR)] =
        picFactory_.createMapChoiceScreen(HOUSETYPE::HOUSE_SARDAUKAR);
    uiGraphic[UI_MapChoiceScreen][static_cast<int>(HOUSETYPE::HOUSE_MERCENARY)] =
        picFactory_.createMapChoiceScreen(HOUSETYPE::HOUSE_MERCENARY);
    uiGraphic[UI_MapChoicePlanet][static_cast<int>(HOUSETYPE::HOUSE_HARKONNEN)] =
        Scaler::doubleSurfaceNN(LoadCPS_RW(file_manager->openFile("PLANET.CPS").get()).get());
    SDL_SetColorKey(uiGraphic[UI_MapChoicePlanet][static_cast<int>(HOUSETYPE::HOUSE_HARKONNEN)].get(), SDL_TRUE, 0);
    uiGraphic[UI_MapChoiceMapOnly][static_cast<int>(HOUSETYPE::HOUSE_HARKONNEN)] =
        Scaler::doubleSurfaceNN(LoadCPS_RW(file_manager->openFile("DUNEMAP.CPS").get()).get());
    SDL_SetColorKey(uiGraphic[UI_MapChoiceMapOnly][static_cast<int>(HOUSETYPE::HOUSE_HARKONNEN)].get(), SDL_TRUE, 0);
    uiGraphic[UI_MapChoiceMap][static_cast<int>(HOUSETYPE::HOUSE_HARKONNEN)] =
        Scaler::doubleSurfaceNN(LoadCPS_RW(file_manager->openFile("DUNERGN.CPS").get()).get());
    SDL_SetColorKey(uiGraphic[UI_MapChoiceMap][static_cast<int>(HOUSETYPE::HOUSE_HARKONNEN)].get(), SDL_TRUE, 0);

    // make black lines inside the map non-transparent
    {
        auto* const surface = uiGraphic[UI_MapChoiceMap][static_cast<int>(HOUSETYPE::HOUSE_HARKONNEN)].get();

        sdl2::surface_lock lock{surface};

        for (auto y = 48; y < 48 + 240; y++) {
            for (auto x = 16; x < 16 + 608; x++) {
                if (getPixel(surface, x, y) == 0) {
                    putPixel(surface, x, y, PALCOLOR_BLACK);
                }
            }
        }
    }

    uiGraphic[UI_MapChoiceClickMap][static_cast<int>(HOUSETYPE::HOUSE_HARKONNEN)] =
        Scaler::doubleSurfaceNN(LoadCPS_RW(file_manager->openFile("RGNCLK.CPS").get()).get());
    uiGraphic[UI_MapChoiceArrow_None][static_cast<int>(HOUSETYPE::HOUSE_HARKONNEN)] =
        Scaler::defaultDoubleSurface(arrows->getPicture(0).get());
    SDL_SetColorKey(uiGraphic[UI_MapChoiceArrow_None][static_cast<int>(HOUSETYPE::HOUSE_HARKONNEN)].get(), SDL_TRUE, 0);
    uiGraphic[UI_MapChoiceArrow_LeftUp][static_cast<int>(HOUSETYPE::HOUSE_HARKONNEN)] =
        Scaler::defaultDoubleSurface(arrows->getPicture(1).get());
    SDL_SetColorKey(uiGraphic[UI_MapChoiceArrow_LeftUp][static_cast<int>(HOUSETYPE::HOUSE_HARKONNEN)].get(), SDL_TRUE,
                    0);
    uiGraphic[UI_MapChoiceArrow_Up][static_cast<int>(HOUSETYPE::HOUSE_HARKONNEN)] =
        Scaler::defaultDoubleSurface(arrows->getPicture(2).get());
    SDL_SetColorKey(uiGraphic[UI_MapChoiceArrow_Up][static_cast<int>(HOUSETYPE::HOUSE_HARKONNEN)].get(), SDL_TRUE, 0);
    uiGraphic[UI_MapChoiceArrow_RightUp][static_cast<int>(HOUSETYPE::HOUSE_HARKONNEN)] =
        Scaler::defaultDoubleSurface(arrows->getPicture(3).get());
    SDL_SetColorKey(uiGraphic[UI_MapChoiceArrow_RightUp][static_cast<int>(HOUSETYPE::HOUSE_HARKONNEN)].get(), SDL_TRUE,
                    0);
    uiGraphic[UI_MapChoiceArrow_Right][static_cast<int>(HOUSETYPE::HOUSE_HARKONNEN)] =
        Scaler::defaultDoubleSurface(arrows->getPicture(4).get());
    SDL_SetColorKey(uiGraphic[UI_MapChoiceArrow_Right][static_cast<int>(HOUSETYPE::HOUSE_HARKONNEN)].get(), SDL_TRUE,
                    0);
    uiGraphic[UI_MapChoiceArrow_RightDown][static_cast<int>(HOUSETYPE::HOUSE_HARKONNEN)] =
        Scaler::defaultDoubleSurface(arrows->getPicture(5).get());
    SDL_SetColorKey(uiGraphic[UI_MapChoiceArrow_RightDown][static_cast<int>(HOUSETYPE::HOUSE_HARKONNEN)].get(),
                    SDL_TRUE, 0);
    uiGraphic[UI_MapChoiceArrow_Down][static_cast<int>(HOUSETYPE::HOUSE_HARKONNEN)] =
        Scaler::defaultDoubleSurface(arrows->getPicture(6).get());
    SDL_SetColorKey(uiGraphic[UI_MapChoiceArrow_Down][static_cast<int>(HOUSETYPE::HOUSE_HARKONNEN)].get(), SDL_TRUE, 0);
    uiGraphic[UI_MapChoiceArrow_LeftDown][static_cast<int>(HOUSETYPE::HOUSE_HARKONNEN)] =
        Scaler::defaultDoubleSurface(arrows->getPicture(7).get());
    SDL_SetColorKey(uiGraphic[UI_MapChoiceArrow_LeftDown][static_cast<int>(HOUSETYPE::HOUSE_HARKONNEN)].get(), SDL_TRUE,
                    0);
    uiGraphic[UI_MapChoiceArrow_Left][static_cast<int>(HOUSETYPE::HOUSE_HARKONNEN)] =
        Scaler::defaultDoubleSurface(arrows->getPicture(8).get());
    SDL_SetColorKey(uiGraphic[UI_MapChoiceArrow_Left][static_cast<int>(HOUSETYPE::HOUSE_HARKONNEN)].get(), SDL_TRUE, 0);

    uiGraphic[UI_StructureSizeLattice][static_cast<int>(HOUSETYPE::HOUSE_HARKONNEN)] =
        LoadPNG_RW(file_manager->openFile("StructureSizeLattice.png").get());
    SDL_SetColorKey(uiGraphic[UI_StructureSizeLattice][static_cast<int>(HOUSETYPE::HOUSE_HARKONNEN)].get(), SDL_TRUE,
                    0);
    uiGraphic[UI_StructureSizeConcrete][static_cast<int>(HOUSETYPE::HOUSE_HARKONNEN)] =
        LoadPNG_RW(file_manager->openFile("StructureSizeConcrete.png").get());
    SDL_SetColorKey(uiGraphic[UI_StructureSizeConcrete][static_cast<int>(HOUSETYPE::HOUSE_HARKONNEN)].get(), SDL_TRUE,
                    0);

    uiGraphic[UI_MapEditor_SideBar][static_cast<int>(HOUSETYPE::HOUSE_HARKONNEN)]   = picFactory_.createSideBar(true);
    uiGraphic[UI_MapEditor_BottomBar][static_cast<int>(HOUSETYPE::HOUSE_HARKONNEN)] = picFactory_.createBottomBar();

    uiGraphic[UI_MapEditor_ExitIcon][static_cast<int>(HOUSETYPE::HOUSE_HARKONNEN)] =
        LoadPNG_RW(file_manager->openFile("MapEditorExitIcon.png").get());
    SDL_SetColorKey(uiGraphic[UI_MapEditor_ExitIcon][static_cast<int>(HOUSETYPE::HOUSE_HARKONNEN)].get(), SDL_TRUE, 0);
    uiGraphic[UI_MapEditor_NewIcon][static_cast<int>(HOUSETYPE::HOUSE_HARKONNEN)] =
        LoadPNG_RW(file_manager->openFile("MapEditorNewIcon.png").get());
    SDL_SetColorKey(uiGraphic[UI_MapEditor_NewIcon][static_cast<int>(HOUSETYPE::HOUSE_HARKONNEN)].get(), SDL_TRUE, 0);
    uiGraphic[UI_MapEditor_LoadIcon][static_cast<int>(HOUSETYPE::HOUSE_HARKONNEN)] =
        LoadPNG_RW(file_manager->openFile("MapEditorLoadIcon.png").get());
    SDL_SetColorKey(uiGraphic[UI_MapEditor_LoadIcon][static_cast<int>(HOUSETYPE::HOUSE_HARKONNEN)].get(), SDL_TRUE, 0);
    uiGraphic[UI_MapEditor_SaveIcon][static_cast<int>(HOUSETYPE::HOUSE_HARKONNEN)] =
        LoadPNG_RW(file_manager->openFile("MapEditorSaveIcon.png").get());
    SDL_SetColorKey(uiGraphic[UI_MapEditor_SaveIcon][static_cast<int>(HOUSETYPE::HOUSE_HARKONNEN)].get(), SDL_TRUE, 0);
    uiGraphic[UI_MapEditor_UndoIcon][static_cast<int>(HOUSETYPE::HOUSE_HARKONNEN)] =
        LoadPNG_RW(file_manager->openFile("MapEditorUndoIcon.png").get());
    SDL_SetColorKey(uiGraphic[UI_MapEditor_UndoIcon][static_cast<int>(HOUSETYPE::HOUSE_HARKONNEN)].get(), SDL_TRUE, 0);
    uiGraphic[UI_MapEditor_RedoIcon][static_cast<int>(HOUSETYPE::HOUSE_HARKONNEN)] =
        LoadPNG_RW(file_manager->openFile("MapEditorRedoIcon.png").get());
    SDL_SetColorKey(uiGraphic[UI_MapEditor_RedoIcon][static_cast<int>(HOUSETYPE::HOUSE_HARKONNEN)].get(), SDL_TRUE, 0);
    uiGraphic[UI_MapEditor_PlayerIcon][static_cast<int>(HOUSETYPE::HOUSE_HARKONNEN)] =
        LoadPNG_RW(file_manager->openFile("MapEditorPlayerIcon.png").get());
    SDL_SetColorKey(uiGraphic[UI_MapEditor_PlayerIcon][static_cast<int>(HOUSETYPE::HOUSE_HARKONNEN)].get(), SDL_TRUE,
                    0);
    uiGraphic[UI_MapEditor_MapSettingsIcon][static_cast<int>(HOUSETYPE::HOUSE_HARKONNEN)] =
        LoadPNG_RW(file_manager->openFile("MapEditorMapSettingsIcon.png").get());
    SDL_SetColorKey(uiGraphic[UI_MapEditor_MapSettingsIcon][static_cast<int>(HOUSETYPE::HOUSE_HARKONNEN)].get(),
                    SDL_TRUE, 0);
    uiGraphic[UI_MapEditor_ChoamIcon][static_cast<int>(HOUSETYPE::HOUSE_HARKONNEN)] = scaleSurface(
        getSubFrame(objPic[ObjPic_Frigate][static_cast<int>(HOUSETYPE::HOUSE_HARKONNEN)][0].get(), 1, 0, 8, 1).get(),
        0.5);
    SDL_SetColorKey(uiGraphic[UI_MapEditor_ChoamIcon][static_cast<int>(HOUSETYPE::HOUSE_HARKONNEN)].get(), SDL_TRUE, 0);
    uiGraphic[UI_MapEditor_ReinforcementsIcon][static_cast<int>(HOUSETYPE::HOUSE_HARKONNEN)] = scaleSurface(
        getSubFrame(objPic[ObjPic_Carryall][static_cast<int>(HOUSETYPE::HOUSE_HARKONNEN)][0].get(), 1, 0, 8, 2).get(),
        0.66667);
    SDL_SetColorKey(uiGraphic[UI_MapEditor_ReinforcementsIcon][static_cast<int>(HOUSETYPE::HOUSE_HARKONNEN)].get(),
                    SDL_TRUE, 0);
    uiGraphic[UI_MapEditor_TeamsIcon][static_cast<int>(HOUSETYPE::HOUSE_HARKONNEN)] =
        getSubFrame(objPic[ObjPic_Troopers][static_cast<int>(HOUSETYPE::HOUSE_HARKONNEN)][0].get(), 0, 0, 4, 4);
    SDL_SetColorKey(uiGraphic[UI_MapEditor_TeamsIcon][static_cast<int>(HOUSETYPE::HOUSE_HARKONNEN)].get(), SDL_TRUE, 0);
    uiGraphic[UI_MapEditor_MirrorNoneIcon][static_cast<int>(HOUSETYPE::HOUSE_HARKONNEN)] =
        LoadPNG_RW(file_manager->openFile("MapEditorMirrorNone.png").get());
    SDL_SetColorKey(uiGraphic[UI_MapEditor_MirrorNoneIcon][static_cast<int>(HOUSETYPE::HOUSE_HARKONNEN)].get(),
                    SDL_TRUE, 0);
    uiGraphic[UI_MapEditor_MirrorHorizontalIcon][static_cast<int>(HOUSETYPE::HOUSE_HARKONNEN)] =
        LoadPNG_RW(file_manager->openFile("MapEditorMirrorHorizontal.png").get());
    SDL_SetColorKey(uiGraphic[UI_MapEditor_MirrorHorizontalIcon][static_cast<int>(HOUSETYPE::HOUSE_HARKONNEN)].get(),
                    SDL_TRUE, 0);
    uiGraphic[UI_MapEditor_MirrorVerticalIcon][static_cast<int>(HOUSETYPE::HOUSE_HARKONNEN)] =
        LoadPNG_RW(file_manager->openFile("MapEditorMirrorVertical.png").get());
    SDL_SetColorKey(uiGraphic[UI_MapEditor_MirrorVerticalIcon][static_cast<int>(HOUSETYPE::HOUSE_HARKONNEN)].get(),
                    SDL_TRUE, 0);
    uiGraphic[UI_MapEditor_MirrorBothIcon][static_cast<int>(HOUSETYPE::HOUSE_HARKONNEN)] =
        LoadPNG_RW(file_manager->openFile("MapEditorMirrorBoth.png").get());
    SDL_SetColorKey(uiGraphic[UI_MapEditor_MirrorBothIcon][static_cast<int>(HOUSETYPE::HOUSE_HARKONNEN)].get(),
                    SDL_TRUE, 0);
    uiGraphic[UI_MapEditor_MirrorPointIcon][static_cast<int>(HOUSETYPE::HOUSE_HARKONNEN)] =
        LoadPNG_RW(file_manager->openFile("MapEditorMirrorPoint.png").get());
    SDL_SetColorKey(uiGraphic[UI_MapEditor_MirrorPointIcon][static_cast<int>(HOUSETYPE::HOUSE_HARKONNEN)].get(),
                    SDL_TRUE, 0);
    uiGraphic[UI_MapEditor_ArrowUp][static_cast<int>(HOUSETYPE::HOUSE_HARKONNEN)] =
        LoadPNG_RW(file_manager->openFile("MapEditorArrowUp.png").get());
    SDL_SetColorKey(uiGraphic[UI_MapEditor_ArrowUp][static_cast<int>(HOUSETYPE::HOUSE_HARKONNEN)].get(), SDL_TRUE, 0);
    uiGraphic[UI_MapEditor_ArrowUp_Active][static_cast<int>(HOUSETYPE::HOUSE_HARKONNEN)] =
        mapSurfaceColorRange(uiGraphic[UI_MapEditor_ArrowUp][static_cast<int>(HOUSETYPE::HOUSE_HARKONNEN)].get(),
                             PALCOLOR_HARKONNEN, PALCOLOR_HARKONNEN - 3);
    uiGraphic[UI_MapEditor_ArrowDown][static_cast<int>(HOUSETYPE::HOUSE_HARKONNEN)] =
        LoadPNG_RW(file_manager->openFile("MapEditorArrowDown.png").get());
    SDL_SetColorKey(uiGraphic[UI_MapEditor_ArrowDown][static_cast<int>(HOUSETYPE::HOUSE_HARKONNEN)].get(), SDL_TRUE, 0);
    uiGraphic[UI_MapEditor_ArrowDown_Active][static_cast<int>(HOUSETYPE::HOUSE_HARKONNEN)] =
        mapSurfaceColorRange(uiGraphic[UI_MapEditor_ArrowDown][static_cast<int>(HOUSETYPE::HOUSE_HARKONNEN)].get(),
                             PALCOLOR_HARKONNEN, PALCOLOR_HARKONNEN - 3);
    uiGraphic[UI_MapEditor_Plus][static_cast<int>(HOUSETYPE::HOUSE_HARKONNEN)] =
        LoadPNG_RW(file_manager->openFile("MapEditorPlus.png").get());
    SDL_SetColorKey(uiGraphic[UI_MapEditor_Plus][static_cast<int>(HOUSETYPE::HOUSE_HARKONNEN)].get(), SDL_TRUE, 0);
    uiGraphic[UI_MapEditor_Plus_Active][static_cast<int>(HOUSETYPE::HOUSE_HARKONNEN)] =
        mapSurfaceColorRange(uiGraphic[UI_MapEditor_Plus][static_cast<int>(HOUSETYPE::HOUSE_HARKONNEN)].get(),
                             PALCOLOR_HARKONNEN, PALCOLOR_HARKONNEN - 3);
    uiGraphic[UI_MapEditor_Minus][static_cast<int>(HOUSETYPE::HOUSE_HARKONNEN)] =
        LoadPNG_RW(file_manager->openFile("MapEditorMinus.png").get());
    SDL_SetColorKey(uiGraphic[UI_MapEditor_Minus][static_cast<int>(HOUSETYPE::HOUSE_HARKONNEN)].get(), SDL_TRUE, 0);
    uiGraphic[UI_MapEditor_Minus_Active][static_cast<int>(HOUSETYPE::HOUSE_HARKONNEN)] =
        mapSurfaceColorRange(uiGraphic[UI_MapEditor_Minus][static_cast<int>(HOUSETYPE::HOUSE_HARKONNEN)].get(),
                             PALCOLOR_HARKONNEN, PALCOLOR_HARKONNEN - 3);
    uiGraphic[UI_MapEditor_RotateLeftIcon][static_cast<int>(HOUSETYPE::HOUSE_HARKONNEN)] =
        LoadPNG_RW(file_manager->openFile("MapEditorRotateLeft.png").get());
    SDL_SetColorKey(uiGraphic[UI_MapEditor_RotateLeftIcon][static_cast<int>(HOUSETYPE::HOUSE_HARKONNEN)].get(),
                    SDL_TRUE, 0);
    uiGraphic[UI_MapEditor_RotateLeftHighlightIcon][static_cast<int>(HOUSETYPE::HOUSE_HARKONNEN)] =
        mapSurfaceColorRange(uiGraphic[UI_MapEditor_RotateLeftIcon][static_cast<int>(HOUSETYPE::HOUSE_HARKONNEN)].get(),
                             PALCOLOR_HARKONNEN, PALCOLOR_HARKONNEN - 3);
    SDL_SetColorKey(uiGraphic[UI_MapEditor_RotateLeftHighlightIcon][static_cast<int>(HOUSETYPE::HOUSE_HARKONNEN)].get(),
                    SDL_TRUE, 0);
    uiGraphic[UI_MapEditor_RotateRightIcon][static_cast<int>(HOUSETYPE::HOUSE_HARKONNEN)] =
        LoadPNG_RW(file_manager->openFile("MapEditorRotateRight.png").get());
    SDL_SetColorKey(uiGraphic[UI_MapEditor_RotateRightIcon][static_cast<int>(HOUSETYPE::HOUSE_HARKONNEN)].get(),
                    SDL_TRUE, 0);
    uiGraphic[UI_MapEditor_RotateRightHighlightIcon][static_cast<int>(HOUSETYPE::HOUSE_HARKONNEN)] =
        mapSurfaceColorRange(
            uiGraphic[UI_MapEditor_RotateRightIcon][static_cast<int>(HOUSETYPE::HOUSE_HARKONNEN)].get(),
            PALCOLOR_HARKONNEN, PALCOLOR_HARKONNEN - 3);
    SDL_SetColorKey(
        uiGraphic[UI_MapEditor_RotateRightHighlightIcon][static_cast<int>(HOUSETYPE::HOUSE_HARKONNEN)].get(), SDL_TRUE,
        0);

    uiGraphic[UI_MapEditor_Sand][static_cast<int>(HOUSETYPE::HOUSE_HARKONNEN)] =
        Scaler::defaultDoubleSurface(icon->getPicture(127).get());
    uiGraphic[UI_MapEditor_Dunes][static_cast<int>(HOUSETYPE::HOUSE_HARKONNEN)] =
        Scaler::defaultDoubleSurface(icon->getPicture(159).get());
    uiGraphic[UI_MapEditor_SpecialBloom][static_cast<int>(HOUSETYPE::HOUSE_HARKONNEN)] =
        Scaler::defaultDoubleSurface(icon->getPicture(209).get());
    uiGraphic[UI_MapEditor_Spice][static_cast<int>(HOUSETYPE::HOUSE_HARKONNEN)] =
        Scaler::defaultDoubleSurface(icon->getPicture(191).get());
    uiGraphic[UI_MapEditor_ThickSpice][static_cast<int>(HOUSETYPE::HOUSE_HARKONNEN)] =
        Scaler::defaultDoubleSurface(icon->getPicture(207).get());
    uiGraphic[UI_MapEditor_SpiceBloom][static_cast<int>(HOUSETYPE::HOUSE_HARKONNEN)] =
        Scaler::defaultDoubleSurface(icon->getPicture(208).get());
    uiGraphic[UI_MapEditor_Slab][static_cast<int>(HOUSETYPE::HOUSE_HARKONNEN)] =
        Scaler::defaultDoubleSurface(icon->getPicture(126).get());
    uiGraphic[UI_MapEditor_Rock][static_cast<int>(HOUSETYPE::HOUSE_HARKONNEN)] =
        Scaler::defaultDoubleSurface(icon->getPicture(143).get());
    uiGraphic[UI_MapEditor_Mountain][static_cast<int>(HOUSETYPE::HOUSE_HARKONNEN)] =
        Scaler::defaultDoubleSurface(icon->getPicture(175).get());

    uiGraphic[UI_MapEditor_Slab1][harkIdx] = icon->getPicture(126);
    uiGraphic[UI_MapEditor_Wall][harkIdx] =
        getSubPicture(objPic[ObjPic_Wall][harkIdx][0].get(), 2 * D2_TILESIZE, 0, D2_TILESIZE, D2_TILESIZE);
    uiGraphic[UI_MapEditor_GunTurret][harkIdx] =
        getSubPicture(objPic[ObjPic_GunTurret][harkIdx][0].get(), 2 * D2_TILESIZE, 0, D2_TILESIZE, D2_TILESIZE);
    uiGraphic[UI_MapEditor_RocketTurret][harkIdx] =
        getSubPicture(objPic[ObjPic_RocketTurret][harkIdx][0].get(), 2 * D2_TILESIZE, 0, D2_TILESIZE, D2_TILESIZE);
    uiGraphic[UI_MapEditor_ConstructionYard][harkIdx] = getSubPicture(
        objPic[ObjPic_ConstructionYard][harkIdx][0].get(), 2 * 2 * D2_TILESIZE, 0, 2 * D2_TILESIZE, 2 * D2_TILESIZE);
    uiGraphic[UI_MapEditor_Windtrap][harkIdx] = getSubPicture(objPic[ObjPic_Windtrap][harkIdx][0].get(),
                                                              2 * 2 * D2_TILESIZE, 0, 2 * D2_TILESIZE, 2 * D2_TILESIZE);
    SDL_Color windtrapColor                   = {70, 70, 70, 255};
    SDL_SetPaletteColors(uiGraphic[UI_MapEditor_Windtrap][harkIdx]->format->palette, &windtrapColor,
                         PALCOLOR_WINDTRAP_COLORCYCLE, 1);
    uiGraphic[UI_MapEditor_Radar][harkIdx] =
        getSubPicture(objPic[ObjPic_Radar][harkIdx][0].get(), 2 * 2 * D2_TILESIZE, 0, 2 * D2_TILESIZE, 2 * D2_TILESIZE);
    uiGraphic[UI_MapEditor_Silo][harkIdx] =
        getSubPicture(objPic[ObjPic_Silo][harkIdx][0].get(), 2 * 2 * D2_TILESIZE, 0, 2 * D2_TILESIZE, 2 * D2_TILESIZE);
    uiGraphic[UI_MapEditor_IX][harkIdx] =
        getSubPicture(objPic[ObjPic_IX][harkIdx][0].get(), 2 * 2 * D2_TILESIZE, 0, 2 * D2_TILESIZE, 2 * D2_TILESIZE);
    uiGraphic[UI_MapEditor_Barracks][harkIdx] = getSubPicture(objPic[ObjPic_Barracks][harkIdx][0].get(),
                                                              2 * 2 * D2_TILESIZE, 0, 2 * D2_TILESIZE, 2 * D2_TILESIZE);
    uiGraphic[UI_MapEditor_WOR][harkIdx] =
        getSubPicture(objPic[ObjPic_WOR][harkIdx][0].get(), 2 * 2 * D2_TILESIZE, 0, 2 * D2_TILESIZE, 2 * D2_TILESIZE);
    uiGraphic[UI_MapEditor_LightFactory][harkIdx] = getSubPicture(
        objPic[ObjPic_LightFactory][harkIdx][0].get(), 2 * 2 * D2_TILESIZE, 0, 2 * D2_TILESIZE, 2 * D2_TILESIZE);
    uiGraphic[UI_MapEditor_Refinery][harkIdx]        = getSubPicture(objPic[ObjPic_Refinery][harkIdx][0].get(),
                                                                     2 * 3 * D2_TILESIZE, 0, 3 * D2_TILESIZE, 2 * D2_TILESIZE);
    uiGraphic[UI_MapEditor_HighTechFactory][harkIdx] = getSubPicture(
        objPic[ObjPic_HighTechFactory][harkIdx][0].get(), 2 * 3 * D2_TILESIZE, 0, 3 * D2_TILESIZE, 2 * D2_TILESIZE);
    uiGraphic[UI_MapEditor_HeavyFactory][harkIdx] = getSubPicture(
        objPic[ObjPic_HeavyFactory][harkIdx][0].get(), 2 * 3 * D2_TILESIZE, 0, 3 * D2_TILESIZE, 2 * D2_TILESIZE);
    uiGraphic[UI_MapEditor_RepairYard][harkIdx] = getSubPicture(
        objPic[ObjPic_RepairYard][harkIdx][0].get(), 2 * 3 * D2_TILESIZE, 0, 3 * D2_TILESIZE, 2 * D2_TILESIZE);
    uiGraphic[UI_MapEditor_Starport][harkIdx] = getSubPicture(objPic[ObjPic_Starport][harkIdx][0].get(),
                                                              2 * 3 * D2_TILESIZE, 0, 3 * D2_TILESIZE, 3 * D2_TILESIZE);
    uiGraphic[UI_MapEditor_Palace][harkIdx]   = getSubPicture(objPic[ObjPic_Palace][harkIdx][0].get(),
                                                              2 * 3 * D2_TILESIZE, 0, 3 * D2_TILESIZE, 3 * D2_TILESIZE);

    uiGraphic[UI_MapEditor_Soldier][harkIdx]   = getSubFrame(objPic[ObjPic_Soldier][harkIdx][0].get(), 0, 0, 4, 3);
    uiGraphic[UI_MapEditor_Trooper][harkIdx]   = getSubFrame(objPic[ObjPic_Trooper][harkIdx][0].get(), 0, 0, 4, 3);
    uiGraphic[UI_MapEditor_Harvester][harkIdx] = getSubFrame(objPic[ObjPic_Harvester][harkIdx][0].get(), 0, 0, 8, 1);
    uiGraphic[UI_MapEditor_Infantry][harkIdx]  = getSubFrame(objPic[ObjPic_Infantry][harkIdx][0].get(), 0, 0, 4, 4);
    uiGraphic[UI_MapEditor_Troopers][harkIdx]  = getSubFrame(objPic[ObjPic_Troopers][harkIdx][0].get(), 0, 0, 4, 4);
    uiGraphic[UI_MapEditor_MCV][harkIdx]       = getSubFrame(objPic[ObjPic_MCV][harkIdx][0].get(), 0, 0, 8, 1);
    uiGraphic[UI_MapEditor_Trike][harkIdx]     = getSubFrame(objPic[ObjPic_Trike][harkIdx][0].get(), 0, 0, 8, 1);
    uiGraphic[UI_MapEditor_Raider][harkIdx]    = getSubFrame(objPic[ObjPic_Trike][harkIdx][0].get(), 0, 0, 8, 1);
    uiGraphic[UI_MapEditor_Raider][harkIdx] =
        combinePictures(uiGraphic[UI_MapEditor_Raider][harkIdx].get(), objPic[ObjPic_Star][harkIdx][1].get(),
                        uiGraphic[UI_MapEditor_Raider][harkIdx]->w - objPic[ObjPic_Star][harkIdx][1]->w,
                        uiGraphic[UI_MapEditor_Raider][harkIdx]->h - objPic[ObjPic_Star][harkIdx][1]->h);
    uiGraphic[UI_MapEditor_Quad][harkIdx] = getSubFrame(objPic[ObjPic_Quad][harkIdx][0].get(), 0, 0, 8, 1);
    uiGraphic[UI_MapEditor_Tank][harkIdx] =
        combinePictures(getSubFrame(objPic[ObjPic_Tank_Base][harkIdx][0].get(), 0, 0, 8, 1).get(),
                        getSubFrame(objPic[ObjPic_Tank_Gun][harkIdx][0].get(), 0, 0, 8, 1).get(), 0, 0);
    uiGraphic[UI_MapEditor_SiegeTank][harkIdx] =
        combinePictures(getSubFrame(objPic[ObjPic_Siegetank_Base][harkIdx][0].get(), 0, 0, 8, 1).get(),
                        getSubFrame(objPic[ObjPic_Siegetank_Gun][harkIdx][0].get(), 0, 0, 8, 1).get(), 2, -4);
    uiGraphic[UI_MapEditor_Launcher][harkIdx] =
        combinePictures(getSubFrame(objPic[ObjPic_Tank_Base][harkIdx][0].get(), 0, 0, 8, 1).get(),
                        getSubFrame(objPic[ObjPic_Launcher_Gun][harkIdx][0].get(), 0, 0, 8, 1).get(), 3, 0);
    uiGraphic[UI_MapEditor_Devastator][harkIdx] =
        combinePictures(getSubFrame(objPic[ObjPic_Devastator_Base][harkIdx][0].get(), 0, 0, 8, 1).get(),
                        getSubFrame(objPic[ObjPic_Devastator_Gun][harkIdx][0].get(), 0, 0, 8, 1).get(), 2, -4);
    uiGraphic[UI_MapEditor_SonicTank][harkIdx] =
        combinePictures(getSubFrame(objPic[ObjPic_Tank_Base][harkIdx][0].get(), 0, 0, 8, 1).get(),
                        getSubFrame(objPic[ObjPic_Sonictank_Gun][harkIdx][0].get(), 0, 0, 8, 1).get(), 3, 1);
    uiGraphic[UI_MapEditor_Deviator][harkIdx] =
        combinePictures(getSubFrame(objPic[ObjPic_Tank_Base][harkIdx][0].get(), 0, 0, 8, 1).get(),
                        getSubFrame(objPic[ObjPic_Launcher_Gun][harkIdx][0].get(), 0, 0, 8, 1).get(), 3, 0);
    uiGraphic[UI_MapEditor_Deviator][harkIdx] =
        combinePictures(uiGraphic[UI_MapEditor_Deviator][harkIdx].get(), objPic[ObjPic_Star][harkIdx][1].get(),
                        uiGraphic[UI_MapEditor_Deviator][harkIdx]->w - objPic[ObjPic_Star][harkIdx][1]->w,
                        uiGraphic[UI_MapEditor_Deviator][harkIdx]->h - objPic[ObjPic_Star][harkIdx][1]->h);
    uiGraphic[UI_MapEditor_Saboteur][harkIdx] = getSubFrame(objPic[ObjPic_Saboteur][harkIdx][0].get(), 0, 0, 4, 3);
    uiGraphic[UI_MapEditor_Sandworm][harkIdx] = getSubFrame(objPic[ObjPic_Sandworm][harkIdx][0].get(), 0, 5, 1, 9);
    uiGraphic[UI_MapEditor_SpecialUnit][harkIdx] =
        combinePictures(getSubFrame(objPic[ObjPic_Devastator_Base][harkIdx][0].get(), 0, 0, 8, 1).get(),
                        getSubFrame(objPic[ObjPic_Devastator_Gun][harkIdx][0].get(), 0, 0, 8, 1).get(), 2, -4);
    uiGraphic[UI_MapEditor_SpecialUnit][harkIdx] =
        combinePictures(uiGraphic[UI_MapEditor_SpecialUnit][harkIdx].get(), objPic[ObjPic_Star][harkIdx][1].get(),
                        uiGraphic[UI_MapEditor_SpecialUnit][harkIdx]->w - objPic[ObjPic_Star][harkIdx][1]->w,
                        uiGraphic[UI_MapEditor_SpecialUnit][harkIdx]->h - objPic[ObjPic_Star][harkIdx][1]->h);
    uiGraphic[UI_MapEditor_Carryall][harkIdx] = getSubFrame(objPic[ObjPic_Carryall][harkIdx][0].get(), 0, 0, 8, 2);
    uiGraphic[UI_MapEditor_Ornithopter][harkIdx] =
        getSubFrame(objPic[ObjPic_Ornithopter][harkIdx][0].get(), 0, 0, 8, 3);

    uiGraphic[UI_MapEditor_Pen1x1][harkIdx] = LoadPNG_RW(file_manager->openFile("MapEditorPen1x1.png").get());
    SDL_SetColorKey(uiGraphic[UI_MapEditor_Pen1x1][harkIdx].get(), SDL_TRUE, 0);
    uiGraphic[UI_MapEditor_Pen3x3][harkIdx] = LoadPNG_RW(file_manager->openFile("MapEditorPen3x3.png").get());
    SDL_SetColorKey(uiGraphic[UI_MapEditor_Pen3x3][harkIdx].get(), SDL_TRUE, 0);
    uiGraphic[UI_MapEditor_Pen5x5][harkIdx] = LoadPNG_RW(file_manager->openFile("MapEditorPen5x5.png").get());
    SDL_SetColorKey(uiGraphic[UI_MapEditor_Pen5x5][harkIdx].get(), SDL_TRUE, 0);

    uiGraphic[UI_Background_Tile][harkIdx] = picFactory_.createBackgroundTile();

    uiGraphic[UI_Background_Logo][harkIdx]                                     = picFactory_.createHarkonnenLogo();
    uiGraphic[UI_Background_Logo][static_cast<int>(HOUSETYPE::HOUSE_ATREIDES)] = picFactory_.createAtreidesLogo();
    uiGraphic[UI_Background_Logo][static_cast<int>(HOUSETYPE::HOUSE_ORDOS)]    = picFactory_.createOrdosLogo();

    // load animations
    animation[Anim_HarkonnenEyes] = menshph->getAnimation(0, 4, true, true);
    animation[Anim_HarkonnenEyes]->setFrameRate(0.3);
    animation[Anim_HarkonnenMouth] = menshph->getAnimation(5, 9, true, true, true);
    animation[Anim_HarkonnenMouth]->setFrameRate(5.0);
    animation[Anim_HarkonnenShoulder] = menshph->getAnimation(10, 10, true, true);
    animation[Anim_HarkonnenShoulder]->setFrameRate(1.0);
    animation[Anim_AtreidesEyes] = menshpa->getAnimation(0, 4, true, true);
    animation[Anim_AtreidesEyes]->setFrameRate(0.5);
    animation[Anim_AtreidesMouth] = menshpa->getAnimation(5, 9, true, true, true);
    animation[Anim_AtreidesMouth]->setFrameRate(5.0);
    animation[Anim_AtreidesShoulder] = menshpa->getAnimation(10, 10, true, true);
    animation[Anim_AtreidesShoulder]->setFrameRate(1.0);
    animation[Anim_AtreidesBook] = menshpa->getAnimation(11, 12, true, true, true);
    animation[Anim_AtreidesBook]->setNumLoops(1);
    animation[Anim_AtreidesBook]->setFrameRate(0.2);
    animation[Anim_OrdosEyes] = menshpo->getAnimation(0, 4, true, true);
    animation[Anim_OrdosEyes]->setFrameRate(0.5);
    animation[Anim_OrdosMouth] = menshpo->getAnimation(5, 9, true, true, true);
    animation[Anim_OrdosMouth]->setFrameRate(5.0);
    animation[Anim_OrdosShoulder] = menshpo->getAnimation(10, 10, true, true);
    animation[Anim_OrdosShoulder]->setFrameRate(1.0);
    animation[Anim_OrdosRing] = menshpo->getAnimation(11, 14, true, true, true);
    animation[Anim_OrdosRing]->setNumLoops(1);
    animation[Anim_OrdosRing]->setFrameRate(6.0);
    animation[Anim_FremenEyes]     = PictureFactory::mapMentatAnimationToFremen(animation[Anim_AtreidesEyes].get());
    animation[Anim_FremenMouth]    = PictureFactory::mapMentatAnimationToFremen(animation[Anim_AtreidesMouth].get());
    animation[Anim_FremenShoulder] = PictureFactory::mapMentatAnimationToFremen(animation[Anim_AtreidesShoulder].get());
    animation[Anim_FremenBook]     = PictureFactory::mapMentatAnimationToFremen(animation[Anim_AtreidesBook].get());
    animation[Anim_SardaukarEyes]  = PictureFactory::mapMentatAnimationToSardaukar(animation[Anim_HarkonnenEyes].get());
    animation[Anim_SardaukarMouth] =
        PictureFactory::mapMentatAnimationToSardaukar(animation[Anim_HarkonnenMouth].get());
    animation[Anim_SardaukarShoulder] =
        PictureFactory::mapMentatAnimationToSardaukar(animation[Anim_HarkonnenShoulder].get());
    animation[Anim_MercenaryEyes]  = PictureFactory::mapMentatAnimationToMercenary(animation[Anim_OrdosEyes].get());
    animation[Anim_MercenaryMouth] = PictureFactory::mapMentatAnimationToMercenary(animation[Anim_OrdosMouth].get());
    animation[Anim_MercenaryShoulder] =
        PictureFactory::mapMentatAnimationToMercenary(animation[Anim_OrdosShoulder].get());
    animation[Anim_MercenaryRing] = PictureFactory::mapMentatAnimationToMercenary(animation[Anim_OrdosRing].get());

    animation[Anim_BeneEyes] = menshpm->getAnimation(0, 4, true, true);
    if (animation[Anim_BeneEyes] != nullptr) {
        animation[Anim_BeneEyes]->setPalette(benePalette);
        animation[Anim_BeneEyes]->setFrameRate(0.5);
    }
    animation[Anim_BeneMouth] = menshpm->getAnimation(5, 9, true, true, true);
    if (animation[Anim_BeneMouth] != nullptr) {
        animation[Anim_BeneMouth]->setPalette(benePalette);
        animation[Anim_BeneMouth]->setFrameRate(5.0);
    }
    // the remaining animation are loaded on demand to save some loading time

    // load map choice pieces
    for (int i = 0; i < NUM_MAPCHOICEPIECES; i++) {
        mapChoicePieces[i][static_cast<int>(HOUSETYPE::HOUSE_HARKONNEN)] =
            Scaler::doubleSurfaceNN(pieces->getPicture(i).get());
        SDL_SetColorKey(mapChoicePieces[i][static_cast<int>(HOUSETYPE::HOUSE_HARKONNEN)].get(), SDL_TRUE, 0);
    }

    // pBackgroundSurface is separate as we never draw it but use it to construct other sprites
    pBackgroundSurface = convertSurfaceToDisplayFormat(picFactory_.createBackground().get());

    { // Scope
        auto replace_color = [&](ObjPic_enum id, HOUSETYPE house, int zoom, uint32_t oldColor, uint32_t newColor) {
            auto display_surface = convertSurfaceToDisplayFormat(getZoomedObjSurface(id, house, zoom));

            replaceColor(display_surface.get(), oldColor, newColor);

            if (SDL_SetSurfaceBlendMode(display_surface.get(), SDL_BlendMode::SDL_BLENDMODE_BLEND))
                THROW(std::runtime_error,
                      std::string("SurfaceLoader(): SDL_SetSurfaceBlendMode() failed: ") + std::string(SDL_GetError()));

            objPic[static_cast<int>(id)][static_cast<int>(house)][zoom] = std::move(display_surface);
        };

        for (auto zoom = 0; zoom < NUM_ZOOMLEVEL; ++zoom) {
            for (auto h = 0; h < static_cast<int>(HOUSETYPE::NUM_HOUSES); ++h) {
                const auto house = static_cast<HOUSETYPE>(h);

                // Create the per-house Windtrap surfaces
                (void)getZoomedObjSurface(static_cast<unsigned int>(ObjPic_Windtrap), house, zoom);

                replace_color(ObjPic_CarryallShadow, house, zoom, COLOR_BLACK, COLOR_SHADOW_TRANSPARENT);
                replace_color(ObjPic_FrigateShadow, house, zoom, COLOR_BLACK, COLOR_SHADOW_TRANSPARENT);
                replace_color(ObjPic_OrnithopterShadow, house, zoom, COLOR_BLACK, COLOR_SHADOW_TRANSPARENT);
            }
        }

        for (auto zoom = 0; zoom < NUM_ZOOMLEVEL; ++zoom) {
            for (auto h = 0; h < static_cast<int>(HOUSETYPE::NUM_HOUSES); ++h) {
                const auto house = static_cast<HOUSETYPE>(h);

                auto& windtrap = objPic[ObjPic_Windtrap][h][zoom];

                if (!windtrap)
                    THROW(std::runtime_error,
                          fmt::format("SurfaceLoader(): Windtrap for house {} and zoom {} does not exist!", h, zoom)
                              .c_str());

                // Windtrap uses palette animation on PALCOLOR_WINDTRAP_COLORCYCLE; fake this
                windtrap = generateWindtrapAnimationFrames(windtrap.get());

                replace_color(ObjPic_Windtrap, house, zoom, COLOR_BLACK, COLOR_FOG_TRANSPARENT);
            }
        }
    }

    // Create map choice arrows
    { // Scope
        for (auto id = UI_MapChoiceArrow_None; id <= UI_MapChoiceArrow_Left;
             id      = static_cast<UIGraphics_Enum>(id + 1)) {
            sdl2::surface_ptr source;

            for_each_housetype([&](const auto& house) {
                auto& surface = uiGraphic[static_cast<int>(id)][static_cast<int>(house)];

                if (HOUSETYPE::HOUSE_HARKONNEN == house)
                    source = std::move(surface);

                if (!source)
                    THROW(std::runtime_error, "No source surface for generating id %d for house %d",
                          static_cast<int>(id), static_cast<int>(house));

                assert(!surface);

                surface = std::move(generateMapChoiceArrowFrames(source.get(), house));
            });
        }
    }
}

SurfaceLoader::~SurfaceLoader() = default;

SDL_Surface* SurfaceLoader::getZoomedObjSurface(unsigned int id, HOUSETYPE house, unsigned int z) {
    if (id >= NUM_OBJPICS) {
        THROW(std::invalid_argument, "SurfaceLoader::getZoomedObjSurface(): Unit Picture with ID %u is not available!",
              id);
    }

    const auto idx = static_cast<int>(house);

    auto& surface = objPic[id][idx][z];

    if (surface == nullptr) {
        constexpr auto harkonnen = static_cast<int>(HOUSETYPE::HOUSE_HARKONNEN);

        // remap to this color
        if (objPic[id][harkonnen][z] == nullptr) {
            THROW(std::runtime_error, "SurfaceLoader::getZoomedObjPic(): Unit Picture with ID %u is not loaded!", id);
        }

        surface = mapSurfaceColorRange(objPic[id][harkonnen][z].get(), PALCOLOR_HARKONNEN, houseToPaletteIndex[idx]);
    }

    return surface.get();
}

SDL_Surface* SurfaceLoader::getSmallDetailSurface(unsigned int id) {
    if (id >= NUM_SMALLDETAILPICS) {
        return nullptr;
    }
    return smallDetailPic[id].get();
}

SDL_Surface* SurfaceLoader::getTinyPictureSurface(unsigned int id) {
    if (id >= NUM_TINYPICTURE) {
        return nullptr;
    }
    return tinyPicture[id].get();
}

SDL_Surface* SurfaceLoader::getUIGraphicSurface(unsigned int id, HOUSETYPE house) {
    if (id >= NUM_UIGRAPHICS) {
        THROW(std::invalid_argument, "SurfaceLoader::getUIGraphicSurface(): UI Graphic with ID %u is not available!",
              id);
    }

    auto& target = uiGraphic[id][static_cast<int>(house)];

    if (target == nullptr) {
        auto* const harkonnen = uiGraphic[id][static_cast<int>(HOUSETYPE::HOUSE_HARKONNEN)].get();

        if (harkonnen == nullptr) {
            THROW(std::runtime_error, "SurfaceLoader::getUIGraphicSurface(): UI Graphic with ID %u is not loaded!", id);
        }

        // remap to this color
        target = mapSurfaceColorRange(harkonnen, PALCOLOR_HARKONNEN, houseToPaletteIndex[static_cast<int>(house)]);
    }

    return target.get();
}

SDL_Surface* SurfaceLoader::getMapChoicePieceSurface(unsigned int num, HOUSETYPE house) {
    if (num >= NUM_MAPCHOICEPIECES) {
        THROW(std::invalid_argument,
              "SurfaceLoader::getMapChoicePieceSurface(): Map Piece with number %u is not available!", num);
    }

    if (mapChoicePieces[num][static_cast<int>(house)] == nullptr) {
        // remap to this color
        if (mapChoicePieces[num][static_cast<int>(HOUSETYPE::HOUSE_HARKONNEN)] == nullptr) {
            THROW(std::runtime_error,
                  "SurfaceLoader::getMapChoicePieceSurface(): Map Piece with number %u is not loaded!", num);
        }

        mapChoicePieces[num][static_cast<int>(house)] =
            mapSurfaceColorRange(mapChoicePieces[num][static_cast<int>(HOUSETYPE::HOUSE_HARKONNEN)].get(),
                                 PALCOLOR_HARKONNEN, houseToPaletteIndex[static_cast<int>(house)]);
    }

    return mapChoicePieces[num][static_cast<int>(house)].get();
}

Animation* SurfaceLoader::getAnimation(unsigned int id) {
    if (id >= NUM_ANIMATION) {
        THROW(std::invalid_argument, "SurfaceLoader::getAnimation(): Animation with ID %u is not available!", id);
    }

    if (animation[id] == nullptr) {
        switch (id) {
            case Anim_HarkonnenPlanet: {
                animation[Anim_HarkonnenPlanet] = loadAnimationFromWsa("FHARK.WSA");
                animation[Anim_HarkonnenPlanet]->setFrameRate(10);
            } break;

            case Anim_AtreidesPlanet: {
                animation[Anim_AtreidesPlanet] = loadAnimationFromWsa("FARTR.WSA");
                animation[Anim_AtreidesPlanet]->setFrameRate(10);
            } break;

            case Anim_OrdosPlanet: {
                animation[Anim_OrdosPlanet] = loadAnimationFromWsa("FORDOS.WSA");
                animation[Anim_OrdosPlanet]->setFrameRate(10);
            } break;

            case Anim_FremenPlanet: {
                animation[Anim_FremenPlanet] = PictureFactory::createFremenPlanet(
                    uiGraphic[UI_Herald_ColoredLarge][static_cast<int>(HOUSETYPE::HOUSE_FREMEN)].get());
                animation[Anim_FremenPlanet]->setFrameRate(10);
            } break;

            case Anim_SardaukarPlanet: {
                animation[Anim_SardaukarPlanet] = PictureFactory::createSardaukarPlanet(
                    getAnimation(Anim_OrdosPlanet),
                    uiGraphic[UI_Herald_ColoredLarge][static_cast<int>(HOUSETYPE::HOUSE_SARDAUKAR)].get());
                animation[Anim_SardaukarPlanet]->setFrameRate(10);
            } break;

            case Anim_MercenaryPlanet: {
                animation[Anim_MercenaryPlanet] = PictureFactory::createMercenaryPlanet(
                    getAnimation(Anim_AtreidesPlanet),
                    uiGraphic[UI_Herald_ColoredLarge][static_cast<int>(HOUSETYPE::HOUSE_MERCENARY)].get());
                animation[Anim_MercenaryPlanet]->setFrameRate(10);
            } break;

            case Anim_Win1: animation[Anim_Win1] = loadAnimationFromWsa("WIN1.WSA"); break;
            case Anim_Win2: animation[Anim_Win2] = loadAnimationFromWsa("WIN2.WSA"); break;
            case Anim_Lose1: animation[Anim_Lose1] = loadAnimationFromWsa("LOSTBILD.WSA"); break;
            case Anim_Lose2: animation[Anim_Lose2] = loadAnimationFromWsa("LOSTVEHC.WSA"); break;
            case Anim_Barracks: animation[Anim_Barracks] = loadAnimationFromWsa("BARRAC.WSA"); break;
            case Anim_Carryall: animation[Anim_Carryall] = loadAnimationFromWsa("CARRYALL.WSA"); break;
            case Anim_ConstructionYard: animation[Anim_ConstructionYard] = loadAnimationFromWsa("CONSTRUC.WSA"); break;
            case Anim_Fremen: animation[Anim_Fremen] = loadAnimationFromWsa("FREMEN.WSA"); break;
            case Anim_DeathHand: animation[Anim_DeathHand] = loadAnimationFromWsa("GOLD-BB.WSA"); break;
            case Anim_Devastator: animation[Anim_Devastator] = loadAnimationFromWsa("HARKTANK.WSA"); break;
            case Anim_Harvester: animation[Anim_Harvester] = loadAnimationFromWsa("HARVEST.WSA"); break;
            case Anim_Radar: animation[Anim_Radar] = loadAnimationFromWsa("HEADQRTS.WSA"); break;
            case Anim_HighTechFactory: animation[Anim_HighTechFactory] = loadAnimationFromWsa("HITCFTRY.WSA"); break;
            case Anim_SiegeTank: animation[Anim_SiegeTank] = loadAnimationFromWsa("HTANK.WSA"); break;
            case Anim_HeavyFactory: animation[Anim_HeavyFactory] = loadAnimationFromWsa("HVYFTRY.WSA"); break;
            case Anim_Trooper: animation[Anim_Trooper] = loadAnimationFromWsa("HYINFY.WSA"); break;
            case Anim_Infantry: animation[Anim_Infantry] = loadAnimationFromWsa("INFANTRY.WSA"); break;
            case Anim_IX: animation[Anim_IX] = loadAnimationFromWsa("IX.WSA"); break;
            case Anim_LightFactory: animation[Anim_LightFactory] = loadAnimationFromWsa("LITEFTRY.WSA"); break;
            case Anim_Tank: animation[Anim_Tank] = loadAnimationFromWsa("LTANK.WSA"); break;
            case Anim_MCV: animation[Anim_MCV] = loadAnimationFromWsa("MCV.WSA"); break;
            case Anim_Deviator: animation[Anim_Deviator] = loadAnimationFromWsa("ORDRTANK.WSA"); break;
            case Anim_Ornithopter: animation[Anim_Ornithopter] = loadAnimationFromWsa("ORNI.WSA"); break;
            case Anim_Raider: animation[Anim_Raider] = loadAnimationFromWsa("OTRIKE.WSA"); break;
            case Anim_Palace: animation[Anim_Palace] = loadAnimationFromWsa("PALACE.WSA"); break;
            case Anim_Quad: animation[Anim_Quad] = loadAnimationFromWsa("QUAD.WSA"); break;
            case Anim_Refinery: animation[Anim_Refinery] = loadAnimationFromWsa("REFINERY.WSA"); break;
            case Anim_RepairYard: animation[Anim_RepairYard] = loadAnimationFromWsa("REPAIR.WSA"); break;
            case Anim_Launcher: animation[Anim_Launcher] = loadAnimationFromWsa("RTANK.WSA"); break;
            case Anim_RocketTurret: animation[Anim_RocketTurret] = loadAnimationFromWsa("RTURRET.WSA"); break;
            case Anim_Saboteur: animation[Anim_Saboteur] = loadAnimationFromWsa("SABOTURE.WSA"); break;
            case Anim_Slab1: animation[Anim_Slab1] = loadAnimationFromWsa("SLAB.WSA"); break;
            case Anim_SonicTank: animation[Anim_SonicTank] = loadAnimationFromWsa("STANK.WSA"); break;
            case Anim_StarPort: animation[Anim_StarPort] = loadAnimationFromWsa("STARPORT.WSA"); break;
            case Anim_Silo: animation[Anim_Silo] = loadAnimationFromWsa("STORAGE.WSA"); break;
            case Anim_Trike: animation[Anim_Trike] = loadAnimationFromWsa("TRIKE.WSA"); break;
            case Anim_GunTurret: animation[Anim_GunTurret] = loadAnimationFromWsa("TURRET.WSA"); break;
            case Anim_Wall: animation[Anim_Wall] = loadAnimationFromWsa("WALL.WSA"); break;
            case Anim_WindTrap: animation[Anim_WindTrap] = loadAnimationFromWsa("WINDTRAP.WSA"); break;
            case Anim_WOR: animation[Anim_WOR] = loadAnimationFromWsa("WOR.WSA"); break;
            case Anim_Sandworm: animation[Anim_Sandworm] = loadAnimationFromWsa("WORM.WSA"); break;
            case Anim_Sardaukar: animation[Anim_Sardaukar] = loadAnimationFromWsa("SARDUKAR.WSA"); break;
            case Anim_Frigate: {
                if (dune::globals::pFileManager->exists("FRIGATE.WSA")) {
                    animation[Anim_Frigate] = loadAnimationFromWsa("FRIGATE.WSA");
                } else {
                    // US-Version 1.07 does not contain FRIGATE.WSA
                    // We replace it with the starport
                    animation[Anim_Frigate] = loadAnimationFromWsa("STARPORT.WSA");
                }
            } break;
            case Anim_Slab4: animation[Anim_Slab4] = loadAnimationFromWsa("4SLAB.WSA"); break;

            default: {
                THROW(std::runtime_error, "SurfaceLoader::getAnimation(): Invalid animation ID %u", id);
            }
        }

        if (id >= Anim_Barracks && id <= Anim_Slab4) {
            animation[id]->setFrameRate(6);
        }
    }

    return animation[id].get();
}

sdl2::surface_ptr SurfaceLoader::createMainBackgroundSurface(int width, int height) const {
    auto surface = createBackgroundSurface(width, height);

    picFactory_.drawMainBackground(surface.get());

    return surface;
}

std::unique_ptr<Shpfile> SurfaceLoader::loadShpfile(const std::string& filename) const {
    try {
        return std::make_unique<Shpfile>(dune::globals::pFileManager->openFile(filename).get());
    } catch (std::exception& e) {
        THROW(std::runtime_error, "Error in file \"" + filename + "\":" + e.what());
    }
}

std::unique_ptr<Wsafile> SurfaceLoader::loadWsafile(const std::string& filename) const {
    try {
        return std::make_unique<Wsafile>(dune::globals::pFileManager->openFile(filename).get());
    } catch (std::exception& e) {
        THROW(std::runtime_error, std::string("Error in file \"" + filename + "\":") + e.what());
    }
}

sdl2::surface_ptr SurfaceLoader::extractSmallDetailPic(const std::string& filename) const {
    sdl2::surface_ptr pSurface{SDL_CreateRGBSurface(0, 91, 55, 8, 0, 0, 0, 0)};

    // create new picture surface
    if (pSurface == nullptr) {
        THROW(sdl_error, "Cannot create new surface: %s!", SDL_GetError());
    }

    { // Scope
        const auto myWsafile = std::make_unique<Wsafile>(dune::globals::pFileManager->openFile(filename).get());

        const sdl2::surface_ptr tmp{myWsafile->getPicture(0)};
        if (tmp == nullptr) {
            THROW(std::runtime_error, "Cannot decode first frame in file '%s'!", filename);
        }

        if (tmp->w != 184 || tmp->h != 112) {
            THROW(std::runtime_error, "Picture '%s' is not of size 184x112!", filename);
        }

        dune::globals::palette.applyToSurface(pSurface.get());

        const sdl2::surface_lock lock_out{pSurface.get()};
        const sdl2::surface_lock lock_in{tmp.get()};

        char* RESTRICT const out      = static_cast<char*>(lock_out.pixels());
        const char* RESTRICT const in = static_cast<const char*>(lock_in.pixels());

        // Now we can copy pixel by pixel
        for (auto y = 0; y < 55; y++) {
            for (auto x = 0; x < 91; x++) {
                out[y * pSurface->pitch + x] = in[(y * 2 + 1) * tmp->pitch + x * 2 + 1];
            }
        }
    }

    return pSurface;
}

sdl2::surface_ptr SurfaceLoader::createBackgroundSurface(int width, int height) const {
    return picFactory_.createBackground(width, height);
}

sdl2::surface_ptr SurfaceLoader::createBackgroundTileSurface() const {
    return picFactory_.createBackgroundTile();
}

std::unique_ptr<Animation> SurfaceLoader::loadAnimationFromWsa(const std::string& filename) const {
    const auto file    = dune::globals::pFileManager->openFile(filename);
    const auto wsafile = std::make_unique<Wsafile>(file.get());

    return wsafile->getAnimation(0, wsafile->getNumFrames() - 1, true, false);
}

sdl2::surface_ptr SurfaceLoader::generateWindtrapAnimationFrames(SDL_Surface* windtrapPic) const {
    static constexpr int windtrapColorQuantizizer = 255 / (NUM_WINDTRAP_ANIMATIONS / 2 - 2);

    const int windtrapSize = windtrapPic->h;
    const int sizeX        = NUM_WINDTRAP_ANIMATIONS_PER_ROW * windtrapSize;
    const int sizeY        = (2 + NUM_WINDTRAP_ANIMATIONS + NUM_WINDTRAP_ANIMATIONS_PER_ROW - 1)
                    / NUM_WINDTRAP_ANIMATIONS_PER_ROW * windtrapSize;
    sdl2::surface_ptr returnPic{SDL_CreateRGBSurface(0, sizeX, sizeY, SCREEN_BPP, RMASK, GMASK, BMASK, AMASK)};
    SDL_SetSurfaceBlendMode(returnPic.get(), SDL_BLENDMODE_NONE);

    // copy building phase
    SDL_Rect src  = {0, 0, 2 * windtrapSize, windtrapSize};
    SDL_Rect dest = src;
    SDL_BlitSurface(windtrapPic, &src, returnPic.get(), &dest);

    src.w = windtrapSize;
    dest.x += dest.w;
    dest.w = windtrapSize;

    for (auto i = 0; i < NUM_WINDTRAP_ANIMATIONS; i++) {
        src.x = i / 3 % 2 == 0 ? 2 * windtrapSize : 3 * windtrapSize;

        SDL_Color windtrapColor;
        if (i < NUM_WINDTRAP_ANIMATIONS / 2) {
            const auto val  = i * windtrapColorQuantizizer;
            windtrapColor.r = static_cast<uint8_t>(std::min(80, val));
            windtrapColor.g = static_cast<uint8_t>(std::min(80, val));
            windtrapColor.b = static_cast<uint8_t>(std::min(255, val));
            windtrapColor.a = 255;
        } else {
            const auto val  = (i - NUM_WINDTRAP_ANIMATIONS / 2) * windtrapColorQuantizizer;
            windtrapColor.r = static_cast<uint8_t>(std::max(0, 80 - val));
            windtrapColor.g = static_cast<uint8_t>(std::max(0, 80 - val));
            windtrapColor.b = static_cast<uint8_t>(std::max(0, 255 - val));
            windtrapColor.a = 255;
        }
        SDL_SetPaletteColors(windtrapPic->format->palette, &windtrapColor, PALCOLOR_WINDTRAP_COLORCYCLE, 1);

        SDL_BlitSurface(windtrapPic, &src, returnPic.get(), &dest);

        dest.x += dest.w;
        dest.y = dest.y + dest.h * (dest.x / sizeX);
        dest.x = dest.x % sizeX;
    }

    if (returnPic->w > 2048 || returnPic->h > 2048) {
        sdl2::log_info("Warning: Size of sprite sheet for windtrap is %dx%d; may exceed hardware limits on older GPUs!",
                       returnPic->w, returnPic->h);
    }

    return returnPic;
}

sdl2::surface_ptr SurfaceLoader::generateMapChoiceArrowFrames(SDL_Surface* arrowPic, HOUSETYPE house) {
    sdl2::surface_ptr returnPic{
        SDL_CreateRGBSurface(0, arrowPic->w * 4, arrowPic->h, SCREEN_BPP, RMASK, GMASK, BMASK, AMASK)};

    SDL_Rect dest = {0, 0, arrowPic->w, arrowPic->h};

    for (int i = 0; i < 4; i++) {
        for (int k = 0; k < 4; k++) {
            const auto house_index   = houseToPaletteIndex[static_cast<int>(house)];
            const auto* const colors = &dune::globals::palette[house_index + (i + k) % 4];

            SDL_SetPaletteColors(arrowPic->format->palette, colors, 251 + k, 1);
        }

        SDL_BlitSurface(arrowPic, nullptr, returnPic.get(), &dest);
        dest.x += dest.w;
    }

    return returnPic;
}

sdl2::surface_ptr SurfaceLoader::generateDoubledObjPic(unsigned int id, int h) const {
    sdl2::surface_ptr pSurface;

    const auto& name    = ObjPicNames.at(id);
    const auto filename = fmt::format("Mask_2x_{}.png", name);

    if (dune::globals::settings.video.scaler == "ScaleHD") {
        auto* const file_manager = dune::globals::pFileManager.get();

        if (file_manager->exists(filename)) {
            pSurface = sdl2::surface_ptr{
                Scaler::doubleTiledSurfaceNN(objPic[id][h][0].get(), objPicTiles[id].x, objPicTiles[id].y)};

            const sdl2::surface_ptr pOverlay = LoadPNG_RW(file_manager->openFile(filename).get());
            SDL_SetColorKey(pOverlay.get(), SDL_TRUE, PALCOLOR_UI_COLORCYCLE);

            // SDL_BlitSurface will silently map PALCOLOR_BLACK to PALCOLOR_TRANSPARENT as both are RGB(0,0,0,255), so
            // make them temporarily different
            pOverlay->format->palette->colors[PALCOLOR_BLACK].g = 1;
            pSurface->format->palette->colors[PALCOLOR_BLACK].g = 1;
            SDL_BlitSurface(pOverlay.get(), nullptr, pSurface.get(), nullptr);
            pOverlay->format->palette->colors[PALCOLOR_BLACK].g = 0;
            pSurface->format->palette->colors[PALCOLOR_BLACK].g = 0;
        } else {
            sdl2::log_info("Warning: No HD sprite sheet for '%s' in zoom level 1!", name);
            pSurface = sdl2::surface_ptr{
                Scaler::defaultDoubleTiledSurface(objPic[id][h][0].get(), objPicTiles[id].x, objPicTiles[id].y)};
        }
    } else {
        pSurface = sdl2::surface_ptr{
            Scaler::defaultDoubleTiledSurface(objPic[id][h][0].get(), objPicTiles[id].x, objPicTiles[id].y)};
    }

    if (pSurface->w > 2048 || pSurface->h > 2048) {
        sdl2::log_info("Warning: Size of sprite sheet for '%s' in zoom level 1 is %dx%d; may exceed hardware limits on "
                       "older GPUs!",
                       name, pSurface->w, pSurface->h);
    }

    return pSurface;
}

sdl2::surface_ptr SurfaceLoader::generateTripledObjPic(unsigned int id, int h) const {
    sdl2::surface_ptr pSurface;

    const auto& name = ObjPicNames.at(id);

    const auto filename = fmt::format("Mask_3x_{}.png", name);

    if (dune::globals::settings.video.scaler == "ScaleHD") {
        auto* const file_manager = dune::globals::pFileManager.get();
        if (file_manager->exists(filename)) {
            pSurface = sdl2::surface_ptr{
                Scaler::tripleTiledSurfaceNN(objPic[id][h][0].get(), objPicTiles[id].x, objPicTiles[id].y)};

            const sdl2::surface_ptr pOverlay = LoadPNG_RW(file_manager->openFile(filename).get());
            SDL_SetColorKey(pOverlay.get(), SDL_TRUE, PALCOLOR_UI_COLORCYCLE);

            // SDL_BlitSurface will silently map PALCOLOR_BLACK to PALCOLOR_TRANSPARENT as both are RGB(0,0,0,255), so
            // make them temporarily different
            pOverlay->format->palette->colors[PALCOLOR_BLACK].g = 1;
            pSurface->format->palette->colors[PALCOLOR_BLACK].g = 1;
            SDL_BlitSurface(pOverlay.get(), nullptr, pSurface.get(), nullptr);
            pOverlay->format->palette->colors[PALCOLOR_BLACK].g = 0;
            pSurface->format->palette->colors[PALCOLOR_BLACK].g = 0;
        } else {
            sdl2::log_info("Warning: No HD sprite sheet for '%s' in zoom level 2!", name);
            pSurface = sdl2::surface_ptr{
                Scaler::defaultTripleTiledSurface(objPic[id][h][0].get(), objPicTiles[id].x, objPicTiles[id].y)};
        }
    } else {
        pSurface = sdl2::surface_ptr{
            Scaler::defaultTripleTiledSurface(objPic[id][h][0].get(), objPicTiles[id].x, objPicTiles[id].y)};
    }

    if (pSurface->w > 2048 || pSurface->h > 2048) {
        sdl2::log_info("Warning: Size of sprite sheet for '%s' in zoom level 2 is %dx%d; may exceed hardware limits on "
                       "older GPUs!",
                       name, pSurface->w, pSurface->h);
    }

    return pSurface;
}
