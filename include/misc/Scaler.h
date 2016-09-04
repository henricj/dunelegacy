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

#ifndef SCALER_H
#define SCALER_H

#include <SDL.h>

#include <string>

typedef SDL_Surface* DoubleSurfaceFunction(SDL_Surface*, bool);
typedef SDL_Surface* DoubleTiledSurfaceFunction(SDL_Surface*, int, int, bool);

class Scaler {
public:

    typedef enum {
        Invalid = -1,
        ScaleHD = 0,
        Scale2x = 1,
        ScaleNN = 2,
        NumScaler = 3
    } ScalerType;

    static DoubleSurfaceFunction*       defaultDoubleSurface;
    static DoubleTiledSurfaceFunction*  defaultDoubleTiledSurface;

    static DoubleSurfaceFunction*       defaultTripleSurface;
    static DoubleTiledSurfaceFunction*  defaultTripleTiledSurface;


    static void setDefaultScaler(Scaler::ScalerType scaler);

    static std::string getScalerName(Scaler::ScalerType scaler) {
        switch(scaler) {
            case Scaler::ScaleHD:   return "ScaleHD";
            case Scaler::Scale2x:   return "Scale2x";
            case Scaler::ScaleNN:   return "ScaleNN";
            default:        return "Unknown";
        }
    };

    static Scaler::ScalerType getScalerByName(std::string scalerName) {
        if(scalerName == "ScaleHD") {
            return Scaler::ScaleHD;
        } else if(scalerName == "Scale2x") {
            return Scaler::Scale2x;
        } else if(scalerName == "ScaleNN") {
            return Scaler::ScaleNN;
        } else {
            return Scaler::Invalid;
        }
    };


    // nearest neighbor scaling
    static SDL_Surface* doubleSurfaceNN(SDL_Surface* src, bool freeSrcSurface = true);
    static SDL_Surface* doubleTiledSurfaceNN(SDL_Surface* src, int tilesX = 1, int tilesY = 1, bool freeSrcSurface = true);

    static SDL_Surface* tripleSurfaceNN(SDL_Surface* src, bool freeSrcSurface = true);
    static SDL_Surface* tripleTiledSurfaceNN(SDL_Surface* src, int tilesX = 1, int tilesY = 1, bool freeSrcSurface = true);


    // scale2x and scale3x algorithms (see http://scale2x.sourceforge.net/algorithm.html)
    static SDL_Surface* doubleSurfaceScale2x(SDL_Surface* src, bool freeSrcSurface = true);
    static SDL_Surface* doubleTiledSurfaceScale2x(SDL_Surface* src, int tilesX = 1, int tilesY = 1, bool freeSrcSurface = true);

    static SDL_Surface* tripleSurfaceScale3x(SDL_Surface* src, bool freeSrcSurface = true);
    static SDL_Surface* tripleTiledSurfaceScale3x(SDL_Surface* src, int tilesX = 1, int tilesY = 1, bool freeSrcSurface = true);

};

#endif // SCALER_H
