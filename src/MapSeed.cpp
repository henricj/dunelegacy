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


#include <misc/SDL2pp.h>

#include "MapSeed.h"

// global seed value
static Uint32 Seed;

// a point that has 2 coordinates
typedef struct  {
    Uint16 x;
    Uint16 y;
} MapSeedPoint;

// some values
const Uint8 BoolArray[] = {0,1,0,0,1,1,1,1,0,0,1,1,1,1,0,0,0,0,1,0,0};

// some offsets
const Sint8 OffsetArray1[]={
    0,-1,1,-16,16,
    -17,17,-15,15,
    -2, 2,-32,32,
    -4, 4,-64,64,
    -30,30,-34,34
};

// some more offsets
const Uint8 OffsetArray2[]={
    0,0,4,0,4,0,4,4,0,0,0,4,0,4,4,4,0,0,0,2,0,
    2,0,4,0,0,2,0,2,0,4,0,4,0,4,2,4,2,4,4,0,4,
    2,4,2,4,4,4,0,0,4,4,2,0,2,2,0,0,2,2,4,0,2,
    2,0,2,2,2,2,2,4,2,2,2,0,4,2,2,4,4,2,2,2,4,
    0,0,4,0,4,0,4,4,0,0,0,4,0,4,4,4,0,0,0,2,0,
    2,0,4,0,0,2,0,2,0,4,0,4,0,4,2,4,2,4,4,0,4,
    2,4,2,4,4,4,4,0,0,4,2,0,2,2,0,0,2,2,4,0,2,
    2,0,2,2,2,2,2,4,2,2,2,0,4,2,2,4,4,2,2,2,4
};

// TileTypes
const Sint16 TileTypes[] = {
    220, 221, 222, 229, 230, 231, 213, 214, 215, 223, 224, 225, 232, 233, 234, 216,
    217, 218, 226, 227, 228, 235, 236, 237, 219, 217, 218, 226, 227, 228, 235, 236,
    237, 238, 239, 244, 245, 125, 240, 246, 247, 241, 242, 248, 249, 241, 243, 248,
    249, 241, 242, 248, 250, 241, 243, 248, 250, 251, 252, 253, 258, 259, 260, 223,
    224, 225, 232, 233, 234, 254, 255, 256, 261, 262, 263, 257, 255, 256, 261, 262,
    263, 254, 255, 256, 261, 264, 265, 257, 255, 256, 261, 264, 265, 254, 255, 256,
    261, 266, 267, 257, 255, 256, 261, 266, 267, 210, 268, 269, 273, 274, 275, 223,
    224, 225, 232, 233, 234, 270, 271, 272, 276, 277, 278, 270, 271, 272, 279, 277,
    278, 270, 271, 272, 276, 277, 278, 270, 271, 272, 279, 277, 278, 270, 271, 272,
    276, 277, 278, 270, 271, 272, 279, 277, 278, 238, 239, 244, 245, 125, 240, 246,
    247, 280, 281, 282, 283, 280, 281, 282, 284, 238, 239, 244, 245, 125, 240, 246,
    247, 285, 286, 288, 289, 287, 286, 290, 289, 143, 291, 295, 296, 125, 240, 246,
    247, 292, 293, 297, 298, 294, 293, 297, 298, 238, 239, 244, 245, 125, 240, 246,
    247, 299, 300, 301, 302, 299, 300, 301, 303, 238, 239, 244, 245, 125, 240, 246,
    247, 304, 305, 306, 307, 304, 305, 306, 308, 210, 211, 212, 220, 221, 222, 229,
    230, 231, 213, 214, 215, 223, 313, 225, 232, 233, 234, 309, 310, 311, 314, 315,
    316, 319, 320, 321, 312, 310, 311, 314, 315, 316, 319, 320, 321, 309, 310, 311
};

// sinus[index] = 127 * sin(pi * index/128)
static const Sint8 sinus[256] = {
    0, 3, 6, 9, 12, 15, 18, 21, 24, 27, 30, 33, 36, 39, 42, 45,
    48, 51, 54, 57, 59, 62, 65, 67, 70, 73, 75, 78, 80, 82, 85, 87,
    89, 91, 94, 96, 98, 100, 101, 103, 105, 107, 108, 110, 111, 113, 114, 116,
    117, 118, 119, 120, 121, 122, 123, 123, 124, 125, 125, 126, 126, 126, 126, 126,
    127, 126, 126, 126, 126, 126, 125, 125, 124, 123, 123, 122, 121, 120, 119, 118,
    117, 116, 114, 113, 112, 110, 108, 107, 105, 103, 102, 100, 98, 96, 94, 91,
    89, 87, 85, 82, 80, 78, 75, 73, 70, 67, 65, 62, 59, 57, 54, 51,
    48, 45, 42, 39, 36, 33, 30, 27, 24, 21, 18, 15, 12, 9, 6, 3,
    0, -3, -6, -9, -12, -15, -18, -21, -24, -27, -30, -33, -36, -39, -42, -45,
    -48, -51, -54, -57, -59, -62, -65, -67, -70, -73, -75, -78, -80, -82, -85, -87,
    -89, -91, -94, -96, -98, -100, -102, -103, -105, -107, -108, -110, -111, -113, -114, -116,
    -117, -118, -119, -120, -121, -122, -123, -123, -124, -125, -125, -126, -126, -126, -126, -126,
    -126, -126, -126, -126, -126, -126, -125, -125, -124, -123, -123, -122, -121, -120, -119, -118,
    -117, -116, -114, -113, -112, -110, -108, -107, -105, -103, -102, -100, -98, -96, -94, -91,
    -89, -87, -85, -82, -80, -78, -75, -73, -70, -67, -65, -62, -59, -57, -54, -51,
    -48, -45, -42, -39, -36, -33, -30, -27, -24, -21, -18, -15, -12, -9, -6, -3
};

/**
    Converts a 2d coordinate to a 1d.
    \param Xcoord   The coordinate in x-direction
    \param Ycoord   The coordinate in y-direction
    \return The 1-dimensional coordinate
*/
static Sint16 MapArray2DToMapArray1D(Sint16 Xcoord,Sint16 Ycoord) {
    return Xcoord | (Ycoord << 6);
}

/**
    Converts a 2d coordinate to a 1d. This function is basically the same
    as MapArray2DToMapArray1D() but limits the Xcoord to (0-63).
    \param Xcoord   The coordinate in x-direction
    \param Ycoord   The coordinate in y-direction
    \return The 1-dimensional coordinate
*/
static Sint16 MapArray2DToMapArray1D_OOB(Sint16 Xcoord,Sint16 Ycoord) {
    return MapArray2DToMapArray1D(Xcoord & 0x3F,Ycoord);
}


/**
    This function smooth the map in the local neighbourhood of index.
    \param  index       The position which should be smoothed
    \param  pMapArray   Pointer to the map that should be smoothed
    \return none
*/
static void SmoothNeighbourhood(Sint16 index, Uint32* pMapArray) {
    Sint16 TileType;
    Sint16 Xcoord;
    Sint16 Ycoord;
    Sint16 Pos;

    TileType = (Sint16) pMapArray[index];

    if(TileType == 8) {
        pMapArray[index] = 9;
        SmoothNeighbourhood(index,pMapArray);
    } else if (TileType == 9) {
        for(Ycoord = -1; Ycoord <= 1; Ycoord++) {
            for(Xcoord = -1; Xcoord <= 1; Xcoord++) {
                Pos = MapArray2DToMapArray1D( (index & 0x3F)+Xcoord,((index >> 6) & 0x3F)+Ycoord);
                if(Pos < 0)
                    continue;
                if(Pos >= 64*64)
                    continue;

                if(BoolArray[pMapArray[Pos]] == 1) {
                    pMapArray[index] = 8;
                    continue;
                } else {
                    if(pMapArray[Pos] == 9)
                        continue;

                    pMapArray[Pos]=8;
                }
            }
        }
    } else {
        if(BoolArray[TileType] == 0) {
            pMapArray[index] = 8;
        }
    }
}

/**
    Creates new random value.
    \return The new random value
*/
static Uint16 SeedRand() {
    Uint8 a;
    Uint8 carry;
    Uint8 old_carry;

    // little endian is more useful for this algorithm
    Seed = SDL_SwapLE32(Seed);
    Uint8* pSeed = (Uint8*) &Seed;

    // shift right
    a = pSeed[0];
    a = a >> 1;

    // shift right in carry
    carry = a & 0x01;
    a = a >> 1;

    // rotate left through carry
    old_carry = carry;
    carry = (pSeed[2] & 0x80) >> 7;
    pSeed[2] = pSeed[2] << 1;
    pSeed[2] = pSeed[2] | old_carry;

    // rotate left through carry
    old_carry = carry;
    carry = (pSeed[1] & 0x80) >> 7;
    pSeed[1] = pSeed[1] << 1;
    pSeed[1] = pSeed[1] | old_carry;

    // invert carry
    carry = (carry == 1 ? 0 : 1);

    // subtract with carry
    a = ((Uint16) a) - (((Uint16) pSeed[0]) + ((Uint16) carry));

    // shift right
    carry = a & 0x01;
    // a = a >> 1;  //< Not needed anymore

    // rotate right through carry
    pSeed[0] = (pSeed[0] >> 1) | (carry << 7);

    // xor
    a = pSeed[0] ^ pSeed[1];

    // convert back to native endianess
    Seed = SDL_SwapLE32(Seed);

    return ((Uint16) a);

}

/**
    Creates a random map.
    \param Para_Seed    Seed of the map
    \param pResultMap   Should be Uint16[64*64]
*/
void createMapWithSeed(Uint32 Para_Seed,Uint16 *pResultMap)
{
    Uint8 Array4x4TerrainGrid[16*16+16+1];
    Uint32 MapArray[65*65];
    MapSeedPoint point;
    Uint16 randNum;
    Uint16 randNum2;
    Uint16 randNum3;
    Sint16 index;
    Sint16 i,j;
    Sint16 Xcoord;
    Sint16 Ycoord;
    Uint16 max;
    Sint16 Point1;
    Sint16 Point2;
    Uint16 pos;

    Uint16 curMapRow[0x80];
    Uint16 oldMapRow[0x80];
    Uint32 Area[3][3];

    Seed = Para_Seed;

    // clear map
    memset(MapArray,0,sizeof(MapArray));

    for(i = 0; i < 16*16+16 ; i++) {
        Array4x4TerrainGrid[i] = SeedRand() & 0x0F;
        if(Array4x4TerrainGrid[i] <= 0x0A)
            continue;

        Array4x4TerrainGrid[i] = 0x0A;
    }

    for(i = SeedRand() & 0x0F;i >= 0 ;i--) {
        randNum = SeedRand() & 0xFF;
        for(j = 0; j < 21; j++) {
            index = randNum + OffsetArray1[j];
            index = index >= 0 ? index : 0;
            index = index <= (16*16+16) ? index : (16*16+16);
            Array4x4TerrainGrid[index] = ((Uint16) Array4x4TerrainGrid[index] + (SeedRand() & 0x0F)) & 0x0F;
        }
    }

    for(i = SeedRand() & 0x03; i >= 0; i--) {
        randNum = SeedRand() & 0xFF;
        for(j = 0; j < 21; j++) {
            index = randNum + OffsetArray1[j];
            index = index >= 0 ? index : 0;
            index = index <= (16*16+16) ? index : (16*16+16);
            Array4x4TerrainGrid[index] = SeedRand() & 0x03;
        }
    }

    for(Ycoord = 0; Ycoord < 64; Ycoord+=4) {
        for(Xcoord = 0; Xcoord < 64; Xcoord+=4) {
            MapArray[MapArray2DToMapArray1D(Xcoord,Ycoord)] = Array4x4TerrainGrid[Ycoord*4+Xcoord/4];
        }
    }

    for(Ycoord = 0; Ycoord < 64; Ycoord+=4) {
        for(Xcoord = 0; Xcoord < 64; Xcoord+=4) {
            for(i = (Xcoord % 8 == 0 ? 21 : 0) ; (Xcoord % 8 == 0 ? 21 : 0) + 21 > i; i++) {
                Point1 =  MapArray2DToMapArray1D(Xcoord + OffsetArray2[4*i],Ycoord + OffsetArray2[4*i+1]);
                Point2 =  MapArray2DToMapArray1D(Xcoord + OffsetArray2[4*i+2],Ycoord + OffsetArray2[4*i+3]);
                pos = (Point1 + Point2) / 2;

                if(pos >= 64*64)
                    continue;

                Point1 =  MapArray2DToMapArray1D_OOB(Xcoord + OffsetArray2[4*i],Ycoord + OffsetArray2[4*i+1]);
                Point2 =  MapArray2DToMapArray1D_OOB(Xcoord + OffsetArray2[4*i+2],Ycoord + OffsetArray2[4*i+3]);

                MapArray[pos] = (MapArray[Point1] + MapArray[Point2] + 1) / 2;
            }

        }
    }

    memset(curMapRow,0,sizeof(curMapRow));

    // apply box-filter to the map
    for(Ycoord = 0; Ycoord < 64; Ycoord++) {

        // save the old row
        memcpy(oldMapRow,curMapRow,sizeof(curMapRow));
        for(i = 0;i < 64; i++) {
            curMapRow[i] = (Uint16) MapArray[Ycoord*64+i];
        }

        for(Xcoord = 0; Xcoord < 64; Xcoord++) {

            Area[0][0] = ((Xcoord > 0) && (Ycoord > 0)) ? oldMapRow[Xcoord-1] : curMapRow[Xcoord];
            Area[1][0] = (Ycoord > 0) ? oldMapRow[Xcoord] : curMapRow[Xcoord];
            Area[2][0] = ((Xcoord < 63) && (Ycoord > 0)) ? oldMapRow[Xcoord+1] : curMapRow[Xcoord];

            Area[0][1] = (Xcoord > 0) ? curMapRow[Xcoord-1] : curMapRow[Xcoord];
            Area[1][1] = curMapRow[Xcoord];
            Area[2][1] = (Xcoord < 63) ? curMapRow[Xcoord+1] : curMapRow[Xcoord];

            Area[0][2] = ((Xcoord > 0) && (Ycoord < 63)) ? MapArray[(Ycoord+1)*64 + Xcoord-1] : curMapRow[Xcoord];
            Area[1][2] = (Ycoord < 63) ? MapArray[(Ycoord+1)*64 + Xcoord] : curMapRow[Xcoord];
            Area[2][2] = ((Xcoord < 63) && (Ycoord < 63)) ? MapArray[(Ycoord+1)*64 + Xcoord+1] : curMapRow[Xcoord];

            MapArray[Ycoord*64 + Xcoord]    =   (   Area[0][0] + Area[1][0] + Area[2][0]
                                                +   Area[0][1] + Area[1][1] + Area[1][2]
                                                +   Area[0][2] + Area[2][1] + Area[2][2] )/9;
        }
    }

    randNum = SeedRand() & 0x0F;
    randNum = (randNum < 8 ? 8 : randNum);
    randNum = (randNum > 0x0C ? 0x0C : randNum);
    point.y = (SeedRand() & 0x03) - 1;
    point.y = ( (randNum-3) < point.y ? randNum-3 : point.y);

    for(i = 0; i < 64*64; i++) {
        point.x = (Uint16) MapArray[i];

        if( (randNum+4) < point.x) {
            MapArray[i] = 0x06;
        } else if(point.x >= randNum) {
            MapArray[i] = 0x04;
        } else if(point.x <= point.y) {
            MapArray[i] = 0x02;
        } else {
            MapArray[i] = 0x00;
        }
    }

    for(i = SeedRand() & 0x2F; i != 0; i--) {
        point.y = SeedRand() & 0x3F;
        point.x = SeedRand() & 0x3F;
        index = MapArray2DToMapArray1D(point.x,point.y);

        if(BoolArray[MapArray[index]] == 1) {
            i++;
            continue;
        }


        randNum = SeedRand() & 0x1F;
        for(j=0; j < randNum; j++) {
            max = SeedRand() & 0x3F;

            if(max == 0) {
                pos = index;
            } else {
                point.y = ((index << 2) & 0xFF00) | 0x80;
                point.x = ((index & 0x3F) << 8) | 0x80;

                randNum2 = SeedRand() & 0xFF;

                while(randNum2 > max)
                    randNum2 = randNum2 >> 1;

                randNum3 = SeedRand() & 0xFF;

                point.x = point.x + (((sinus[randNum3] * randNum2) >> 7) << 4);
                point.y = point.y + ((((-1) * sinus[(randNum3+64) % 256] * randNum2) >> 7) << 4);


                if(/*(point.x < 0) || */(point.x > 0x4000) || /*(point.y < 0) ||*/ (point.y > 0x4000)) {
                    pos =  index;
                } else {
                    pos = ((point.y & 0xFF00) >> 2) | (point.x >> 8);
                }
            }

            if(pos >= 64*64) {
                j--;
                continue;
            }

            /*
            if(pos < 0) {
                j--;
                continue;
            }*/

            SmoothNeighbourhood(pos,MapArray);
        }
    }


    //smoothing
    for(i = 0; i < 64; i++) {
        curMapRow[i] = (Uint16) MapArray[i];
    }

    for(Ycoord = 0; Ycoord < 64; Ycoord++) {
        memcpy(oldMapRow,curMapRow,sizeof(curMapRow));

        for(i = 0; i < 64; i++) {
            curMapRow[i] = (Uint16) MapArray[Ycoord*64+i];
        }

        for(Xcoord = 0; Xcoord < 64; Xcoord++) {

            Area[1][0] = (Ycoord > 0) ? oldMapRow[Xcoord] : MapArray[Ycoord*64+Xcoord];
            Area[0][1] = (Xcoord > 0) ? curMapRow[Xcoord-1] : MapArray[Ycoord*64+Xcoord];
            Area[1][1] = MapArray[Ycoord*64+Xcoord];
            Area[2][1] = (Xcoord < 63) ? curMapRow[Xcoord+1] : MapArray[Ycoord*64+Xcoord];
            Area[1][2] = (Ycoord < 63) ? MapArray[(Ycoord+1)*64+Xcoord] : MapArray[Ycoord*64+Xcoord];

            MapArray[Ycoord*64+Xcoord] = 0;

            switch(Area[1][1]) {

            case 4:
                if( (Area[1][0] == 4) || (Area[1][0] == 6) )
                    MapArray[Ycoord*64+Xcoord] |= 0x01;

                if( (Area[2][1] == 4) || (Area[2][1] == 6) )
                    MapArray[Ycoord*64+Xcoord] |= 0x02;

                if( (Area[1][2] == 4) || (Area[1][2] == 6) )
                    MapArray[Ycoord*64+Xcoord] |= 0x04;

                if( (Area[0][1] == 4) || (Area[0][1] == 6) )
                    MapArray[Ycoord*64+Xcoord] |= 0x08;

                break;

            case 8:
                if( (Area[1][0] == 8) || (Area[1][0] == 9) )
                    MapArray[Ycoord*64+Xcoord] |= 0x01;

                if( (Area[2][1] == 8) || (Area[2][1] == 9) )
                    MapArray[Ycoord*64+Xcoord] |= 0x02;

                if( (Area[1][2] == 8) || (Area[1][2] == 9) )
                    MapArray[Ycoord*64+Xcoord] |= 0x04;

                if( (Area[0][1] == 8) || (Area[0][1] == 9) )
                    MapArray[Ycoord*64+Xcoord] |= 0x08;

                break;

            default:
                if(Area[1][0] == Area[1][1])
                    MapArray[Ycoord*64+Xcoord] |= 0x01;

                if(Area[2][1] == Area[1][1])
                    MapArray[Ycoord*64+Xcoord] |= 0x02;

                if(Area[1][2] == Area[1][1])
                    MapArray[Ycoord*64+Xcoord] |= 0x04;

                if(Area[0][1] == Area[1][1])
                    MapArray[Ycoord*64+Xcoord] |= 0x08;

                break;
            }

            if(Area[1][1] == 0)
                MapArray[Ycoord*64+Xcoord] = 0;

            if(Area[1][1] == 4)
                MapArray[Ycoord*64+Xcoord]++;

            if(Area[1][1] == 2)
                MapArray[Ycoord*64+Xcoord] += 0x11;

            if(Area[1][1] == 6)
                MapArray[Ycoord*64+Xcoord] += 0x21;

            if(Area[1][1] == 8)
                MapArray[Ycoord*64+Xcoord] += 0x31;

            if(Area[1][1] == 9)
                MapArray[Ycoord*64+Xcoord] += 0x41;
        }
    }

    //create resulting array
    for(i = 0; i < 64*64; i++) {
        MapArray[i] = (MapArray[i] & 0xFE00) | ((MapArray[i] <= 85) ? (MapArray[i]+127) : TileTypes[MapArray[i]-85]) | 0xF800;
        pResultMap[i] = MapArray[i] & 0x1FF;
    }
}

MapData createMapWithSeed(Uint32 Para_Seed, int mapscale) {
    Uint16 SeedMap[64*64];
    createMapWithSeed(Para_Seed,SeedMap);

    int sizeX = 0;
    int sizeY = 0;
    int logicalOffsetX = 0;
    int logicalOffsetY = 0;

    switch(mapscale) {
        case 0: {
            sizeX = 62;
            sizeY = 62;
            logicalOffsetX = 1;
            logicalOffsetY = 1;
        } break;

        case 1: {
            sizeX = 32;
            sizeY = 32;
            logicalOffsetX = 16;
            logicalOffsetY = 16;
        } break;

        case 2:
        default: {
            sizeX = 21;
            sizeY = 21;
            logicalOffsetX = 11;
            logicalOffsetY = 11;
        } break;
    }

    MapData mapData(sizeX, sizeY);

    for(int y = 0; y < sizeY; y++) {
        for(int x = 0; x < sizeX; x++) {

            TERRAINTYPE terrainType = Terrain_Sand;

            unsigned char seedmaptype = SeedMap[(y+logicalOffsetY)*64+x+logicalOffsetX] >> 4;
            switch(seedmaptype) {
                case 0x7: {
                    // Normal sand
                    terrainType = Terrain_Sand;
                } break;

                case 0x2:
                case 0x8: {
                    // Rock or building
                    terrainType = Terrain_Rock;
                } break;

                case 0x9: {
                    // Sand dunes
                    terrainType = Terrain_Dunes;
                } break;

                case 0xa: {
                    // Mountain
                    terrainType = Terrain_Mountain;
                } break;

                case 0xb: {
                    // Spice
                    terrainType = Terrain_Spice;
                } break;

                case 0xc: {
                    // Thick spice
                    terrainType = Terrain_ThickSpice;
                } break;
            }

            mapData(x,y) = terrainType;
        }
    }

    return mapData;
}
