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

#include <misc/Scaler.h>

#include <algorithm>

DoubleSurfaceFunction* Scaler::defaultDoubleSurface = Scaler::doubleSurfaceScale2x;
DoubleTiledSurfaceFunction* Scaler::defaultDoubleTiledSurface = Scaler::doubleTiledSurfaceScale2x;

DoubleSurfaceFunction* Scaler::defaultTripleSurface = Scaler::tripleSurfaceScale3x;
DoubleTiledSurfaceFunction* Scaler::defaultTripleTiledSurface = Scaler::tripleTiledSurfaceScale3x;


void Scaler::setDefaultScaler(Scaler::ScalerType scaler) {
    switch(scaler) {
        case Scaler::ScaleNN: {
            Scaler::defaultDoubleSurface = Scaler::doubleSurfaceNN;
            Scaler::defaultDoubleTiledSurface = Scaler::doubleTiledSurfaceNN;
            Scaler::defaultTripleSurface = Scaler::tripleSurfaceNN;
            Scaler::defaultTripleTiledSurface = Scaler::tripleTiledSurfaceNN;
        } break;

        case Scaler::Scale2x:
        default: {
            Scaler::defaultDoubleSurface = Scaler::doubleSurfaceScale2x;
            Scaler::defaultDoubleTiledSurface = Scaler::doubleTiledSurfaceScale2x;
            Scaler::defaultTripleSurface = Scaler::tripleSurfaceScale3x;
            Scaler::defaultTripleTiledSurface = Scaler::tripleTiledSurfaceScale3x;
        } break;
    }
}


/**
    This function doubles a surface by making 4 same-colored pixels out of one.
    \param	src				the source image
    \param	freeSrcSurface	true = free the source surface after scaling
    \return	the scaled surface (caller must free it with SDL_FreeSurface when finished)
*/
SDL_Surface* Scaler::doubleSurfaceNN(SDL_Surface* src, bool freeSrcSurface) {
	if(src == NULL) {
	    return NULL;
	}

	SDL_Surface *returnPic;

	// create new picture surface
	if((returnPic = SDL_CreateRGBSurface(0,src->w * 2,src->h * 2,8,0,0,0,0))== NULL) {
	    if(freeSrcSurface) SDL_FreeSurface(src);
		return NULL;
	}

    SDL_SetPaletteColors(returnPic->format->palette, src->format->palette->colors, 0, src->format->palette->ncolors);
    Uint32 ckey;
    bool has_ckey = !SDL_GetColorKey(src, &ckey);
    if (has_ckey) {
        SDL_SetColorKey(returnPic, SDL_TRUE, ckey);
    }
    if (src->flags & SDL_RLEACCEL) {
        SDL_SetSurfaceRLE(returnPic, SDL_TRUE);
    }

	SDL_LockSurface(returnPic);
	SDL_LockSurface(src);

	//Now we can copy pixel by pixel
	for(int y = 0; y < src->h;y++) {
		for(int x = 0; x < src->w; x++) {
			char val = *( ((char*) (src->pixels)) + y*src->pitch + x);
			*( ((char*) (returnPic->pixels)) + 2*y*returnPic->pitch + 2*x) = val;
			*( ((char*) (returnPic->pixels)) + 2*y*returnPic->pitch + 2*x+1) = val;
			*( ((char*) (returnPic->pixels)) + (2*y+1)*returnPic->pitch + 2*x) = val;
			*( ((char*) (returnPic->pixels)) + (2*y+1)*returnPic->pitch + 2*x+1) = val;
		}
	}

	SDL_UnlockSurface(src);
	SDL_UnlockSurface(returnPic);

    if(freeSrcSurface) SDL_FreeSurface(src);

	return returnPic;
}


/**
    This function is a wrapper around Scaler::doubleSurfaceNN.
    \param	src				the source image
    \param	tilesX			ignored
    \param	tilesY			ignored
    \param	freeSrcSurface	true = free the source surface after scaling
    \return	the scaled surface (caller must free it with SDL_FreeSurface when finished)
*/
SDL_Surface* Scaler::doubleTiledSurfaceNN(SDL_Surface* src, int tilesX, int tilesY, bool freeSrcSurface) {
    return doubleSurfaceNN(src, freeSrcSurface);
}


/**
    This function triples a surface by making 9 same-colored pixels out of one.
    \param	src				the source image
    \param	freeSrcSurface	true = free the source surface after scaling
    \return	the scaled surface (caller must free it with SDL_FreeSurface when finished)
*/
SDL_Surface* Scaler::tripleSurfaceNN(SDL_Surface* src, bool freeSrcSurface) {
	if(src == NULL) {
	    return NULL;
	}

	SDL_Surface *returnPic;

	// create new picture surface
	if((returnPic = SDL_CreateRGBSurface(0,src->w * 3,src->h * 3,8,0,0,0,0))== NULL) {
	    if(freeSrcSurface) SDL_FreeSurface(src);
		return NULL;
	}

	SDL_SetPaletteColors(returnPic->format->palette, src->format->palette->colors, 0, src->format->palette->ncolors);
    Uint32 ckey;
    bool has_ckey = !SDL_GetColorKey(src, &ckey);
    if (has_ckey) {
        SDL_SetColorKey(returnPic, SDL_TRUE, ckey);
    }
    if (src->flags & SDL_RLEACCEL) {
        SDL_SetSurfaceRLE(returnPic, SDL_TRUE);
    }

	SDL_LockSurface(returnPic);
	SDL_LockSurface(src);

	//Now we can copy pixel by pixel
	for(int y = 0; y < src->h;y++) {
		for(int x = 0; x < src->w; x++) {
			char val = *( ((char*) (src->pixels)) + y*src->pitch + x);
			*( ((char*) (returnPic->pixels)) + 3*y*returnPic->pitch + 3*x) = val;
			*( ((char*) (returnPic->pixels)) + 3*y*returnPic->pitch + 3*x+1) = val;
			*( ((char*) (returnPic->pixels)) + 3*y*returnPic->pitch + 3*x+2) = val;
			*( ((char*) (returnPic->pixels)) + (3*y+1)*returnPic->pitch + 3*x) = val;
			*( ((char*) (returnPic->pixels)) + (3*y+1)*returnPic->pitch + 3*x+1) = val;
			*( ((char*) (returnPic->pixels)) + (3*y+1)*returnPic->pitch + 3*x+2) = val;
			*( ((char*) (returnPic->pixels)) + (3*y+2)*returnPic->pitch + 3*x) = val;
			*( ((char*) (returnPic->pixels)) + (3*y+2)*returnPic->pitch + 3*x+1) = val;
			*( ((char*) (returnPic->pixels)) + (3*y+2)*returnPic->pitch + 3*x+2) = val;
		}
	}

	SDL_UnlockSurface(src);
	SDL_UnlockSurface(returnPic);

    if(freeSrcSurface) SDL_FreeSurface(src);

	return returnPic;
}


/**
    This function is a wrapper around Scaler::tripleSurfaceNN.
    \param	src				the source image
    \param	tilesX			ignored
    \param	tilesY			ignored
    \param	freeSrcSurface	true = free the source surface after scaling
    \return	the scaled surface (caller must free it with SDL_FreeSurface when finished)
*/
SDL_Surface* Scaler::tripleTiledSurfaceNN(SDL_Surface* src, int tilesX, int tilesY, bool freeSrcSurface) {
    return tripleSurfaceNN(src, freeSrcSurface);
}





/**
    This function doubles a surface while smoothing edges (see http://scale2x.sourceforge.net/algorithm.html ).
    \param	src				the source image
    \param	freeSrcSurface	true = free the source surface after scaling
    \return	the scaled surface (caller must free it with SDL_FreeSurface when finished)
*/
SDL_Surface* Scaler::doubleSurfaceScale2x(SDL_Surface* src, bool freeSrcSurface) {
    return doubleTiledSurfaceScale2x(src, 1, 1, freeSrcSurface);
}


/**
    This function doubles a surface while smoothing edges (see http://scale2x.sourceforge.net/algorithm.html ).
    \param	src				the source image
    \param	tilesX			number of subimages in x direction
    \param	tilesY			number of subimages in y direction
    \param	freeSrcSurface	true = free the source surface after scaling
    \return	the scaled surface (caller must free it with SDL_FreeSurface when finished)
*/
SDL_Surface* Scaler::doubleTiledSurfaceScale2x(SDL_Surface* src, int tilesX, int tilesY, bool freeSrcSurface) {
	if(src == NULL) {
		return NULL;
	}

	int srcWidth = src->w;
	int srcHeight = src->h;

	SDL_Surface* dest = SDL_CreateRGBSurface(0, srcWidth*2, srcHeight*2, 8, 0, 0, 0, 0);
	if(dest == NULL) {
        if(freeSrcSurface) SDL_FreeSurface(src);
		return NULL;
	}
    SDL_SetPaletteColors(dest->format->palette, src->format->palette->colors, 0, src->format->palette->ncolors);
    Uint32 ckey;
    bool has_ckey = !SDL_GetColorKey(src, &ckey);
    if (has_ckey) {
        SDL_SetColorKey(dest, SDL_TRUE, ckey);
    }
    if (src->flags & SDL_RLEACCEL) {
        SDL_SetSurfaceRLE(dest, SDL_TRUE);
    }

	Uint8* srcPixels = (Uint8*) src->pixels;
	Uint8* destPixels = (Uint8*) dest->pixels;

	int tileWidth = srcWidth / tilesX;
	int tileHeight = srcHeight / tilesY;

	for(int j=0;j<tilesY;++j) {
		for(int i=0;i<tilesX;++i) {



			/*

			Scale center pixel E into 4 new pixels

				Source			  Dest
			+---+---+---+
			| A | B | C |		+--+--+
			+---+---+---+		|E0|E1|
			| D | E | F |	->	+--+--+
			+---+---+---+		|E2|E3|
			| G | H | I |		+--+--+
			+---+---+---+

			*/

			for(int y = 0; y < tileHeight; ++y) {
				for(int x = 0; x < tileWidth; ++x) {
					Uint8 E = *( srcPixels + (j*tileHeight+y)*src->pitch + (i*tileWidth+x) );
					Uint8 B = *( srcPixels + (j*tileHeight+std::max(0,y-1))*src->pitch + (i*tileWidth+x) );
					Uint8 H = *( srcPixels + (j*tileHeight+std::min(tileHeight-1,y+1))*src->pitch + (i*tileWidth+x) );
					Uint8 D = *( srcPixels + (j*tileHeight+y)*src->pitch + (i*tileWidth+std::max(0,x-1)) );
					Uint8 F = *( srcPixels + (j*tileHeight+y)*src->pitch + (i*tileWidth+std::min(tileWidth-1,x+1)) );

					Uint8 E0, E1, E2, E3;

					if(B != H && D != F) {
						E0 = (D == B) ? D : E;
						E1 = (B == F) ? F : E;
						E2 = (D == H) ? D : E;
						E3 = (H == F) ? F : E;
					} else {
						E0 = E;
						E1 = E;
						E2 = E;
						E3 = E;
					}

					*(destPixels + (j*tileHeight+y)*2*dest->pitch + (i*tileWidth+x)*2) = E0;
					*(destPixels + (j*tileHeight+y)*2*dest->pitch + (i*tileWidth+x)*2 + 1) = E1;
					*(destPixels + ((j*tileHeight+y)*2+1)*dest->pitch + (i*tileWidth+x)*2) = E2;
					*(destPixels + ((j*tileHeight+y)*2+1)*dest->pitch + (i*tileWidth+x)*2 + 1) = E3;
				}
			}

		}
	}

	if(freeSrcSurface) {
		SDL_FreeSurface(src);
	}

	return dest;
}


/**
    This function triples a surface while smoothing edges (see http://scale2x.sourceforge.net/algorithm.html ).
    \param	src				the source image
    \param	freeSrcSurface	true = free the source surface after scaling
    \return	the scaled surface (caller must free it with SDL_FreeSurface when finished)
*/
SDL_Surface* Scaler::tripleSurfaceScale3x(SDL_Surface* src, bool freeSrcSurface) {
    return tripleTiledSurfaceScale3x(src, 1, 1, freeSrcSurface);
}


/**
    This function triples a surface while smoothing edges (see http://scale2x.sourceforge.net/algorithm.html ).
    \param	src				the source image
    \param	tilesX			number of subimages in x direction
    \param	tilesY			number of subimages in y direction
    \param	freeSrcSurface	true = free the source surface after scaling
    \return	the scaled surface (caller must free it with SDL_FreeSurface when finished)
*/
SDL_Surface* Scaler::tripleTiledSurfaceScale3x(SDL_Surface* src, int tilesX, int tilesY, bool freeSrcSurface) {
	if(src == NULL) {
		return NULL;
	}

	int srcWidth = src->w;
	int srcHeight = src->h;

	SDL_Surface* dest = SDL_CreateRGBSurface(0, srcWidth*3, srcHeight*3, 8, 0, 0, 0, 0);
	if(dest == NULL) {
	    if(freeSrcSurface) SDL_FreeSurface(src);
		return NULL;
	}

	Uint32 ckey;
    bool has_ckey = !SDL_GetColorKey(src, &ckey);
    if (has_ckey) {
        SDL_SetColorKey(dest, SDL_TRUE, ckey);
    }
    if (src->flags & SDL_RLEACCEL) {
        SDL_SetSurfaceRLE(dest, SDL_TRUE);
    }

	Uint8* srcPixels = (Uint8*) src->pixels;
	Uint8* destPixels = (Uint8*) dest->pixels;

	int tileWidth = srcWidth / tilesX;
	int tileHeight = srcHeight / tilesY;

	for(int j=0;j<tilesY;++j) {
		for(int i=0;i<tilesX;++i) {



			/*

			Scale center pixel E into 4 new pixels

				Source			   Dest
			+---+---+---+		+--+--+--+
			| A | B | C |		|E0|E1|E2|
			+---+---+---+		+--+--+--+
			| D | E | F |	->	|E3|E4|E5|
			+---+---+---+		+--+--+--+
			| G | H | I |		|E6|E7|E8|
			+---+---+---+		+--+--+--+

			*/

			for(int y = 0; y < tileHeight; ++y) {
				for(int x = 0; x < tileWidth; ++x) {
					Uint8 A = *( srcPixels + (j*tileHeight+std::max(0,y-1))*src->pitch + (i*tileWidth+std::max(0,x-1)) );
					Uint8 B = *( srcPixels + (j*tileHeight+std::max(0,y-1))*src->pitch + (i*tileWidth+x) );
					Uint8 C = *( srcPixels + (j*tileHeight+std::max(0,y-1))*src->pitch + (i*tileWidth+std::min(tileWidth-1,x+1)) );
					Uint8 D = *( srcPixels + (j*tileHeight+y)*src->pitch + (i*tileWidth+std::max(0,x-1)) );
					Uint8 E = *( srcPixels + (j*tileHeight+y)*src->pitch + (i*tileWidth+x) );
					Uint8 F = *( srcPixels + (j*tileHeight+y)*src->pitch + (i*tileWidth+std::min(tileWidth-1,x+1)) );
					Uint8 G = *( srcPixels + (j*tileHeight+std::min(tileHeight-1,y+1))*src->pitch + (i*tileWidth+std::max(0,x-1)) );
					Uint8 H = *( srcPixels + (j*tileHeight+std::min(tileHeight-1,y+1))*src->pitch + (i*tileWidth+x) );
					Uint8 I = *( srcPixels + (j*tileHeight+std::min(tileHeight-1,y+1))*src->pitch + (i*tileWidth+std::min(tileWidth-1,x+1)) );

					Uint8 E0, E1, E2, E3, E4, E5, E6, E7, E8;

					if(B != H && D != F) {
						E0 = (D == B) ? D : E;
						E1 = (((D == B) && (E != C)) || ((B == F) && (E != A))) ? B : E;
						E2 = (B == F) ? F : E;
						E3 = (((D == B && E != G)) || ((D == H) && (E != A))) ? D : E;
						E4 = E;
						E5 = (((B == F) && (E != I)) || ((H == F) && (E != C))) ? F : E;
						E6 = (D == H) ? D : E;
						E7 = (((D == H) && (E != I)) || ((H == F) && (E != G))) ? H : E;
						E8 = (H == F) ? F : E;
					} else {
						E0 = E;
						E1 = E;
						E2 = E;
						E3 = E;
						E4 = E;
						E5 = E;
						E6 = E;
						E7 = E;
						E8 = E;
					}

					*(destPixels + (j*tileHeight+y)*3*dest->pitch + (i*tileWidth+x)*3) = E0;
					*(destPixels + (j*tileHeight+y)*3*dest->pitch + (i*tileWidth+x)*3 + 1) = E1;
					*(destPixels + (j*tileHeight+y)*3*dest->pitch + (i*tileWidth+x)*3 + 2) = E2;
					*(destPixels + ((j*tileHeight+y)*3+1)*dest->pitch + (i*tileWidth+x)*3) = E3;
					*(destPixels + ((j*tileHeight+y)*3+1)*dest->pitch + (i*tileWidth+x)*3 + 1) = E4;
					*(destPixels + ((j*tileHeight+y)*3+1)*dest->pitch + (i*tileWidth+x)*3 + 2) = E5;
					*(destPixels + ((j*tileHeight+y)*3+2)*dest->pitch + (i*tileWidth+x)*3) = E6;
					*(destPixels + ((j*tileHeight+y)*3+2)*dest->pitch + (i*tileWidth+x)*3 + 1) = E7;
					*(destPixels + ((j*tileHeight+y)*3+2)*dest->pitch + (i*tileWidth+x)*3 + 2) = E8;
				}
			}

		}
	}

	if(freeSrcSurface) {
		SDL_FreeSurface(src);
	}

	return dest;
}


