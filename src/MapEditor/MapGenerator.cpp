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

#include <MapEditor/MapGenerator.h>

#include <globals.h>

#include <misc/Random.h>


#define ROCKFILLER 2        //how many times random generator will try to remove sand "holes" for rock from the map
#define SPICEFILLER 2       //for spice
#define DUNESFILLER 1

class MapGenerator {

public:

    MapGenerator(int sizeX, int sizeY, int randSeed, int rockfields = ROCKFIELDS, int spicefields = SPICEFIELDS, MirrorMode mirrorMode = MirrorModeNone)
     : map(sizeX, sizeY), randGen(randSeed), rockfields(rockfields), spicefields(spicefields) {
         mapMirror = MapMirror::createMapMirror(mirrorMode, sizeX, sizeY);
    }

    MapData& getMap() {
        return map;
    }

    /**
        Creates a random map
    */
    void generateMap() {
        // the whole map shall be of type Terrain_Sand

        for(int i = 0; i < rockfields; i++) {
            int spotX = randGen.rand(0, map.getSizeX()-1);
            int spotY = randGen.rand(0, map.getSizeY()-1);

            makeSpot(spotX, spotY, Terrain_Rock);
        }

        for(int i = 0; i < ROCKFILLER; i++) {
            thickSpots(Terrain_Rock); //SPOT ROCK
        }

        // Spice fields
        for(int i = 0; i < spicefields; i++) {
            int spotX = randGen.rand(0, map.getSizeX()-1);
            int spotY = randGen.rand(0, map.getSizeY()-1);

            makeSpot(spotX, spotY, Terrain_Spice);
        }

        for(int i = 0; i < SPICEFILLER; i++) {
            thickSpots(Terrain_Spice);
        }

        for(int i = 0; i < SPICEFILLER; i++) {
            thickThickSpiceSpots();
        }

        // Spice fields
        for(int i = 0; i < DUNEFIELDS; i++) {
            int spotX = randGen.rand(0, map.getSizeX()-1);
            int spotY = randGen.rand(0, map.getSizeY()-1);

            makeSpot(spotX, spotY, Terrain_Dunes);
        }

        for(int i = 0; i < DUNESFILLER; i++) {
            thickSpots(Terrain_Dunes);
        }

        addRockBits(randGen.rand(0,9));
        addSpiceBlooms(randGen.rand(0,9));


    }

private:

    bool fixTileCoordinate(int& x, int& y) {
        bool error = false;

        if(x < 0) {
            x = 0;
            error = true;
        } else if(x >= map.getSizeX()) {
            x = map.getSizeX() - 1;
            error = true;
        }

        if(y < 0) {
            y = 0;
            error = true;
        } else if(y >= map.getSizeY()) {
            y = map.getSizeY() - 1;
            error = true;
        }

        return error;
    }

    /**
        Checks if the tile to the left of (x,y) is of type tile
        \param x    x-coordinate in tile coordinates
        \param y    y-coordinate in tile coordinates
        \param type  the tile type to check
        \return true if of tile type, false otherwise
    */
    bool onLeft(int x, int y, TERRAINTYPE type) {
        x--;
        fixTileCoordinate(x, y);

        return (map(x,y) == type);
    }

    /**
        Checks if the tile to the right of (x,y) is of type tile
        \param x x-coordinate in tile coordinates
        \param y y-coordinate in tile coordinates
        \param type  the tile type to check
        \return true if of tile type, false otherwise
    */
    bool onRight(int x, int y, TERRAINTYPE type) {
        x++;
        fixTileCoordinate(x, y);

        return (map(x,y) == type);
    }

    /**
        Checks if the tile above (x,y) is of type tile
        \param x    x-coordinate in tile coordinates
        \param y    y-coordinate in tile coordinates
        \param type  the tile type to check
        \return true if of tile type, false otherwise
    */
    bool onUp(int x, int y, TERRAINTYPE type) {
        y--;
        fixTileCoordinate(x, y);

        return (map(x,y) == type);
    }

    /**
        Checks if the tile below (x,y) is of type tile
        \param x    x-coordinate in tile coordinates
        \param y    y-coordinate in tile coordinates
        \param type  the tile type to check
        \return true if of tile type, false otherwise
    */
    bool onDown(int x, int y, TERRAINTYPE type) {
        y++;
        fixTileCoordinate(x, y);

        return (map(x,y) == type);
    }

    /**
        Count how many tiles around (x,y) are of type tile
        \param x    x-coordinate in tile coordinates
        \param y    y-coordinate in tile coordinates
        \param type  the tile type to check
        \return number of surounding tiles of tile type (0 to 4)
    */
    int side4(int x, int y, TERRAINTYPE type) {
        // Check at 4 sides for 'tile'
        int flag = 0;

        if(onLeft(x, y, type))      flag++;
        if(onRight(x, y, type))     flag++;
        if(onUp(x, y, type))        flag++;
        if(onDown(x, y, type))      flag++;

        return flag;
    }



    /**
        Removes holes in rock and spice
        \param type  the type to remove holes from
    */
    void thickSpots(TERRAINTYPE type) {
        for(int i = 0; i < map.getSizeX(); i++) {
            for(int j = 0; j < map.getSizeY(); j++) {
                if(map(i,j) != type) {
                    // Found something else than what thickining

                    if(side4(i, j, type) >= 3) {
                        // Seems enough of the type around it so make this also of this type
                        for(int m=0; m < mapMirror->getSize(); m++) {
                            Coord position = mapMirror->getCoord(Coord(i, j), m);
                            map(position.x,position.y) = type;
                        }
                    }

                    if(side4(i, j, type) == 2) {
                        // Gamble, fifty fifty... set this type or not?
                        if(randGen.rand(0,1) == 1) {
                            for(int m=0; m < mapMirror->getSize(); m++) {
                                Coord position = mapMirror->getCoord(Coord(i, j), m);
                                map(position.x,position.y) = type;
                            }
                        }
                    }
                }
            }
        }
    }


    /**
        Removes holes in thick spice
    */
    void thickThickSpiceSpots() {
        for(int i = 0; i < map.getSizeX(); i++) {
            for(int j = 0; j < map.getSizeY(); j++) {

                int numSpiceTiles = side4(i,j,Terrain_Spice)+side4(i,j,Terrain_ThickSpice);

                if(map(i,j) != Terrain_ThickSpice && (numSpiceTiles>=4)) {
                    // Found something else than what thickining

                    if(side4(i, j, Terrain_ThickSpice) >= 3) {
                        // Seems enough of ThickSpice around it so make this also ThickSpice
                        for(int m=0; m < mapMirror->getSize(); m++) {
                            Coord position = mapMirror->getCoord(Coord(i, j), m);
                            map(position.x,position.y) = Terrain_ThickSpice;
                        }
                    }

                    if(side4(i, j, Terrain_ThickSpice) == 2) {
                        // Gamble, fifty fifty... set this to ThickSpice or not?
                        if(randGen.rand(0,1) == 1) {
                            for(int m=0; m < mapMirror->getSize(); m++) {
                                Coord position = mapMirror->getCoord(Coord(i, j), m);
                                map(position.x,position.y) = Terrain_ThickSpice;
                            }
                        }
                    }
                }
            }
        }
    }

    /**
        This function creates a spot of type type.
        \param x        the x coordinate in tile coordinates to start making the spot
        \param y        the y coordinate in tile coordinates to start making the spot
        \param type      type of the spot
    */
    void makeSpot(int x, int y, TERRAINTYPE type) {
        int spotSize = (640 * map.getSizeX()*map.getSizeY())/(64*64);
        for(int j = 0; j < spotSize; j++) {
            int dir = randGen.rand(0,3);    // Random Dir

            switch(dir) {
                case 0 : x--; break;
                case 1 : x++; break;
                case 2 : y--; break;
                case 3 : y++; break;
            }

            fixTileCoordinate(x, y);

            TERRAINTYPE type2Place = type;

            if(type == Terrain_Spice) {
                if(map(x,y) == Terrain_Rock) {
                    // Do not place the spice spot, priority is ROCK!
                    continue;
                } else if((map(x,y) == Terrain_Spice) && ((side4(x,y,Terrain_Spice)+side4(x,y,Terrain_ThickSpice)) >= 4)) {
                    // "upgrade" spice to thick spice
                    type2Place = Terrain_ThickSpice;
                } else if(map(x,y) == Terrain_ThickSpice) {
                    // do not "downgrade" thick spice to spice
                    type2Place = Terrain_ThickSpice;
                }
            } else if(type == Terrain_Dunes) {
                if(map(x,y) != Terrain_Sand) {
                    // Do not place the dunes spot, priority is ROCK and SPICE!
                    continue;
                }
            }



            for(int m=0; m < mapMirror->getSize(); m++) {
                Coord position = mapMirror->getCoord(Coord(x, y), m);
                map(position.x,position.y) = type2Place;
            }
        }
    }


    /**
        Adds amount number of rock tiles to the map
        \param amount the number of rock tiles to add
    */
    void addRockBits(int amount) {
        int done = 0;
        for(int j = 0; (done < amount) && (j < 1000) ; j++) {
            int spotX = randGen.rand(0, map.getSizeX()-1);
            int spotY = randGen.rand(0, map.getSizeY()-1);

            if(map(spotX, spotY) == Terrain_Sand) {
                for(int m=0; m < mapMirror->getSize(); m++) {
                    Coord position = mapMirror->getCoord(Coord(spotX, spotY), m);
                    map(position.x,position.y) = Terrain_Rock;
                }
                done++;
            }
        }
    }

    /**
        Adds amount number of spice blooms to the map
        \param amount the number of spice blooms to add
    */
    void addSpiceBlooms(int amount) {
        int done = 0;
        for(int j = 0; (done < amount) && (j < 1000) ; j++) {
            int spotX = randGen.rand(0, map.getSizeX()-1);
            int spotY = randGen.rand(0, map.getSizeY()-1);

            if(map(spotX, spotY) == Terrain_Sand) {
                for(int m=0; m < mapMirror->getSize(); m++) {
                    Coord position = mapMirror->getCoord(Coord(spotX, spotY), m);
                    map(position.x,position.y) = Terrain_SpiceBloom;
                }
                done++;
            }
        }
    }

private:
    MapData map;

    Random randGen;

    int rockfields;
    int spicefields;

    std::unique_ptr<MapMirror>      mapMirror;
};

/**
    Creates a random map
    \param sizeX        width of the new map (in tiles)
    \param sizeY        height of the new map (in tiles)
    \param randSeed     the seed value for the random generator
    \param rockfields   num rock fields to add
    \param spicefields  num spice fields to add
    \return the generated map
*/
MapData generateRandomMap(int sizeX, int sizeY, int randSeed, int rockfields, int spicefields, MirrorMode mirrorMode) {
    MapGenerator mapGenerator(sizeX, sizeY, randSeed, rockfields, spicefields, mirrorMode);

    mapGenerator.generateMap();

    return mapGenerator.getMap();
}
