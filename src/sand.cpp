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

#include <misc/string_util.h>

#include <algorithm>

/**
    This function draws the cursor to the screen. The coordinate is read from
    the two global variables drawnMouseX and drawnMouseY.
*/
void drawCursor() {
    if(!(SDL_GetAppState() & SDL_APPMOUSEFOCUS)) {
        return;
    }



	SDL_Surface* surface = pGFXManager->getUIGraphic(cursorFrame);

    SDL_Rect dest = { static_cast<Sint16>(drawnMouseX), static_cast<Sint16>(drawnMouseY), static_cast<Uint16>(surface->w), static_cast<Uint16>(surface->h) };

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

	if(SDL_BlitSurface(surface, NULL, screen, &dest) != 0) {
        fprintf(stderr,"drawCursor(): %s\n", SDL_GetError());
	}
}

/**
    This function resolves the picture corresponding to one item id.
    \param itemID   the id of the item to resolve (e.g. Unit_Quad)
    \return the surface corresponding. This surface should not be freed or modified. NULL on error.
*/
SDL_Surface* resolveItemPicture(int itemID, HOUSETYPE house) {
	int newPicID;

	switch(itemID) {
        case Structure_Barracks:            newPicID = Picture_Barracks;		    break;
		case Structure_ConstructionYard:    newPicID = Picture_ConstructionYard;	break;
		case Structure_GunTurret:			newPicID = Picture_GunTurret;		    break;
		case Structure_HeavyFactory:		newPicID = Picture_HeavyFactory;		break;
		case Structure_HighTechFactory:		newPicID = Picture_HighTechFactory;		break;
		case Structure_IX:			        newPicID = Picture_IX;		            break;
		case Structure_LightFactory:		newPicID = Picture_LightFactory;		break;
		case Structure_Palace:			    newPicID = Picture_Palace;		        break;
		case Structure_Radar:			    newPicID = Picture_Radar;		        break;
		case Structure_Refinery:			newPicID = Picture_Refinery;	       	break;
		case Structure_RepairYard:			newPicID = Picture_RepairYard;		    break;
		case Structure_RocketTurret:		newPicID = Picture_RocketTurret;		break;
		case Structure_Silo:			    newPicID = Picture_Silo;		        break;
		case Structure_Slab1:			    newPicID = Picture_Slab1;               break;
		case Structure_Slab4:			    newPicID = Picture_Slab4;		        break;
		case Structure_StarPort:			newPicID = Picture_StarPort;	    	break;
		case Structure_Wall:			    newPicID = Picture_Wall;	        	break;
		case Structure_WindTrap:		   	newPicID = Picture_WindTrap;	    	break;
		case Structure_WOR:			        newPicID = Picture_WOR;		            break;

		case Unit_Carryall:			        newPicID = Picture_Carryall;		    break;
		case Unit_Devastator:		       	newPicID = Picture_Devastator;		    break;
		case Unit_Deviator:			        newPicID = Picture_Deviator;	    	break;
		case Unit_Frigate:			        newPicID = Picture_Frigate;	            break;
		case Unit_Harvester:		    	newPicID = Picture_Harvester;		    break;
		case Unit_Launcher:			        newPicID = Picture_Launcher;	    	break;
		case Unit_MCV:			            newPicID = Picture_MCV;		            break;
		case Unit_Ornithopter:			    newPicID = Picture_Ornithopter;		    break;
		case Unit_Quad:		            	newPicID = Picture_Quad;		        break;
		case Unit_RaiderTrike:			    newPicID = Picture_RaiderTrike;		    break;
		case Unit_SiegeTank:		    	newPicID = Picture_SiegeTank;	    	break;
		case Unit_SonicTank:		    	newPicID = Picture_SonicTank;	    	break;
		case Unit_Tank:			            newPicID = Picture_Tank;		        break;
		case Unit_Trike:		        	newPicID = Picture_Trike;		        break;
		case Unit_Saboteur:			        newPicID = Picture_Saboteur;	    	break;
		case Unit_Sandworm:			        newPicID = Picture_Sandworm;           	break;
		case Unit_Soldier:			        newPicID = Picture_Soldier;	        	break;
		case Unit_Trooper: {
		    switch(house) {
		        case HOUSE_SARDAUKAR:      newPicID = Picture_Sardaukar;           break;
		        case HOUSE_FREMEN:         newPicID = Picture_Fremen;              break;
                default:                   newPicID = Picture_Trooper;             break;
		    }
        } break;
		case Unit_Special:                  newPicID = Picture_Special;            break;
		case Unit_Infantry:                 newPicID = Picture_Soldier;             break;
		case Unit_Troopers: {
		    switch(house) {
		        case HOUSE_SARDAUKAR:      newPicID = Picture_Sardaukar;           break;
		        case HOUSE_FREMEN:         newPicID = Picture_Fremen;              break;
                default:                   newPicID = Picture_Trooper;             break;
		    }
        } break;

		default:
            throw std::invalid_argument("resolveItemPicture(): Invalid item ID " + stringify(itemID) + "!");
        break;
    }

    return pGFXManager->getSmallDetailPic(newPicID);
}


/**
    This function returns the anim id based on the passed filename.
    \param  filename    the filename (e.g. STARPORT.WSA)
    \return the id of the animation (e.g. Anim_StarPort)
*/
int getAnimByFilename(std::string filename) {
    convertToLower(filename);

    if(filename == "fartr.wsa")         return Anim_AtreidesPlanet;
    else if(filename == "fhark.wsa")    return Anim_HarkonnenPlanet;
    else if(filename == "fordos.wsa")   return Anim_OrdosPlanet;
    else if(filename == "win1.wsa")     return Anim_Win1;
    else if(filename == "win2.wsa")     return Anim_Win2;
    else if(filename == "lostbild.wsa") return Anim_Lose1;
    else if(filename == "lostvehc.wsa") return Anim_Lose2;
    else if(filename == "barrac.wsa")   return Anim_Barracks;
    else if(filename == "carryall.wsa") return Anim_Carryall;
    else if(filename == "construc.wsa") return Anim_ConstructionYard;
    else if(filename == "fremen.wsa")   return Anim_Fremen;
    else if(filename == "gold-bb.wsa")  return Anim_DeathHand;
    else if(filename == "harktank.wsa") return Anim_Devastator;
    else if(filename == "harvest.wsa")  return Anim_Harvester;
    else if(filename == "headqrts.wsa") return Anim_Radar;
    else if(filename == "hitcftry.wsa") return Anim_HighTechFactory;
    else if(filename == "htank.wsa")    return Anim_SiegeTank;
    else if(filename == "hvyftry.wsa")  return Anim_HeavyFactory;
    else if(filename == "hyinfy.wsa")   return Anim_Trooper;
    else if(filename == "infantry.wsa") return Anim_Infantry;
    else if(filename == "ix.wsa")       return Anim_IX;
    else if(filename == "liteftry.wsa") return Anim_LightFactory;
    else if(filename == "ltank.wsa")    return Anim_Tank;
    else if(filename == "mcv.wsa")      return Anim_MCV;
    else if(filename == "ordrtank.wsa") return Anim_Deviator;
    else if(filename == "orni.wsa")     return Anim_Ornithopter;
    else if(filename == "otrike.wsa")   return Anim_Raider;
    else if(filename == "palace.wsa")   return Anim_Palace;
    else if(filename == "quad.wsa")     return Anim_Quad;
    else if(filename == "refinery.wsa") return Anim_Refinery;
    else if(filename == "repair.wsa")   return Anim_RepairYard;
    else if(filename == "rtank.wsa")    return Anim_Launcher;
    else if(filename == "rturret.wsa")  return Anim_RocketTurret;
    else if(filename == "saboture.wsa") return Anim_Saboteur;
    else if(filename == "slab.wsa")     return Anim_Slab1;
    else if(filename == "stank.wsa")    return Anim_SonicTank;
    else if(filename == "starport.wsa") return Anim_StarPort;
    else if(filename == "storage.wsa")  return Anim_Silo;
    else if(filename == "trike.wsa")    return Anim_Trike;
    else if(filename == "turret.wsa")   return Anim_GunTurret;
    else if(filename == "wall.wsa")     return Anim_Wall;
    else if(filename == "windtrap.wsa") return Anim_WindTrap;
    else if(filename == "wor.wsa")      return Anim_WOR;
    else if(filename == "worm.wsa")     return Anim_Sandworm;
    else if(filename == "sardukar.wsa") return Anim_Sardaukar;
    else if(filename == "frigate.wsa")  return Anim_Frigate;
    else if(filename == "4slab.wsa")    return Anim_Slab4;
    else                                return 0;
}

/**
    This function returns the size of the specified item.
    \param ItemID   the id of the item (e.g. Structure_HeavyFactory)
    \return a Coord containg the size (e.g. (3,2) ). Returns (0,0) on error.
*/
Coord getStructureSize(int itemID) {

	switch(itemID) {
		case Structure_Barracks:			return Coord(2,2); break;
		case Structure_ConstructionYard:	return Coord(2,2); break;
		case Structure_GunTurret: 			return Coord(1,1); break;
		case Structure_HeavyFactory: 		return Coord(3,2); break;
		case Structure_HighTechFactory:		return Coord(3,2); break;
		case Structure_IX:					return Coord(2,2); break;
		case Structure_LightFactory:		return Coord(2,2); break;
		case Structure_Palace:				return Coord(3,3); break;
		case Structure_Radar:				return Coord(2,2); break;
		case Structure_Refinery:			return Coord(3,2); break;
		case Structure_RepairYard:			return Coord(3,2); break;
		case Structure_RocketTurret:		return Coord(1,1); break;
		case Structure_Silo:				return Coord(2,2); break;
		case Structure_StarPort:			return Coord(3,3); break;
		case Structure_Slab1:				return Coord(1,1); break;
		case Structure_Slab4:				return Coord(2,2); break;
		case Structure_Wall:				return Coord(1,1); break;
		case Structure_WindTrap:			return Coord(2,2); break;
		case Structure_WOR:					return Coord(2,2); break;
		default:							return Coord(0,0); break;
	}

	return Coord(0,0);
}

/**
    This function return the item id of an item specified by name. There may be multiple names for
    one item. The case of the name is ignored.
    \param name the name of the item (e.g. "rocket-turret" or "r-turret".
    \return the id of the item (e.g. Structure_RocketTurret)
*/
Uint32  getItemIDByName(std::string name) {
    convertToLower(name);

    if(name == "barracks")                                              return Structure_Barracks;
    else if((name == "const yard") || (name == "construction yard"))    return Structure_ConstructionYard;
    else if((name == "r-turret") || (name == "rocket-turret"))          return Structure_RocketTurret;
    else if((name == "turret") || (name == "gun-turret"))               return Structure_GunTurret;
    else if((name == "heavy fctry") || (name == "heavy factory"))       return Structure_HeavyFactory;
	else if((name == "hi-tech") || (name == "hightech factory"))        return Structure_HighTechFactory;
	else if((name == "ix") || (name == "house ix"))                     return Structure_IX;
    else if((name == "light fctry") || (name == "light factory"))       return Structure_LightFactory;
	else if(name == "palace")                                           return Structure_Palace;
    else if((name == "outpost") || (name == "radar"))                   return Structure_Radar;
	else if(name == "refinery")                                         return Structure_Refinery;
    else if((name == "repair") || (name == "repair yard"))              return Structure_RepairYard;
	else if((name == "spice silo") || (name == "silo"))                 return Structure_Silo;
	else if((name == "concrete") || (name == "slab1"))                  return Structure_Slab1;
	else if(name == "slab4")                                            return Structure_Slab4;
    else if((name == "star port") || (name == "starport"))              return Structure_StarPort;
    else if(name == "wall")                                             return Structure_Wall;
    else if(name == "windtrap")                                         return Structure_WindTrap;
    else if(name == "wor")                                              return Structure_WOR;
    else if((name == "carryall") || (name == "carry-all"))              return Unit_Carryall;
	else if((name == "devastator") || (name == "devistator"))           return Unit_Devastator;
    else if(name == "deviator")                                         return Unit_Deviator;
    else if(name == "frigate")                                          return Unit_Frigate;
	else if(name == "harvester")                                        return Unit_Harvester;
	else if(name == "soldier")                                          return Unit_Soldier;
	else if(name == "launcher")                                         return Unit_Launcher;
	else if(name == "mcv")                                              return Unit_MCV;
	else if((name == "thopters") || (name == "'thopters")
            || (name == "thopter") || (name == "'thopter")
            || (name == "ornithopter"))                                 return Unit_Ornithopter;
	else if(name == "quad")                                             return Unit_Quad;
	else if(name == "saboteur")                                         return Unit_Saboteur;
    else if(name == "sandworm")                                         return Unit_Sandworm;
    else if(name == "siege tank")                                       return Unit_SiegeTank;
	else if((name == "sonic tank") || (name == "sonictank"))            return Unit_SonicTank;
	else if(name == "tank")                                             return Unit_Tank;
    else if(name == "trike")                                            return Unit_Trike;
    else if((name == "raider trike") || (name == "raider"))             return Unit_RaiderTrike;
	else if(name == "trooper")                                          return Unit_Trooper;
	else if(name == "special")                                          return Unit_Special;
	else if(name == "infantry")                                         return Unit_Infantry;
	else if(name == "troopers")                                         return Unit_Troopers;
	else                                                                return ItemID_Invalid;
}


/**
    This function returns the name of an item id.
    \param itemID the id of the item (e.g. Unit_Sandworm)
    \return the name of the item (e.g. "Sandworm").
*/
std::string getItemNameByID(Uint32 itemID) {
	switch(itemID) {
		case Structure_Barracks:            return "Barracks";		    break;
		case Structure_ConstructionYard:    return "Const Yard";	    break;
		case Structure_GunTurret:			return "Turret";		    break;
		case Structure_HeavyFactory:		return "Heavy Fctry";		break;
		case Structure_HighTechFactory:		return "Hi-Tech";	        break;
		case Structure_IX:			        return "IX";		        break;
		case Structure_LightFactory:		return "Light Fctry";		break;
		case Structure_Palace:			    return "Palace";		    break;
		case Structure_Radar:			    return "Outpost";		    break;
		case Structure_Refinery:			return "Refinery";	       	break;
		case Structure_RepairYard:			return "Repair";		    break;
		case Structure_RocketTurret:		return "R-Turret";		    break;
		case Structure_Silo:			    return "Spice Silo";		break;
		case Structure_Slab1:			    return "Concrete";         break;
		case Structure_Slab4:			    return "Slab4";		    break;
		case Structure_StarPort:			return "Starport";	    	break;
		case Structure_Wall:			    return "Wall";	        	break;
		case Structure_WindTrap:		   	return "Windtrap";	    	break;
		case Structure_WOR:			        return "WOR";		        break;

		case Unit_Carryall:			        return "Carryall";		    break;
		case Unit_Devastator:		       	return "Devastator";		break;
		case Unit_Deviator:			        return "Deviator";	    	break;
		case Unit_Frigate:			        return "Frigate";          break;
		case Unit_Harvester:		    	return "Harvester";	    break;
		case Unit_Launcher:			        return "Launcher";	    	break;
		case Unit_MCV:			            return "MCV";		        break;
		case Unit_Ornithopter:			    return "'Thopter";		    break;
		case Unit_Quad:		            	return "Quad";		        break;
		case Unit_RaiderTrike:			    return "Raider Trike";		break;
		case Unit_SiegeTank:		    	return "Siege Tank";	    break;
		case Unit_SonicTank:		    	return "Sonic Tank";	    break;
		case Unit_Tank:			            return "Tank";		        break;
		case Unit_Trike:		        	return "Trike";		    break;
		case Unit_Saboteur:			        return "Saboteur";	    	break;
		case Unit_Sandworm:                 return "Sandworm";		    break;
		case Unit_Soldier:			        return "Soldier";	        break;
		case Unit_Trooper:			        return "Trooper";		    break;
		case Unit_Special:			        return "Special";		    break;
		case Unit_Infantry:			        return "Infantry";		    break;
		case Unit_Troopers:			        return "Troopers";		    break;

		default:
            throw std::invalid_argument("getItemNameByID(): Invalid item ID!");
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
            throw std::invalid_argument("resolveItemName(): Invalid item ID!");
        break;
    }
}


/**
    This function returns the number of each house providing the house name as a string. The comparison is
    done case-insensitive.
    \param name the name of the house (e.g."Atreides")
    \return the number of the house (e.g. HOUSE_ATREIDES). HOUSE_INVALID is returned on error.
*/
HOUSETYPE getHouseByName(std::string name) {
    convertToLower(name);

    if(name == "harkonnen")         return HOUSE_HARKONNEN;
    else if(name == "atreides")     return HOUSE_ATREIDES;
    else if(name == "ordos")        return HOUSE_ORDOS;
    else if(name == "fremen")       return HOUSE_FREMEN;
	else if(name == "sardaukar")    return HOUSE_SARDAUKAR;
	else if(name == "mercenary")    return HOUSE_MERCENARY;
    else                            return HOUSE_INVALID;
}

/**
    This function returns the name of house the house number.
    \param house the number of the house (e.g. HOUSE_ATREIDES)
    \return the name of the house (e.g. "Atreides").
*/
std::string getHouseNameByNumber(HOUSETYPE house) {
    static const char* houseName[NUM_HOUSES] = {  "Harkonnen",
                                                    "Atreides",
                                                    "Ordos",
                                                    "Fremen",
                                                    "Sardaukar",
                                                    "Mercenary"
                                                };

    if(house >= 0 && house < NUM_HOUSES) {
        return houseName[house];
    } else {
        fprintf(stderr,"getHouseNameByNumber(): Invalid house number %d!\n", house);
        exit(EXIT_FAILURE);
    }
}

ATTACKMODE getAttackModeByName(std::string name) {
    convertToLower(name);

    if(name == "guard")                             return GUARD;
    else if(name == "area guard")                   return AREAGUARD;
    else if(name == "ambush")                       return AMBUSH;
	else if((name == "hunt") || (name == "attack")) return HUNT;
    else if(name == "harvest")                      return HARVEST;
    else if(name == "sabotage")                     return SABOTAGE;
    else if(name == "stop")                         return STOP;
    else if(name == "capture")                      return CAPTURE;
    else                                            return ATTACKMODE_INVALID;
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
            throw std::invalid_argument("getAttackModeNameByMode(): Invalid attack mode!");
        break;
    }
}


int getColorByTerrainType(int terrainType) {
    int color = COLOR_BLACK;
    switch (terrainType) {
        case Terrain_Dunes:         color = COLOR_DESERTSAND;   break;
        case Terrain_Mountain:      color = COLOR_MOUNTAIN;     break;
        case Terrain_Rock:          color = COLOR_DARKGREY;     break;
        case Terrain_Sand:          color = COLOR_DESERTSAND;   break;
        case Terrain_Spice:         color = COLOR_SPICE;        break;
        case Terrain_ThickSpice:    color = COLOR_THICKSPICE;   break;
        case Terrain_SpiceBloom:    color = COLOR_RED;          break;
        case Terrain_SpecialBloom:  color = COLOR_RED;          break;
        case Terrain_Slab:          color = COLOR_DARKGREY;     break;
        default:                   color = COLOR_DARKGREY;     break;
    }

    return color;
}



DropLocation getDropLocationByName(std::string name) {
    convertToLower(name);

    if(name == "north") {
        return Drop_North;
    } else if(name == "east") {
        return Drop_East;
    } else if(name == "south") {
        return Drop_South;
    } else if(name == "west") {
        return Drop_West;
    } else if(name == "air") {
        return Drop_Air;
    } else if(name == "visible") {
        return Drop_Visible;
    } else if(name == "enemybase") {
        return Drop_Enemybase;
    } else if(name == "homebase") {
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
            throw std::invalid_argument("getDropLocationNameByID(): Invalid drop location!");
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
            throw std::invalid_argument("resolveDropLocationName(): Invalid drop location!");
        break;
    }
}

TeamBehavior getTeamBehaviorByName(std::string name) {
    convertToLower(name);

    if(name == "normal") {
        return TeamBehavior_Normal;
    } else if(name == "guard") {
        return TeamBehavior_Guard;
    } else if(name == "kamikaze") {
        return TeamBehavior_Kamikaze;
    } else if(name == "staging") {
        return TeamBehavior_Staging;
    } else if(name == "flee") {
        return TeamBehavior_Flee;
    } else {
        return TeamBehavior_Invalid;
    }
}


std::string getTeamBehaviorNameByID(TeamBehavior teamBehavior) {
    switch(teamBehavior) {
        case TeamBehavior_Normal:     return "Normal";     break;
        case TeamBehavior_Guard:      return "Guard";      break;
        case TeamBehavior_Kamikaze:   return "Kamikaze";   break;
        case TeamBehavior_Staging:    return "Staging";    break;
        case TeamBehavior_Flee:       return "Flee";       break;
        default:
            throw std::invalid_argument("getTeamBehaviorNameByID(): Invalid team behavior!");
        break;
    }
}


TeamType getTeamTypeByName(std::string name) {
    convertToLower(name);

    if(name == "foot") {
        return TeamType_Foot;
    } else if(name == "wheel" || name == "wheeled") {
        return TeamType_Wheeled;
    } else if(name == "track" || name == "tracked") {
        return TeamType_Tracked;
    } else if(name == "winged") {
        return TeamType_Winged;
    } else if(name == "slither") {
        return TeamType_Slither;
    } else if(name == "harvester") {
        return TeamType_Harvester;
    } else {
        return TeamType_Invalid;
    }
}


std::string getTeamTypeNameByID(TeamType teamType) {
    switch(teamType) {
        case TeamType_Foot:      return "Foot";      break;
        case TeamType_Wheeled:   return "Wheeled";   break;
        case TeamType_Tracked:   return "Tracked";   break;
        case TeamType_Winged:    return "Winged";    break;
        case TeamType_Slither:   return "Slither";   break;
        case TeamType_Harvester: return "Harvester"; break;
        default:
            throw std::invalid_argument("getTeamTypeNameByID(): Invalid team type!");
        break;
    }
}


/**
    This function returns the house-dependent weakness of a unit to get deviated
    \param  house   the house of the unit (choose the real owner);
*/
FixPoint getDeviateWeakness(HOUSETYPE house) {
    // Deviators are crap enough, remove the weakness nerf
    //return 1;

    switch(house) {
	    case HOUSE_HARKONNEN:   return FixPt(0,78);
        case HOUSE_ATREIDES:    return FixPt(0,30);
	    case HOUSE_ORDOS:       return FixPt(0,50);
	    case HOUSE_FREMEN:      return FixPt(0,08);
	    case HOUSE_SARDAUKAR:   return FixPt(0,04);
	    case HOUSE_MERCENARY:   return FixPt(0,50);
	    default:                return FixPt(0,00);
    }
}



/**
	Starts a game replay
	\param	filename	the filename of the replay file
*/
void startReplay(std::string filename) {
    printf("Initing Replay:\n");
    currentGame = new Game();
    currentGame->initReplay(filename);

    printf("Initialization finished!\n");
    fflush(stdout);

    currentGame->runMainLoop();

    delete currentGame;

    // Change music to menu music
    musicPlayer->changeMusic(MUSIC_MENU);
}


/**
	Starts a new game. If this game is quit it might start another game. This other game is also started from
	this function. This is done until there is no more game to be started.
	\param init	contains all the information to start the game
*/
void startSinglePlayerGame(const GameInitSettings& init)
{
    GameInitSettings currentGameInitInfo = init;

	while(1) {

        printf("Initing Game:\n");
		currentGame = new Game();
		currentGame->initGame(currentGameInitInfo);

		printf("Initialization finished!\n");
		fflush(stdout);

		// get init settings from game as it might have changed (through loading the game)
		currentGameInitInfo = currentGame->getGameInitSettings();

		currentGame->runMainLoop();

		bool bGetNext = true;
		while(bGetNext) {
			switch(currentGame->whatNext()) {
				case GAME_DEBRIEFING_WIN: {
					fprintf(stdout,"Debriefing...");
					fflush(stdout);
					BriefingMenu* pBriefing = new BriefingMenu(currentGameInitInfo.getHouseID(), currentGameInitInfo.getMission(), DEBRIEFING_WIN);
					pBriefing->showMenu();
					delete pBriefing;
					fprintf(stdout,"\t\t\tfinished\n");
					fflush(stdout);

					fprintf(stdout,"Game statistics...");
					fflush(stdout);
                    CampaignStatsMenu* pCampaignStats = new CampaignStatsMenu(missionNumberToLevelNumber(currentGameInitInfo.getMission()));
					pCampaignStats->showMenu();
					delete pCampaignStats;
					fprintf(stdout,"\t\tfinished\n");
					fflush(stdout);

					int houseID = currentGameInitInfo.getHouseID();

					if(currentGameInitInfo.getGameType() == GAMETYPE_CAMPAIGN) {
                        int level = missionNumberToLevelNumber(currentGameInitInfo.getMission());

                        if(level == 4 && (houseID == HOUSE_HARKONNEN || houseID == HOUSE_ATREIDES || houseID == HOUSE_ORDOS)) {
                            fprintf(stdout, "playing meanwhile.....");fflush(stdout);
                            Meanwhile* pMeanwhile = new Meanwhile(houseID,true);
                            pMeanwhile->run();
                            delete pMeanwhile;
                            fprintf(stdout, "\t\tfinished\n"); fflush(stdout);
                        } else if(level == 8 && (houseID == HOUSE_HARKONNEN || houseID == HOUSE_ATREIDES || houseID == HOUSE_ORDOS)) {
                            fprintf(stdout, "playing meanwhile.....");fflush(stdout);
                            Meanwhile* pMeanwhile = new Meanwhile(houseID,false);
                            pMeanwhile->run();
                            delete pMeanwhile;
                            fprintf(stdout, "\t\tfinished\n"); fflush(stdout);
                        } else if(level == 9) {
                            fprintf(stdout, "playing finale.....");fflush(stdout);
                            Finale* pFinale = new Finale(houseID);
                            pFinale->run();
                            delete pFinale;
                            fprintf(stdout, "\t\tfinished\n"); fflush(stdout);
                        }
					}
				} break;

				case GAME_DEBRIEFING_LOST: {
					fprintf(stdout,"Debriefing...");
					fflush(stdout);
					BriefingMenu* pBriefing = new BriefingMenu(currentGameInitInfo.getHouseID(), currentGameInitInfo.getMission(), DEBRIEFING_LOST);
					pBriefing->showMenu();
					delete pBriefing;
					fprintf(stdout,"\t\t\tfinished\n");
					fflush(stdout);
				} break;

				case GAME_CUSTOM_GAME_STATS: {
					fprintf(stdout,"Game statistics...");
					fflush(stdout);
                    CustomGameStatsMenu* pCustomGameStats = new CustomGameStatsMenu();
					pCustomGameStats->showMenu();
					delete pCustomGameStats;
					fprintf(stdout,"\t\tfinished\n");
					fflush(stdout);
				} break;

				case GAME_LOAD:
				case GAME_NEXTMISSION: {
					currentGameInitInfo = currentGame->getNextGameInitSettings();
					delete currentGame;
					bGetNext = false;
				} break;

				case GAME_RETURN_TO_MENU:
				default: {
					delete currentGame;
					currentGame = NULL;

                    // Change music to menu music
                    musicPlayer->changeMusic(MUSIC_MENU);

					return;
				} break;
			}
		}


	}
}

/**
	Starts a new multiplayer game.
	\param init	contains all the information to start the game
*/
void startMultiPlayerGame(const GameInitSettings& init)
{
    GameInitSettings currentGameInitInfo = init;

    printf("Initing Game:\n");
    currentGame = new Game();
    currentGame->initGame(currentGameInitInfo);

    printf("Initialization finished!\n");
    fflush(stdout);

    // get init settings from game as it might have changed (through loading the game)
    currentGameInitInfo = currentGame->getGameInitSettings();

    currentGame->runMainLoop();

    if(currentGame->whatNext() == GAME_CUSTOM_GAME_STATS) {
        fprintf(stdout,"Game statistics...");
        fflush(stdout);
        CustomGameStatsMenu* pCustomGameStats = new CustomGameStatsMenu();
        pCustomGameStats->showMenu();
        delete pCustomGameStats;
        fprintf(stdout,"\t\tfinished\n");
        fflush(stdout);
    }

    delete currentGame;
    currentGame = NULL;
}
