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

#include <sand.h>
#include <engine/engine_sand.h>


#include <globals.h>

#include <FileClasses/GFXManager.h>
#include <FileClasses/TextManager.h>
#include <FileClasses/music/MusicPlayer.h>

#include <Menu/BriefingMenu.h>
#include <Menu/CampaignStatsMenu.h>
#include <Menu/CustomGameStatsMenu.h>

#include <CutScenes/Meanwhile.h>
#include <CutScenes/Finale.h>

#include <Game.h>
#include <engine/GameInitSettings.h>
#include <engine/data.h>

#include <misc/exceptions.h>

/**
    This function draws the cursor to the screen. The coordinate is read from
    the two global variables drawnMouseX and drawnMouseY.
*/
void drawCursor() {
    if(!(SDL_GetWindowFlags(window) & SDL_WINDOW_MOUSE_FOCUS)) {
        return;
    }

    const auto* const tex = pGFXManager->getUIGraphic(cursorFrame);

    auto dest = calcDrawingRect(tex, drawnMouseX, drawnMouseY);

    //reposition image so pointing on right spot

    if (cursorFrame == UI_CursorRight) {
        dest.x -= dest.w/2;
    } else if (cursorFrame == UI_CursorDown) {
        dest.y -= dest.h/2;
    }

    if ((cursorFrame == UI_CursorAttack_Zoomlevel0) || (cursorFrame == UI_CursorMove_Zoomlevel0)) {
        dest.x -= dest.w/2;
        dest.y -= dest.h/2;
    }

    tex->draw(renderer, dest.x, dest.y);
}

/**
    This function resolves the picture corresponding to one item id.
    \param itemID   the id of the item to resolve (e.g. Unit_Quad)
    \return the surface corresponding. This surface should not be freed or modified. nullptr on error.
*/
const DuneTexture* resolveItemPicture(ItemID_enum itemID, HOUSETYPE house) {
    int newPicID = 0;

    // clang-format off
    switch(itemID) {
        case Structure_Barracks:            newPicID = Picture_Barracks;            break;
        case Structure_ConstructionYard:    newPicID = Picture_ConstructionYard;    break;
        case Structure_GunTurret:           newPicID = Picture_GunTurret;           break;
        case Structure_HeavyFactory:        newPicID = Picture_HeavyFactory;        break;
        case Structure_HighTechFactory:     newPicID = Picture_HighTechFactory;     break;
        case Structure_IX:                  newPicID = Picture_IX;                  break;
        case Structure_LightFactory:        newPicID = Picture_LightFactory;        break;
        case Structure_Palace:              newPicID = Picture_Palace;              break;
        case Structure_Radar:               newPicID = Picture_Radar;               break;
        case Structure_Refinery:            newPicID = Picture_Refinery;            break;
        case Structure_RepairYard:          newPicID = Picture_RepairYard;          break;
        case Structure_RocketTurret:        newPicID = Picture_RocketTurret;        break;
        case Structure_Silo:                newPicID = Picture_Silo;                break;
        case Structure_Slab1:               newPicID = Picture_Slab1;               break;
        case Structure_Slab4:               newPicID = Picture_Slab4;               break;
        case Structure_StarPort:            newPicID = Picture_StarPort;            break;
        case Structure_Wall:                newPicID = Picture_Wall;                break;
        case Structure_WindTrap:            newPicID = Picture_WindTrap;            break;
        case Structure_WOR:                 newPicID = Picture_WOR;                 break;

        case Unit_Carryall:                 newPicID = Picture_Carryall;            break;
        case Unit_Devastator:               newPicID = Picture_Devastator;          break;
        case Unit_Deviator:                 newPicID = Picture_Deviator;            break;
        case Unit_Frigate:                  newPicID = Picture_Frigate;             break;
        case Unit_Harvester:                newPicID = Picture_Harvester;           break;
        case Unit_Launcher:                 newPicID = Picture_Launcher;            break;
        case Unit_MCV:                      newPicID = Picture_MCV;                 break;
        case Unit_Ornithopter:              newPicID = Picture_Ornithopter;         break;
        case Unit_Quad:                     newPicID = Picture_Quad;                break;
        case Unit_RaiderTrike:              newPicID = Picture_RaiderTrike;         break;
        case Unit_SiegeTank:                newPicID = Picture_SiegeTank;           break;
        case Unit_SonicTank:                newPicID = Picture_SonicTank;           break;
        case Unit_Tank:                     newPicID = Picture_Tank;                break;
        case Unit_Trike:                    newPicID = Picture_Trike;               break;
        case Unit_Saboteur:                 newPicID = Picture_Saboteur;            break;
        case Unit_Sandworm:                 newPicID = Picture_Sandworm;            break;
        case Unit_Soldier:                  newPicID = Picture_Soldier;             break;
        case Unit_Trooper: {
            switch(house) {
                case HOUSETYPE::HOUSE_SARDAUKAR: newPicID = Picture_Sardaukar; break;
                case HOUSETYPE::HOUSE_FREMEN: newPicID = Picture_Fremen; break;
                default:                    newPicID = Picture_Trooper;             break;
            }
        } break;
        case Unit_Special:                  newPicID = Picture_Special;             break;
        case Unit_Infantry:                 newPicID = Picture_Soldier;             break;
        case Unit_Troopers: {
            switch(house) {
                case HOUSETYPE::HOUSE_SARDAUKAR: newPicID = Picture_Sardaukar; break;
                case HOUSETYPE::HOUSE_FREMEN: newPicID = Picture_Fremen; break;
                default:                    newPicID = Picture_Trooper;             break;
            }
        } break;

        default:
            THROW(std::invalid_argument, "resolveItemPicture(): Invalid item ID " + std::to_string(itemID) + "!");
    }
    // clang-format on

    return pGFXManager->getSmallDetailPic(newPicID);
}


/**
    This function returns the anim id based on the passed filename.
    \param  filename    the filename (e.g. STARPORT.WSA)
    \return the id of the animation (e.g. Anim_StarPort)
*/
int getAnimByFilename(const std::string& filename) {
    const auto lowerFilename = strToLower(filename);

    // clang-format off
    if(lowerFilename == "fartr.wsa")       return Anim_AtreidesPlanet;
    if(lowerFilename == "fhark.wsa")       return Anim_HarkonnenPlanet;
    if(lowerFilename == "fordos.wsa")      return Anim_OrdosPlanet;
    if(lowerFilename == "win1.wsa")        return Anim_Win1;
    if(lowerFilename == "win2.wsa")        return Anim_Win2;
    if(lowerFilename == "lostbild.wsa")    return Anim_Lose1;
    if(lowerFilename == "lostvehc.wsa")    return Anim_Lose2;
    if(lowerFilename == "barrac.wsa")      return Anim_Barracks;
    if(lowerFilename == "carryall.wsa")    return Anim_Carryall;
    if(lowerFilename == "construc.wsa")    return Anim_ConstructionYard;
    if(lowerFilename == "fremen.wsa")      return Anim_Fremen;
    if(lowerFilename == "gold-bb.wsa")     return Anim_DeathHand;
    if(lowerFilename == "harktank.wsa")    return Anim_Devastator;
    if(lowerFilename == "harvest.wsa")     return Anim_Harvester;
    if(lowerFilename == "headqrts.wsa")    return Anim_Radar;
    if(lowerFilename == "hitcftry.wsa")    return Anim_HighTechFactory;
    if(lowerFilename == "htank.wsa")       return Anim_SiegeTank;
    if(lowerFilename == "hvyftry.wsa")     return Anim_HeavyFactory;
    if(lowerFilename == "hyinfy.wsa")      return Anim_Trooper;
    if(lowerFilename == "infantry.wsa")    return Anim_Infantry;
    if(lowerFilename == "ix.wsa")          return Anim_IX;
    if(lowerFilename == "liteftry.wsa")    return Anim_LightFactory;
    if(lowerFilename == "ltank.wsa")       return Anim_Tank;
    if(lowerFilename == "mcv.wsa")         return Anim_MCV;
    if(lowerFilename == "ordrtank.wsa")    return Anim_Deviator;
    if(lowerFilename == "orni.wsa")        return Anim_Ornithopter;
    if(lowerFilename == "otrike.wsa")      return Anim_Raider;
    if(lowerFilename == "palace.wsa")      return Anim_Palace;
    if(lowerFilename == "quad.wsa")        return Anim_Quad;
    if(lowerFilename == "refinery.wsa")    return Anim_Refinery;
    if(lowerFilename == "repair.wsa")      return Anim_RepairYard;
    if(lowerFilename == "rtank.wsa")       return Anim_Launcher;
    if(lowerFilename == "rturret.wsa")     return Anim_RocketTurret;
    if(lowerFilename == "saboture.wsa")    return Anim_Saboteur;
    if(lowerFilename == "slab.wsa")        return Anim_Slab1;
    if(lowerFilename == "stank.wsa")       return Anim_SonicTank;
    if(lowerFilename == "starport.wsa")    return Anim_StarPort;
    if(lowerFilename == "storage.wsa")     return Anim_Silo;
    if(lowerFilename == "trike.wsa")       return Anim_Trike;
    if(lowerFilename == "turret.wsa")      return Anim_GunTurret;
    if(lowerFilename == "wall.wsa")        return Anim_Wall;
    if(lowerFilename == "windtrap.wsa")    return Anim_WindTrap;
    if(lowerFilename == "wor.wsa")         return Anim_WOR;
    if(lowerFilename == "worm.wsa")        return Anim_Sandworm;
    if(lowerFilename == "sardukar.wsa")    return Anim_Sardaukar;
    if(lowerFilename == "frigate.wsa")     return Anim_Frigate;
    if(lowerFilename == "4slab.wsa")       return Anim_Slab4;
    // clang-format on
    return 0;
}


/**
    This function resolves the name corresponding to one item id.
    \param itemID   the id of the item to resolve (e.g. Unit_Quad)
    \return the string corresponding.
*/
std::string resolveItemName(ItemID_enum itemID) {
    // clang-format off
    switch(itemID) {
        case Structure_Barracks:            return _("@DUNE.ENG|253#Barracks");
        case Structure_ConstructionYard:    return _("@DUNE.ENG|249#Construction Yard");
        case Structure_GunTurret:           return _("@DUNE.ENG|263#Gun Turret");
        case Structure_HeavyFactory:        return _("@DUNE.ENG|241#Heacy Factory");
        case Structure_HighTechFactory:     return _("@DUNE.ENG|243#Hightech Factory");
        case Structure_IX:                  return _("@DUNE.ENG|245#House IX");
        case Structure_LightFactory:        return _("@DUNE.ENG|239#Light Factory");
        case Structure_Palace:              return _("@DUNE.ENG|237#Palace");
        case Structure_Radar:               return _("@DUNE.ENG|269#Radar");
        case Structure_Refinery:            return _("@DUNE.ENG|256#Refinery");
        case Structure_RepairYard:          return _("@DUNE.ENG|259#Repair Yard");
        case Structure_RocketTurret:        return _("@DUNE.ENG|265#Rocket Turret");
        case Structure_Silo:                return _("@DUNE.ENG|267#Silo");
        case Structure_Slab1:               return _("@DUNE.ENG|233#Slab");
        case Structure_Slab4:               return _("@DUNE.ENG|235#Slab (2x2)");
        case Structure_StarPort:            return _("@DUNE.ENG|255#Starport");
        case Structure_Wall:                return _("@DUNE.ENG|261#Wall");
        case Structure_WindTrap:            return _("@DUNE.ENG|251#Windtrap");
        case Structure_WOR:                 return _("@DUNE.ENG|247#WOR");

        case Unit_Carryall:                 return _("@DUNE.ENG|195#Carryall");
        case Unit_Devastator:               return _("@DUNE.ENG|217#Devastator");
        case Unit_Deviator:                 return _("@DUNE.ENG|211#Deviator");
        case Unit_Frigate:                  return _("Frigate");
        case Unit_Harvester:                return _("@DUNE.ENG|227#Harvester");
        case Unit_Launcher:                 return _("@DUNE.ENG|209#Launcher");
        case Unit_MCV:                      return _("@DUNE.ENG|229#MCV");
        case Unit_Ornithopter:              return _("@DUNE.ENG|197#Ornithopter");
        case Unit_Quad:                     return _("@DUNE.ENG|225#Quad");
        case Unit_RaiderTrike:              return _("@DUNE.ENG|223#Raider Trike");
        case Unit_SiegeTank:                return _("@DUNE.ENG|215#Siege Tank");
        case Unit_SonicTank:                return _("@DUNE.ENG|219#Sonic Tank");
        case Unit_Tank:                     return _("@DUNE.ENG|213#Tank");
        case Unit_Trike:                    return _("@DUNE.ENG|221#Trike");
        case Unit_Saboteur:                 return _("@DUNE.ENG|207#Saboteur");
        case Unit_Sandworm:                 return _("@DUNE.ENG|231#Sandworm");
        case Unit_Soldier:                  return _("@DUNE.ENG|203#Soldier");
        case Unit_Trooper:                  return _("@DUNE.ENG|205#Trooper");
        case Unit_Special:                  return _("Sonic/Devast./Devia.");
        case Unit_Infantry:                 return _("@DUNE.ENG|199#Infantry");
        case Unit_Troopers:                 return _("@DUNE.ENG|201#Troopers");

        default:
            THROW(std::invalid_argument, "resolveItemName(): Invalid item ID!");
    }
    // clang-format on
}




Uint32 getColorByTerrainType(TERRAINTYPE terrainType) {
    auto color = COLOR_BLACK;
    // clang-format off
    switch (terrainType) {
        case TERRAINTYPE::Terrain_Dunes:         color = COLOR_DESERTSAND;  break;
        case TERRAINTYPE::Terrain_Mountain:      color = COLOR_MOUNTAIN;    break;
        case TERRAINTYPE::Terrain_Rock:          color = COLOR_ROCK;        break;
        case TERRAINTYPE::Terrain_Sand:          color = COLOR_DESERTSAND;  break;
        case TERRAINTYPE::Terrain_Spice:         color = COLOR_SPICE;       break;
        case TERRAINTYPE::Terrain_ThickSpice:    color = COLOR_THICKSPICE;  break;
        case TERRAINTYPE::Terrain_SpiceBloom:    color = COLOR_BLOOM;       break;
        case TERRAINTYPE::Terrain_SpecialBloom:  color = COLOR_BLOOM;       break;
        case TERRAINTYPE::Terrain_Slab:          color = COLOR_ROCK;        break;
        default:                                 color = COLOR_ROCK;        break;
    }
    // clang-format on

    return color;
}


std::string resolveDropLocationName(DropLocation dropLocation) {
    // clang-format off
    switch(dropLocation) {
        case DropLocation::Drop_North:     return _("top edge");
        case DropLocation::Drop_East:      return _("right edge");
        case DropLocation::Drop_South:     return _("bottom edge");
        case DropLocation::Drop_West:      return _("left edge");
        case DropLocation::Drop_Air:       return _("random");
        case DropLocation::Drop_Visible:   return _("middle");
        case DropLocation::Drop_Enemybase: return _("enemy base");
        case DropLocation::Drop_Homebase:  return _("home base");
        default:
            THROW(std::invalid_argument, "resolveDropLocationName(): Invalid drop location!");
    }
    // clang-format on
}


/**
    Starts a game replay
    \param  filename    the filename of the replay file
*/
void startReplay(const std::filesystem::path& filename) {
    sdl2::log_info("Initializing replay...");

    auto cleanup = gsl::finally([&] { currentGame.reset(); });

    currentGame = std::make_unique<Game>();

    currentGame->initReplay(filename);

    const GameContext context{*currentGame, *currentGame->getMap(), currentGame->getObjectManager()};
    currentGame->runMainLoop(context);

    currentGame.reset();

    // Change music to menu music
    musicPlayer->changeMusic(MUSIC_MENU);
}


/**
    Starts a new game. If this game is quit it might start another game. This other game is also started from
    this function. This is done until there is no more game to be started.
    \param init contains all the information to start the game
*/
void startSinglePlayerGame(const Dune::Engine::GameInitSettings& init) {
    auto currentGameInitInfo = init;

    auto cleanup = gsl::finally([&] { currentGame.reset(); });

    while(true) {
        // Make sure to delete the old game (if any) before creating a new one since
        // its destructor has global side effects.  If we let std::unique_ptr<> handle
        // this responsibility, then the new Game instance will be constructed before the
        // assignment to currentGame causes the old one to be deleted.
        currentGame.reset();

        sdl2::log_info("Initializing game...");

        currentGame = std::make_unique<Game>();
        currentGame->initGame(currentGameInitInfo);

        // get init settings from game as it might have changed (through loading the game)
        currentGameInitInfo = currentGame->getGameInitSettings();

        GameContext context{*currentGame, *currentGame->getMap(), currentGame->getObjectManager()};
        context.game.runMainLoop(context);

        sdl2::log_info("Game completed after %.1f seconds", currentGame->getGameTime() * (1.0 / 1000));

        bool bGetNext = true;
        while(bGetNext) {
            switch(context.game.whatNext()) {
                case GAME_DEBRIEFING_WIN: {
                    sdl2::log_info("Debriefing...");
                    { // Scope
                        BriefingMenu briefing{currentGameInitInfo.getHouseID(), currentGameInitInfo.getMission(), DEBRIEFING_WIN};
                        briefing.showMenu();
                    }

                    sdl2::log_info("Game statistics...");
                    { // Scope
                        CampaignStatsMenu campaignStats(missionNumberToLevelNumber(currentGameInitInfo.getMission()));
                        campaignStats.showMenu();
                    }

                    const auto houseID = currentGameInitInfo.getHouseID();

                    if(currentGameInitInfo.getGameType() == GameType::Campaign) {
                        const int level = missionNumberToLevelNumber(currentGameInitInfo.getMission());

                        if(level == 4 && (houseID == HOUSETYPE::HOUSE_HARKONNEN ||
                                          houseID == HOUSETYPE::HOUSE_ATREIDES || houseID == HOUSETYPE::HOUSE_ORDOS)) {
                            sdl2::log_info("Playing meanwhile...");
                            Meanwhile meanwhile(houseID, true);
                            meanwhile.run();
                        } else if(level == 8 &&
                                  (houseID == HOUSETYPE::HOUSE_HARKONNEN || houseID == HOUSETYPE::HOUSE_ATREIDES ||
                                   houseID == HOUSETYPE::HOUSE_ORDOS)) {
                            sdl2::log_info("Playing meanwhile...");
                            Meanwhile meanwhile(houseID, false);
                            meanwhile.run();
                        } else if(level == 9) {
                            sdl2::log_info("Playing finale.....");
                            Finale finale(houseID);
                            finale.run();
                        }
                    }
                } break;

                case GAME_DEBRIEFING_LOST: {
                    sdl2::log_info("Debriefing...");
                    BriefingMenu briefing{currentGameInitInfo.getHouseID(), currentGameInitInfo.getMission(),
                                          DEBRIEFING_LOST};
                    briefing.showMenu();
                } break;

                case GAME_CUSTOM_GAME_STATS: {
                    sdl2::log_info("Game statistics...");
                    CustomGameStatsMenu stats;
                    stats.showMenu();
                } break;

                case GAME_LOAD:
                case GAME_NEXTMISSION: {
                    currentGameInitInfo = currentGame->getNextGameInitSettings();
                    bGetNext            = false;
                } break;

                case GAME_RETURN_TO_MENU:
                default: {
                    // Change music to menu music
                    musicPlayer->changeMusic(MUSIC_MENU);

                    return;
                }
            }
        }
    }
}

/**
    Starts a new multiplayer game.
    \param init contains all the information to start the game
*/
void startMultiPlayerGame(const Dune::Engine::GameInitSettings& init) {
    auto currentGameInitInfo = init;

    sdl2::log_info("Initializing game...");
    currentGame = std::make_unique<Game>();

    auto cleanup = gsl::finally([&] { currentGame.reset(); });

    currentGame->initGame(currentGameInitInfo);

    // get init settings from game as it might have changed (through loading the game)
    currentGameInitInfo = currentGame->getGameInitSettings();

    const GameContext context{*currentGame, *currentGame->getMap(), currentGame->getObjectManager()};
    currentGame->runMainLoop(context);

    if(currentGame->whatNext() == GAME_CUSTOM_GAME_STATS) {
        sdl2::log_info("Game statistics...");
        CustomGameStatsMenu stats;
        stats.showMenu();
    }
}
