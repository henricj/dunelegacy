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

#include <FileClasses/GFXManager.h>

#include <globals.h>

#include <FileClasses/FileManager.h>
#include <FileClasses/TextManager.h>
#include <FileClasses/PictureFactory.h>
#include <FileClasses/LoadSavePNG.h>
#include <FileClasses/Shpfile.h>
#include <FileClasses/Cpsfile.h>
#include <FileClasses/Icnfile.h>
#include <FileClasses/Wsafile.h>
#include <FileClasses/Palfile.h>

#include <misc/draw_util.h>
#include <misc/Scaler.h>
#include <misc/exceptions.h>

/**
    Number of columns and rows each obj pic has
*/
static const Coord objPicTiles[] {
    { 8, 1 },   // ObjPic_Tank_Base
    { 8, 1 },   // ObjPic_Tank_Gun
    { 8, 1 },   // ObjPic_Siegetank_Base
    { 8, 1 },   // ObjPic_Siegetank_Gun
    { 8, 1 },   // ObjPic_Devastator_Base
    { 8, 1 },   // ObjPic_Devastator_Gun
    { 8, 1 },   // ObjPic_Sonictank_Gun
    { 8, 1 },   // ObjPic_Launcher_Gun
    { 8, 1 },   // ObjPic_Quad
    { 8, 1 },   // ObjPic_Trike
    { 8, 1 },   // ObjPic_Harvester
    { 8, 3 },   // ObjPic_Harvester_Sand
    { 8, 1 },   // ObjPic_MCV
    { 8, 2 },   // ObjPic_Carryall
    { 8, 2 },   // ObjPic_CarryallShadow
    { 8, 1 },   // ObjPic_Frigate
    { 8, 1 },   // ObjPic_FrigateShadow
    { 8, 3 },   // ObjPic_Ornithopter
    { 8, 3 },   // ObjPic_OrnithopterShadow
    { 4, 3 },   // ObjPic_Trooper
    { 4, 3 },   // ObjPic_Troopers
    { 4, 3 },   // ObjPic_Soldier
    { 4, 3 },   // ObjPic_Infantry
    { 4, 3 },   // ObjPic_Saboteur
    { 1, 9 },   // ObjPic_Sandworm
    { 4, 1 },   // ObjPic_ConstructionYard
    { 4, 1 },   // ObjPic_Windtrap
    { 10, 1 },  // ObjPic_Refinery
    { 4, 1 },   // ObjPic_Barracks
    { 4, 1 },   // ObjPic_WOR
    { 4, 1 },   // ObjPic_Radar
    { 6, 1 },   // ObjPic_LightFactory
    { 4, 1 },   // ObjPic_Silo
    { 8, 1 },   // ObjPic_HeavyFactory
    { 8, 1 },   // ObjPic_HighTechFactory
    { 4, 1 },   // ObjPic_IX
    { 4, 1 },   // ObjPic_Palace
    { 10, 1 },  // ObjPic_RepairYard
    { 10, 1 },  // ObjPic_Starport
    { 10, 1 },  // ObjPic_GunTurret
    { 10, 1 },  // ObjPic_RocketTurret
    { 25, 3 },  // ObjPic_Wall
    { 16, 1 },  // ObjPic_Bullet_SmallRocket
    { 16, 1 },  // ObjPic_Bullet_MediumRocket
    { 16, 1 },  // ObjPic_Bullet_LargeRocket
    { 1, 1 },   // ObjPic_Bullet_Small
    { 1, 1 },   // ObjPic_Bullet_Medium
    { 1, 1 },   // ObjPic_Bullet_Large
    { 1, 1 },   // ObjPic_Bullet_Sonic
    { 1, 1 },   // ObjPic_Bullet_SonicTemp
    { 5, 1 },   // ObjPic_Hit_Gas
    { 1, 1 },   // ObjPic_Hit_ShellSmall
    { 1, 1 },   // ObjPic_Hit_ShellMedium
    { 1, 1 },   // ObjPic_Hit_ShellLarge
    { 5, 1 },   // ObjPic_ExplosionSmall
    { 5, 1 },   // ObjPic_ExplosionMedium1
    { 5, 1 },   // ObjPic_ExplosionMedium2
    { 5, 1 },   // ObjPic_ExplosionLarge1
    { 5, 1 },   // ObjPic_ExplosionLarge2
    { 2, 1 },   // ObjPic_ExplosionSmallUnit
    { 21, 1 },  // ObjPic_ExplosionFlames
    { 3, 1 },   // ObjPic_ExplosionSpiceBloom
    { 6, 1 },   // ObjPic_DeadInfantry
    { 6, 1 },   // ObjPic_DeadAirUnit
    { 3, 1 },   // ObjPic_Smoke
    { 1, 1 },   // ObjPic_SandwormShimmerMask
    { 1, 1 },   // ObjPic_SandwormShimmerTemp
    { NUM_TERRAIN_TILES_X, NUM_TERRAIN_TILES_Y },  // ObjPic_Terrain
    { 14, 1 },  // ObjPic_DestroyedStructure
    { 6, 1 },   // ObjPic_RockDamage
    { 3, 1 },   // ObjPic_SandDamage
    { 16, 1 },  // ObjPic_Terrain_Hidden
    { 16, 1 },  // ObjPic_Terrain_HiddenFog
    { 8, 1 },   // ObjPic_Terrain_Tracks
    { 1, 1 },   // ObjPic_Star
};


GFXManager::GFXManager() {

    // open all shp files
    std::unique_ptr<Shpfile> units = loadShpfile("UNITS.SHP");
    std::unique_ptr<Shpfile> units1 = loadShpfile("UNITS1.SHP");
    std::unique_ptr<Shpfile> units2 = loadShpfile("UNITS2.SHP");
    std::unique_ptr<Shpfile> mouse = loadShpfile("MOUSE.SHP");
    std::unique_ptr<Shpfile> shapes = loadShpfile("SHAPES.SHP");
    std::unique_ptr<Shpfile> menshpa = loadShpfile("MENSHPA.SHP");
    std::unique_ptr<Shpfile> menshph = loadShpfile("MENSHPH.SHP");
    std::unique_ptr<Shpfile> menshpo = loadShpfile("MENSHPO.SHP");
    std::unique_ptr<Shpfile> menshpm = loadShpfile("MENSHPM.SHP");

    std::unique_ptr<Shpfile> choam;
    if(pFileManager->exists("CHOAM." + _("LanguageFileExtension"))) {
        choam = loadShpfile("CHOAM." + _("LanguageFileExtension"));
    } else if(pFileManager->exists("CHOAMSHP.SHP")) {
        choam = loadShpfile("CHOAMSHP.SHP");
    } else {
        THROW(std::runtime_error, "GFXManager::GFXManager(): Cannot open CHOAMSHP.SHP or CHOAM."+_("LanguageFileExtension")+"!");
    }

    std::unique_ptr<Shpfile> bttn;
    if(pFileManager->exists("BTTN." + _("LanguageFileExtension"))) {
        bttn = loadShpfile("BTTN." + _("LanguageFileExtension"));
    } else {
        // The US-Version has the buttons in SHAPES.SHP
        // => bttn == nullptr
    }

    std::unique_ptr<Shpfile> mentat;
    if(pFileManager->exists("MENTAT." + _("LanguageFileExtension"))) {
        mentat = loadShpfile("MENTAT." + _("LanguageFileExtension"));
    } else {
        mentat = loadShpfile("MENTAT.SHP");
    }

    std::unique_ptr<Shpfile> pieces = loadShpfile("PIECES.SHP");
    std::unique_ptr<Shpfile> arrows = loadShpfile("ARROWS.SHP");

    // Load icon file
    std::unique_ptr<Icnfile> icon = std::make_unique<Icnfile>(  pFileManager->openFile("ICON.ICN").get(),
                                                                pFileManager->openFile("ICON.MAP").get());

    // Load radar static
    std::unique_ptr<Wsafile> radar = loadWsafile("STATIC.WSA");

    // open bene palette
    Palette benePalette = LoadPalette_RW(pFileManager->openFile("BENE.PAL").get());

    //create PictureFactory
    std::unique_ptr<PictureFactory> PicFactory = std::make_unique<PictureFactory>();



    // load object pics in the original resolution
    objPic[ObjPic_Tank_Base][HOUSE_HARKONNEN][0] = units2->getPictureArray(8,1,GROUNDUNIT_ROW(0));
    objPic[ObjPic_Tank_Gun][HOUSE_HARKONNEN][0] = units2->getPictureArray(8,1,GROUNDUNIT_ROW(5));
    objPic[ObjPic_Siegetank_Base][HOUSE_HARKONNEN][0] = units2->getPictureArray(8,1,GROUNDUNIT_ROW(10));
    objPic[ObjPic_Siegetank_Gun][HOUSE_HARKONNEN][0] = units2->getPictureArray(8,1,GROUNDUNIT_ROW(15));
    objPic[ObjPic_Devastator_Base][HOUSE_HARKONNEN][0] = units2->getPictureArray(8,1,GROUNDUNIT_ROW(20));
    objPic[ObjPic_Devastator_Gun][HOUSE_HARKONNEN][0] = units2->getPictureArray(8,1,GROUNDUNIT_ROW(25));
    objPic[ObjPic_Sonictank_Gun][HOUSE_HARKONNEN][0] = units2->getPictureArray(8,1,GROUNDUNIT_ROW(30));
    objPic[ObjPic_Launcher_Gun][HOUSE_HARKONNEN][0] = units2->getPictureArray(8,1,GROUNDUNIT_ROW(35));
    objPic[ObjPic_Quad][HOUSE_HARKONNEN][0] = units->getPictureArray(8,1,GROUNDUNIT_ROW(0));
    objPic[ObjPic_Trike][HOUSE_HARKONNEN][0] = units->getPictureArray(8,1,GROUNDUNIT_ROW(5));
    objPic[ObjPic_Harvester][HOUSE_HARKONNEN][0] = units->getPictureArray(8,1,GROUNDUNIT_ROW(10));
    objPic[ObjPic_Harvester_Sand][HOUSE_HARKONNEN][0] = units1->getPictureArray(8,3,HARVESTERSAND_ROW(72),HARVESTERSAND_ROW(73),HARVESTERSAND_ROW(74));
    objPic[ObjPic_MCV][HOUSE_HARKONNEN][0] = units->getPictureArray(8,1,GROUNDUNIT_ROW(15));
    objPic[ObjPic_Carryall][HOUSE_HARKONNEN][0] = units->getPictureArray(8,2,AIRUNIT_ROW(45),AIRUNIT_ROW(48));
    objPic[ObjPic_CarryallShadow][HOUSE_HARKONNEN][0] = nullptr;    // create shadow after scaling
    objPic[ObjPic_Frigate][HOUSE_HARKONNEN][0] = units->getPictureArray(8,1,AIRUNIT_ROW(60));
    objPic[ObjPic_FrigateShadow][HOUSE_HARKONNEN][0] = nullptr;     // create shadow after scaling
    objPic[ObjPic_Ornithopter][HOUSE_HARKONNEN][0] = units->getPictureArray(8,3,ORNITHOPTER_ROW(51),ORNITHOPTER_ROW(52),ORNITHOPTER_ROW(53));
    objPic[ObjPic_OrnithopterShadow][HOUSE_HARKONNEN][0] = nullptr; // create shadow after scaling
    objPic[ObjPic_Trooper][HOUSE_HARKONNEN][0] = units->getPictureArray(4,3,INFANTRY_ROW(82),INFANTRY_ROW(83),INFANTRY_ROW(84));
    objPic[ObjPic_Troopers][HOUSE_HARKONNEN][0] = units->getPictureArray(4,4,MULTIINFANTRY_ROW(103),MULTIINFANTRY_ROW(104),MULTIINFANTRY_ROW(105),MULTIINFANTRY_ROW(106));
    objPic[ObjPic_Soldier][HOUSE_HARKONNEN][0] = units->getPictureArray(4,3,INFANTRY_ROW(73),INFANTRY_ROW(74),INFANTRY_ROW(75));
    objPic[ObjPic_Infantry][HOUSE_HARKONNEN][0] = units->getPictureArray(4,4,MULTIINFANTRY_ROW(91),MULTIINFANTRY_ROW(92),MULTIINFANTRY_ROW(93),MULTIINFANTRY_ROW(94));
    objPic[ObjPic_Saboteur][HOUSE_HARKONNEN][0] = units->getPictureArray(4,3,INFANTRY_ROW(63),INFANTRY_ROW(64),INFANTRY_ROW(65));
    objPic[ObjPic_Sandworm][HOUSE_HARKONNEN][0] = units1->getPictureArray(1,9,71|TILE_NORMAL,70|TILE_NORMAL,69|TILE_NORMAL,68|TILE_NORMAL,67|TILE_NORMAL,68|TILE_NORMAL,69|TILE_NORMAL,70|TILE_NORMAL,71|TILE_NORMAL);
    objPic[ObjPic_ConstructionYard][HOUSE_HARKONNEN][0] = icon->getPictureArray(17);
    objPic[ObjPic_Windtrap][HOUSE_HARKONNEN][0] = icon->getPictureArray(19);
    objPic[ObjPic_Refinery][HOUSE_HARKONNEN][0] = icon->getPictureArray(21);
    objPic[ObjPic_Barracks][HOUSE_HARKONNEN][0] = icon->getPictureArray(18);
    objPic[ObjPic_WOR][HOUSE_HARKONNEN][0] = icon->getPictureArray(16);
    objPic[ObjPic_Radar][HOUSE_HARKONNEN][0] = icon->getPictureArray(26);
    objPic[ObjPic_LightFactory][HOUSE_HARKONNEN][0] = icon->getPictureArray(12);
    objPic[ObjPic_Silo][HOUSE_HARKONNEN][0] = icon->getPictureArray(25);
    objPic[ObjPic_HeavyFactory][HOUSE_HARKONNEN][0] = icon->getPictureArray(13);
    objPic[ObjPic_HighTechFactory][HOUSE_HARKONNEN][0] = icon->getPictureArray(14);
    objPic[ObjPic_IX][HOUSE_HARKONNEN][0] = icon->getPictureArray(15);
    objPic[ObjPic_Palace][HOUSE_HARKONNEN][0] = icon->getPictureArray(11);
    objPic[ObjPic_RepairYard][HOUSE_HARKONNEN][0] = icon->getPictureArray(22);
    objPic[ObjPic_Starport][HOUSE_HARKONNEN][0] = icon->getPictureArray(20);
    objPic[ObjPic_GunTurret][HOUSE_HARKONNEN][0] = icon->getPictureArray(23);
    objPic[ObjPic_RocketTurret][HOUSE_HARKONNEN][0] = icon->getPictureArray(24);
    objPic[ObjPic_Wall][HOUSE_HARKONNEN][0] = icon->getPictureArray(6,25,3,1);
    objPic[ObjPic_Bullet_SmallRocket][HOUSE_HARKONNEN][0] = units->getPictureArray(16,1,ROCKET_ROW(35));
    objPic[ObjPic_Bullet_MediumRocket][HOUSE_HARKONNEN][0] = units->getPictureArray(16,1,ROCKET_ROW(20));
    objPic[ObjPic_Bullet_LargeRocket][HOUSE_HARKONNEN][0] = units->getPictureArray(16,1,ROCKET_ROW(40));
    objPic[ObjPic_Bullet_Small][HOUSE_HARKONNEN][0] = units1->getPicture(23);
    objPic[ObjPic_Bullet_Medium][HOUSE_HARKONNEN][0] = units1->getPicture(24);
    objPic[ObjPic_Bullet_Large][HOUSE_HARKONNEN][0] = units1->getPicture(25);
    objPic[ObjPic_Bullet_Sonic][HOUSE_HARKONNEN][0] = units1->getPicture(10);
    replaceColor(objPic[ObjPic_Bullet_Sonic][HOUSE_HARKONNEN][0].get(), PALCOLOR_WHITE, PALCOLOR_BLACK);
    objPic[ObjPic_Bullet_SonicTemp][HOUSE_HARKONNEN][0] = units1->getPicture(10);
    objPic[ObjPic_Hit_Gas][HOUSE_ORDOS][0] = units1->getPictureArray(5,1,57|TILE_NORMAL,58|TILE_NORMAL,59|TILE_NORMAL,60|TILE_NORMAL,61|TILE_NORMAL);
    objPic[ObjPic_Hit_Gas][HOUSE_HARKONNEN][0] = mapSurfaceColorRange(objPic[ObjPic_Hit_Gas][HOUSE_ORDOS][0].get(), PALCOLOR_ORDOS, PALCOLOR_HARKONNEN);
    objPic[ObjPic_Hit_ShellSmall][HOUSE_HARKONNEN][0] = units1->getPicture(2);
    objPic[ObjPic_Hit_ShellMedium][HOUSE_HARKONNEN][0] = units1->getPicture(3);
    objPic[ObjPic_Hit_ShellLarge][HOUSE_HARKONNEN][0] = units1->getPicture(4);
    objPic[ObjPic_ExplosionSmall][HOUSE_HARKONNEN][0] = units1->getPictureArray(5,1,32|TILE_NORMAL,33|TILE_NORMAL,34|TILE_NORMAL,35|TILE_NORMAL,36|TILE_NORMAL);
    objPic[ObjPic_ExplosionMedium1][HOUSE_HARKONNEN][0] = units1->getPictureArray(5,1,47|TILE_NORMAL,48|TILE_NORMAL,49|TILE_NORMAL,50|TILE_NORMAL,51|TILE_NORMAL);
    objPic[ObjPic_ExplosionMedium2][HOUSE_HARKONNEN][0] = units1->getPictureArray(5,1,52|TILE_NORMAL,53|TILE_NORMAL,54|TILE_NORMAL,55|TILE_NORMAL,56|TILE_NORMAL);
    objPic[ObjPic_ExplosionLarge1][HOUSE_HARKONNEN][0] = units1->getPictureArray(5,1,37|TILE_NORMAL,38|TILE_NORMAL,39|TILE_NORMAL,40|TILE_NORMAL,41|TILE_NORMAL);
    objPic[ObjPic_ExplosionLarge2][HOUSE_HARKONNEN][0] = units1->getPictureArray(5,1,42|TILE_NORMAL,43|TILE_NORMAL,44|TILE_NORMAL,45|TILE_NORMAL,46|TILE_NORMAL);
    objPic[ObjPic_ExplosionSmallUnit][HOUSE_HARKONNEN][0] = units1->getPictureArray(2,1,0|TILE_NORMAL,1|TILE_NORMAL);
    objPic[ObjPic_ExplosionFlames][HOUSE_HARKONNEN][0] = units1->getPictureArray(21,1,  11|TILE_NORMAL,12|TILE_NORMAL,13|TILE_NORMAL,17|TILE_NORMAL,18|TILE_NORMAL,19|TILE_NORMAL,17|TILE_NORMAL,
                                                                                    18|TILE_NORMAL,19|TILE_NORMAL,17|TILE_NORMAL,18|TILE_NORMAL,19|TILE_NORMAL,17|TILE_NORMAL,18|TILE_NORMAL,
                                                                                    19|TILE_NORMAL,17|TILE_NORMAL,18|TILE_NORMAL,19|TILE_NORMAL,20|TILE_NORMAL,21|TILE_NORMAL,22|TILE_NORMAL);
    objPic[ObjPic_ExplosionSpiceBloom][HOUSE_HARKONNEN][0] = units1->getPictureArray(3,1,7|TILE_NORMAL,6|TILE_NORMAL,5|TILE_NORMAL);
    objPic[ObjPic_DeadInfantry][HOUSE_HARKONNEN][0] = icon->getPictureArray(4,1,1,6);
    objPic[ObjPic_DeadAirUnit][HOUSE_HARKONNEN][0] = icon->getPictureArray(3,1,1,6);
    objPic[ObjPic_Smoke][HOUSE_HARKONNEN][0] = units1->getPictureArray(3,1,29|TILE_NORMAL,30|TILE_NORMAL,31|TILE_NORMAL);
    objPic[ObjPic_SandwormShimmerMask][HOUSE_HARKONNEN][0] = units1->getPicture(10);
    replaceColor(objPic[ObjPic_SandwormShimmerMask][HOUSE_HARKONNEN][0].get(), PALCOLOR_WHITE, PALCOLOR_BLACK);
    objPic[ObjPic_SandwormShimmerTemp][HOUSE_HARKONNEN][0] = units1->getPicture(10);
    objPic[ObjPic_Terrain][HOUSE_HARKONNEN][0] = icon->getPictureRow(124,209,NUM_TERRAIN_TILES_X);
    objPic[ObjPic_DestroyedStructure][HOUSE_HARKONNEN][0] = icon->getPictureRow2(14, 33, 125, 213, 214, 215, 223, 224, 225, 232, 233, 234, 240, 246, 247);
    objPic[ObjPic_RockDamage][HOUSE_HARKONNEN][0] = icon->getPictureRow(1,6);
    objPic[ObjPic_SandDamage][HOUSE_HARKONNEN][0] = icon->getPictureRow(7,12);
    objPic[ObjPic_Terrain_Hidden][HOUSE_HARKONNEN][0] = icon->getPictureRow(108,123);
    objPic[ObjPic_Terrain_HiddenFog][HOUSE_HARKONNEN][0] = icon->getPictureRow(108,123);
    objPic[ObjPic_Terrain_Tracks][HOUSE_HARKONNEN][0] = icon->getPictureRow(25,32);
    objPic[ObjPic_Star][HOUSE_HARKONNEN][0] = LoadPNG_RW(pFileManager->openFile("Star5x5.png").get());
    objPic[ObjPic_Star][HOUSE_HARKONNEN][1] = LoadPNG_RW(pFileManager->openFile("Star7x7.png").get());
    objPic[ObjPic_Star][HOUSE_HARKONNEN][2] = LoadPNG_RW(pFileManager->openFile("Star11x11.png").get());

    SDL_Color fogTransparent = { 0, 0, 0, 96};
    SDL_SetPaletteColors(objPic[ObjPic_Terrain_HiddenFog][HOUSE_HARKONNEN][0]->format->palette, &fogTransparent, PALCOLOR_BLACK, 1);

    // scale obj pics and apply color key
    for(int id = 0; id < NUM_OBJPICS; id++) {
        for(int h = 0; h < (int) NUM_HOUSES; h++) {
            if(objPic[id][h][0] != nullptr) {
                if(objPic[id][h][1] == nullptr) {
                    objPic[id][h][1] = generateDoubledObjPic(id, h);
                }
                SDL_SetColorKey(objPic[id][h][1].get(), SDL_TRUE, PALCOLOR_TRANSPARENT);

                if(objPic[id][h][2] == nullptr) {
                    objPic[id][h][2] = generateTripledObjPic(id, h);
                }
                SDL_SetColorKey(objPic[id][h][2].get(), SDL_TRUE, PALCOLOR_TRANSPARENT);

                SDL_SetColorKey(objPic[id][h][0].get(), SDL_TRUE, PALCOLOR_TRANSPARENT);
            }
        }
    }

    objPic[ObjPic_CarryallShadow][HOUSE_HARKONNEN][0] = createShadowSurface(objPic[ObjPic_Carryall][HOUSE_HARKONNEN][0].get());
    objPic[ObjPic_CarryallShadow][HOUSE_HARKONNEN][1] = createShadowSurface(objPic[ObjPic_Carryall][HOUSE_HARKONNEN][1].get());
    objPic[ObjPic_CarryallShadow][HOUSE_HARKONNEN][2] = createShadowSurface(objPic[ObjPic_Carryall][HOUSE_HARKONNEN][2].get());
    objPic[ObjPic_FrigateShadow][HOUSE_HARKONNEN][0] = createShadowSurface(objPic[ObjPic_Frigate][HOUSE_HARKONNEN][0].get());
    objPic[ObjPic_FrigateShadow][HOUSE_HARKONNEN][1] = createShadowSurface(objPic[ObjPic_Frigate][HOUSE_HARKONNEN][1].get());
    objPic[ObjPic_FrigateShadow][HOUSE_HARKONNEN][2] = createShadowSurface(objPic[ObjPic_Frigate][HOUSE_HARKONNEN][2].get());
    objPic[ObjPic_OrnithopterShadow][HOUSE_HARKONNEN][0] = createShadowSurface(objPic[ObjPic_Ornithopter][HOUSE_HARKONNEN][0].get());
    objPic[ObjPic_OrnithopterShadow][HOUSE_HARKONNEN][1] = createShadowSurface(objPic[ObjPic_Ornithopter][HOUSE_HARKONNEN][1].get());
    objPic[ObjPic_OrnithopterShadow][HOUSE_HARKONNEN][2] = createShadowSurface(objPic[ObjPic_Ornithopter][HOUSE_HARKONNEN][2].get());

    // load small detail pics
    smallDetailPicTex[Picture_Barracks] = extractSmallDetailPic("BARRAC.WSA");
    smallDetailPicTex[Picture_ConstructionYard] = extractSmallDetailPic("CONSTRUC.WSA");
    smallDetailPicTex[Picture_Carryall] = extractSmallDetailPic("CARRYALL.WSA");
    smallDetailPicTex[Picture_Devastator] = extractSmallDetailPic("HARKTANK.WSA");
    smallDetailPicTex[Picture_Deviator] = extractSmallDetailPic("ORDRTANK.WSA");
    smallDetailPicTex[Picture_DeathHand] = extractSmallDetailPic("GOLD-BB.WSA");
    smallDetailPicTex[Picture_Fremen] = extractSmallDetailPic("FREMEN.WSA");
    if(pFileManager->exists("FRIGATE.WSA")) {
        smallDetailPicTex[Picture_Frigate] = extractSmallDetailPic("FRIGATE.WSA");
    } else {
        // US-Version 1.07 does not contain FRIGATE.WSA
        // We replace it with the starport
        smallDetailPicTex[Picture_Frigate] = extractSmallDetailPic("STARPORT.WSA");
    }
    smallDetailPicTex[Picture_GunTurret] = extractSmallDetailPic("TURRET.WSA");
    smallDetailPicTex[Picture_Harvester] = extractSmallDetailPic("HARVEST.WSA");
    smallDetailPicTex[Picture_HeavyFactory] = extractSmallDetailPic("HVYFTRY.WSA");
    smallDetailPicTex[Picture_HighTechFactory] = extractSmallDetailPic("HITCFTRY.WSA");
    smallDetailPicTex[Picture_Soldier] = extractSmallDetailPic("INFANTRY.WSA");
    smallDetailPicTex[Picture_IX] = extractSmallDetailPic("IX.WSA");
    smallDetailPicTex[Picture_Launcher] = extractSmallDetailPic("RTANK.WSA");
    smallDetailPicTex[Picture_LightFactory] = extractSmallDetailPic("LITEFTRY.WSA");
    smallDetailPicTex[Picture_MCV] = extractSmallDetailPic("MCV.WSA");
    smallDetailPicTex[Picture_Ornithopter] = extractSmallDetailPic("ORNI.WSA");
    smallDetailPicTex[Picture_Palace] = extractSmallDetailPic("PALACE.WSA");
    smallDetailPicTex[Picture_Quad] = extractSmallDetailPic("QUAD.WSA");
    smallDetailPicTex[Picture_Radar] = extractSmallDetailPic("HEADQRTS.WSA");
    smallDetailPicTex[Picture_RaiderTrike] = extractSmallDetailPic("OTRIKE.WSA");
    smallDetailPicTex[Picture_Refinery] = extractSmallDetailPic("REFINERY.WSA");
    smallDetailPicTex[Picture_RepairYard] = extractSmallDetailPic("REPAIR.WSA");
    smallDetailPicTex[Picture_RocketTurret] = extractSmallDetailPic("RTURRET.WSA");
    smallDetailPicTex[Picture_Saboteur] = extractSmallDetailPic("SABOTURE.WSA");
    smallDetailPicTex[Picture_Sandworm] = extractSmallDetailPic("WORM.WSA");
    smallDetailPicTex[Picture_Sardaukar] = extractSmallDetailPic("SARDUKAR.WSA");
    smallDetailPicTex[Picture_SiegeTank] = extractSmallDetailPic("HTANK.WSA");
    smallDetailPicTex[Picture_Silo] = extractSmallDetailPic("STORAGE.WSA");
    smallDetailPicTex[Picture_Slab1] = extractSmallDetailPic("SLAB.WSA");
    smallDetailPicTex[Picture_Slab4] = extractSmallDetailPic("4SLAB.WSA");
    smallDetailPicTex[Picture_SonicTank] = extractSmallDetailPic("STANK.WSA");
    smallDetailPicTex[Picture_Special]  = nullptr;
    smallDetailPicTex[Picture_StarPort] = extractSmallDetailPic("STARPORT.WSA");
    smallDetailPicTex[Picture_Tank] = extractSmallDetailPic("LTANK.WSA");
    smallDetailPicTex[Picture_Trike] = extractSmallDetailPic("TRIKE.WSA");
    smallDetailPicTex[Picture_Trooper] = extractSmallDetailPic("HYINFY.WSA");
    smallDetailPicTex[Picture_Wall] = extractSmallDetailPic("WALL.WSA");
    smallDetailPicTex[Picture_WindTrap] = extractSmallDetailPic("WINDTRAP.WSA");
    smallDetailPicTex[Picture_WOR] = extractSmallDetailPic("WOR.WSA");
    // unused: FARTR.WSA, FHARK.WSA, FORDOS.WSA


    tinyPictureTex[TinyPicture_Spice] = convertSurfaceToTexture(shapes->getPicture(94));
    tinyPictureTex[TinyPicture_Barracks] = convertSurfaceToTexture(shapes->getPicture(62));
    tinyPictureTex[TinyPicture_ConstructionYard] = convertSurfaceToTexture(shapes->getPicture(60));
    tinyPictureTex[TinyPicture_GunTurret] = convertSurfaceToTexture(shapes->getPicture(67));
    tinyPictureTex[TinyPicture_HeavyFactory] = convertSurfaceToTexture(shapes->getPicture(56));
    tinyPictureTex[TinyPicture_HighTechFactory] = convertSurfaceToTexture(shapes->getPicture(57));
    tinyPictureTex[TinyPicture_IX] = convertSurfaceToTexture(shapes->getPicture(58));
    tinyPictureTex[TinyPicture_LightFactory] = convertSurfaceToTexture(shapes->getPicture(55));
    tinyPictureTex[TinyPicture_Palace] = convertSurfaceToTexture(shapes->getPicture(54));
    tinyPictureTex[TinyPicture_Radar] = convertSurfaceToTexture(shapes->getPicture(70));
    tinyPictureTex[TinyPicture_Refinery] = convertSurfaceToTexture(shapes->getPicture(64));
    tinyPictureTex[TinyPicture_RepairYard] = convertSurfaceToTexture(shapes->getPicture(65));
    tinyPictureTex[TinyPicture_RocketTurret] = convertSurfaceToTexture(shapes->getPicture(68));
    tinyPictureTex[TinyPicture_Silo] = convertSurfaceToTexture(shapes->getPicture(69));
    tinyPictureTex[TinyPicture_Slab1] = convertSurfaceToTexture(shapes->getPicture(53));
    tinyPictureTex[TinyPicture_Slab4] = convertSurfaceToTexture(shapes->getPicture(71));
    tinyPictureTex[TinyPicture_StarPort] = convertSurfaceToTexture(shapes->getPicture(63));
    tinyPictureTex[TinyPicture_Wall] = convertSurfaceToTexture(shapes->getPicture(66));
    tinyPictureTex[TinyPicture_WindTrap] = convertSurfaceToTexture(shapes->getPicture(61));
    tinyPictureTex[TinyPicture_WOR] = convertSurfaceToTexture(shapes->getPicture(59));
    tinyPictureTex[TinyPicture_Carryall] = convertSurfaceToTexture(shapes->getPicture(77));
    tinyPictureTex[TinyPicture_Devastator] = convertSurfaceToTexture(shapes->getPicture(75));
    tinyPictureTex[TinyPicture_Deviator] = convertSurfaceToTexture(shapes->getPicture(86));
    tinyPictureTex[TinyPicture_Frigate] = convertSurfaceToTexture(shapes->getPicture(77));    // use carryall picture
    tinyPictureTex[TinyPicture_Harvester] = convertSurfaceToTexture(shapes->getPicture(88));
    tinyPictureTex[TinyPicture_Soldier] = convertSurfaceToTexture(shapes->getPicture(90));
    tinyPictureTex[TinyPicture_Launcher] = convertSurfaceToTexture(shapes->getPicture(73));
    tinyPictureTex[TinyPicture_MCV] = convertSurfaceToTexture(shapes->getPicture(89));
    tinyPictureTex[TinyPicture_Ornithopter] = convertSurfaceToTexture(shapes->getPicture(85));
    tinyPictureTex[TinyPicture_Quad] = convertSurfaceToTexture(shapes->getPicture(74));
    tinyPictureTex[TinyPicture_Saboteur] = convertSurfaceToTexture(shapes->getPicture(84));
    tinyPictureTex[TinyPicture_Sandworm] = convertSurfaceToTexture(shapes->getPicture(93));
    tinyPictureTex[TinyPicture_SiegeTank] = convertSurfaceToTexture(shapes->getPicture(72));
    tinyPictureTex[TinyPicture_SonicTank] = convertSurfaceToTexture(shapes->getPicture(79));
    tinyPictureTex[TinyPicture_Tank] = convertSurfaceToTexture(shapes->getPicture(78));
    tinyPictureTex[TinyPicture_Trike] = convertSurfaceToTexture(shapes->getPicture(80));
    tinyPictureTex[TinyPicture_RaiderTrike] = convertSurfaceToTexture(shapes->getPicture(87));
    tinyPictureTex[TinyPicture_Trooper] = convertSurfaceToTexture(shapes->getPicture(76));
    tinyPictureTex[TinyPicture_Special] = convertSurfaceToTexture(shapes->getPicture(75));    // use devastator picture
    tinyPictureTex[TinyPicture_Infantry] = convertSurfaceToTexture(shapes->getPicture(81));
    tinyPictureTex[TinyPicture_Troopers] = convertSurfaceToTexture(shapes->getPicture(91));

    // load UI graphics
    uiGraphic[UI_RadarAnimation][HOUSE_HARKONNEN] = Scaler::doubleSurfaceNN(radar->getAnimationAsPictureRow(NUM_STATIC_ANIMATIONS_PER_ROW).get());

    uiGraphic[UI_CursorNormal][HOUSE_HARKONNEN] = mouse->getPicture(0);
    SDL_SetColorKey(uiGraphic[UI_CursorNormal][HOUSE_HARKONNEN].get(), SDL_TRUE, 0);
    uiGraphic[UI_CursorUp][HOUSE_HARKONNEN] = mouse->getPicture(1);
    SDL_SetColorKey(uiGraphic[UI_CursorUp][HOUSE_HARKONNEN].get(), SDL_TRUE, 0);
    uiGraphic[UI_CursorRight][HOUSE_HARKONNEN] = mouse->getPicture(2);
    SDL_SetColorKey(uiGraphic[UI_CursorRight][HOUSE_HARKONNEN].get(), SDL_TRUE, 0);
    uiGraphic[UI_CursorDown][HOUSE_HARKONNEN] = mouse->getPicture(3);
    SDL_SetColorKey(uiGraphic[UI_CursorDown][HOUSE_HARKONNEN].get(), SDL_TRUE, 0);
    uiGraphic[UI_CursorLeft][HOUSE_HARKONNEN] = mouse->getPicture(4);
    SDL_SetColorKey(uiGraphic[UI_CursorLeft][HOUSE_HARKONNEN].get() , SDL_TRUE, 0);

    uiGraphic[UI_CursorMove_Zoomlevel0][HOUSE_HARKONNEN] = mouse->getPicture(5);
    SDL_SetColorKey(uiGraphic[UI_CursorMove_Zoomlevel0][HOUSE_HARKONNEN].get(), SDL_TRUE, 0);
    uiGraphic[UI_CursorMove_Zoomlevel1][HOUSE_HARKONNEN] = Scaler::defaultDoubleTiledSurface(uiGraphic[UI_CursorMove_Zoomlevel0][HOUSE_HARKONNEN].get(), 1, 1);
    SDL_SetColorKey(uiGraphic[UI_CursorMove_Zoomlevel1][HOUSE_HARKONNEN].get(), SDL_TRUE, 0);
    uiGraphic[UI_CursorMove_Zoomlevel2][HOUSE_HARKONNEN] = Scaler::defaultTripleTiledSurface(uiGraphic[UI_CursorMove_Zoomlevel0][HOUSE_HARKONNEN].get(), 1, 1);
    SDL_SetColorKey(uiGraphic[UI_CursorMove_Zoomlevel2][HOUSE_HARKONNEN].get(), SDL_TRUE, 0);

    uiGraphic[UI_CursorAttack_Zoomlevel0][HOUSE_HARKONNEN] = mapSurfaceColorRange(uiGraphic[UI_CursorMove_Zoomlevel0][HOUSE_HARKONNEN].get(), 232, PALCOLOR_HARKONNEN);
    SDL_SetColorKey(uiGraphic[UI_CursorAttack_Zoomlevel0][HOUSE_HARKONNEN].get(), SDL_TRUE, 0);
    uiGraphic[UI_CursorAttack_Zoomlevel1][HOUSE_HARKONNEN] = Scaler::defaultDoubleTiledSurface(uiGraphic[UI_CursorAttack_Zoomlevel0][HOUSE_HARKONNEN].get(), 1, 1);
    SDL_SetColorKey(uiGraphic[UI_CursorAttack_Zoomlevel1][HOUSE_HARKONNEN].get(), SDL_TRUE, 0);
    uiGraphic[UI_CursorAttack_Zoomlevel2][HOUSE_HARKONNEN] = Scaler::defaultTripleTiledSurface(uiGraphic[UI_CursorAttack_Zoomlevel0][HOUSE_HARKONNEN].get(), 1, 1);
    SDL_SetColorKey(uiGraphic[UI_CursorAttack_Zoomlevel2][HOUSE_HARKONNEN].get(), SDL_TRUE, 0);

    uiGraphic[UI_CursorCapture_Zoomlevel0][HOUSE_HARKONNEN] = LoadPNG_RW(pFileManager->openFile("Capture.png").get());
    SDL_SetColorKey(uiGraphic[UI_CursorCapture_Zoomlevel0][HOUSE_HARKONNEN].get(), SDL_TRUE, 0);
    uiGraphic[UI_CursorCapture_Zoomlevel1][HOUSE_HARKONNEN] = Scaler::defaultDoubleTiledSurface(uiGraphic[UI_CursorCapture_Zoomlevel0][HOUSE_HARKONNEN].get(), 1, 1);
    SDL_SetColorKey(uiGraphic[UI_CursorCapture_Zoomlevel1][HOUSE_HARKONNEN].get(), SDL_TRUE, 0);
    uiGraphic[UI_CursorCapture_Zoomlevel2][HOUSE_HARKONNEN] = Scaler::defaultTripleTiledSurface(uiGraphic[UI_CursorCapture_Zoomlevel0][HOUSE_HARKONNEN].get(), 1, 1);
    SDL_SetColorKey(uiGraphic[UI_CursorCapture_Zoomlevel2][HOUSE_HARKONNEN].get(), SDL_TRUE, 0);

    uiGraphic[UI_CursorCarryallDrop_Zoomlevel0][HOUSE_HARKONNEN] = LoadPNG_RW(pFileManager->openFile("CarryallDrop.png").get());
    SDL_SetColorKey(uiGraphic[UI_CursorCarryallDrop_Zoomlevel0][HOUSE_HARKONNEN].get(), SDL_TRUE, 0);
    uiGraphic[UI_CursorCarryallDrop_Zoomlevel1][HOUSE_HARKONNEN] = Scaler::defaultDoubleTiledSurface(uiGraphic[UI_CursorCarryallDrop_Zoomlevel0][HOUSE_HARKONNEN].get(), 1, 1);
    SDL_SetColorKey(uiGraphic[UI_CursorCarryallDrop_Zoomlevel1][HOUSE_HARKONNEN].get(), SDL_TRUE, 0);
    uiGraphic[UI_CursorCarryallDrop_Zoomlevel2][HOUSE_HARKONNEN] = Scaler::defaultTripleTiledSurface(uiGraphic[UI_CursorCarryallDrop_Zoomlevel0][HOUSE_HARKONNEN].get(), 1, 1);
    SDL_SetColorKey(uiGraphic[UI_CursorCarryallDrop_Zoomlevel2][HOUSE_HARKONNEN].get(), SDL_TRUE, 0);

    uiGraphic[UI_ReturnIcon][HOUSE_HARKONNEN] = LoadPNG_RW(pFileManager->openFile("Return.png").get());
    SDL_SetColorKey(uiGraphic[UI_ReturnIcon][HOUSE_HARKONNEN].get(), SDL_TRUE, 0);

    uiGraphic[UI_DeployIcon][HOUSE_HARKONNEN] = LoadPNG_RW(pFileManager->openFile("Deploy.png").get());
    SDL_SetColorKey(uiGraphic[UI_DeployIcon][HOUSE_HARKONNEN].get(), SDL_TRUE, 0);

    uiGraphic[UI_DestructIcon][HOUSE_HARKONNEN] = LoadPNG_RW(pFileManager->openFile("Destruct.png").get());
    SDL_SetColorKey(uiGraphic[UI_DestructIcon][HOUSE_HARKONNEN].get(), SDL_TRUE, 0);

    uiGraphic[UI_SendToRepairIcon][HOUSE_HARKONNEN] = LoadPNG_RW(pFileManager->openFile("SendToRepair.png").get());
    SDL_SetColorKey(uiGraphic[UI_SendToRepairIcon][HOUSE_HARKONNEN].get(), SDL_TRUE, 0);

    uiGraphic[UI_CreditsDigits][HOUSE_HARKONNEN] = shapes->getPictureArray(10,1,2|TILE_NORMAL,3|TILE_NORMAL,4|TILE_NORMAL,5|TILE_NORMAL,6|TILE_NORMAL,
                                                                                7|TILE_NORMAL,8|TILE_NORMAL,9|TILE_NORMAL,10|TILE_NORMAL,11|TILE_NORMAL);
    uiGraphic[UI_SideBar][HOUSE_HARKONNEN] = PicFactory->createSideBar(false);
    uiGraphic[UI_Indicator][HOUSE_HARKONNEN] = units1->getPictureArray(3,1,8|TILE_NORMAL,9|TILE_NORMAL,10|TILE_NORMAL);
    SDL_SetColorKey(uiGraphic[UI_Indicator][HOUSE_HARKONNEN].get(), SDL_TRUE, 0);
    SDL_Color indicatorTransparent = { 255, 255, 255, 48 };
    SDL_SetPaletteColors(uiGraphic[UI_Indicator][HOUSE_HARKONNEN]->format->palette, &indicatorTransparent, PALCOLOR_WHITE, 1);
    uiGraphic[UI_InvalidPlace_Zoomlevel0][HOUSE_HARKONNEN] = PicFactory->createPlacingGrid(16, PALCOLOR_LIGHTRED);
    uiGraphic[UI_InvalidPlace_Zoomlevel1][HOUSE_HARKONNEN] = PicFactory->createPlacingGrid(32, PALCOLOR_LIGHTRED);
    uiGraphic[UI_InvalidPlace_Zoomlevel2][HOUSE_HARKONNEN] = PicFactory->createPlacingGrid(48, PALCOLOR_LIGHTRED);
    uiGraphic[UI_ValidPlace_Zoomlevel0][HOUSE_HARKONNEN] = PicFactory->createPlacingGrid(16, PALCOLOR_LIGHTGREEN);
    uiGraphic[UI_ValidPlace_Zoomlevel1][HOUSE_HARKONNEN] = PicFactory->createPlacingGrid(32, PALCOLOR_LIGHTGREEN);
    uiGraphic[UI_ValidPlace_Zoomlevel2][HOUSE_HARKONNEN] = PicFactory->createPlacingGrid(48, PALCOLOR_LIGHTGREEN);
    uiGraphic[UI_GreyPlace_Zoomlevel0][HOUSE_HARKONNEN] = PicFactory->createPlacingGrid(16, PALCOLOR_LIGHTGREY);
    uiGraphic[UI_GreyPlace_Zoomlevel1][HOUSE_HARKONNEN] = PicFactory->createPlacingGrid(32, PALCOLOR_LIGHTGREY);
    uiGraphic[UI_GreyPlace_Zoomlevel2][HOUSE_HARKONNEN] = PicFactory->createPlacingGrid(48, PALCOLOR_LIGHTGREY);
    uiGraphic[UI_MenuBackground][HOUSE_HARKONNEN] = PicFactory->createMainBackground();
    uiGraphic[UI_GameStatsBackground][HOUSE_HARKONNEN] = PicFactory->createGameStatsBackground(HOUSE_HARKONNEN);
    uiGraphic[UI_GameStatsBackground][HOUSE_ATREIDES] = PicFactory->createGameStatsBackground(HOUSE_ATREIDES);
    uiGraphic[UI_GameStatsBackground][HOUSE_ORDOS] = PicFactory->createGameStatsBackground(HOUSE_ORDOS);
    uiGraphic[UI_GameStatsBackground][HOUSE_FREMEN] = PicFactory->createGameStatsBackground(HOUSE_FREMEN);
    uiGraphic[UI_GameStatsBackground][HOUSE_SARDAUKAR] = PicFactory->createGameStatsBackground(HOUSE_SARDAUKAR);
    uiGraphic[UI_GameStatsBackground][HOUSE_MERCENARY] = PicFactory->createGameStatsBackground(HOUSE_MERCENARY);
    uiGraphic[UI_SelectionBox_Zoomlevel0][HOUSE_HARKONNEN] = LoadPNG_RW(pFileManager->openFile("UI_SelectionBox.png").get());
    SDL_SetColorKey(uiGraphic[UI_SelectionBox_Zoomlevel0][HOUSE_HARKONNEN].get(), SDL_TRUE, 0);
    uiGraphic[UI_SelectionBox_Zoomlevel1][HOUSE_HARKONNEN] = Scaler::defaultDoubleTiledSurface(uiGraphic[UI_SelectionBox_Zoomlevel0][HOUSE_HARKONNEN].get(), 1, 1);
    SDL_SetColorKey(uiGraphic[UI_SelectionBox_Zoomlevel1][HOUSE_HARKONNEN].get(), SDL_TRUE, 0);
    uiGraphic[UI_SelectionBox_Zoomlevel2][HOUSE_HARKONNEN] = Scaler::defaultTripleTiledSurface(uiGraphic[UI_SelectionBox_Zoomlevel0][HOUSE_HARKONNEN].get(), 1, 1);
    SDL_SetColorKey(uiGraphic[UI_SelectionBox_Zoomlevel2][HOUSE_HARKONNEN].get(), SDL_TRUE, 0);
    uiGraphic[UI_OtherPlayerSelectionBox_Zoomlevel0][HOUSE_HARKONNEN] = LoadPNG_RW(pFileManager->openFile("UI_OtherPlayerSelectionBox.png").get());
    SDL_SetColorKey(uiGraphic[UI_OtherPlayerSelectionBox_Zoomlevel0][HOUSE_HARKONNEN].get(), SDL_TRUE, 0);
    uiGraphic[UI_OtherPlayerSelectionBox_Zoomlevel1][HOUSE_HARKONNEN] = Scaler::defaultDoubleTiledSurface(uiGraphic[UI_OtherPlayerSelectionBox_Zoomlevel0][HOUSE_HARKONNEN].get(), 1, 1);
    SDL_SetColorKey(uiGraphic[UI_OtherPlayerSelectionBox_Zoomlevel1][HOUSE_HARKONNEN].get(), SDL_TRUE, 0);
    uiGraphic[UI_OtherPlayerSelectionBox_Zoomlevel2][HOUSE_HARKONNEN] = Scaler::defaultTripleTiledSurface(uiGraphic[UI_OtherPlayerSelectionBox_Zoomlevel0][HOUSE_HARKONNEN].get(), 1, 1);
    SDL_SetColorKey(uiGraphic[UI_OtherPlayerSelectionBox_Zoomlevel2][HOUSE_HARKONNEN].get(), SDL_TRUE, 0);
    uiGraphic[UI_TopBar][HOUSE_HARKONNEN] = PicFactory->createTopBar();
    uiGraphic[UI_ButtonUp][HOUSE_HARKONNEN] = choam->getPicture(0);
    uiGraphic[UI_ButtonUp_Pressed][HOUSE_HARKONNEN] = choam->getPicture(1);
    uiGraphic[UI_ButtonDown][HOUSE_HARKONNEN] = choam->getPicture(2);
    uiGraphic[UI_ButtonDown_Pressed][HOUSE_HARKONNEN] = choam->getPicture(3);
    uiGraphic[UI_BuilderListUpperCap][HOUSE_HARKONNEN] = PicFactory->createBuilderListUpperCap();
    uiGraphic[UI_BuilderListLowerCap][HOUSE_HARKONNEN] = PicFactory->createBuilderListLowerCap();
    uiGraphic[UI_CustomGamePlayersArrow][HOUSE_HARKONNEN] = LoadPNG_RW(pFileManager->openFile("CustomGamePlayers_Arrow.png").get());
    SDL_SetColorKey(uiGraphic[UI_CustomGamePlayersArrow][HOUSE_HARKONNEN].get(), SDL_TRUE, 0);
    uiGraphic[UI_CustomGamePlayersArrowNeutral][HOUSE_HARKONNEN] = LoadPNG_RW(pFileManager->openFile("CustomGamePlayers_ArrowNeutral.png").get());
    SDL_SetColorKey(uiGraphic[UI_CustomGamePlayersArrowNeutral][HOUSE_HARKONNEN].get(), SDL_TRUE, 0);
    uiGraphic[UI_MessageBox][HOUSE_HARKONNEN] = PicFactory->createMessageBoxBorder();

    if(bttn != nullptr) {
        uiGraphic[UI_Mentat][HOUSE_HARKONNEN] = bttn->getPicture(0);
        uiGraphic[UI_Mentat_Pressed][HOUSE_HARKONNEN] = bttn->getPicture(1);
        uiGraphic[UI_Options][HOUSE_HARKONNEN] = bttn->getPicture(2);
        uiGraphic[UI_Options_Pressed][HOUSE_HARKONNEN] = bttn->getPicture(3);
    } else {
        uiGraphic[UI_Mentat][HOUSE_HARKONNEN] = shapes->getPicture(94);
        uiGraphic[UI_Mentat_Pressed][HOUSE_HARKONNEN] = shapes->getPicture(95);
        uiGraphic[UI_Options][HOUSE_HARKONNEN] = shapes->getPicture(96);
        uiGraphic[UI_Options_Pressed][HOUSE_HARKONNEN] = shapes->getPicture(97);
    }

    uiGraphic[UI_Upgrade][HOUSE_HARKONNEN] = choam->getPicture(4);
    SDL_SetColorKey(uiGraphic[UI_Upgrade][HOUSE_HARKONNEN].get(), SDL_TRUE, 0);
    uiGraphic[UI_Upgrade_Pressed][HOUSE_HARKONNEN] = choam->getPicture(5);
    SDL_SetColorKey(uiGraphic[UI_Upgrade_Pressed][HOUSE_HARKONNEN].get(), SDL_TRUE, 0);
    uiGraphic[UI_Repair][HOUSE_HARKONNEN] = LoadPNG_RW(pFileManager->openFile("Button_Repair.png").get());
    uiGraphic[UI_Repair_Pressed][HOUSE_HARKONNEN] = LoadPNG_RW(pFileManager->openFile("Button_RepairPushed.png").get());
    uiGraphic[UI_Minus][HOUSE_HARKONNEN] = LoadPNG_RW(pFileManager->openFile("Button_Minus.png").get());
    uiGraphic[UI_Minus_Active][HOUSE_HARKONNEN] = mapSurfaceColorRange(uiGraphic[UI_Minus][HOUSE_HARKONNEN].get(), PALCOLOR_HARKONNEN, PALCOLOR_HARKONNEN-2);
    uiGraphic[UI_Minus_Pressed][HOUSE_HARKONNEN] = LoadPNG_RW(pFileManager->openFile("Button_MinusPushed.png").get());
    uiGraphic[UI_Plus][HOUSE_HARKONNEN] = LoadPNG_RW(pFileManager->openFile("Button_Plus.png").get());
    uiGraphic[UI_Plus_Active][HOUSE_HARKONNEN] = mapSurfaceColorRange(uiGraphic[UI_Plus][HOUSE_HARKONNEN].get(), PALCOLOR_HARKONNEN, PALCOLOR_HARKONNEN-2);
    uiGraphic[UI_Plus_Pressed][HOUSE_HARKONNEN] = LoadPNG_RW(pFileManager->openFile("Button_PlusPushed.png").get());
    uiGraphic[UI_MissionSelect][HOUSE_HARKONNEN] = LoadPNG_RW(pFileManager->openFile("Menu_MissionSelect.png").get());
    PicFactory->drawFrame(uiGraphic[UI_MissionSelect][HOUSE_HARKONNEN].get(),PictureFactory::SimpleFrame,nullptr);
    SDL_SetColorKey(uiGraphic[UI_MissionSelect][HOUSE_HARKONNEN].get(), SDL_TRUE, 0);
    uiGraphic[UI_OptionsMenu][HOUSE_HARKONNEN] = PicFactory->createOptionsMenu();
    uiGraphic[UI_LoadSaveWindow][HOUSE_HARKONNEN] = PicFactory->createMenu(280,228);
    uiGraphic[UI_NewMapWindow][HOUSE_HARKONNEN] = PicFactory->createMenu(600,440);
    uiGraphic[UI_DuneLegacy][HOUSE_HARKONNEN] = LoadPNG_RW(pFileManager->openFile("DuneLegacy.png").get());
    uiGraphic[UI_GameMenu][HOUSE_HARKONNEN] = PicFactory->createMenu(uiGraphic[UI_DuneLegacy][HOUSE_HARKONNEN].get(),158);
    PicFactory->drawFrame(uiGraphic[UI_DuneLegacy][HOUSE_HARKONNEN].get(),PictureFactory::SimpleFrame);

    uiGraphic[UI_PlanetBackground][HOUSE_HARKONNEN] = LoadCPS_RW(pFileManager->openFile("BIGPLAN.CPS").get());
    PicFactory->drawFrame(uiGraphic[UI_PlanetBackground][HOUSE_HARKONNEN].get(),PictureFactory::SimpleFrame);
    uiGraphic[UI_MenuButtonBorder][HOUSE_HARKONNEN] = PicFactory->createFrame(PictureFactory::DecorationFrame1,190,123,false);

    PicFactory->drawFrame(uiGraphic[UI_DuneLegacy][HOUSE_HARKONNEN].get(),PictureFactory::SimpleFrame);

    uiGraphic[UI_MentatBackground][HOUSE_HARKONNEN] = Scaler::defaultDoubleSurface(LoadCPS_RW(pFileManager->openFile("MENTATH.CPS").get()).get());
    uiGraphic[UI_MentatBackground][HOUSE_ATREIDES] = Scaler::defaultDoubleSurface(LoadCPS_RW(pFileManager->openFile("MENTATA.CPS").get()).get());
    uiGraphic[UI_MentatBackground][HOUSE_ORDOS] = Scaler::defaultDoubleSurface(LoadCPS_RW(pFileManager->openFile("MENTATO.CPS").get()).get());
    uiGraphic[UI_MentatBackground][HOUSE_FREMEN] = PictureFactory::mapMentatSurfaceToFremen(uiGraphic[UI_MentatBackground][HOUSE_ATREIDES].get());
    uiGraphic[UI_MentatBackground][HOUSE_SARDAUKAR] = PictureFactory::mapMentatSurfaceToSardaukar(uiGraphic[UI_MentatBackground][HOUSE_HARKONNEN].get());
    uiGraphic[UI_MentatBackground][HOUSE_MERCENARY] = PictureFactory::mapMentatSurfaceToMercenary(uiGraphic[UI_MentatBackground][HOUSE_ORDOS].get());

    uiGraphic[UI_MentatBackgroundBene][HOUSE_HARKONNEN] = Scaler::defaultDoubleSurface(LoadCPS_RW(pFileManager->openFile("MENTATM.CPS").get()).get());
    if(uiGraphic[UI_MentatBackgroundBene][HOUSE_HARKONNEN] != nullptr) {
        benePalette.applyToSurface(uiGraphic[UI_MentatBackgroundBene][HOUSE_HARKONNEN].get());
    }

    uiGraphic[UI_MentatHouseChoiceInfoQuestion][HOUSE_HARKONNEN] = PicFactory->createMentatHouseChoiceQuestion(HOUSE_HARKONNEN, benePalette);
    uiGraphic[UI_MentatHouseChoiceInfoQuestion][HOUSE_ATREIDES] = PicFactory->createMentatHouseChoiceQuestion(HOUSE_ATREIDES, benePalette);
    uiGraphic[UI_MentatHouseChoiceInfoQuestion][HOUSE_ORDOS] = PicFactory->createMentatHouseChoiceQuestion(HOUSE_ORDOS, benePalette);
    uiGraphic[UI_MentatHouseChoiceInfoQuestion][HOUSE_SARDAUKAR] = PicFactory->createMentatHouseChoiceQuestion(HOUSE_SARDAUKAR, benePalette);
    uiGraphic[UI_MentatHouseChoiceInfoQuestion][HOUSE_FREMEN] = PicFactory->createMentatHouseChoiceQuestion(HOUSE_FREMEN, benePalette);
    uiGraphic[UI_MentatHouseChoiceInfoQuestion][HOUSE_MERCENARY] = PicFactory->createMentatHouseChoiceQuestion(HOUSE_MERCENARY, benePalette);

    uiGraphic[UI_MentatYes][HOUSE_HARKONNEN] = Scaler::defaultDoubleSurface(mentat->getPicture(0).get());
    uiGraphic[UI_MentatYes_Pressed][HOUSE_HARKONNEN] = Scaler::defaultDoubleSurface(mentat->getPicture(1).get());
    uiGraphic[UI_MentatNo][HOUSE_HARKONNEN] = Scaler::defaultDoubleSurface(mentat->getPicture(2).get());
    uiGraphic[UI_MentatNo_Pressed][HOUSE_HARKONNEN] = Scaler::defaultDoubleSurface(mentat->getPicture(3).get());
    uiGraphic[UI_MentatExit][HOUSE_HARKONNEN] = Scaler::defaultDoubleSurface(mentat->getPicture(4).get());
    uiGraphic[UI_MentatExit_Pressed][HOUSE_HARKONNEN] = Scaler::defaultDoubleSurface(mentat->getPicture(5).get());
    uiGraphic[UI_MentatProcced][HOUSE_HARKONNEN] = Scaler::defaultDoubleSurface(mentat->getPicture(6).get());
    uiGraphic[UI_MentatProcced_Pressed][HOUSE_HARKONNEN] = Scaler::defaultDoubleSurface(mentat->getPicture(7).get());
    uiGraphic[UI_MentatRepeat][HOUSE_HARKONNEN] = Scaler::defaultDoubleSurface(mentat->getPicture(8).get());
    uiGraphic[UI_MentatRepeat_Pressed][HOUSE_HARKONNEN] = Scaler::defaultDoubleSurface(mentat->getPicture(9).get());

    { // Scope
        sdl2::surface_ptr pHouseChoiceBackground;
        if (pFileManager->exists("HERALD." + _("LanguageFileExtension"))) {
            pHouseChoiceBackground = LoadCPS_RW(pFileManager->openFile("HERALD." + _("LanguageFileExtension")).get());
        }
        else {
            pHouseChoiceBackground = LoadCPS_RW(pFileManager->openFile("HERALD.CPS").get());
        }

        uiGraphic[UI_HouseSelect][HOUSE_HARKONNEN] = PicFactory->createHouseSelect(pHouseChoiceBackground.get());
        uiGraphic[UI_SelectYourHouseLarge][HOUSE_HARKONNEN] = Scaler::defaultDoubleSurface(getSubPicture(pHouseChoiceBackground.get(), 0, 0, 320, 50).get());
        uiGraphic[UI_Herald_Colored][HOUSE_ATREIDES] = getSubPicture(pHouseChoiceBackground.get(), 20, 54, 83, 91);
        uiGraphic[UI_Herald_ColoredLarge][HOUSE_ATREIDES] = Scaler::defaultDoubleSurface(uiGraphic[UI_Herald_Colored][HOUSE_ATREIDES].get());
        uiGraphic[UI_Herald_Colored][HOUSE_ORDOS] = getSubPicture(pHouseChoiceBackground.get(), 117, 54, 83, 91);
        uiGraphic[UI_Herald_ColoredLarge][HOUSE_ORDOS] = Scaler::defaultDoubleSurface(uiGraphic[UI_Herald_Colored][HOUSE_ORDOS].get());
        uiGraphic[UI_Herald_Colored][HOUSE_HARKONNEN] = getSubPicture(pHouseChoiceBackground.get(), 215, 54, 83, 91);
        uiGraphic[UI_Herald_ColoredLarge][HOUSE_HARKONNEN] = Scaler::defaultDoubleSurface(uiGraphic[UI_Herald_Colored][HOUSE_HARKONNEN].get());
        uiGraphic[UI_Herald_Colored][HOUSE_FREMEN] = PicFactory->createHeraldFre(uiGraphic[UI_Herald_Colored][HOUSE_HARKONNEN].get());
        uiGraphic[UI_Herald_ColoredLarge][HOUSE_FREMEN] = Scaler::defaultDoubleSurface(uiGraphic[UI_Herald_Colored][HOUSE_FREMEN].get());
        uiGraphic[UI_Herald_Colored][HOUSE_SARDAUKAR] = PicFactory->createHeraldSard(uiGraphic[UI_Herald_Colored][HOUSE_ORDOS].get(), uiGraphic[UI_Herald_Colored][HOUSE_ATREIDES].get());
        uiGraphic[UI_Herald_ColoredLarge][HOUSE_SARDAUKAR] = Scaler::defaultDoubleSurface(uiGraphic[UI_Herald_Colored][HOUSE_SARDAUKAR].get());
        uiGraphic[UI_Herald_Colored][HOUSE_MERCENARY] = PicFactory->createHeraldMerc(uiGraphic[UI_Herald_Colored][HOUSE_ATREIDES].get(), uiGraphic[UI_Herald_Colored][HOUSE_ORDOS].get());
        uiGraphic[UI_Herald_ColoredLarge][HOUSE_MERCENARY] = Scaler::defaultDoubleSurface(uiGraphic[UI_Herald_Colored][HOUSE_MERCENARY].get());
    }

    uiGraphic[UI_Herald_Grey][HOUSE_HARKONNEN] = PicFactory->createGreyHouseChoice(uiGraphic[UI_Herald_Colored][HOUSE_HARKONNEN].get());
    uiGraphic[UI_Herald_Grey][HOUSE_ATREIDES] = PicFactory->createGreyHouseChoice(uiGraphic[UI_Herald_Colored][HOUSE_ATREIDES].get());
    uiGraphic[UI_Herald_Grey][HOUSE_ORDOS] = PicFactory->createGreyHouseChoice(uiGraphic[UI_Herald_Colored][HOUSE_ORDOS].get());
    uiGraphic[UI_Herald_Grey][HOUSE_FREMEN] = PicFactory->createGreyHouseChoice(uiGraphic[UI_Herald_Colored][HOUSE_FREMEN].get());
    uiGraphic[UI_Herald_Grey][HOUSE_SARDAUKAR] = PicFactory->createGreyHouseChoice(uiGraphic[UI_Herald_Colored][HOUSE_SARDAUKAR].get());
    uiGraphic[UI_Herald_Grey][HOUSE_MERCENARY] = PicFactory->createGreyHouseChoice(uiGraphic[UI_Herald_Colored][HOUSE_MERCENARY].get());

    uiGraphic[UI_Herald_ArrowLeft][HOUSE_HARKONNEN] = LoadPNG_RW(pFileManager->openFile("ArrowLeft.png").get());
    uiGraphic[UI_Herald_ArrowLeftLarge][HOUSE_HARKONNEN] = Scaler::defaultDoubleSurface(uiGraphic[UI_Herald_ArrowLeft][HOUSE_HARKONNEN].get());
    uiGraphic[UI_Herald_ArrowLeftHighlight][HOUSE_HARKONNEN] = LoadPNG_RW(pFileManager->openFile("ArrowLeftHighlight.png").get());
    uiGraphic[UI_Herald_ArrowLeftHighlightLarge][HOUSE_HARKONNEN] = Scaler::defaultDoubleSurface(uiGraphic[UI_Herald_ArrowLeftHighlight][HOUSE_HARKONNEN].get());
    uiGraphic[UI_Herald_ArrowRight][HOUSE_HARKONNEN] = LoadPNG_RW(pFileManager->openFile("ArrowRight.png").get());
    uiGraphic[UI_Herald_ArrowRightLarge][HOUSE_HARKONNEN] = Scaler::defaultDoubleSurface(uiGraphic[UI_Herald_ArrowRight][HOUSE_HARKONNEN].get());
    uiGraphic[UI_Herald_ArrowRightHighlight][HOUSE_HARKONNEN] = LoadPNG_RW(pFileManager->openFile("ArrowRightHighlight.png").get());
    uiGraphic[UI_Herald_ArrowRightHighlightLarge][HOUSE_HARKONNEN] = Scaler::defaultDoubleSurface(uiGraphic[UI_Herald_ArrowRightHighlight][HOUSE_HARKONNEN].get());

    uiGraphic[UI_MapChoiceScreen][HOUSE_HARKONNEN] = PicFactory->createMapChoiceScreen(HOUSE_HARKONNEN);
    uiGraphic[UI_MapChoiceScreen][HOUSE_ATREIDES] = PicFactory->createMapChoiceScreen(HOUSE_ATREIDES);
    uiGraphic[UI_MapChoiceScreen][HOUSE_ORDOS] = PicFactory->createMapChoiceScreen(HOUSE_ORDOS);
    uiGraphic[UI_MapChoiceScreen][HOUSE_FREMEN] = PicFactory->createMapChoiceScreen(HOUSE_FREMEN);
    uiGraphic[UI_MapChoiceScreen][HOUSE_SARDAUKAR] = PicFactory->createMapChoiceScreen(HOUSE_SARDAUKAR);
    uiGraphic[UI_MapChoiceScreen][HOUSE_MERCENARY] = PicFactory->createMapChoiceScreen(HOUSE_MERCENARY);
    uiGraphic[UI_MapChoicePlanet][HOUSE_HARKONNEN] = Scaler::doubleSurfaceNN(LoadCPS_RW(pFileManager->openFile("PLANET.CPS").get()).get());
    SDL_SetColorKey(uiGraphic[UI_MapChoicePlanet][HOUSE_HARKONNEN].get(), SDL_TRUE, 0);
    uiGraphic[UI_MapChoiceMapOnly][HOUSE_HARKONNEN] = Scaler::doubleSurfaceNN(LoadCPS_RW(pFileManager->openFile("DUNEMAP.CPS").get()).get());
    SDL_SetColorKey(uiGraphic[UI_MapChoiceMapOnly][HOUSE_HARKONNEN].get(), SDL_TRUE, 0);
    uiGraphic[UI_MapChoiceMap][HOUSE_HARKONNEN] = Scaler::doubleSurfaceNN(LoadCPS_RW(pFileManager->openFile("DUNERGN.CPS").get()).get());
    SDL_SetColorKey(uiGraphic[UI_MapChoiceMap][HOUSE_HARKONNEN].get(), SDL_TRUE, 0);

    // make black lines inside the map non-transparent
    {
        const auto surface = uiGraphic[UI_MapChoiceMap][HOUSE_HARKONNEN].get();

        sdl2::surface_lock lock{ surface };

        for(auto y = 48; y < 48+240; y++) {
            for(auto x = 16; x < 16 + 608; x++) {
                if(getPixel(surface, x, y) == 0) {
                    putPixel(surface, x, y, PALCOLOR_BLACK);
                }
            }
        }
    }

    uiGraphic[UI_MapChoiceClickMap][HOUSE_HARKONNEN] = Scaler::doubleSurfaceNN(LoadCPS_RW(pFileManager->openFile("RGNCLK.CPS").get()).get());
    uiGraphic[UI_MapChoiceArrow_None][HOUSE_HARKONNEN] = Scaler::defaultDoubleSurface(arrows->getPicture(0).get());
    SDL_SetColorKey(uiGraphic[UI_MapChoiceArrow_None][HOUSE_HARKONNEN].get(), SDL_TRUE, 0);
    uiGraphic[UI_MapChoiceArrow_LeftUp][HOUSE_HARKONNEN] = Scaler::defaultDoubleSurface(arrows->getPicture(1).get());
    SDL_SetColorKey(uiGraphic[UI_MapChoiceArrow_LeftUp][HOUSE_HARKONNEN].get(), SDL_TRUE, 0);
    uiGraphic[UI_MapChoiceArrow_Up][HOUSE_HARKONNEN] = Scaler::defaultDoubleSurface(arrows->getPicture(2).get());
    SDL_SetColorKey(uiGraphic[UI_MapChoiceArrow_Up][HOUSE_HARKONNEN].get(), SDL_TRUE, 0);
    uiGraphic[UI_MapChoiceArrow_RightUp][HOUSE_HARKONNEN] = Scaler::defaultDoubleSurface(arrows->getPicture(3).get());
    SDL_SetColorKey(uiGraphic[UI_MapChoiceArrow_RightUp][HOUSE_HARKONNEN].get(), SDL_TRUE, 0);
    uiGraphic[UI_MapChoiceArrow_Right][HOUSE_HARKONNEN] = Scaler::defaultDoubleSurface(arrows->getPicture(4).get());
    SDL_SetColorKey(uiGraphic[UI_MapChoiceArrow_Right][HOUSE_HARKONNEN].get(), SDL_TRUE, 0);
    uiGraphic[UI_MapChoiceArrow_RightDown][HOUSE_HARKONNEN] = Scaler::defaultDoubleSurface(arrows->getPicture(5).get());
    SDL_SetColorKey(uiGraphic[UI_MapChoiceArrow_RightDown][HOUSE_HARKONNEN].get(), SDL_TRUE, 0);
    uiGraphic[UI_MapChoiceArrow_Down][HOUSE_HARKONNEN] = Scaler::defaultDoubleSurface(arrows->getPicture(6).get());
    SDL_SetColorKey(uiGraphic[UI_MapChoiceArrow_Down][HOUSE_HARKONNEN].get(), SDL_TRUE, 0);
    uiGraphic[UI_MapChoiceArrow_LeftDown][HOUSE_HARKONNEN] = Scaler::defaultDoubleSurface(arrows->getPicture(7).get());
    SDL_SetColorKey(uiGraphic[UI_MapChoiceArrow_LeftDown][HOUSE_HARKONNEN].get(), SDL_TRUE, 0);
    uiGraphic[UI_MapChoiceArrow_Left][HOUSE_HARKONNEN] = Scaler::defaultDoubleSurface(arrows->getPicture(8).get());
    SDL_SetColorKey(uiGraphic[UI_MapChoiceArrow_Left][HOUSE_HARKONNEN].get(), SDL_TRUE, 0);

    uiGraphic[UI_StructureSizeLattice][HOUSE_HARKONNEN] = LoadPNG_RW(pFileManager->openFile("StructureSizeLattice.png").get());
    SDL_SetColorKey(uiGraphic[UI_StructureSizeLattice][HOUSE_HARKONNEN].get(), SDL_TRUE, 0);
    uiGraphic[UI_StructureSizeConcrete][HOUSE_HARKONNEN] = LoadPNG_RW(pFileManager->openFile("StructureSizeConcrete.png").get());
    SDL_SetColorKey(uiGraphic[UI_StructureSizeConcrete][HOUSE_HARKONNEN].get(), SDL_TRUE, 0);

    uiGraphic[UI_MapEditor_SideBar][HOUSE_HARKONNEN] = PicFactory->createSideBar(true);
    uiGraphic[UI_MapEditor_BottomBar][HOUSE_HARKONNEN] = PicFactory->createBottomBar();

    uiGraphic[UI_MapEditor_ExitIcon][HOUSE_HARKONNEN] = LoadPNG_RW(pFileManager->openFile("MapEditorExitIcon.png").get());
    SDL_SetColorKey(uiGraphic[UI_MapEditor_ExitIcon][HOUSE_HARKONNEN].get(), SDL_TRUE, 0);
    uiGraphic[UI_MapEditor_NewIcon][HOUSE_HARKONNEN] = LoadPNG_RW(pFileManager->openFile("MapEditorNewIcon.png").get());
    SDL_SetColorKey(uiGraphic[UI_MapEditor_NewIcon][HOUSE_HARKONNEN].get(), SDL_TRUE, 0);
    uiGraphic[UI_MapEditor_LoadIcon][HOUSE_HARKONNEN] = LoadPNG_RW(pFileManager->openFile("MapEditorLoadIcon.png").get());
    SDL_SetColorKey(uiGraphic[UI_MapEditor_LoadIcon][HOUSE_HARKONNEN].get(), SDL_TRUE, 0);
    uiGraphic[UI_MapEditor_SaveIcon][HOUSE_HARKONNEN] = LoadPNG_RW(pFileManager->openFile("MapEditorSaveIcon.png").get());
    SDL_SetColorKey(uiGraphic[UI_MapEditor_SaveIcon][HOUSE_HARKONNEN].get(), SDL_TRUE, 0);
    uiGraphic[UI_MapEditor_UndoIcon][HOUSE_HARKONNEN] = LoadPNG_RW(pFileManager->openFile("MapEditorUndoIcon.png").get());
    SDL_SetColorKey(uiGraphic[UI_MapEditor_UndoIcon][HOUSE_HARKONNEN].get(), SDL_TRUE, 0);
    uiGraphic[UI_MapEditor_RedoIcon][HOUSE_HARKONNEN] = LoadPNG_RW(pFileManager->openFile("MapEditorRedoIcon.png").get());
    SDL_SetColorKey(uiGraphic[UI_MapEditor_RedoIcon][HOUSE_HARKONNEN].get(), SDL_TRUE, 0);
    uiGraphic[UI_MapEditor_PlayerIcon][HOUSE_HARKONNEN] = LoadPNG_RW(pFileManager->openFile("MapEditorPlayerIcon.png").get());
    SDL_SetColorKey(uiGraphic[UI_MapEditor_PlayerIcon][HOUSE_HARKONNEN].get(), SDL_TRUE, 0);
    uiGraphic[UI_MapEditor_MapSettingsIcon][HOUSE_HARKONNEN] = LoadPNG_RW(pFileManager->openFile("MapEditorMapSettingsIcon.png").get());
    SDL_SetColorKey(uiGraphic[UI_MapEditor_MapSettingsIcon][HOUSE_HARKONNEN].get(), SDL_TRUE, 0);
    uiGraphic[UI_MapEditor_ChoamIcon][HOUSE_HARKONNEN] = scaleSurface(getSubFrame(objPic[ObjPic_Frigate][HOUSE_HARKONNEN][0].get(),1,0,8,1).get(), 0.5);
    SDL_SetColorKey(uiGraphic[UI_MapEditor_ChoamIcon][HOUSE_HARKONNEN].get(), SDL_TRUE, 0);
    uiGraphic[UI_MapEditor_ReinforcementsIcon][HOUSE_HARKONNEN] = scaleSurface(getSubFrame(objPic[ObjPic_Carryall][HOUSE_HARKONNEN][0].get(),1,0,8,2).get(), 0.66667);
    SDL_SetColorKey(uiGraphic[UI_MapEditor_ReinforcementsIcon][HOUSE_HARKONNEN].get(), SDL_TRUE, 0);
    uiGraphic[UI_MapEditor_TeamsIcon][HOUSE_HARKONNEN] = getSubFrame(objPic[ObjPic_Troopers][HOUSE_HARKONNEN][0].get(),0,0,4,4);
    SDL_SetColorKey(uiGraphic[UI_MapEditor_TeamsIcon][HOUSE_HARKONNEN].get(), SDL_TRUE, 0);
    uiGraphic[UI_MapEditor_MirrorNoneIcon][HOUSE_HARKONNEN] = LoadPNG_RW(pFileManager->openFile("MapEditorMirrorNone.png").get());
    SDL_SetColorKey(uiGraphic[UI_MapEditor_MirrorNoneIcon][HOUSE_HARKONNEN].get(), SDL_TRUE, 0);
    uiGraphic[UI_MapEditor_MirrorHorizontalIcon][HOUSE_HARKONNEN] = LoadPNG_RW(pFileManager->openFile("MapEditorMirrorHorizontal.png").get());
    SDL_SetColorKey(uiGraphic[UI_MapEditor_MirrorHorizontalIcon][HOUSE_HARKONNEN].get(), SDL_TRUE, 0);
    uiGraphic[UI_MapEditor_MirrorVerticalIcon][HOUSE_HARKONNEN] = LoadPNG_RW(pFileManager->openFile("MapEditorMirrorVertical.png").get());
    SDL_SetColorKey(uiGraphic[UI_MapEditor_MirrorVerticalIcon][HOUSE_HARKONNEN].get(), SDL_TRUE, 0);
    uiGraphic[UI_MapEditor_MirrorBothIcon][HOUSE_HARKONNEN] = LoadPNG_RW(pFileManager->openFile("MapEditorMirrorBoth.png").get());
    SDL_SetColorKey(uiGraphic[UI_MapEditor_MirrorBothIcon][HOUSE_HARKONNEN].get(), SDL_TRUE, 0);
    uiGraphic[UI_MapEditor_MirrorPointIcon][HOUSE_HARKONNEN] = LoadPNG_RW(pFileManager->openFile("MapEditorMirrorPoint.png").get());
    SDL_SetColorKey(uiGraphic[UI_MapEditor_MirrorPointIcon][HOUSE_HARKONNEN].get(), SDL_TRUE, 0);
    uiGraphic[UI_MapEditor_ArrowUp][HOUSE_HARKONNEN] = LoadPNG_RW(pFileManager->openFile("MapEditorArrowUp.png").get());
    SDL_SetColorKey(uiGraphic[UI_MapEditor_ArrowUp][HOUSE_HARKONNEN].get(), SDL_TRUE, 0);
    uiGraphic[UI_MapEditor_ArrowUp_Active][HOUSE_HARKONNEN] = mapSurfaceColorRange(uiGraphic[UI_MapEditor_ArrowUp][HOUSE_HARKONNEN].get(), PALCOLOR_HARKONNEN, PALCOLOR_HARKONNEN-3);
    uiGraphic[UI_MapEditor_ArrowDown][HOUSE_HARKONNEN] = LoadPNG_RW(pFileManager->openFile("MapEditorArrowDown.png").get());
    SDL_SetColorKey(uiGraphic[UI_MapEditor_ArrowDown][HOUSE_HARKONNEN].get(), SDL_TRUE, 0);
    uiGraphic[UI_MapEditor_ArrowDown_Active][HOUSE_HARKONNEN] = mapSurfaceColorRange(uiGraphic[UI_MapEditor_ArrowDown][HOUSE_HARKONNEN].get(), PALCOLOR_HARKONNEN, PALCOLOR_HARKONNEN-3);
    uiGraphic[UI_MapEditor_Plus][HOUSE_HARKONNEN] = LoadPNG_RW(pFileManager->openFile("MapEditorPlus.png").get());
    SDL_SetColorKey(uiGraphic[UI_MapEditor_Plus][HOUSE_HARKONNEN].get(), SDL_TRUE, 0);
    uiGraphic[UI_MapEditor_Plus_Active][HOUSE_HARKONNEN] = mapSurfaceColorRange(uiGraphic[UI_MapEditor_Plus][HOUSE_HARKONNEN].get(), PALCOLOR_HARKONNEN, PALCOLOR_HARKONNEN-3);
    uiGraphic[UI_MapEditor_Minus][HOUSE_HARKONNEN] = LoadPNG_RW(pFileManager->openFile("MapEditorMinus.png").get());
    SDL_SetColorKey(uiGraphic[UI_MapEditor_Minus][HOUSE_HARKONNEN].get(), SDL_TRUE, 0);
    uiGraphic[UI_MapEditor_Minus_Active][HOUSE_HARKONNEN] = mapSurfaceColorRange(uiGraphic[UI_MapEditor_Minus][HOUSE_HARKONNEN].get(), PALCOLOR_HARKONNEN, PALCOLOR_HARKONNEN-3);
    uiGraphic[UI_MapEditor_RotateLeftIcon][HOUSE_HARKONNEN] = LoadPNG_RW(pFileManager->openFile("MapEditorRotateLeft.png").get());
    SDL_SetColorKey(uiGraphic[UI_MapEditor_RotateLeftIcon][HOUSE_HARKONNEN].get(), SDL_TRUE, 0);
    uiGraphic[UI_MapEditor_RotateLeftHighlightIcon][HOUSE_HARKONNEN] = mapSurfaceColorRange(uiGraphic[UI_MapEditor_RotateLeftIcon][HOUSE_HARKONNEN].get(), PALCOLOR_HARKONNEN, PALCOLOR_HARKONNEN-3);
    SDL_SetColorKey(uiGraphic[UI_MapEditor_RotateLeftHighlightIcon][HOUSE_HARKONNEN].get(), SDL_TRUE, 0);
    uiGraphic[UI_MapEditor_RotateRightIcon][HOUSE_HARKONNEN] = LoadPNG_RW(pFileManager->openFile("MapEditorRotateRight.png").get());
    SDL_SetColorKey(uiGraphic[UI_MapEditor_RotateRightIcon][HOUSE_HARKONNEN].get(), SDL_TRUE, 0);
    uiGraphic[UI_MapEditor_RotateRightHighlightIcon][HOUSE_HARKONNEN] = mapSurfaceColorRange(uiGraphic[UI_MapEditor_RotateRightIcon][HOUSE_HARKONNEN].get(), PALCOLOR_HARKONNEN, PALCOLOR_HARKONNEN-3);
    SDL_SetColorKey(uiGraphic[UI_MapEditor_RotateRightHighlightIcon][HOUSE_HARKONNEN].get(), SDL_TRUE, 0);

    uiGraphic[UI_MapEditor_Sand][HOUSE_HARKONNEN] = Scaler::defaultDoubleSurface(icon->getPicture(127).get());
    uiGraphic[UI_MapEditor_Dunes][HOUSE_HARKONNEN] = Scaler::defaultDoubleSurface(icon->getPicture(159).get());
    uiGraphic[UI_MapEditor_SpecialBloom][HOUSE_HARKONNEN] = Scaler::defaultDoubleSurface(icon->getPicture(209).get());
    uiGraphic[UI_MapEditor_Spice][HOUSE_HARKONNEN] = Scaler::defaultDoubleSurface(icon->getPicture(191).get());
    uiGraphic[UI_MapEditor_ThickSpice][HOUSE_HARKONNEN] = Scaler::defaultDoubleSurface(icon->getPicture(207).get());
    uiGraphic[UI_MapEditor_SpiceBloom][HOUSE_HARKONNEN] = Scaler::defaultDoubleSurface(icon->getPicture(208).get());
    uiGraphic[UI_MapEditor_Slab][HOUSE_HARKONNEN] = Scaler::defaultDoubleSurface(icon->getPicture(126).get());
    uiGraphic[UI_MapEditor_Rock][HOUSE_HARKONNEN] = Scaler::defaultDoubleSurface(icon->getPicture(143).get());
    uiGraphic[UI_MapEditor_Mountain][HOUSE_HARKONNEN] = Scaler::defaultDoubleSurface(icon->getPicture(175).get());

    uiGraphic[UI_MapEditor_Slab1][HOUSE_HARKONNEN] = icon->getPicture(126);
    uiGraphic[UI_MapEditor_Wall][HOUSE_HARKONNEN] = getSubPicture(objPic[ObjPic_Wall][HOUSE_HARKONNEN][0].get(),2*D2_TILESIZE,0,D2_TILESIZE,D2_TILESIZE);
    uiGraphic[UI_MapEditor_GunTurret][HOUSE_HARKONNEN] = getSubPicture(objPic[ObjPic_GunTurret][HOUSE_HARKONNEN][0].get(),2*D2_TILESIZE,0,D2_TILESIZE,D2_TILESIZE);
    uiGraphic[UI_MapEditor_RocketTurret][HOUSE_HARKONNEN] = getSubPicture(objPic[ObjPic_RocketTurret][HOUSE_HARKONNEN][0].get(),2*D2_TILESIZE,0,D2_TILESIZE,D2_TILESIZE);
    uiGraphic[UI_MapEditor_ConstructionYard][HOUSE_HARKONNEN] = getSubPicture(objPic[ObjPic_ConstructionYard][HOUSE_HARKONNEN][0].get(),2*2*D2_TILESIZE,0,2*D2_TILESIZE,2*D2_TILESIZE);
    uiGraphic[UI_MapEditor_Windtrap][HOUSE_HARKONNEN] = getSubPicture(objPic[ObjPic_Windtrap][HOUSE_HARKONNEN][0].get(),2*2*D2_TILESIZE,0,2*D2_TILESIZE,2*D2_TILESIZE);
    SDL_Color windtrapColor = { 70, 70, 70, 255};
    SDL_SetPaletteColors(uiGraphic[UI_MapEditor_Windtrap][HOUSE_HARKONNEN]->format->palette, &windtrapColor, PALCOLOR_WINDTRAP_COLORCYCLE, 1);
    uiGraphic[UI_MapEditor_Radar][HOUSE_HARKONNEN] = getSubPicture(objPic[ObjPic_Radar][HOUSE_HARKONNEN][0].get(),2*2*D2_TILESIZE,0,2*D2_TILESIZE,2*D2_TILESIZE);
    uiGraphic[UI_MapEditor_Silo][HOUSE_HARKONNEN] = getSubPicture(objPic[ObjPic_Silo][HOUSE_HARKONNEN][0].get(),2*2*D2_TILESIZE,0,2*D2_TILESIZE,2*D2_TILESIZE);
    uiGraphic[UI_MapEditor_IX][HOUSE_HARKONNEN] = getSubPicture(objPic[ObjPic_IX][HOUSE_HARKONNEN][0].get(),2*2*D2_TILESIZE,0,2*D2_TILESIZE,2*D2_TILESIZE);
    uiGraphic[UI_MapEditor_Barracks][HOUSE_HARKONNEN] = getSubPicture(objPic[ObjPic_Barracks][HOUSE_HARKONNEN][0].get(),2*2*D2_TILESIZE,0,2*D2_TILESIZE,2*D2_TILESIZE);
    uiGraphic[UI_MapEditor_WOR][HOUSE_HARKONNEN] = getSubPicture(objPic[ObjPic_WOR][HOUSE_HARKONNEN][0].get(),2*2*D2_TILESIZE,0,2*D2_TILESIZE,2*D2_TILESIZE);
    uiGraphic[UI_MapEditor_LightFactory][HOUSE_HARKONNEN] = getSubPicture(objPic[ObjPic_LightFactory][HOUSE_HARKONNEN][0].get(),2*2*D2_TILESIZE,0,2*D2_TILESIZE,2*D2_TILESIZE);
    uiGraphic[UI_MapEditor_Refinery][HOUSE_HARKONNEN] = getSubPicture(objPic[ObjPic_Refinery][HOUSE_HARKONNEN][0].get(),2*3*D2_TILESIZE,0,3*D2_TILESIZE,2*D2_TILESIZE);
    uiGraphic[UI_MapEditor_HighTechFactory][HOUSE_HARKONNEN] = getSubPicture(objPic[ObjPic_HighTechFactory][HOUSE_HARKONNEN][0].get(),2*3*D2_TILESIZE,0,3*D2_TILESIZE,2*D2_TILESIZE);
    uiGraphic[UI_MapEditor_HeavyFactory][HOUSE_HARKONNEN] = getSubPicture(objPic[ObjPic_HeavyFactory][HOUSE_HARKONNEN][0].get(),2*3*D2_TILESIZE,0,3*D2_TILESIZE,2*D2_TILESIZE);
    uiGraphic[UI_MapEditor_RepairYard][HOUSE_HARKONNEN] = getSubPicture(objPic[ObjPic_RepairYard][HOUSE_HARKONNEN][0].get(),2*3*D2_TILESIZE,0,3*D2_TILESIZE,2*D2_TILESIZE);
    uiGraphic[UI_MapEditor_Starport][HOUSE_HARKONNEN] = getSubPicture(objPic[ObjPic_Starport][HOUSE_HARKONNEN][0].get(),2*3*D2_TILESIZE,0,3*D2_TILESIZE,3*D2_TILESIZE);
    uiGraphic[UI_MapEditor_Palace][HOUSE_HARKONNEN] = getSubPicture(objPic[ObjPic_Palace][HOUSE_HARKONNEN][0].get(),2*3*D2_TILESIZE,0,3*D2_TILESIZE,3*D2_TILESIZE);

    uiGraphic[UI_MapEditor_Soldier][HOUSE_HARKONNEN] = getSubFrame(objPic[ObjPic_Soldier][HOUSE_HARKONNEN][0].get(),0,0,4,3);
    uiGraphic[UI_MapEditor_Trooper][HOUSE_HARKONNEN] = getSubFrame(objPic[ObjPic_Trooper][HOUSE_HARKONNEN][0].get(),0,0,4,3);
    uiGraphic[UI_MapEditor_Harvester][HOUSE_HARKONNEN] = getSubFrame(objPic[ObjPic_Harvester][HOUSE_HARKONNEN][0].get(),0,0,8,1);
    uiGraphic[UI_MapEditor_Infantry][HOUSE_HARKONNEN] = getSubFrame(objPic[ObjPic_Infantry][HOUSE_HARKONNEN][0].get(),0,0,4,4);
    uiGraphic[UI_MapEditor_Troopers][HOUSE_HARKONNEN] = getSubFrame(objPic[ObjPic_Troopers][HOUSE_HARKONNEN][0].get(),0,0,4,4);
    uiGraphic[UI_MapEditor_MCV][HOUSE_HARKONNEN] = getSubFrame(objPic[ObjPic_MCV][HOUSE_HARKONNEN][0].get(),0,0,8,1);
    uiGraphic[UI_MapEditor_Trike][HOUSE_HARKONNEN] = getSubFrame(objPic[ObjPic_Trike][HOUSE_HARKONNEN][0].get(),0,0,8,1);
    uiGraphic[UI_MapEditor_Raider][HOUSE_HARKONNEN] = getSubFrame(objPic[ObjPic_Trike][HOUSE_HARKONNEN][0].get(),0,0,8,1);
    uiGraphic[UI_MapEditor_Raider][HOUSE_HARKONNEN] = combinePictures(uiGraphic[UI_MapEditor_Raider][HOUSE_HARKONNEN].get(), objPic[ObjPic_Star][HOUSE_HARKONNEN][1].get(),
                                                                      uiGraphic[UI_MapEditor_Raider][HOUSE_HARKONNEN]->w - objPic[ObjPic_Star][HOUSE_HARKONNEN][1]->w,
                                                                      uiGraphic[UI_MapEditor_Raider][HOUSE_HARKONNEN]->h - objPic[ObjPic_Star][HOUSE_HARKONNEN][1]->h);
    uiGraphic[UI_MapEditor_Quad][HOUSE_HARKONNEN] = getSubFrame(objPic[ObjPic_Quad][HOUSE_HARKONNEN][0].get(),0,0,8,1);
    uiGraphic[UI_MapEditor_Tank][HOUSE_HARKONNEN] = combinePictures(getSubFrame(objPic[ObjPic_Tank_Base][HOUSE_HARKONNEN][0].get(),0,0,8,1).get(), getSubFrame(objPic[ObjPic_Tank_Gun][HOUSE_HARKONNEN][0].get(),0,0,8,1).get(), 0, 0);
    uiGraphic[UI_MapEditor_SiegeTank][HOUSE_HARKONNEN] = combinePictures(getSubFrame(objPic[ObjPic_Siegetank_Base][HOUSE_HARKONNEN][0].get(),0,0,8,1).get(), getSubFrame(objPic[ObjPic_Siegetank_Gun][HOUSE_HARKONNEN][0].get(),0,0,8,1).get(), 2, -4);
    uiGraphic[UI_MapEditor_Launcher][HOUSE_HARKONNEN] = combinePictures(getSubFrame(objPic[ObjPic_Tank_Base][HOUSE_HARKONNEN][0].get(),0,0,8,1).get(), getSubFrame(objPic[ObjPic_Launcher_Gun][HOUSE_HARKONNEN][0].get(),0,0,8,1).get(), 3, 0);
    uiGraphic[UI_MapEditor_Devastator][HOUSE_HARKONNEN] = combinePictures(getSubFrame(objPic[ObjPic_Devastator_Base][HOUSE_HARKONNEN][0].get(),0,0,8,1).get(), getSubFrame(objPic[ObjPic_Devastator_Gun][HOUSE_HARKONNEN][0].get(),0,0,8,1).get(), 2, -4);
    uiGraphic[UI_MapEditor_SonicTank][HOUSE_HARKONNEN] = combinePictures(getSubFrame(objPic[ObjPic_Tank_Base][HOUSE_HARKONNEN][0].get(),0,0,8,1).get(), getSubFrame(objPic[ObjPic_Sonictank_Gun][HOUSE_HARKONNEN][0].get(),0,0,8,1).get(), 3, 1);
    uiGraphic[UI_MapEditor_Deviator][HOUSE_HARKONNEN] = combinePictures(getSubFrame(objPic[ObjPic_Tank_Base][HOUSE_HARKONNEN][0].get(),0,0,8,1).get(), getSubFrame(objPic[ObjPic_Launcher_Gun][HOUSE_HARKONNEN][0].get(),0,0,8,1).get(), 3, 0);
    uiGraphic[UI_MapEditor_Deviator][HOUSE_HARKONNEN] = combinePictures(uiGraphic[UI_MapEditor_Deviator][HOUSE_HARKONNEN].get(), objPic[ObjPic_Star][HOUSE_HARKONNEN][1].get(),
                                                                  uiGraphic[UI_MapEditor_Deviator][HOUSE_HARKONNEN]->w - objPic[ObjPic_Star][HOUSE_HARKONNEN][1]->w,
                                                                  uiGraphic[UI_MapEditor_Deviator][HOUSE_HARKONNEN]->h - objPic[ObjPic_Star][HOUSE_HARKONNEN][1]->h);
    uiGraphic[UI_MapEditor_Saboteur][HOUSE_HARKONNEN] = getSubFrame(objPic[ObjPic_Saboteur][HOUSE_HARKONNEN][0].get(),0,0,4,3);
    uiGraphic[UI_MapEditor_Sandworm][HOUSE_HARKONNEN] = getSubFrame(objPic[ObjPic_Sandworm][HOUSE_HARKONNEN][0].get(),0,5,1,9);
    uiGraphic[UI_MapEditor_SpecialUnit][HOUSE_HARKONNEN] = combinePictures(getSubFrame(objPic[ObjPic_Devastator_Base][HOUSE_HARKONNEN][0].get(),0,0,8,1).get(), getSubFrame(objPic[ObjPic_Devastator_Gun][HOUSE_HARKONNEN][0].get(),0,0,8,1).get(), 2, -4);
    uiGraphic[UI_MapEditor_SpecialUnit][HOUSE_HARKONNEN] = combinePictures(uiGraphic[UI_MapEditor_SpecialUnit][HOUSE_HARKONNEN].get(), objPic[ObjPic_Star][HOUSE_HARKONNEN][1].get(),
                                                                  uiGraphic[UI_MapEditor_SpecialUnit][HOUSE_HARKONNEN]->w - objPic[ObjPic_Star][HOUSE_HARKONNEN][1]->w,
                                                                  uiGraphic[UI_MapEditor_SpecialUnit][HOUSE_HARKONNEN]->h - objPic[ObjPic_Star][HOUSE_HARKONNEN][1]->h);
    uiGraphic[UI_MapEditor_Carryall][HOUSE_HARKONNEN] = getSubFrame(objPic[ObjPic_Carryall][HOUSE_HARKONNEN][0].get(),0,0,8,2);
    uiGraphic[UI_MapEditor_Ornithopter][HOUSE_HARKONNEN] = getSubFrame(objPic[ObjPic_Ornithopter][HOUSE_HARKONNEN][0].get(),0,0,8,3);

    uiGraphic[UI_MapEditor_Pen1x1][HOUSE_HARKONNEN] = LoadPNG_RW(pFileManager->openFile("MapEditorPen1x1.png").get());
    SDL_SetColorKey(uiGraphic[UI_MapEditor_Pen1x1][HOUSE_HARKONNEN].get(), SDL_TRUE, 0);
    uiGraphic[UI_MapEditor_Pen3x3][HOUSE_HARKONNEN] = LoadPNG_RW(pFileManager->openFile("MapEditorPen3x3.png").get());
    SDL_SetColorKey(uiGraphic[UI_MapEditor_Pen3x3][HOUSE_HARKONNEN].get(), SDL_TRUE, 0);
    uiGraphic[UI_MapEditor_Pen5x5][HOUSE_HARKONNEN] = LoadPNG_RW(pFileManager->openFile("MapEditorPen5x5.png").get());
    SDL_SetColorKey(uiGraphic[UI_MapEditor_Pen5x5][HOUSE_HARKONNEN].get(), SDL_TRUE, 0);



    // load animations
    animation[Anim_HarkonnenEyes] = menshph->getAnimation(0,4,true,true);
    animation[Anim_HarkonnenEyes]->setFrameRate(0.3);
    animation[Anim_HarkonnenMouth] = menshph->getAnimation(5,9,true,true,true);
    animation[Anim_HarkonnenMouth]->setFrameRate(5.0);
    animation[Anim_HarkonnenShoulder] = menshph->getAnimation(10,10,true,true);
    animation[Anim_HarkonnenShoulder]->setFrameRate(1.0);
    animation[Anim_AtreidesEyes] = menshpa->getAnimation(0,4,true,true);
    animation[Anim_AtreidesEyes]->setFrameRate(0.5);
    animation[Anim_AtreidesMouth] = menshpa->getAnimation(5,9,true,true,true);
    animation[Anim_AtreidesMouth]->setFrameRate(5.0);
    animation[Anim_AtreidesShoulder] = menshpa->getAnimation(10,10,true,true);
    animation[Anim_AtreidesShoulder]->setFrameRate(1.0);
    animation[Anim_AtreidesBook] = menshpa->getAnimation(11,12,true,true,true);
    animation[Anim_AtreidesBook]->setNumLoops(1);
    animation[Anim_AtreidesBook]->setFrameRate(0.2);
    animation[Anim_OrdosEyes] = menshpo->getAnimation(0,4,true,true);
    animation[Anim_OrdosEyes]->setFrameRate(0.5);
    animation[Anim_OrdosMouth] = menshpo->getAnimation(5,9,true,true,true);
    animation[Anim_OrdosMouth]->setFrameRate(5.0);
    animation[Anim_OrdosShoulder] = menshpo->getAnimation(10,10,true,true);
    animation[Anim_OrdosShoulder]->setFrameRate(1.0);
    animation[Anim_OrdosRing] = menshpo->getAnimation(11,14,true,true,true);
    animation[Anim_OrdosRing]->setNumLoops(1);
    animation[Anim_OrdosRing]->setFrameRate(6.0);
    animation[Anim_FremenEyes] = PictureFactory::mapMentatAnimationToFremen(animation[Anim_AtreidesEyes].get());
    animation[Anim_FremenMouth] = PictureFactory::mapMentatAnimationToFremen(animation[Anim_AtreidesMouth].get());
    animation[Anim_FremenShoulder] = PictureFactory::mapMentatAnimationToFremen(animation[Anim_AtreidesShoulder].get());
    animation[Anim_FremenBook] = PictureFactory::mapMentatAnimationToFremen(animation[Anim_AtreidesBook].get());
    animation[Anim_SardaukarEyes] = PictureFactory::mapMentatAnimationToSardaukar(animation[Anim_HarkonnenEyes].get());
    animation[Anim_SardaukarMouth] = PictureFactory::mapMentatAnimationToSardaukar(animation[Anim_HarkonnenMouth].get());
    animation[Anim_SardaukarShoulder] = PictureFactory::mapMentatAnimationToSardaukar(animation[Anim_HarkonnenShoulder].get());
    animation[Anim_MercenaryEyes] = PictureFactory::mapMentatAnimationToMercenary(animation[Anim_OrdosEyes].get());
    animation[Anim_MercenaryMouth] = PictureFactory::mapMentatAnimationToMercenary(animation[Anim_OrdosMouth].get());
    animation[Anim_MercenaryShoulder] = PictureFactory::mapMentatAnimationToMercenary(animation[Anim_OrdosShoulder].get());
    animation[Anim_MercenaryRing] = PictureFactory::mapMentatAnimationToMercenary(animation[Anim_OrdosRing].get());

    animation[Anim_BeneEyes] = menshpm->getAnimation(0,4,true,true);
    if(animation[Anim_BeneEyes] != nullptr) {
        animation[Anim_BeneEyes]->setPalette(benePalette);
        animation[Anim_BeneEyes]->setFrameRate(0.5);
    }
    animation[Anim_BeneMouth] = menshpm->getAnimation(5,9,true,true,true);
    if(animation[Anim_BeneMouth] != nullptr) {
        animation[Anim_BeneMouth]->setPalette(benePalette);
        animation[Anim_BeneMouth]->setFrameRate(5.0);
    }
    // the remaining animation are loaded on demand to save some loading time

    // load map choice pieces
    for(int i = 0; i < NUM_MAPCHOICEPIECES; i++) {
        mapChoicePieces[i][HOUSE_HARKONNEN] = Scaler::doubleSurfaceNN(pieces->getPicture(i).get());
        SDL_SetColorKey(mapChoicePieces[i][HOUSE_HARKONNEN].get(), SDL_TRUE, 0);
    }

    // pBackgroundSurface is separate as we never draw it but use it to construct other sprites
    pBackgroundSurface = convertSurfaceToDisplayFormat(PicFactory->createBackground().get());
}

GFXManager::~GFXManager() = default;

SDL_Texture* GFXManager::getZoomedObjPic(unsigned int id, int house, unsigned int z) {
    if(id >= NUM_OBJPICS) {
        THROW(std::invalid_argument, "GFXManager::getZoomedObjPic(): Unit Picture with ID %u is not available!", id);
    }

    if(objPic[id][house][z] == nullptr) {
        // remap to this color
        if(objPic[id][HOUSE_HARKONNEN][z] == nullptr) {
            THROW(std::runtime_error, "GFXManager::getZoomedObjPic(): Unit Picture with ID %u is not loaded!", id);
        }

        objPic[id][house][z] = mapSurfaceColorRange(objPic[id][HOUSE_HARKONNEN][z].get(), PALCOLOR_HARKONNEN, houseToPaletteIndex[house]);
    }

    if(objPicTex[id][house][z] == nullptr) {
        // now convert to display format
        if(id == ObjPic_Windtrap) {
            // Windtrap uses palette animation on PALCOLOR_WINDTRAP_COLORCYCLE; fake this
            objPicTex[id][house][z] = convertSurfaceToTexture(generateWindtrapAnimationFrames(objPic[id][house][z].get()));
        } else if(id == ObjPic_Bullet_SonicTemp) {
            objPicTex[id][house][z] = sdl2::texture_ptr{ SDL_CreateTexture(renderer, SCREEN_FORMAT, SDL_TEXTUREACCESS_TARGET, objPic[id][house][z]->w, objPic[id][house][z]->h) };
        } else if(id == ObjPic_SandwormShimmerTemp) {
            objPicTex[id][house][z] = sdl2::texture_ptr{ SDL_CreateTexture(renderer, SCREEN_FORMAT, SDL_TEXTUREACCESS_TARGET, objPic[id][house][z]->w, objPic[id][house][z]->h) };
        } else {
            objPicTex[id][house][z] = convertSurfaceToTexture(objPic[id][house][z].get());
        }
    }

    return objPicTex[id][house][z].get();
}

zoomable_texture GFXManager::getObjPic(unsigned int id, int house) {
    if(id >= NUM_OBJPICS) {
        THROW(std::invalid_argument, "GFXManager::getObjPic(): Unit Picture with ID %u is not available!", id);
    }

    for(int z = 0; z < NUM_ZOOMLEVEL; z++) {
        if(objPicTex[id][house][z] == nullptr) {
            getZoomedObjPic(id, house, z);  // no assignment as the return value is already stored in objPicTex
        }
    }

    return zoomable_texture{ objPicTex[id][house][0].get(), objPicTex[id][house][1].get(), objPicTex[id][house][2].get() };
}


SDL_Texture* GFXManager::getSmallDetailPic(unsigned int id) {
    if(id >= NUM_SMALLDETAILPICS) {
        return nullptr;
    }
    return smallDetailPicTex[id].get();
}


SDL_Texture* GFXManager::getTinyPicture(unsigned int id) {
    if(id >= NUM_TINYPICTURE) {
        return nullptr;
    }
    return tinyPictureTex[id].get();
}


SDL_Surface* GFXManager::getUIGraphicSurface(unsigned int id, int house) {
    if(id >= NUM_UIGRAPHICS) {
        THROW(std::invalid_argument, "GFXManager::getUIGraphicSurface(): UI Graphic with ID %u is not available!", id);
    }

    if(uiGraphic[id][house] == nullptr) {
        // remap to this color
        if(uiGraphic[id][HOUSE_HARKONNEN] == nullptr) {
            THROW(std::runtime_error, "GFXManager::getUIGraphicSurface(): UI Graphic with ID %u is not loaded!", id);
        }

        uiGraphic[id][house] = mapSurfaceColorRange(uiGraphic[id][HOUSE_HARKONNEN].get(), PALCOLOR_HARKONNEN, houseToPaletteIndex[house]);
    }

    return uiGraphic[id][house].get();
}

SDL_Texture* GFXManager::getUIGraphic(unsigned int id, int house) {
    if(id >= NUM_UIGRAPHICS) {
        THROW(std::invalid_argument, "GFXManager::getUIGraphic(): UI Graphic with ID %u is not available!", id);
    }

    if(uiGraphicTex[id][house] == nullptr) {
        SDL_Surface* pSurface = getUIGraphicSurface(id, house);

        if(id >= UI_MapChoiceArrow_None && id <= UI_MapChoiceArrow_Left) {
            uiGraphicTex[id][house] = convertSurfaceToTexture(generateMapChoiceArrowFrames(pSurface, house));
        } else {
            uiGraphicTex[id][house] = convertSurfaceToTexture(pSurface);
        }
    }

    return uiGraphicTex[id][house].get();
}

SDL_Surface* GFXManager::getMapChoicePieceSurface(unsigned int num, int house) {
    if(num >= NUM_MAPCHOICEPIECES) {
        THROW(std::invalid_argument, "GFXManager::getMapChoicePieceSurface(): Map Piece with number %u is not available!", num);
    }

    if(mapChoicePieces[num][house] == nullptr) {
        // remap to this color
        if(mapChoicePieces[num][HOUSE_HARKONNEN] == nullptr) {
            THROW(std::runtime_error, "GFXManager::getMapChoicePieceSurface(): Map Piece with number %u is not loaded!", num);
        }

        mapChoicePieces[num][house] = mapSurfaceColorRange(mapChoicePieces[num][HOUSE_HARKONNEN].get(), PALCOLOR_HARKONNEN, houseToPaletteIndex[house]);
    }

    return mapChoicePieces[num][house].get();
}

SDL_Texture* GFXManager::getMapChoicePiece(unsigned int num, int house) {
    if(num >= NUM_MAPCHOICEPIECES) {
        THROW(std::invalid_argument, "GFXManager::getMapChoicePiece(): Map Piece with number %u is not available!", num);
    }

    if(mapChoicePiecesTex[num][house] == nullptr) {
        mapChoicePiecesTex[num][house] = convertSurfaceToTexture(getMapChoicePieceSurface(num, house));
    }

    return mapChoicePiecesTex[num][house].get();
}

Animation* GFXManager::getAnimation(unsigned int id) {
    if(id >= NUM_ANIMATION) {
        THROW(std::invalid_argument, "GFXManager::getAnimation(): Animation with ID %u is not available!", id);
    }

    if(animation[id] == nullptr) {
        switch(id) {
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
                animation[Anim_FremenPlanet] = PictureFactory::createFremenPlanet(uiGraphic[UI_Herald_ColoredLarge][HOUSE_FREMEN].get());
                animation[Anim_FremenPlanet]->setFrameRate(10);
            } break;

            case Anim_SardaukarPlanet: {
                animation[Anim_SardaukarPlanet] = PictureFactory::createSardaukarPlanet(getAnimation(Anim_OrdosPlanet), uiGraphic[UI_Herald_ColoredLarge][HOUSE_SARDAUKAR].get());
                animation[Anim_SardaukarPlanet]->setFrameRate(10);
            } break;

            case Anim_MercenaryPlanet: {
                animation[Anim_MercenaryPlanet] = PictureFactory::createMercenaryPlanet(getAnimation(Anim_AtreidesPlanet), uiGraphic[UI_Herald_ColoredLarge][HOUSE_MERCENARY].get());
                animation[Anim_MercenaryPlanet]->setFrameRate(10);
            } break;

            case Anim_Win1:             animation[Anim_Win1] = loadAnimationFromWsa("WIN1.WSA");                 break;
            case Anim_Win2:             animation[Anim_Win2] = loadAnimationFromWsa("WIN2.WSA");                 break;
            case Anim_Lose1:            animation[Anim_Lose1] = loadAnimationFromWsa("LOSTBILD.WSA");            break;
            case Anim_Lose2:            animation[Anim_Lose2] = loadAnimationFromWsa("LOSTVEHC.WSA");            break;
            case Anim_Barracks:         animation[Anim_Barracks] = loadAnimationFromWsa("BARRAC.WSA");           break;
            case Anim_Carryall:         animation[Anim_Carryall] = loadAnimationFromWsa("CARRYALL.WSA");         break;
            case Anim_ConstructionYard: animation[Anim_ConstructionYard] = loadAnimationFromWsa("CONSTRUC.WSA"); break;
            case Anim_Fremen:           animation[Anim_Fremen] = loadAnimationFromWsa("FREMEN.WSA");             break;
            case Anim_DeathHand:        animation[Anim_DeathHand] = loadAnimationFromWsa("GOLD-BB.WSA");         break;
            case Anim_Devastator:       animation[Anim_Devastator] = loadAnimationFromWsa("HARKTANK.WSA");       break;
            case Anim_Harvester:        animation[Anim_Harvester] = loadAnimationFromWsa("HARVEST.WSA");         break;
            case Anim_Radar:            animation[Anim_Radar] = loadAnimationFromWsa("HEADQRTS.WSA");            break;
            case Anim_HighTechFactory:  animation[Anim_HighTechFactory] = loadAnimationFromWsa("HITCFTRY.WSA");  break;
            case Anim_SiegeTank:        animation[Anim_SiegeTank] = loadAnimationFromWsa("HTANK.WSA");           break;
            case Anim_HeavyFactory:     animation[Anim_HeavyFactory] = loadAnimationFromWsa("HVYFTRY.WSA");      break;
            case Anim_Trooper:          animation[Anim_Trooper] = loadAnimationFromWsa("HYINFY.WSA");            break;
            case Anim_Infantry:         animation[Anim_Infantry] = loadAnimationFromWsa("INFANTRY.WSA");         break;
            case Anim_IX:               animation[Anim_IX] = loadAnimationFromWsa("IX.WSA");                     break;
            case Anim_LightFactory:     animation[Anim_LightFactory] = loadAnimationFromWsa("LITEFTRY.WSA");     break;
            case Anim_Tank:             animation[Anim_Tank] = loadAnimationFromWsa("LTANK.WSA");                break;
            case Anim_MCV:              animation[Anim_MCV] = loadAnimationFromWsa("MCV.WSA");                   break;
            case Anim_Deviator:         animation[Anim_Deviator] = loadAnimationFromWsa("ORDRTANK.WSA");         break;
            case Anim_Ornithopter:      animation[Anim_Ornithopter] = loadAnimationFromWsa("ORNI.WSA");          break;
            case Anim_Raider:           animation[Anim_Raider] = loadAnimationFromWsa("OTRIKE.WSA");             break;
            case Anim_Palace:           animation[Anim_Palace] = loadAnimationFromWsa("PALACE.WSA");             break;
            case Anim_Quad:             animation[Anim_Quad] = loadAnimationFromWsa("QUAD.WSA");                 break;
            case Anim_Refinery:         animation[Anim_Refinery] = loadAnimationFromWsa("REFINERY.WSA");         break;
            case Anim_RepairYard:       animation[Anim_RepairYard] = loadAnimationFromWsa("REPAIR.WSA");         break;
            case Anim_Launcher:         animation[Anim_Launcher] = loadAnimationFromWsa("RTANK.WSA");            break;
            case Anim_RocketTurret:     animation[Anim_RocketTurret] = loadAnimationFromWsa("RTURRET.WSA");      break;
            case Anim_Saboteur:         animation[Anim_Saboteur] = loadAnimationFromWsa("SABOTURE.WSA");         break;
            case Anim_Slab1:            animation[Anim_Slab1] = loadAnimationFromWsa("SLAB.WSA");                break;
            case Anim_SonicTank:        animation[Anim_SonicTank] = loadAnimationFromWsa("STANK.WSA");           break;
            case Anim_StarPort:         animation[Anim_StarPort] = loadAnimationFromWsa("STARPORT.WSA");         break;
            case Anim_Silo:             animation[Anim_Silo] = loadAnimationFromWsa("STORAGE.WSA");              break;
            case Anim_Trike:            animation[Anim_Trike] = loadAnimationFromWsa("TRIKE.WSA");               break;
            case Anim_GunTurret:        animation[Anim_GunTurret] = loadAnimationFromWsa("TURRET.WSA");          break;
            case Anim_Wall:             animation[Anim_Wall] = loadAnimationFromWsa("WALL.WSA");                 break;
            case Anim_WindTrap:         animation[Anim_WindTrap] = loadAnimationFromWsa("WINDTRAP.WSA");         break;
            case Anim_WOR:              animation[Anim_WOR] = loadAnimationFromWsa("WOR.WSA");                   break;
            case Anim_Sandworm:         animation[Anim_Sandworm] = loadAnimationFromWsa("WORM.WSA");             break;
            case Anim_Sardaukar:        animation[Anim_Sardaukar] = loadAnimationFromWsa("SARDUKAR.WSA");        break;
            case Anim_Frigate: {
                if(pFileManager->exists("FRIGATE.WSA")) {
                    animation[Anim_Frigate] = loadAnimationFromWsa("FRIGATE.WSA");
                } else {
                    // US-Version 1.07 does not contain FRIGATE.WSA
                    // We replace it with the starport
                    animation[Anim_Frigate] = loadAnimationFromWsa("STARPORT.WSA");
                }
            } break;
            case Anim_Slab4:            animation[Anim_Slab4] = loadAnimationFromWsa("4SLAB.WSA");               break;

            default: {
                THROW(std::runtime_error, "GFXManager::getAnimation(): Invalid animation ID %u", id);
            } break;
        }

        if(id >= Anim_Barracks && id <= Anim_Slab4) {
            animation[id]->setFrameRate(6);
        }
    }

    return animation[id].get();
}

std::unique_ptr<Shpfile> GFXManager::loadShpfile(const std::string& filename) const {
    try {
        return std::make_unique<Shpfile>(pFileManager->openFile(filename).get());
    } catch (std::exception &e) {
        THROW(std::runtime_error, "Error in file \"" + filename + "\":" + e.what());
    }
}

std::unique_ptr<Wsafile> GFXManager::loadWsafile(const std::string& filename) const {
    try {
        return std::make_unique<Wsafile>(pFileManager->openFile(filename).get());
    } catch (std::exception &e) {
        THROW(std::runtime_error, std::string("Error in file \"" + filename + "\":") + e.what());
    }
}

sdl2::texture_ptr GFXManager::extractSmallDetailPic(const std::string& filename) const
{
    sdl2::surface_ptr pSurface{ SDL_CreateRGBSurface(0, 91, 55, 8, 0, 0, 0, 0) };

    // create new picture surface
    if (pSurface == nullptr) {
        THROW(sdl_error, "Cannot create new surface: %s!", SDL_GetError());
    }

    { // Scope
        auto myWsafile = std::make_unique<Wsafile>(pFileManager->openFile(filename).get());

        sdl2::surface_ptr tmp{ myWsafile->getPicture(0) };
        if(tmp == nullptr) {
            THROW(std::runtime_error, "Cannot decode first frame in file '%s'!", filename);
        }

        if((tmp->w != 184) || (tmp->h != 112)) {
            THROW(std::runtime_error, "Picture '%s' is not of size 184x112!", filename);
        }

        palette.applyToSurface(pSurface.get());

        sdl2::surface_lock lock_out{ pSurface.get() };
        sdl2::surface_lock lock_in{ tmp.get() };

        char * RESTRICT const out = static_cast<char*>(lock_out.pixels());
        const char * RESTRICT const in = static_cast<const char*>(lock_in.pixels());

        //Now we can copy pixel by pixel
        for (auto y = 0; y < 55; y++) {
            for (auto x = 0; x < 91; x++) {
                out[y*pSurface->pitch + x] = in[((y * 2) + 1)*tmp->pitch + (x * 2) + 1];
            }
        }
    }

    return convertSurfaceToTexture(pSurface.get());
}

std::unique_ptr<Animation> GFXManager::loadAnimationFromWsa(const std::string& filename) const {
    auto file = pFileManager->openFile(filename);
    auto wsafile = std::make_unique<Wsafile>(file.get());
    auto animation = wsafile->getAnimation(0,wsafile->getNumFrames() - 1,true,false);
    return animation;
}

sdl2::surface_ptr GFXManager::generateWindtrapAnimationFrames(SDL_Surface* windtrapPic) const {
    int windtrapColorQuantizizer = 255/((NUM_WINDTRAP_ANIMATIONS/2)-2);
    int windtrapSize = windtrapPic->h;
    int sizeX = NUM_WINDTRAP_ANIMATIONS_PER_ROW*windtrapSize;
    int sizeY = ((2+NUM_WINDTRAP_ANIMATIONS+NUM_WINDTRAP_ANIMATIONS_PER_ROW-1)/NUM_WINDTRAP_ANIMATIONS_PER_ROW)*windtrapSize;
    sdl2::surface_ptr returnPic{ SDL_CreateRGBSurface(0, sizeX, sizeY, SCREEN_BPP, RMASK, GMASK, BMASK, AMASK) };
    SDL_SetSurfaceBlendMode(returnPic.get(), SDL_BLENDMODE_NONE);

    // copy building phase
    SDL_Rect src = { 0, 0, 2*windtrapSize, windtrapSize};
    SDL_Rect dest = src;
    SDL_BlitSurface(windtrapPic, &src, returnPic.get(), &dest);

    src.w = windtrapSize;
    dest.x += dest.w;
    dest.w = windtrapSize;

    for(int i = 0; i < NUM_WINDTRAP_ANIMATIONS; i++) {
        src.x = ((i/3) % 2 == 0) ? 2*windtrapSize : 3*windtrapSize;

        SDL_Color windtrapColor;
        if(i < NUM_WINDTRAP_ANIMATIONS/2) {
            int val = i*windtrapColorQuantizizer;
            windtrapColor.r = static_cast<Uint8>(std::min(80, val));
            windtrapColor.g = static_cast<Uint8>(std::min(80, val));
            windtrapColor.b = static_cast<Uint8>(std::min(255, val));
            windtrapColor.a = 255;
        } else {
            int val = (i-NUM_WINDTRAP_ANIMATIONS/2)*windtrapColorQuantizizer;
            windtrapColor.r = static_cast<Uint8>(std::max(0, 80-val));
            windtrapColor.g = static_cast<Uint8>(std::max(0, 80-val));
            windtrapColor.b = static_cast<Uint8>(std::max(0, 255-val));
            windtrapColor.a = 255;
        }
        SDL_SetPaletteColors(windtrapPic->format->palette, &windtrapColor, PALCOLOR_WINDTRAP_COLORCYCLE, 1);

        SDL_BlitSurface(windtrapPic, &src, returnPic.get(), &dest);

        dest.x += dest.w;
        dest.y = dest.y + dest.h * (dest.x / sizeX);
        dest.x = dest.x % sizeX;
    }

    if((returnPic->w > 2048) || (returnPic->h > 2048)) {
        SDL_Log("Warning: Size of sprite sheet for windtrap is %dx%d; may exceed hardware limits on older GPUs!", returnPic->w, returnPic->h);
    }

    return returnPic;
}


sdl2::surface_ptr GFXManager::generateMapChoiceArrowFrames(SDL_Surface* arrowPic, int house) const {
    sdl2::surface_ptr returnPic{ SDL_CreateRGBSurface(0, arrowPic->w * 4, arrowPic->h, SCREEN_BPP, RMASK, GMASK, BMASK, AMASK) };

    SDL_Rect dest = {0, 0, arrowPic->w, arrowPic->h};

    for(int i = 0; i < 4; i++) {
        for(int k = 0; k < 4; k++) {
            SDL_SetPaletteColors(arrowPic->format->palette, &palette[houseToPaletteIndex[house]+((i+k)%4)], 251+k, 1);
        }

        SDL_BlitSurface(arrowPic, nullptr, returnPic.get(), &dest);
        dest.x += dest.w;
    }

    return returnPic;
}

sdl2::surface_ptr GFXManager::generateDoubledObjPic(unsigned int id, int h) const {
    sdl2::surface_ptr pSurface;
    std::string filename = "Mask_2x_" + ObjPicNames.at(id) + ".png";
    if(settings.video.scaler == "ScaleHD") {
        if(pFileManager->exists(filename)) {
            pSurface = sdl2::surface_ptr{ Scaler::doubleTiledSurfaceNN(objPic[id][h][0].get(), objPicTiles[id].x, objPicTiles[id].y) };

            sdl2::surface_ptr pOverlay = LoadPNG_RW(pFileManager->openFile(filename).get());
            SDL_SetColorKey(pOverlay.get(), SDL_TRUE, PALCOLOR_UI_COLORCYCLE);

            // SDL_BlitSurface will silently map PALCOLOR_BLACK to PALCOLOR_TRANSPARENT as both are RGB(0,0,0,255), so make them temporarily different
            pOverlay->format->palette->colors[PALCOLOR_BLACK].g = 1;
            pSurface->format->palette->colors[PALCOLOR_BLACK].g = 1;
            SDL_BlitSurface(pOverlay.get(), NULL, pSurface.get(), NULL);
            pOverlay->format->palette->colors[PALCOLOR_BLACK].g = 0;
            pSurface->format->palette->colors[PALCOLOR_BLACK].g = 0;
        } else {
            SDL_Log("Warning: No HD sprite sheet for '%s' in zoom level 1!", ObjPicNames.at(id).c_str());
            pSurface = sdl2::surface_ptr{ Scaler::defaultDoubleTiledSurface(objPic[id][h][0].get(), objPicTiles[id].x, objPicTiles[id].y) };
        }
    } else {
        pSurface = sdl2::surface_ptr{ Scaler::defaultDoubleTiledSurface(objPic[id][h][0].get(), objPicTiles[id].x, objPicTiles[id].y) };
    }

    if((pSurface->w > 2048) || (pSurface->h > 2048)) {
        SDL_Log("Warning: Size of sprite sheet for '%s' in zoom level 1 is %dx%d; may exceed hardware limits on older GPUs!", ObjPicNames.at(id).c_str(), pSurface->w, pSurface->h);
    }

    return pSurface;
}

sdl2::surface_ptr GFXManager::generateTripledObjPic(unsigned int id, int h) const {
    sdl2::surface_ptr pSurface;
    const std::string filename = "Mask_3x_" + ObjPicNames.at(id) + ".png";
    if(settings.video.scaler == "ScaleHD") {
        if(pFileManager->exists(filename)) {
            pSurface = sdl2::surface_ptr{ Scaler::tripleTiledSurfaceNN(objPic[id][h][0].get(), objPicTiles[id].x, objPicTiles[id].y) };

            sdl2::surface_ptr pOverlay = LoadPNG_RW(pFileManager->openFile(filename).get());
            SDL_SetColorKey(pOverlay.get(), SDL_TRUE, PALCOLOR_UI_COLORCYCLE);

            // SDL_BlitSurface will silently map PALCOLOR_BLACK to PALCOLOR_TRANSPARENT as both are RGB(0,0,0,255), so make them temporarily different
            pOverlay->format->palette->colors[PALCOLOR_BLACK].g = 1;
            pSurface->format->palette->colors[PALCOLOR_BLACK].g = 1;
            SDL_BlitSurface(pOverlay.get(), NULL, pSurface.get(), NULL);
            pOverlay->format->palette->colors[PALCOLOR_BLACK].g = 0;
            pSurface->format->palette->colors[PALCOLOR_BLACK].g = 0;
        } else {
            SDL_Log("Warning: No HD sprite sheet for '%s' in zoom level 2!", ObjPicNames.at(id).c_str());
            pSurface = sdl2::surface_ptr{ Scaler::defaultTripleTiledSurface(objPic[id][h][0].get(), objPicTiles[id].x, objPicTiles[id].y) };
        }
    } else {
        pSurface = sdl2::surface_ptr{ Scaler::defaultTripleTiledSurface(objPic[id][h][0].get(), objPicTiles[id].x, objPicTiles[id].y) };
    }


    if((pSurface->w > 2048) || (pSurface->h > 2048)) {
        SDL_Log("Warning: Size of sprite sheet for '%s' in zoom level 2 is %dx%d; may exceed hardware limits on older GPUs!", ObjPicNames.at(id).c_str(), pSurface->w, pSurface->h);
    }

    return pSurface;
}
