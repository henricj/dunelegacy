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
#include <GameInitSettings.h>
#include <data.h>

#include <misc/exceptions.h>

#include <algorithm>

/**
    This function draws the cursor to the screen. The coordinate is read from
    the two global variables drawnMouseX and drawnMouseY.
*/
void drawCursor() {
    if(!(SDL_GetWindowFlags(window) & SDL_WINDOW_MOUSE_FOCUS)) {
        return;
    }

    SDL_Texture* tex = pGFXManager->getUIGraphic(cursorFrame);

    SDL_Rect dest = calcDrawingRect(tex, drawnMouseX, drawnMouseY);

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

    SDL_RenderCopy(renderer, tex, nullptr, &dest);
}

/**
    This function resolves the picture corresponding to one item id.
    \param itemID   the id of the item to resolve (e.g. Unit_Quad)
    \return the surface corresponding. This surface should not be freed or modified. nullptr on error.
*/
SDL_Texture* resolveItemPicture(int itemID, HOUSETYPE house) {
    int newPicID;

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
                case HOUSE_SARDAUKAR:       newPicID = Picture_Sardaukar;           break;
                case HOUSE_FREMEN:          newPicID = Picture_Fremen;              break;
                default:                    newPicID = Picture_Trooper;             break;
            }
        } break;
        case Unit_Special:                  newPicID = Picture_Special;             break;
        case Unit_Infantry:                 newPicID = Picture_Soldier;             break;
        case Unit_Troopers: {
            switch(house) {
                case HOUSE_SARDAUKAR:       newPicID = Picture_Sardaukar;           break;
                case HOUSE_FREMEN:          newPicID = Picture_Fremen;              break;
                default:                    newPicID = Picture_Trooper;             break;
            }
        } break;

        default:
            THROW(std::invalid_argument, "resolveItemPicture(): Invalid item ID " + std::to_string(itemID) + "!");
        break;
    }

    return pGFXManager->getSmallDetailPic(newPicID);
}


/**
    This function returns the anim id based on the passed filename.
    \param  filename    the filename (e.g. STARPORT.WSA)
    \return the id of the animation (e.g. Anim_StarPort)
*/
int getAnimByFilename(const std::string& filename) {
    const std::string lowerFilename = strToLower(filename);

    if(lowerFilename == "fartr.wsa")            return Anim_AtreidesPlanet;
    else if(lowerFilename == "fhark.wsa")       return Anim_HarkonnenPlanet;
    else if(lowerFilename == "fordos.wsa")      return Anim_OrdosPlanet;
    else if(lowerFilename == "win1.wsa")        return Anim_Win1;
    else if(lowerFilename == "win2.wsa")        return Anim_Win2;
    else if(lowerFilename == "lostbild.wsa")    return Anim_Lose1;
    else if(lowerFilename == "lostvehc.wsa")    return Anim_Lose2;
    else if(lowerFilename == "barrac.wsa")      return Anim_Barracks;
    else if(lowerFilename == "carryall.wsa")    return Anim_Carryall;
    else if(lowerFilename == "construc.wsa")    return Anim_ConstructionYard;
    else if(lowerFilename == "fremen.wsa")      return Anim_Fremen;
    else if(lowerFilename == "gold-bb.wsa")     return Anim_DeathHand;
    else if(lowerFilename == "harktank.wsa")    return Anim_Devastator;
    else if(lowerFilename == "harvest.wsa")     return Anim_Harvester;
    else if(lowerFilename == "headqrts.wsa")    return Anim_Radar;
    else if(lowerFilename == "hitcftry.wsa")    return Anim_HighTechFactory;
    else if(lowerFilename == "htank.wsa")       return Anim_SiegeTank;
    else if(lowerFilename == "hvyftry.wsa")     return Anim_HeavyFactory;
    else if(lowerFilename == "hyinfy.wsa")      return Anim_Trooper;
    else if(lowerFilename == "infantry.wsa")    return Anim_Infantry;
    else if(lowerFilename == "ix.wsa")          return Anim_IX;
    else if(lowerFilename == "liteftry.wsa")    return Anim_LightFactory;
    else if(lowerFilename == "ltank.wsa")       return Anim_Tank;
    else if(lowerFilename == "mcv.wsa")         return Anim_MCV;
    else if(lowerFilename == "ordrtank.wsa")    return Anim_Deviator;
    else if(lowerFilename == "orni.wsa")        return Anim_Ornithopter;
    else if(lowerFilename == "otrike.wsa")      return Anim_Raider;
    else if(lowerFilename == "palace.wsa")      return Anim_Palace;
    else if(lowerFilename == "quad.wsa")        return Anim_Quad;
    else if(lowerFilename == "refinery.wsa")    return Anim_Refinery;
    else if(lowerFilename == "repair.wsa")      return Anim_RepairYard;
    else if(lowerFilename == "rtank.wsa")       return Anim_Launcher;
    else if(lowerFilename == "rturret.wsa")     return Anim_RocketTurret;
    else if(lowerFilename == "saboture.wsa")    return Anim_Saboteur;
    else if(lowerFilename == "slab.wsa")        return Anim_Slab1;
    else if(lowerFilename == "stank.wsa")       return Anim_SonicTank;
    else if(lowerFilename == "starport.wsa")    return Anim_StarPort;
    else if(lowerFilename == "storage.wsa")     return Anim_Silo;
    else if(lowerFilename == "trike.wsa")       return Anim_Trike;
    else if(lowerFilename == "turret.wsa")      return Anim_GunTurret;
    else if(lowerFilename == "wall.wsa")        return Anim_Wall;
    else if(lowerFilename == "windtrap.wsa")    return Anim_WindTrap;
    else if(lowerFilename == "wor.wsa")         return Anim_WOR;
    else if(lowerFilename == "worm.wsa")        return Anim_Sandworm;
    else if(lowerFilename == "sardukar.wsa")    return Anim_Sardaukar;
    else if(lowerFilename == "frigate.wsa")     return Anim_Frigate;
    else if(lowerFilename == "4slab.wsa")       return Anim_Slab4;
    else                                        return 0;
}

/**
    This function returns the size of the specified item.
    \param ItemID   the id of the item (e.g. Structure_HeavyFactory)
    \return a Coord containg the size (e.g. (3,2) ). Returns (0,0) on error.
*/
Coord getStructureSize(int itemID) {

    switch(itemID) {
        case Structure_Barracks:            return Coord(2,2); break;
        case Structure_ConstructionYard:    return Coord(2,2); break;
        case Structure_GunTurret:           return Coord(1,1); break;
        case Structure_HeavyFactory:        return Coord(3,2); break;
        case Structure_HighTechFactory:     return Coord(3,2); break;
        case Structure_IX:                  return Coord(2,2); break;
        case Structure_LightFactory:        return Coord(2,2); break;
        case Structure_Palace:              return Coord(3,3); break;
        case Structure_Radar:               return Coord(2,2); break;
        case Structure_Refinery:            return Coord(3,2); break;
        case Structure_RepairYard:          return Coord(3,2); break;
        case Structure_RocketTurret:        return Coord(1,1); break;
        case Structure_Silo:                return Coord(2,2); break;
        case Structure_StarPort:            return Coord(3,3); break;
        case Structure_Slab1:               return Coord(1,1); break;
        case Structure_Slab4:               return Coord(2,2); break;
        case Structure_Wall:                return Coord(1,1); break;
        case Structure_WindTrap:            return Coord(2,2); break;
        case Structure_WOR:                 return Coord(2,2); break;
        default:                            return Coord(0,0); break;
    }
}

/**
    This function return the item id of an item specified by name. There may be multiple names for
    one item. The case of the name is ignored.
    \param name the name of the item (e.g. "rocket-turret" or "r-turret".
    \return the id of the item (e.g. Structure_RocketTurret)
*/
Uint32  getItemIDByName(const std::string& name) {
    const std::string lowerName = strToLower(name);

    if(lowerName == "barracks")                                                 return Structure_Barracks;
    else if((lowerName == "const yard") || (lowerName == "construction yard"))  return Structure_ConstructionYard;
    else if((lowerName == "r-turret") || (lowerName == "rocket-turret"))        return Structure_RocketTurret;
    else if((lowerName == "turret") || (lowerName == "gun-turret"))             return Structure_GunTurret;
    else if((lowerName == "heavy fctry") || (lowerName == "heavy factory"))     return Structure_HeavyFactory;
    else if((lowerName == "hi-tech") || (lowerName == "hightech factory"))      return Structure_HighTechFactory;
    else if((lowerName == "ix") || (lowerName == "house ix"))                   return Structure_IX;
    else if((lowerName == "light fctry") || (lowerName == "light factory"))     return Structure_LightFactory;
    else if(lowerName == "palace")                                              return Structure_Palace;
    else if((lowerName == "outpost") || (lowerName == "radar"))                 return Structure_Radar;
    else if(lowerName == "refinery")                                            return Structure_Refinery;
    else if((lowerName == "repair") || (lowerName == "repair yard"))            return Structure_RepairYard;
    else if((lowerName == "spice silo") || (lowerName == "silo"))               return Structure_Silo;
    else if((lowerName == "concrete") || (lowerName == "slab1"))                return Structure_Slab1;
    else if(lowerName == "slab4")                                               return Structure_Slab4;
    else if((lowerName == "star port") || (lowerName == "starport"))            return Structure_StarPort;
    else if(lowerName == "wall")                                                return Structure_Wall;
    else if(lowerName == "windtrap")                                            return Structure_WindTrap;
    else if(lowerName == "wor")                                                 return Structure_WOR;
    else if((lowerName == "carryall") || (lowerName == "carry-all"))            return Unit_Carryall;
    else if((lowerName == "devastator") || (lowerName == "devistator"))         return Unit_Devastator;
    else if(lowerName == "deviator")                                            return Unit_Deviator;
    else if(lowerName == "frigate")                                             return Unit_Frigate;
    else if(lowerName == "harvester")                                           return Unit_Harvester;
    else if(lowerName == "soldier")                                             return Unit_Soldier;
    else if(lowerName == "launcher")                                            return Unit_Launcher;
    else if(lowerName == "mcv")                                                 return Unit_MCV;
    else if((lowerName == "thopters") || (lowerName == "'thopters")
            || (lowerName == "thopter") || (lowerName == "'thopter")
            || (lowerName == "ornithopter"))                                    return Unit_Ornithopter;
    else if(lowerName == "quad")                                                return Unit_Quad;
    else if(lowerName == "saboteur")                                            return Unit_Saboteur;
    else if(lowerName == "sandworm")                                            return Unit_Sandworm;
    else if(lowerName == "siege tank")                                          return Unit_SiegeTank;
    else if((lowerName == "sonic tank") || (lowerName == "sonictank"))          return Unit_SonicTank;
    else if(lowerName == "tank")                                                return Unit_Tank;
    else if(lowerName == "trike")                                               return Unit_Trike;
    else if((lowerName == "raider trike") || (lowerName == "raider"))           return Unit_RaiderTrike;
    else if(lowerName == "trooper")                                             return Unit_Trooper;
    else if(lowerName == "special")                                             return Unit_Special;
    else if(lowerName == "infantry")                                            return Unit_Infantry;
    else if(lowerName == "troopers")                                            return Unit_Troopers;
    else                                                                        return ItemID_Invalid;
}


/**
    This function returns the name of an item id.
    \param itemID the id of the item (e.g. Unit_Sandworm)
    \return the name of the item (e.g. "Sandworm").
*/
std::string getItemNameByID(Uint32 itemID) {
    switch(itemID) {
        case Structure_Barracks:            return "Barracks";          break;
        case Structure_ConstructionYard:    return "Const Yard";        break;
        case Structure_GunTurret:           return "Turret";            break;
        case Structure_HeavyFactory:        return "Heavy Fctry";       break;
        case Structure_HighTechFactory:     return "Hi-Tech";           break;
        case Structure_IX:                  return "IX";                break;
        case Structure_LightFactory:        return "Light Fctry";       break;
        case Structure_Palace:              return "Palace";            break;
        case Structure_Radar:               return "Outpost";           break;
        case Structure_Refinery:            return "Refinery";          break;
        case Structure_RepairYard:          return "Repair";            break;
        case Structure_RocketTurret:        return "R-Turret";          break;
        case Structure_Silo:                return "Spice Silo";        break;
        case Structure_Slab1:               return "Concrete";         break;
        case Structure_Slab4:               return "Slab4";         break;
        case Structure_StarPort:            return "Starport";          break;
        case Structure_Wall:                return "Wall";              break;
        case Structure_WindTrap:            return "Windtrap";          break;
        case Structure_WOR:                 return "WOR";               break;

        case Unit_Carryall:                 return "Carryall";          break;
        case Unit_Devastator:               return "Devastator";        break;
        case Unit_Deviator:                 return "Deviator";          break;
        case Unit_Frigate:                  return "Frigate";          break;
        case Unit_Harvester:                return "Harvester";     break;
        case Unit_Launcher:                 return "Launcher";          break;
        case Unit_MCV:                      return "MCV";               break;
        case Unit_Ornithopter:              return "'Thopter";          break;
        case Unit_Quad:                     return "Quad";              break;
        case Unit_RaiderTrike:              return "Raider Trike";      break;
        case Unit_SiegeTank:                return "Siege Tank";        break;
        case Unit_SonicTank:                return "Sonic Tank";        break;
        case Unit_Tank:                     return "Tank";              break;
        case Unit_Trike:                    return "Trike";         break;
        case Unit_Saboteur:                 return "Saboteur";          break;
        case Unit_Sandworm:                 return "Sandworm";          break;
        case Unit_Soldier:                  return "Soldier";           break;
        case Unit_Trooper:                  return "Trooper";           break;
        case Unit_Special:                  return "Special";           break;
        case Unit_Infantry:                 return "Infantry";          break;
        case Unit_Troopers:                 return "Troopers";          break;

        default:
            THROW(std::invalid_argument, "getItemNameByID(): Invalid item ID!");
        break;
    }
}


/**
    This function resolves the name corresponding to one item id.
    \param itemID   the id of the item to resolve (e.g. Unit_Quad)
    \return the string corresponding.
*/
std::string resolveItemName(int itemID) {
    switch(itemID) {
        case Structure_Barracks:            return _("@DUNE.ENG|253#Barracks");            break;
        case Structure_ConstructionYard:    return _("@DUNE.ENG|249#Construction Yard");   break;
        case Structure_GunTurret:           return _("@DUNE.ENG|263#Gun Turret");          break;
        case Structure_HeavyFactory:        return _("@DUNE.ENG|241#Heacy Factory");       break;
        case Structure_HighTechFactory:     return _("@DUNE.ENG|243#Hightech Factory");    break;
        case Structure_IX:                  return _("@DUNE.ENG|245#House IX");            break;
        case Structure_LightFactory:        return _("@DUNE.ENG|239#Light Factory");       break;
        case Structure_Palace:              return _("@DUNE.ENG|237#Palace");              break;
        case Structure_Radar:               return _("@DUNE.ENG|269#Radar");               break;
        case Structure_Refinery:            return _("@DUNE.ENG|256#Refinery");            break;
        case Structure_RepairYard:          return _("@DUNE.ENG|259#Repair Yard");         break;
        case Structure_RocketTurret:        return _("@DUNE.ENG|265#Rocket Turret");       break;
        case Structure_Silo:                return _("@DUNE.ENG|267#Silo");                break;
        case Structure_Slab1:               return _("@DUNE.ENG|233#Slab");                break;
        case Structure_Slab4:               return _("@DUNE.ENG|235#Slab (2x2)");          break;
        case Structure_StarPort:            return _("@DUNE.ENG|255#Starport");            break;
        case Structure_Wall:                return _("@DUNE.ENG|261#Wall");                break;
        case Structure_WindTrap:            return _("@DUNE.ENG|251#Windtrap");            break;
        case Structure_WOR:                 return _("@DUNE.ENG|247#WOR");                 break;

        case Unit_Carryall:                 return _("@DUNE.ENG|195#Carryall");            break;
        case Unit_Devastator:               return _("@DUNE.ENG|217#Devastator");          break;
        case Unit_Deviator:                 return _("@DUNE.ENG|211#Deviator");            break;
        case Unit_Frigate:                  return _("Frigate");                           break;
        case Unit_Harvester:                return _("@DUNE.ENG|227#Harvester");           break;
        case Unit_Launcher:                 return _("@DUNE.ENG|209#Launcher");            break;
        case Unit_MCV:                      return _("@DUNE.ENG|229#MCV");                 break;
        case Unit_Ornithopter:              return _("@DUNE.ENG|197#Ornithopter");         break;
        case Unit_Quad:                     return _("@DUNE.ENG|225#Quad");                break;
        case Unit_RaiderTrike:              return _("@DUNE.ENG|223#Raider Trike");        break;
        case Unit_SiegeTank:                return _("@DUNE.ENG|215#Siege Tank");          break;
        case Unit_SonicTank:                return _("@DUNE.ENG|219#Sonic Tank");          break;
        case Unit_Tank:                     return _("@DUNE.ENG|213#Tank");                break;
        case Unit_Trike:                    return _("@DUNE.ENG|221#Trike");               break;
        case Unit_Saboteur:                 return _("@DUNE.ENG|207#Saboteur");            break;
        case Unit_Sandworm:                 return _("@DUNE.ENG|231#Sandworm");            break;
        case Unit_Soldier:                  return _("@DUNE.ENG|203#Soldier");             break;
        case Unit_Trooper:                  return _("@DUNE.ENG|205#Trooper");             break;
        case Unit_Special:                  return _("Sonic/Devast./Devia.");              break;
        case Unit_Infantry:                 return _("@DUNE.ENG|199#Infantry");            break;
        case Unit_Troopers:                 return _("@DUNE.ENG|201#Troopers");            break;

        default:
            THROW(std::invalid_argument, "resolveItemName(): Invalid item ID!");
        break;
    }
}


/**
    This function returns the number of each house providing the house name as a string. The comparison is
    done case-insensitive.
    \param name the name of the house (e.g."Atreides")
    \return the number of the house (e.g. HOUSE_ATREIDES). HOUSE_INVALID is returned on error.
*/
HOUSETYPE getHouseByName(const std::string& name) {
    const std::string lowerName = strToLower(name);

    if(lowerName == "harkonnen")         return HOUSE_HARKONNEN;
    else if(lowerName == "atreides")     return HOUSE_ATREIDES;
    else if(lowerName == "ordos")        return HOUSE_ORDOS;
    else if(lowerName == "fremen")       return HOUSE_FREMEN;
    else if(lowerName == "sardaukar")    return HOUSE_SARDAUKAR;
    else if(lowerName == "mercenary")    return HOUSE_MERCENARY;
    else                                return HOUSE_INVALID;
}

/**
    This function returns the name of house the house number.
    \param house the number of the house (e.g. HOUSE_ATREIDES)
    \return the name of the house (e.g. "Atreides").
*/
std::string getHouseNameByNumber(HOUSETYPE house) {
    if(house >= 0 && house < NUM_HOUSES) {
        static const char* const houseName[NUM_HOUSES] = {  "Harkonnen",
                                                            "Atreides",
                                                            "Ordos",
                                                            "Fremen",
                                                            "Sardaukar",
                                                            "Mercenary"
                                                   };
        return houseName[house];
    } else {
        THROW(std::invalid_argument, "Invalid house number %d!", house);
    }
}

ATTACKMODE getAttackModeByName(const std::string& name) {
    const std::string lowerName = strToLower(name);

    if(lowerName == "guard")                                    return GUARD;
    else if(lowerName == "area guard")                          return AREAGUARD;
    else if(lowerName == "ambush")                              return AMBUSH;
    else if((lowerName == "hunt") || (lowerName == "attack"))   return HUNT;
    else if(lowerName == "harvest")                             return HARVEST;
    else if(lowerName == "sabotage")                            return SABOTAGE;
    else if(lowerName == "stop")                                return STOP;
    else if(lowerName == "capture")                             return CAPTURE;
    else if(lowerName == "retreat")                             return RETREAT;
    else                                                        return ATTACKMODE_INVALID;
}


std::string getAttackModeNameByMode(ATTACKMODE attackMode) {
    switch(attackMode) {
        case GUARD:     return "Guard";         break;
        case AREAGUARD: return "Area Guard";    break;
        case AMBUSH:    return "Ambush";        break;
        case HUNT:      return "Hunt";          break;
        case HARVEST:   return "Harvest";       break;
        case SABOTAGE:  return "Sabotage";      break;
        case STOP:      return "Stop";          break;
        case CAPTURE:   return "Capture";          break;
        default:
            THROW(std::invalid_argument, "getAttackModeNameByMode(): Invalid attack mode!");
        break;
    }
}


Uint32 getColorByTerrainType(int terrainType) {
    Uint32 color = COLOR_BLACK;
    switch (terrainType) {
        case Terrain_Dunes:         color = COLOR_DESERTSAND;  break;
        case Terrain_Mountain:      color = COLOR_MOUNTAIN;    break;
        case Terrain_Rock:          color = COLOR_ROCK;        break;
        case Terrain_Sand:          color = COLOR_DESERTSAND;  break;
        case Terrain_Spice:         color = COLOR_SPICE;       break;
        case Terrain_ThickSpice:    color = COLOR_THICKSPICE;  break;
        case Terrain_SpiceBloom:    color = COLOR_BLOOM;       break;
        case Terrain_SpecialBloom:  color = COLOR_BLOOM;       break;
        case Terrain_Slab:          color = COLOR_ROCK;        break;
        default:                    color = COLOR_ROCK;        break;
    }

    return color;
}



DropLocation getDropLocationByName(const std::string& name) {
    const std::string lowerName = strToLower(name);

    if(lowerName == "north") {
        return Drop_North;
    } else if(lowerName == "east") {
        return Drop_East;
    } else if(lowerName == "south") {
        return Drop_South;
    } else if(lowerName == "west") {
        return Drop_West;
    } else if(lowerName == "air") {
        return Drop_Air;
    } else if(lowerName == "visible") {
        return Drop_Visible;
    } else if(lowerName == "enemybase") {
        return Drop_Enemybase;
    } else if(lowerName == "homebase") {
        return Drop_Homebase;
    } else {
        return Drop_Invalid;
    }
}


std::string getDropLocationNameByID(DropLocation dropLocation) {
    switch(dropLocation) {
        case Drop_North:     return "North";     break;
        case Drop_East:      return "East";      break;
        case Drop_South:     return "South";     break;
        case Drop_West:      return "West";      break;
        case Drop_Air:       return "Air";       break;
        case Drop_Visible:   return "Visible";   break;
        case Drop_Enemybase: return "Enemybase"; break;
        case Drop_Homebase:  return "Homebase";  break;
        default:
            THROW(std::invalid_argument, "getDropLocationNameByID(): Invalid drop location!");
        break;
    }
}

std::string resolveDropLocationName(DropLocation dropLocation) {
    switch(dropLocation) {
        case Drop_North:     return _("top edge");     break;
        case Drop_East:      return _("right edge");   break;
        case Drop_South:     return _("bottom edge");  break;
        case Drop_West:      return _("left edge");    break;
        case Drop_Air:       return _("random");       break;
        case Drop_Visible:   return _("middle");       break;
        case Drop_Enemybase: return _("enemy base");   break;
        case Drop_Homebase:  return _("home base");    break;
        default:
            THROW(std::invalid_argument, "resolveDropLocationName(): Invalid drop location!");
        break;
    }
}

AITeamBehavior getAITeamBehaviorByName(const std::string& name) {
    const std::string lowerName = strToLower(name);

    if(lowerName == "normal") {
        return AITeamBehavior_Normal;
    } else if(lowerName == "guard") {
        return AITeamBehavior_Guard;
    } else if(lowerName == "kamikaze") {
        return AITeamBehavior_Kamikaze;
    } else if(lowerName == "staging") {
        return AITeamBehavior_Staging;
    } else if(lowerName == "flee") {
        return AITeamBehavior_Flee;
    } else {
        return AITeamBehavior_Invalid;
    }
}


std::string getAITeamBehaviorNameByID(AITeamBehavior aiTeamBehavior) {
    switch(aiTeamBehavior) {
        case AITeamBehavior_Normal:     return "Normal";     break;
        case AITeamBehavior_Guard:      return "Guard";      break;
        case AITeamBehavior_Kamikaze:   return "Kamikaze";   break;
        case AITeamBehavior_Staging:    return "Staging";    break;
        case AITeamBehavior_Flee:       return "Flee";       break;
        default:
            THROW(std::invalid_argument, "getAITeamBehaviorNameByID(): Invalid team behavior!");
        break;
    }
}


AITeamType getAITeamTypeByName(const std::string& name) {
    const std::string lowerName = strToLower(name);

    if(lowerName == "foot") {
        return AITeamType_Foot;
    } else if(lowerName == "wheel" || lowerName == "wheeled") {
        return AITeamType_Wheeled;
    } else if(lowerName == "track" || lowerName == "tracked") {
        return AITeamType_Tracked;
    } else if(lowerName == "winged") {
        return AITeamType_Winged;
    } else if(lowerName == "slither") {
        return AITeamType_Slither;
    } else if(lowerName == "harvester") {
        return AITeamType_Harvester;
    } else {
        return AITeamType_Invalid;
    }
}


std::string getAITeamTypeNameByID(AITeamType aiTeamType) {
    switch(aiTeamType) {
        case AITeamType_Foot:      return "Foot";      break;
        case AITeamType_Wheeled:   return "Wheeled";   break;
        case AITeamType_Tracked:   return "Tracked";   break;
        case AITeamType_Winged:    return "Winged";    break;
        case AITeamType_Slither:   return "Slither";   break;
        case AITeamType_Harvester: return "Harvester"; break;
        default:
            THROW(std::invalid_argument, "getAITeamTypeNameByID(): Invalid team type!");
        break;
    }
}


/**
    This function returns the house-dependent weakness of a unit to get deviated
    \param  house   the house of the unit (choose the real owner);
*/
FixPoint getDeviateWeakness(HOUSETYPE house) {

    // Deviators are crap enough. If this is a custom game remove the weakness nerf
    // So that Ordos is playable for Humans
    if(currentGame->gameType == GameType::CustomGame || currentGame->gameType == GameType::CustomMultiplayer){
        return 1.00_fix;
    } else {
        switch(house) {
            case HOUSE_HARKONNEN:   return 0.78_fix;
            case HOUSE_ATREIDES:    return 0.30_fix;
            case HOUSE_ORDOS:       return 0.50_fix;
            case HOUSE_FREMEN:      return 0.08_fix;
            case HOUSE_SARDAUKAR:   return 0.04_fix;
            case HOUSE_MERCENARY:   return 0.50_fix;
            default:                return 0.00_fix;
        }
    }
}



/**
    Starts a game replay
    \param  filename    the filename of the replay file
*/
void startReplay(const std::string& filename) {
    SDL_Log("Initializing replay...");
    try {
        currentGame = new Game();
        currentGame->initReplay(filename);

        currentGame->runMainLoop();

        delete currentGame;
        currentGame = nullptr;

        // Change music to menu music
        musicPlayer->changeMusic(MUSIC_MENU);
    } catch(...) {
        delete currentGame;
        currentGame = nullptr;
        throw;
    }
}


/**
    Starts a new game. If this game is quit it might start another game. This other game is also started from
    this function. This is done until there is no more game to be started.
    \param init contains all the information to start the game
*/
void startSinglePlayerGame(const GameInitSettings& init)
{
    GameInitSettings currentGameInitInfo = init;

    while(1) {

        try {

            SDL_Log("Initializing game...");
            currentGame = new Game();
            currentGame->initGame(currentGameInitInfo);

            // get init settings from game as it might have changed (through loading the game)
            currentGameInitInfo = currentGame->getGameInitSettings();

            currentGame->runMainLoop();

            bool bGetNext = true;
            while(bGetNext) {
                switch(currentGame->whatNext()) {
                    case GAME_DEBRIEFING_WIN: {
                        SDL_Log("Debriefing...");
                        {
                            BriefingMenu briefing(currentGameInitInfo.getHouseID(), currentGameInitInfo.getMission(), DEBRIEFING_WIN);
                            briefing.showMenu();
                        }

                        SDL_Log("Game statistics...");
                        {
                            CampaignStatsMenu campaignStats(missionNumberToLevelNumber(currentGameInitInfo.getMission()));
                            campaignStats.showMenu();
                        }

                        const int houseID = currentGameInitInfo.getHouseID();

                        if(currentGameInitInfo.getGameType() == GameType::Campaign) {
                            const int level = missionNumberToLevelNumber(currentGameInitInfo.getMission());

                            if(level == 4 && (houseID == HOUSE_HARKONNEN || houseID == HOUSE_ATREIDES || houseID == HOUSE_ORDOS)) {
                                SDL_Log("Playing meanwhile...");
                                Meanwhile meanwhile(houseID,true);
                                meanwhile.run();
                            } else if(level == 8 && (houseID == HOUSE_HARKONNEN || houseID == HOUSE_ATREIDES || houseID == HOUSE_ORDOS)) {
                                SDL_Log("Playing meanwhile...");
                                Meanwhile meanwhile(houseID,false);
                                meanwhile.run();
                            } else if(level == 9) {
                                SDL_Log("Playing finale.....");
                                Finale finale(houseID);
                                finale.run();
                            }
                        }
                    } break;

                    case GAME_DEBRIEFING_LOST: {
                        SDL_Log("Debriefing...");
                        BriefingMenu briefing(currentGameInitInfo.getHouseID(), currentGameInitInfo.getMission(), DEBRIEFING_LOST);
                        briefing.showMenu();
                    } break;

                    case GAME_CUSTOM_GAME_STATS: {
                        SDL_Log("Game statistics...");
                        CustomGameStatsMenu stats;
                        stats.showMenu();
                    } break;

                    case GAME_LOAD:
                    case GAME_NEXTMISSION: {
                        currentGameInitInfo = currentGame->getNextGameInitSettings();
                        bGetNext = false;
                    } break;

                    case GAME_RETURN_TO_MENU:
                    default: {
                        delete currentGame;
                        currentGame = nullptr;

                        // Change music to menu music
                        musicPlayer->changeMusic(MUSIC_MENU);

                        return;
                    } break;
                }
            }
        } catch(...) {
            delete currentGame;
            currentGame = nullptr;
            throw;
        }
    }
}

/**
    Starts a new multiplayer game.
    \param init contains all the information to start the game
*/
void startMultiPlayerGame(const GameInitSettings& init)
{
    GameInitSettings currentGameInitInfo = init;

    SDL_Log("Initializing game...");
    try {
        currentGame = new Game();
        currentGame->initGame(currentGameInitInfo);

        // get init settings from game as it might have changed (through loading the game)
        currentGameInitInfo = currentGame->getGameInitSettings();

        currentGame->runMainLoop();

        if(currentGame->whatNext() == GAME_CUSTOM_GAME_STATS) {
            SDL_Log("Game statistics...");
            CustomGameStatsMenu stats;
            stats.showMenu();
        }

        delete currentGame;
        currentGame = nullptr;
    } catch(...) {
        delete currentGame;
        currentGame = nullptr;
        throw;
    }
}
