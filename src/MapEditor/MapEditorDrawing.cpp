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

#include <MapEditor/MapEditor.h>

#include <MapEditor/MapMirror.h>

#include <FileClasses/GFXManager.h>
#include <FileClasses/LoadSavePNG.h>

#include <structures/Wall.h>

#include "misc/DrawingRectHelper.h"
#include "misc/dune_sdl.h"
#include <misc/FileSystem.h>
#include <misc/draw_util.h>

#include "Renderer/DuneRenderer.h"

#include "ScreenBorder.h"
#include <Tile.h>
#include <globals.h>
#include <mmath.h>
#include <sand.h>

#include <algorithm>

void MapEditor::drawScreen() {
    auto* const renderer = dune::globals::renderer.get();

    // clear whole screen
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);

    // the actual map
    drawMap(dune::globals::screenborder.get(), false);

    pInterface_->draw(Point(0, 0));
    pInterface_->drawOverlay(Point(0, 0));

    // Cursor
    drawCursor();

    Dune_RenderPresent(renderer);
}

void MapEditor::drawCursor() {

    if (!(SDL_GetWindowFlags(dune::globals::window.get()) & SDL_WINDOW_MOUSE_FOCUS))
        return;

    const auto* const gfx = dune::globals::pGFXManager.get();
    using dune::globals::drawnMouseX;
    using dune::globals::drawnMouseY;

    const DuneTexture* pCursor = nullptr;
    SDL_FRect dest{};

    if (scrollLeftMode_ || scrollRightMode_ || scrollUpMode_ || scrollDownMode_) {
        if (scrollLeftMode_ && !scrollRightMode_) {
            pCursor = gfx->getUIGraphic(UI_CursorLeft);
            dest    = calcDrawingRect(pCursor, drawnMouseX, dune::globals::drawnMouseY - 5, HAlign::Left, VAlign::Top);
        } else if (scrollRightMode_ && !scrollLeftMode_) {
            pCursor = gfx->getUIGraphic(UI_CursorRight);
            dest = calcDrawingRect(pCursor, drawnMouseX, dune::globals::drawnMouseY - 5, HAlign::Center, VAlign::Top);
        }

        if (pCursor == nullptr) {
            if (scrollUpMode_ && !scrollDownMode_) {
                pCursor = gfx->getUIGraphic(UI_CursorUp);
                dest = calcDrawingRect(pCursor, drawnMouseX - 5, dune::globals::drawnMouseY, HAlign::Left, VAlign::Top);
            } else if (scrollDownMode_ && !scrollUpMode_) {
                pCursor = gfx->getUIGraphic(UI_CursorDown);
                dest =
                    calcDrawingRect(pCursor, drawnMouseX - 5, dune::globals::drawnMouseY, HAlign::Left, VAlign::Center);
            } else {
                pCursor = gfx->getUIGraphic(UI_CursorNormal);
                dest    = calcDrawingRect(pCursor, drawnMouseX, dune::globals::drawnMouseY, HAlign::Left, VAlign::Top);
            }
        }
    } else {
        pCursor = gfx->getUIGraphic(UI_CursorNormal);
        dest    = calcDrawingRect(pCursor, drawnMouseX, dune::globals::drawnMouseY, HAlign::Left, VAlign::Top);

        if ((drawnMouseX < sideBarPos_.x) && (dune::globals::drawnMouseY > topBarPos_.h)
            && (currentMirrorMode_ != MirrorModeNone) && (!pInterface_->hasChildWindow())) {

            const DuneTexture* pMirrorIcon = nullptr;
            switch (currentMirrorMode_) {
                case MirrorModeHorizontal: pMirrorIcon = gfx->getUIGraphic(UI_MapEditor_MirrorHorizontalIcon); break;
                case MirrorModeVertical: pMirrorIcon = gfx->getUIGraphic(UI_MapEditor_MirrorVerticalIcon); break;
                case MirrorModeBoth: pMirrorIcon = gfx->getUIGraphic(UI_MapEditor_MirrorBothIcon); break;
                case MirrorModePoint: pMirrorIcon = gfx->getUIGraphic(UI_MapEditor_MirrorPointIcon); break;
                default: pMirrorIcon = gfx->getUIGraphic(UI_MapEditor_MirrorNoneIcon); break;
            }

            if (pMirrorIcon)
                pMirrorIcon->draw(dune::globals::renderer.get(), drawnMouseX + 5, dune::globals::drawnMouseY + 5);
        }
    }

    if (pCursor)
        pCursor->draw(dune::globals::renderer.get(), dest.x, dest.y);
}

void MapEditor::drawMap(ScreenBorder* pScreenborder, bool bCompleteMap) const {
    const auto zoomedTilesize = world2zoomedWorld(TILESIZE);

    Coord TopLeftTile     = pScreenborder->getTopLeftTile();
    Coord BottomRightTile = pScreenborder->getBottomRightTile();

    // extend the view a little bit to avoid graphical glitches
    TopLeftTile.x     = std::max(0, TopLeftTile.x - 1);
    TopLeftTile.y     = std::max(0, TopLeftTile.y - 1);
    BottomRightTile.x = std::min(map_.getSizeX() - 1, BottomRightTile.x + 1);
    BottomRightTile.y = std::min(map_.getSizeY() - 1, BottomRightTile.y + 1);

    const auto zoom = dune::globals::currentZoomlevel;

    // Load Terrain Surface
    const auto* const terrainSprite = dune::globals::pGFXManager->getZoomedObjPic(ObjPic_Terrain, zoom);

    /* draw ground */
    for (int y = TopLeftTile.y; y <= BottomRightTile.y; y++) {
        for (int x = TopLeftTile.x; x <= BottomRightTile.x; x++) {

            int tile = 0;

            switch (getTerrain(x, y)) {
                case TERRAINTYPE::Terrain_Slab: {
                    tile = static_cast<int>(Tile::TERRAINTILETYPE::TerrainTile_Slab);
                } break;

                case TERRAINTYPE::Terrain_Sand: {
                    tile = static_cast<int>(Tile::TERRAINTILETYPE::TerrainTile_Sand);
                } break;

                case TERRAINTYPE::Terrain_Rock: {
                    // determine which surrounding tiles are rock
                    const int up = (y - 1 < 0) || (getTerrain(x, y - 1) == TERRAINTYPE::Terrain_Rock)
                                || (getTerrain(x, y - 1) == TERRAINTYPE::Terrain_Slab)
                                || (getTerrain(x, y - 1) == TERRAINTYPE::Terrain_Mountain);
                    const int right = (x + 1 >= map_.getSizeX()) || (getTerrain(x + 1, y) == TERRAINTYPE::Terrain_Rock)
                                   || (getTerrain(x + 1, y) == TERRAINTYPE::Terrain_Slab)
                                   || (getTerrain(x + 1, y) == TERRAINTYPE::Terrain_Mountain);
                    const int down = (y + 1 >= map_.getSizeY()) || (getTerrain(x, y + 1) == TERRAINTYPE::Terrain_Rock)
                                  || (getTerrain(x, y + 1) == TERRAINTYPE::Terrain_Slab)
                                  || (getTerrain(x, y + 1) == TERRAINTYPE::Terrain_Mountain);
                    const int left = (x - 1 < 0) || (getTerrain(x - 1, y) == TERRAINTYPE::Terrain_Rock)
                                  || (getTerrain(x - 1, y) == TERRAINTYPE::Terrain_Slab)
                                  || (getTerrain(x - 1, y) == TERRAINTYPE::Terrain_Mountain);

                    tile = static_cast<int>(Tile::TERRAINTILETYPE::TerrainTile_Rock)
                         + (up | (right << 1) | (down << 2) | (left << 3));
                } break;

                case TERRAINTYPE::Terrain_Dunes: {
                    // determine which surrounding tiles are dunes
                    const int up = (y - 1 < 0) || (getTerrain(x, y - 1) == TERRAINTYPE::Terrain_Dunes);
                    const int right =
                        (x + 1 >= map_.getSizeX()) || (getTerrain(x + 1, y) == TERRAINTYPE::Terrain_Dunes);
                    const int down = (y + 1 >= map_.getSizeY()) || (getTerrain(x, y + 1) == TERRAINTYPE::Terrain_Dunes);
                    const int left = (x - 1 < 0) || (getTerrain(x - 1, y) == TERRAINTYPE::Terrain_Dunes);

                    tile = static_cast<int>(Tile::TERRAINTILETYPE::TerrainTile_Dunes)
                         + (up | (right << 1) | (down << 2) | (left << 3));
                } break;

                case TERRAINTYPE::Terrain_Mountain: {
                    // determine which surrounding tiles are mountains
                    const int up = (y - 1 < 0) || (getTerrain(x, y - 1) == TERRAINTYPE::Terrain_Mountain);
                    const int right =
                        (x + 1 >= map_.getSizeX()) || (getTerrain(x + 1, y) == TERRAINTYPE::Terrain_Mountain);
                    const int down =
                        (y + 1 >= map_.getSizeY()) || (getTerrain(x, y + 1) == TERRAINTYPE::Terrain_Mountain);
                    const int left = (x - 1 < 0) || (getTerrain(x - 1, y) == TERRAINTYPE::Terrain_Mountain);

                    tile = static_cast<int>(Tile::TERRAINTILETYPE::TerrainTile_Mountain)
                         + (up | (right << 1) | (down << 2) | (left << 3));
                } break;

                case TERRAINTYPE::Terrain_Spice: {
                    // determine which surrounding tiles are spice
                    const int up = (y - 1 < 0) || (getTerrain(x, y - 1) == TERRAINTYPE::Terrain_Spice)
                                || (getTerrain(x, y - 1) == TERRAINTYPE::Terrain_ThickSpice);
                    const int right = (x + 1 >= map_.getSizeX()) || (getTerrain(x + 1, y) == TERRAINTYPE::Terrain_Spice)
                                   || (getTerrain(x + 1, y) == TERRAINTYPE::Terrain_ThickSpice);
                    const int down = (y + 1 >= map_.getSizeY()) || (getTerrain(x, y + 1) == TERRAINTYPE::Terrain_Spice)
                                  || (getTerrain(x, y + 1) == TERRAINTYPE::Terrain_ThickSpice);
                    const int left = (x - 1 < 0) || (getTerrain(x - 1, y) == TERRAINTYPE::Terrain_Spice)
                                  || (getTerrain(x - 1, y) == TERRAINTYPE::Terrain_ThickSpice);

                    tile = static_cast<int>(Tile::TERRAINTILETYPE::TerrainTile_Spice)
                         + (up | (right << 1) | (down << 2) | (left << 3));
                } break;

                case TERRAINTYPE::Terrain_ThickSpice: {
                    // determine which surrounding tiles are thick spice
                    const int up = (y - 1 < 0) || (getTerrain(x, y - 1) == TERRAINTYPE::Terrain_ThickSpice);
                    const int right =
                        (x + 1 >= map_.getSizeX()) || (getTerrain(x + 1, y) == TERRAINTYPE::Terrain_ThickSpice);
                    const int down =
                        (y + 1 >= map_.getSizeY()) || (getTerrain(x, y + 1) == TERRAINTYPE::Terrain_ThickSpice);
                    const int left = (x - 1 < 0) || (getTerrain(x - 1, y) == TERRAINTYPE::Terrain_ThickSpice);

                    tile = static_cast<int>(Tile::TERRAINTILETYPE::TerrainTile_ThickSpice)
                         + (up | (right << 1) | (down << 2) | (left << 3));
                } break;

                case TERRAINTYPE::Terrain_SpiceBloom: {
                    tile = static_cast<int>(Tile::TERRAINTILETYPE::TerrainTile_SpiceBloom);
                } break;

                case TERRAINTYPE::Terrain_SpecialBloom: {
                    tile = static_cast<int>(Tile::TERRAINTILETYPE::TerrainTile_SpecialBloom);
                } break;

                default: {
                    THROW(std::runtime_error, "MapEditor::DrawMap(): Invalid terrain type");
                }
            }

            // draw map[x][y]
            const SDL_Rect source{(tile % NUM_TERRAIN_TILES_X) * zoomedTilesize,
                                  (tile / NUM_TERRAIN_TILES_X) * zoomedTilesize,
                                  zoomedTilesize,
                                  zoomedTilesize};
            const SDL_FRect drawLocation{(pScreenborder->world2screenX(x * TILESIZE)),
                                         (pScreenborder->world2screenY(y * TILESIZE)),
                                         static_cast<float>(zoomedTilesize),
                                         static_cast<float>(zoomedTilesize)};
            Dune_RenderCopyF(dune::globals::renderer.get(), terrainSprite, &source, &drawLocation);
        }
    }

    std::vector<int> selectedStructures = getMirrorStructures(selectedStructureID_);

    auto* const gfx      = dune::globals::pGFXManager.get();
    auto* const renderer = dune::globals::renderer.get();

    for (const auto& structure : structures_) {

        Coord position = structure.position_;

        SDL_Rect selectionDest;
        if (structure.itemID_ == Structure_Slab1) {
            // Load Terrain sprite
            SDL_Rect source = {static_cast<int>(Tile::TERRAINTILETYPE::TerrainTile_Slab) * zoomedTilesize,
                               0,
                               zoomedTilesize,
                               zoomedTilesize};
            SDL_FRect dest  = {(pScreenborder->world2screenX(position.x * TILESIZE)),
                               (pScreenborder->world2screenY(position.y * TILESIZE)),
                               static_cast<float>(zoomedTilesize),
                               static_cast<float>(zoomedTilesize)};

            Dune_RenderCopyF(renderer, terrainSprite, &source, &dest);

            selectionDest = SDL_Rect{
                static_cast<int>(dest.x), static_cast<int>(dest.y), static_cast<int>(dest.w), static_cast<int>(dest.h)};
        } else if (structure.itemID_ == Structure_Slab4) {
            // Load Terrain Surface
            for (int y = position.y; y < position.y + 2; y++) {
                for (int x = position.x; x < position.x + 2; x++) {
                    SDL_Rect source = {static_cast<int>(Tile::TERRAINTILETYPE::TerrainTile_Slab) * zoomedTilesize,
                                       0,
                                       zoomedTilesize,
                                       zoomedTilesize};
                    SDL_FRect dest  = {(pScreenborder->world2screenX(x * TILESIZE)),
                                       (pScreenborder->world2screenY(y * TILESIZE)),
                                       static_cast<float>(zoomedTilesize),
                                       static_cast<float>(zoomedTilesize)};

                    Dune_RenderCopyF(renderer, terrainSprite, &source, &dest);
                }
            }

            selectionDest.x = pScreenborder->world2screenX(position.x * TILESIZE);
            selectionDest.y = pScreenborder->world2screenY(position.y * TILESIZE);
            selectionDest.w = world2zoomedWorld(2 * TILESIZE);
            selectionDest.h = world2zoomedWorld(2 * TILESIZE);
        } else if (structure.itemID_ == Structure_Wall) {
            bool left  = false;
            bool down  = false;
            bool right = false;
            bool up    = false;
            for (const Structure& structure1 : structures_) {
                if (structure1.itemID_ == Structure_Wall) {
                    if ((structure1.position_.x == position.x - 1) && (structure1.position_.y == position.y))
                        left = true;
                    if ((structure1.position_.x == position.x) && (structure1.position_.y == position.y + 1))
                        down = true;
                    if ((structure1.position_.x == position.x + 1) && (structure1.position_.y == position.y))
                        right = true;
                    if ((structure1.position_.x == position.x) && (structure1.position_.y == position.y - 1))
                        up = true;
                }
            }

            auto maketile = 0;
            if ((left) && (right) && (up) && (down)) {
                maketile = Wall::Wall_Full; // solid wall
            } else if ((!left) && (right) && (up) && (down)) {
                maketile = Wall::Wall_UpDownRight; // missing left edge
            } else if ((left) && (!right) && (up) && (down)) {
                maketile = Wall::Wall_UpDownLeft; // missing right edge
            } else if ((left) && (right) && (!up) && (down)) {
                maketile = Wall::Wall_DownLeftRight; // missing top edge
            } else if ((left) && (right) && (up) && (!down)) {
                maketile = Wall::Wall_UpLeftRight; // missing bottom edge
            } else if ((!left) && (right) && (!up) && (down)) {
                maketile = Wall::Wall_DownRight; // missing top left edge
            } else if ((left) && (!right) && (up) && (!down)) {
                maketile = Wall::Wall_UpLeft; // missing bottom right edge
            } else if ((left) && (!right) && (!up) && (down)) {
                maketile = Wall::Wall_DownLeft; // missing top right edge
            } else if ((!left) && (right) && (up) && (!down)) {
                maketile = Wall::Wall_UpRight; // missing bottom left edge
            } else if ((left) && (!right) && (!up) && (!down)) {
                maketile = Wall::Wall_LeftRight; // missing above, right and below
            } else if ((!left) && (right) && (!up) && (!down)) {
                maketile = Wall::Wall_LeftRight; // missing above, left and below
            } else if ((!left) && (!right) && (up) && (!down)) {
                maketile = Wall::Wall_UpDown; // only up
            } else if ((!left) && (!right) && (!up) && (down)) {
                maketile = Wall::Wall_UpDown; // only down
            } else if ((left) && (right) && (!up) && (!down)) {
                maketile = Wall::Wall_LeftRight; // missing above and below
            } else if ((!left) && (!right) && (up) && (down)) {
                maketile = Wall::Wall_UpDown; // missing left and right
            } else if ((!left) && (!right) && (!up) && (!down)) {
                maketile = Wall::Wall_Standalone; // missing left and right
            }

            // Load Wall texture
            const auto* const WallSprite = gfx->getZoomedObjPic(ObjPic_Wall, zoom);

            const SDL_Rect source{maketile * zoomedTilesize, 0, zoomedTilesize, zoomedTilesize};
            const SDL_FRect dest{(pScreenborder->world2screenX(position.x * TILESIZE)),
                                 (pScreenborder->world2screenY(position.y * TILESIZE)),
                                 static_cast<float>(zoomedTilesize),
                                 static_cast<float>(zoomedTilesize)};

            Dune_RenderCopyF(renderer, WallSprite, &source, &dest);

            selectionDest = SDL_Rect{
                static_cast<int>(dest.x), static_cast<int>(dest.y), static_cast<int>(dest.w), static_cast<int>(dest.h)};
        } else {

            ObjPic_enum objectPic{};

            // clang-format off
            switch(structure.itemID_) {
                case Structure_Barracks:            objectPic = ObjPic_Barracks;            break;
                case Structure_ConstructionYard:    objectPic = ObjPic_ConstructionYard;    break;
                case Structure_GunTurret:           objectPic = ObjPic_GunTurret;           break;
                case Structure_HeavyFactory:        objectPic = ObjPic_HeavyFactory;        break;
                case Structure_HighTechFactory:     objectPic = ObjPic_HighTechFactory;     break;
                case Structure_IX:                  objectPic = ObjPic_IX;                  break;
                case Structure_LightFactory:        objectPic = ObjPic_LightFactory;        break;
                case Structure_Palace:              objectPic = ObjPic_Palace;              break;
                case Structure_Radar:               objectPic = ObjPic_Radar;               break;
                case Structure_Refinery:            objectPic = ObjPic_Refinery;            break;
                case Structure_RepairYard:          objectPic = ObjPic_RepairYard;          break;
                case Structure_RocketTurret:        objectPic = ObjPic_RocketTurret;        break;
                case Structure_Silo:                objectPic = ObjPic_Silo;                break;
                case Structure_StarPort:            objectPic = ObjPic_Starport;            break;
                case Structure_Wall:                objectPic = ObjPic_Wall;                break;
                case Structure_WindTrap:            objectPic = ObjPic_Windtrap;            break;
                case Structure_WOR:                 objectPic = ObjPic_WOR;                 break;
                default:                            objectPic = {};                         break;
            }
            // clang-format on

            const auto* ObjectSprite = gfx->getZoomedObjPic(objectPic, structure.house_, zoom);

            Coord frameSize = world2zoomedWorld(getStructureSize(structure.itemID_) * TILESIZE);

            SDL_Rect source = {
                frameSize.x * (structure.itemID_ == Structure_WindTrap ? 9 : 2), 0, frameSize.x, frameSize.y};
            SDL_FRect dest = {(pScreenborder->world2screenX(position.x * TILESIZE)),
                              (pScreenborder->world2screenY(position.y * TILESIZE)),
                              static_cast<float>(frameSize.x),
                              static_cast<float>(frameSize.y)};

            Dune_RenderCopyF(renderer, ObjectSprite, &source, &dest);

            selectionDest = SDL_Rect{
                static_cast<int>(dest.x), static_cast<int>(dest.y), static_cast<int>(dest.w), static_cast<int>(dest.h)};
        }

        // draw selection frame
        if (!bCompleteMap && (std::ranges::find(selectedStructures, structure.id_) != selectedStructures.end())) {
            // now draw the selection box thing, with parts at all corners of structure

            DuneDrawSelectionBox(renderer, selectionDest);
        }
    }

    for (const Unit& unit : units_) {

        const Coord& position = unit.position_;

        static constexpr auto tankTurretOffset =
            std::to_array<Coord>({{0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}});

        static constexpr auto siegeTankTurretOffset =
            std::to_array<Coord>({{8, -12}, {0, -20}, {0, -20}, {-4, -20}, {-8, -12}, {-8, -4}, {-4, -12}, {8, -4}});

        static constexpr auto sonicTankTurretOffset =
            std::to_array<Coord>({{0, -8}, {0, -8}, {0, -8}, {0, -8}, {0, -8}, {0, -8}, {0, -8}, {0, -8}});

        static constexpr auto launcherTurretOffset =
            std::to_array<Coord>({{0, -12}, {0, -8}, {0, -8}, {0, -8}, {0, -12}, {0, -8}, {0, -8}, {0, -8}});

        static constexpr auto devastatorTurretOffset =
            std::to_array<Coord>({{8, -16}, {-4, -12}, {0, -16}, {4, -12}, {-8, -16}, {0, -12}, {-4, -12}, {0, -12}});

        ObjPic_enum objectPicBase{};
        auto framesX = NUM_ANGLES;
        auto framesY = 1;

        auto objectPicGun     = static_cast<ObjPic_enum>(-1);
        const auto* gunOffset = static_cast<decltype(tankTurretOffset)*>(nullptr);

        // clang-format off
        switch(unit.itemID_) {
            case Unit_Carryall:         objectPicBase = ObjPic_Carryall;        framesY = 2;                                                                    break;
            case Unit_Devastator:       objectPicBase = ObjPic_Devastator_Base; objectPicGun = ObjPic_Devastator_Gun;   gunOffset = &devastatorTurretOffset;    break;
            case Unit_Deviator:         objectPicBase = ObjPic_Tank_Base;       objectPicGun = ObjPic_Launcher_Gun;     gunOffset = &launcherTurretOffset;      break;
            case Unit_Frigate:          objectPicBase = ObjPic_Frigate;                                                                                         break;
            case Unit_Harvester:        objectPicBase = ObjPic_Harvester;                                                                                       break;
            case Unit_Soldier:          objectPicBase = ObjPic_Soldier;         framesX = 4;    framesY = 3;                                                    break;
            case Unit_Launcher:         objectPicBase = ObjPic_Tank_Base;       objectPicGun = ObjPic_Launcher_Gun;     gunOffset = &launcherTurretOffset;      break;
            case Unit_MCV:              objectPicBase = ObjPic_MCV;                                                                                             break;
            case Unit_Ornithopter:      objectPicBase = ObjPic_Ornithopter;     framesY = 3;                                                                    break;
            case Unit_Quad:             objectPicBase = ObjPic_Quad;                                                                                            break;
            case Unit_Saboteur:         objectPicBase = ObjPic_Saboteur;        framesX = 4;    framesY = 3;                                                    break;
            case Unit_Sandworm:         objectPicBase = ObjPic_Sandworm;        framesX = 1;    framesY = 9;                                                    break;
            case Unit_SiegeTank:        objectPicBase = ObjPic_Siegetank_Base;  objectPicGun = ObjPic_Siegetank_Gun;    gunOffset = &siegeTankTurretOffset;     break;
            case Unit_SonicTank:        objectPicBase = ObjPic_Tank_Base;       objectPicGun = ObjPic_Sonictank_Gun;    gunOffset = &sonicTankTurretOffset;     break;
            case Unit_Tank:             objectPicBase = ObjPic_Tank_Base;       objectPicGun = ObjPic_Tank_Gun;         gunOffset = &tankTurretOffset;          break;
            case Unit_Trike:
            case Unit_RaiderTrike:      objectPicBase = ObjPic_Trike;                                                                                           break;
            case Unit_Trooper:          objectPicBase = ObjPic_Trooper;         framesX = 4;    framesY = 3;                                                    break;
            case Unit_Special:          objectPicBase = ObjPic_Devastator_Base; objectPicGun = ObjPic_Devastator_Gun;   gunOffset = &devastatorTurretOffset;    break;
            case Unit_Infantry:         objectPicBase = ObjPic_Infantry;         framesX = 4;    framesY = 4;                                                   break;
            case Unit_Troopers:         objectPicBase = ObjPic_Troopers;         framesX = 4;    framesY = 4;                                                   break;
            default: {
                assert(0);
            } break;
        }
        // clang-format on

        const auto* const pObjectSprite = gfx->getZoomedObjPic(objectPicBase, unit.house_, zoom);

        const auto angle_int = static_cast<int>(unit.angle_);
        int angle_           = angle_int / (NUM_ANGLES / framesX);

        int frame = (unit.itemID_ == Unit_Sandworm) ? 5 : 0;

        auto source = calcSpriteSourceRect(pObjectSprite, angle_, framesX, frame, framesY);

        int frameSizeX = source.w;
        int frameSizeY = source.h;

        const auto drawLocation =
            calcSpriteDrawingRectF(pObjectSprite,
                                   pScreenborder->world2screenX((position.x * TILESIZE) + (TILESIZE / 2)),
                                   pScreenborder->world2screenY((position.y * TILESIZE) + (TILESIZE / 2)),
                                   framesX,
                                   framesY,
                                   HAlign::Center,
                                   VAlign::Center);

        Dune_RenderCopyF(renderer, pObjectSprite, &source, &drawLocation);

        if (objectPicGun >= 0) {
            const auto* const pGunSprite = gfx->getZoomedObjPic(objectPicGun, unit.house_, zoom);

            auto source2 = calcSpriteSourceRect(pGunSprite, angle_int, NUM_ANGLES);

            const auto& gun = (*gunOffset)[angle_int];
            const auto sx   = pScreenborder->world2screenX((position.x * TILESIZE) + (TILESIZE / 2) + gun.x);
            const auto sy   = pScreenborder->world2screenY((position.y * TILESIZE) + (TILESIZE / 2) + gun.y);

            const auto drawLocation2 =
                calcSpriteDrawingRectF(pGunSprite, sx, sy, NUM_ANGLES, 1, HAlign::Center, VAlign::Center);

            Dune_RenderCopyF(renderer, pGunSprite, &source2, &drawLocation2);
        }

        if (unit.itemID_ == Unit_RaiderTrike || unit.itemID_ == Unit_Deviator || unit.itemID_ == Unit_Special) {
            const auto* const pStarSprite = gfx->getZoomedObjPic(ObjPic_Star, zoom);

            auto drawLocation2 = calcDrawingRect(
                pStarSprite,
                pScreenborder->world2screenX((position.x * TILESIZE) + (TILESIZE / 2)) + frameSizeX / 2 - 1,
                pScreenborder->world2screenY((position.y * TILESIZE) + (TILESIZE / 2)) + frameSizeY / 2 - 1,
                HAlign::Right,
                VAlign::Bottom);

            pStarSprite->draw(renderer, drawLocation2.x, drawLocation2.y);
        }
    }

    // draw tactical pos rectangle (the starting screen)
    if (!bCompleteMap && getMapVersion() < 2 && mapInfo_.tacticalPos.isValid()) {

        SDL_FRect dest;
        dest.x = pScreenborder->world2screenX(mapInfo_.tacticalPos.x * TILESIZE);
        dest.y = pScreenborder->world2screenY(mapInfo_.tacticalPos.y * TILESIZE);
        dest.w = world2zoomedWorld(15 * TILESIZE);
        dest.h = world2zoomedWorld(10 * TILESIZE);

        renderDrawRectF(renderer, &dest, COLOR_DARKGREY);
    }

    const DuneTexture* validPlace   = nullptr;
    const DuneTexture* invalidPlace = nullptr;
    const DuneTexture* greyPlace    = nullptr;

    switch (zoom) {
        case 0: {
            validPlace   = gfx->getUIGraphic(UI_ValidPlace_Zoomlevel0);
            invalidPlace = gfx->getUIGraphic(UI_InvalidPlace_Zoomlevel0);
            greyPlace    = gfx->getUIGraphic(UI_GreyPlace_Zoomlevel0);
        } break;

        case 1: {
            validPlace   = gfx->getUIGraphic(UI_ValidPlace_Zoomlevel1);
            invalidPlace = gfx->getUIGraphic(UI_InvalidPlace_Zoomlevel1);
            greyPlace    = gfx->getUIGraphic(UI_GreyPlace_Zoomlevel1);
        } break;

        case 2:
        default: {
            validPlace   = gfx->getUIGraphic(UI_ValidPlace_Zoomlevel2);
            invalidPlace = gfx->getUIGraphic(UI_InvalidPlace_Zoomlevel2);
            greyPlace    = gfx->getUIGraphic(UI_GreyPlace_Zoomlevel2);
        } break;
    }

    if (!bCompleteMap && !pInterface_->hasChildWindow()
        && pScreenborder->isScreenCoordInsideMap(dune::globals::drawnMouseX, dune::globals::drawnMouseY)) {

        int xPos = pScreenborder->screen2MapX(dune::globals::drawnMouseX);
        int yPos = pScreenborder->screen2MapY(dune::globals::drawnMouseY);

        if (currentEditorMode_.mode_ == EditorMode::EditorMode_Terrain) {

            int halfsize = currentEditorMode_.pen_size_ / 2;

            for (int m = 0; m < mapMirror_->getSize(); m++) {

                Coord position = mapMirror_->getCoord(Coord(xPos, yPos), m);

                SDL_Rect dest;
                dest.x = pScreenborder->world2screenX((position.x - halfsize) * TILESIZE);
                dest.y = pScreenborder->world2screenY((position.y - halfsize) * TILESIZE);
                dest.w = world2zoomedWorld(currentEditorMode_.pen_size_ * TILESIZE);
                dest.h = world2zoomedWorld(currentEditorMode_.pen_size_ * TILESIZE);

                DuneDrawSelectionBox(renderer, dest);
            }

        } else if (currentEditorMode_.mode_ == EditorMode::EditorMode_Structure) {
            Coord structureSize = getStructureSize(currentEditorMode_.itemID_);

            for (int m = 0; m < mapMirror_->getSize(); m++) {

                Coord position = mapMirror_->getCoord(Coord(xPos, yPos), m, structureSize);

                for (int x = position.x; x < (position.x + structureSize.x); x++) {
                    for (int y = position.y; y < (position.y + structureSize.y); y++) {
                        const auto* image = validPlace;

                        // check if mirroring of the original (!) position is possible
                        if (!mapMirror_->mirroringPossible(Coord(xPos, yPos), structureSize)) {
                            image = invalidPlace;
                        }

                        // check all mirrored places
                        for (int k = 0; k < mapMirror_->getSize(); k++) {
                            Coord pos = mapMirror_->getCoord(Coord(x, y), k);

                            if (!map_.isInsideMap(pos.x, pos.y)
                                || isTileBlocked(pos.x, pos.y, true, (currentEditorMode_.itemID_ != Structure_Slab1))) {
                                image = invalidPlace;
                            } else if ((image != invalidPlace) && (map_(pos.x, pos.y) != TERRAINTYPE::Terrain_Rock)) {
                                image = greyPlace;
                            }
                        }

                        SDL_FRect drawLocation{pScreenborder->world2screenX(x * TILESIZE),
                                               pScreenborder->world2screenY(y * TILESIZE),
                                               static_cast<float>(zoomedTilesize),
                                               static_cast<float>(zoomedTilesize)};
                        Dune_RenderCopyF(renderer, image, nullptr, &drawLocation);
                    }
                }
            }
        } else if (currentEditorMode_.mode_ == EditorMode::EditorMode_Unit) {
            for (int m = 0; m < mapMirror_->getSize(); m++) {

                Coord position = mapMirror_->getCoord(Coord(xPos, yPos), m);

                const auto* image = validPlace;
                // check all mirrored places
                for (int k = 0; k < mapMirror_->getSize(); k++) {
                    Coord pos = mapMirror_->getCoord(position, k);

                    if (!map_.isInsideMap(pos.x, pos.y) || isTileBlocked(pos.x, pos.y, false, true)) {
                        image = invalidPlace;
                    }
                }
                auto drawLocation = calcDrawingRect(image,
                                                    pScreenborder->world2screenX(position.x * TILESIZE),
                                                    pScreenborder->world2screenY(position.y * TILESIZE));
                Dune_RenderCopyF(renderer, image, nullptr, &drawLocation);
            }
        } else if (currentEditorMode_.mode_ == EditorMode::EditorMode_TacticalPos) {
            // draw tactical pos rectangle (the starting screen)
            if (mapInfo_.tacticalPos.isValid()) {

                SDL_FRect dest = {pScreenborder->world2screenX(xPos * TILESIZE),
                                  pScreenborder->world2screenY(yPos * TILESIZE),
                                  static_cast<float>(world2zoomedWorld(15 * TILESIZE)),
                                  static_cast<float>(world2zoomedWorld(10 * TILESIZE))};
                renderDrawRectF(renderer, &dest, COLOR_WHITE);
            }
        }
    }

    // draw selection rect for units (selection rect for structures_ is already drawn)
    if (!bCompleteMap) {
        std::vector<int> selectedUnits = getMirrorUnits(selectedUnitID_);

        for (const Unit& unit : units_) {
            if (std::ranges::find(selectedUnits, unit.id_) != selectedUnits.end()) {
                const Coord& position = unit.position_;

                const DuneTexture* selectionBox = nullptr;

                switch (dune::globals::currentZoomlevel) {
                    case 0: selectionBox = gfx->getUIGraphic(UI_SelectionBox_Zoomlevel0); break;
                    case 1: selectionBox = gfx->getUIGraphic(UI_SelectionBox_Zoomlevel1); break;
                    case 2:
                    default: selectionBox = gfx->getUIGraphic(UI_SelectionBox_Zoomlevel2); break;
                }

                auto dest = calcDrawingRect(selectionBox,
                                            pScreenborder->world2screenX((position.x * TILESIZE) + (TILESIZE / 2)),
                                            pScreenborder->world2screenY((position.y * TILESIZE) + (TILESIZE / 2)),
                                            HAlign::Center,
                                            VAlign::Center);

                Dune_RenderCopyF(renderer, selectionBox, nullptr, &dest);
            }
        }
    }

    // draw selection rect for map items (spice bloom, special bloom or spice field)
    if (!bCompleteMap && selectedMapItemCoord_.isValid()
        && ((std::ranges::find(spiceBlooms_, selectedMapItemCoord_) != spiceBlooms_.end())
            || (std::ranges::find(specialBlooms_, selectedMapItemCoord_) != specialBlooms_.end())
            || (std::ranges::find(spiceFields_, selectedMapItemCoord_) != spiceFields_.end()))) {

        const DuneTexture* selectionBox = nullptr;

        switch (zoom) {
            case 0: selectionBox = gfx->getUIGraphic(UI_SelectionBox_Zoomlevel0); break;
            case 1: selectionBox = gfx->getUIGraphic(UI_SelectionBox_Zoomlevel1); break;
            case 2:
            default: selectionBox = gfx->getUIGraphic(UI_SelectionBox_Zoomlevel2); break;
        }

        auto dest = calcDrawingRect(selectionBox,
                                    pScreenborder->world2screenX((selectedMapItemCoord_.x * TILESIZE) + (TILESIZE / 2)),
                                    pScreenborder->world2screenY((selectedMapItemCoord_.y * TILESIZE) + (TILESIZE / 2)),
                                    HAlign::Center,
                                    VAlign::Center);

        Dune_RenderCopyF(renderer, selectionBox, nullptr, &dest);
    }
}

void MapEditor::saveMapshot() {
    const int oldCurrentZoomlevel = dune::globals::currentZoomlevel;

    dune::globals::currentZoomlevel = 0;

    auto mapshotFilename =
        lastSaveName_.empty() ? std::filesystem::path{generateMapname()} : getBasename(lastSaveName_, true);

    mapshotFilename += ".png";

    const auto sizeX = world2zoomedWorld(map_.getSizeX() * TILESIZE);
    const auto sizeY = world2zoomedWorld(map_.getSizeY() * TILESIZE);

    const SDL_FRect board{0, 0, static_cast<float>(sizeX), static_cast<float>(sizeY)};

    ScreenBorder tmpScreenborder(board);
    tmpScreenborder.adjustScreenBorderToMapsize(map_.getSizeX(), map_.getSizeY());

    auto* const renderer = dune::globals::renderer.get();

    const auto renderTarget =
        sdl2::texture_ptr{SDL_CreateTexture(renderer, SCREEN_FORMAT, SDL_TEXTUREACCESS_TARGET, sizeX, sizeY)};
    if (renderTarget == nullptr) {
        sdl2::log_info("SDL_CreateTexture() failed: {}", SDL_GetError());
        dune::globals::currentZoomlevel = oldCurrentZoomlevel;
        return;
    }

    auto* const oldRenderTarget = SDL_GetRenderTarget(renderer);
    if (SDL_SetRenderTarget(renderer, renderTarget.get()) != 0) {
        sdl2::log_info("SDL_SetRenderTarget() failed: {}", SDL_GetError());
        SDL_SetRenderTarget(renderer, oldRenderTarget);
        dune::globals::currentZoomlevel = oldCurrentZoomlevel;
        return;
    }

    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);

    drawMap(&tmpScreenborder, true);

    const auto pMapshotSurface = renderReadSurface(renderer);
    SavePNG(pMapshotSurface.get(), mapshotFilename.u8string().c_str());

    SDL_SetRenderTarget(renderer, oldRenderTarget);

    dune::globals::currentZoomlevel = oldCurrentZoomlevel;
}
