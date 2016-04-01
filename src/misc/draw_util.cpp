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

#include <misc/draw_util.h>

#include <globals.h>

#include <stdexcept>


Uint32 getPixel(SDL_Surface *surface, int x, int y) {
    int bpp = surface->format->BytesPerPixel;
    /* Here p is the address to the pixel we want to retrieve */
    Uint8 *p = (Uint8 *)surface->pixels + (y * surface->pitch) + (x * bpp);

    switch(bpp) {
    case 1:
        return *p;

    case 2:
        return *(Uint16 *)p;

    case 3:
        if(SDL_BYTEORDER == SDL_BIG_ENDIAN)
            return p[0] << 16 | p[1] << 8 | p[2];
        else
            return p[0] | p[1] << 8 | p[2] << 16;

    case 4:
        Uint32 value;
        value = *(Uint32 *)p;
        Uint8 r,g,b,a;
        SDL_GetRGBA(value,surface->format,&r,&g,&b,&a);
        return RGBA(r,g,b,a);

    default:
        throw std::runtime_error("getPixel(): Invalid bpp value!");
    }
}


void putPixel(SDL_Surface *surface, int x, int y, Uint32 color) {
	if(x >= 0 && x < surface->w && y >=0 && y < surface->h) {
		int bpp = surface->format->BytesPerPixel;
		/* Here p is the address to the pixel want to set */
		Uint8 *p = (Uint8 *)surface->pixels + y * surface->pitch + x * bpp;

		switch(bpp) {
		case 1:
			*p = color;
			break;

		case 2:
			*(Uint16 *)p = color;
			break;

		case 3:
			if(SDL_BYTEORDER == SDL_BIG_ENDIAN) {
				p[0] = (color>> 16) & 0xff;
				p[1] = (color>> 8) & 0xff;
				p[2] = color& 0xff;
			} else {
				p[0] = color& 0xff;
				p[1] = (color>> 8) & 0xff;
				p[2] = (color>> 16) & 0xff;
			}
			break;

		case 4:
			*(Uint32 *)p = MapRGBA(surface->format, color);
			break;
		}
	}
}


void drawHLineNoLock(SDL_Surface *surface, int x1, int y, int x2, Uint32 color) {
	int	min = x1;
	int	max = x2;

	if(min > max) {
		int temp = max;
		max = min;
		min = temp;
	}

	for(int i = min; i <= max; i++) {
		putPixel(surface, i, y, color);
	}
}


void drawVLineNoLock(SDL_Surface *surface, int x, int y1, int y2, Uint32 color) {
	int	min = y1;
	int	max = y2;

	if(min > max) {
		int temp = max;
		max = min;
		min = temp;
	}

	for(int i = min; i <= max; i++) {
		putPixel(surface, x, i, color);
	}
}


void drawHLine(SDL_Surface *surface, int x1, int y, int x2, Uint32 color) {
	if(!SDL_MUSTLOCK(surface) || (SDL_LockSurface(surface) == 0)) {
		drawHLineNoLock(surface, x1, y, x2, color);

		if(SDL_MUSTLOCK(surface)) {
			SDL_UnlockSurface(surface);
		}
	}
}


void drawVLine(SDL_Surface *surface, int x, int y1, int y2, Uint32 color) {
	if(!SDL_MUSTLOCK(surface) || (SDL_LockSurface(surface) == 0)) {
		drawVLineNoLock(surface, x, y1, y2, color);

		if (SDL_MUSTLOCK(surface)) {
			SDL_UnlockSurface(surface);
		}
	}
}


void drawRect(SDL_Surface *surface, int x1, int y1, int x2, int y2, Uint32 color) {
	if(!SDL_MUSTLOCK(surface) || (SDL_LockSurface(surface) == 0)) {
		int	min = x1;
		int max = x2;

		if(min > max) {
			int temp = max;
			max = min;
			min = temp;
		}

		for(int i = min; i <= max; i++) {
			putPixel(surface, i, y1, color);
			putPixel(surface, i, y2, color);
		}

		min = y1+1;
		max = y2;
		if(min > max) {
			int temp = max;
			max = min;
			min = temp;
		}

		for(int j = min; j < max; j++) {
			putPixel(surface, x1, j, color);
			putPixel(surface, x2, j, color);
		}

		if(SDL_MUSTLOCK(surface)) {
			SDL_UnlockSurface(surface);
		}
	}
}

SDL_Surface* renderReadSurface(SDL_Renderer* renderer) {
    SDL_Rect rendererSize = getRendererSize();
    SDL_Surface* pScreen = SDL_CreateRGBSurface(0, rendererSize.w, rendererSize.h, SCREEN_BPP, RMASK, GMASK, BMASK, AMASK);
    if((pScreen == NULL) || (SDL_RenderReadPixels(renderer, NULL, SCREEN_FORMAT, pScreen->pixels, pScreen->pitch) != 0)) {
        fprintf(stderr,"renderReadSurface() failed: %s\n", SDL_GetError());
        SDL_FreeSurface(pScreen);
        return NULL;
	}
    return pScreen;
}

void replaceColor(SDL_Surface *surface, Uint32 oldColor, Uint32 newColor) {
	if(!SDL_MUSTLOCK(surface) || (SDL_LockSurface(surface) == 0)) {
        for(int y = 0; y < surface->h; y++) {
            Uint8 *p = (Uint8 *)surface->pixels + (y * surface->pitch);

            for(int x = 0; x < surface->w; x++, ++p) {
                Uint32 color = getPixel(surface, x, y);
                if(color == oldColor) {
                    putPixel(surface, x, y, newColor);
                }
            }
        }

        if(SDL_MUSTLOCK(surface)) {
			SDL_UnlockSurface(surface);
		}
	}
}

void mapColor(SDL_Surface *surface, Uint8 colorMap[256]) {
	if(!SDL_MUSTLOCK(surface) || (SDL_LockSurface(surface) == 0)) {
        for(int y = 0; y < surface->h; y++) {
            Uint8 *p = (Uint8 *)surface->pixels + (y * surface->pitch);

            for(int x = 0; x < surface->w; x++, ++p) {
                *p = colorMap[*p];
            }
        }

        if(SDL_MUSTLOCK(surface)) {
			SDL_UnlockSurface(surface);
		}
	}
}


SDL_Surface* copySurface(SDL_Surface* inSurface) {
	//return SDL_DisplayFormat(inSurface);
	SDL_Surface *surface;
	if( (surface = SDL_ConvertSurface(inSurface, inSurface->format, inSurface->flags)) == NULL) {
        throw std::invalid_argument(std::string("copySurface(): SDL_ConvertSurface() failed: ") + std::string(SDL_GetError()));
	}

	SDL_BlendMode mode;
	SDL_GetSurfaceBlendMode(inSurface, &mode);
    SDL_SetSurfaceBlendMode(surface, mode);

	return surface;
}


SDL_Surface* convertSurfaceToDisplayFormat(SDL_Surface* inSurface, bool freeSrcSurface) {
    SDL_Surface* pSurface;
    if( (pSurface = SDL_ConvertSurfaceFormat(inSurface, SCREEN_FORMAT, 0)) == NULL) {
        if(freeSrcSurface) {
            SDL_FreeSurface(inSurface);
        }
        throw std::invalid_argument(std::string("convertSurfaceToDisplayFormat(): SDL_ConvertSurfaceFormat() failed: ") + std::string(SDL_GetError()));
	}
    if(freeSrcSurface) {
        SDL_FreeSurface(inSurface);
    }
    return pSurface;
}


SDL_Texture* convertSurfaceToTexture(SDL_Surface* inSurface, bool freeSrcSurface) {
    if(inSurface == NULL) {
        return NULL;
    }

    if(inSurface->w <= 0 || inSurface->h <= 0) {
        if(freeSrcSurface) {
            SDL_FreeSurface(inSurface);
        }
        return NULL;
    }

    SDL_Texture* pTexture;
    if( (pTexture = SDL_CreateTextureFromSurface(renderer, inSurface)) == NULL) {
        if(freeSrcSurface) {
            SDL_FreeSurface(inSurface);
        }
        throw std::invalid_argument(std::string("convertSurfaceToTexture(): SDL_CreateTextureFromSurface() failed: ") + std::string(SDL_GetError()));
	}
    if(freeSrcSurface) {
        SDL_FreeSurface(inSurface);
    }
    return pTexture;
}


SDL_Surface* scaleSurface(SDL_Surface *surf, double ratio, bool freeSrcSurface) {
	SDL_Surface *scaled = SDL_CreateRGBSurface(0, (int) (surf->w * ratio),(int) (surf->h * ratio),8,0,0,0,0);
    if(scaled == NULL) {
        if(freeSrcSurface) {
            SDL_FreeSurface(surf);
        }

        return NULL;
    }
    SDL_SetPaletteColors(scaled->format->palette, surf->format->palette->colors, 0, surf->format->palette->ncolors);
    Uint32 ckey;
    bool has_ckey = !SDL_GetColorKey(surf, &ckey);
    if (has_ckey) {
        SDL_SetColorKey(scaled, SDL_TRUE, ckey);
    }
    if (surf->flags & SDL_RLEACCEL) {
        SDL_SetSurfaceRLE(scaled, SDL_TRUE);
    }

	SDL_LockSurface(scaled);
	SDL_LockSurface(surf);

	int X2 = (int)(surf->w * ratio);
	int Y2 = (int)(surf->h * ratio);

	for(int x = 0;x < X2;++x)
		for(int y = 0;y < Y2;++y)
			putPixel(scaled,x,y,getPixel(surf,(int) (x/ratio), (int) (y/ratio)));

	SDL_UnlockSurface(scaled);
	SDL_UnlockSurface(surf);

	if(freeSrcSurface) {
		SDL_FreeSurface(surf);
	}

	return scaled;
}


SDL_Surface* getSubPicture(SDL_Surface* pic, int left, int top, int width, int height) {
	if(pic == NULL) {
	    throw std::invalid_argument("getSubPicture(): pic == NULL!");
	}

	SDL_Surface *returnPic;

	// create new picture surface
    if(pic->format->BitsPerPixel == 8) {
        if((returnPic = SDL_CreateRGBSurface(0, width, height, 8, 0, 0, 0, 0))== NULL) {
            throw std::runtime_error("getSubPicture(): Cannot create new Picture!");
        }
        SDL_SetPaletteColors(returnPic->format->palette, pic->format->palette->colors, 0, pic->format->palette->ncolors);
        Uint32 ckey;
        bool has_ckey = !SDL_GetColorKey(pic, &ckey);
        if (has_ckey) {
            SDL_SetColorKey(returnPic, SDL_TRUE, ckey);
        }
        if (pic->flags & SDL_RLEACCEL) {
            SDL_SetSurfaceRLE(returnPic, SDL_TRUE);
        }
	} else {
        if((returnPic = SDL_CreateRGBSurface(0, width, height, 32, RMASK, GMASK, BMASK, AMASK))== NULL) {
            throw std::runtime_error("getSubPicture(): Cannot create new Picture!");
        }
	}

	SDL_Rect srcRect = {left,top,width,height};
	SDL_BlitSurface(pic,&srcRect,returnPic,NULL);

	return returnPic;
}

SDL_Surface* getSubFrame(SDL_Surface* pic, int i, int j, int numX, int numY) {
	if(pic == NULL) {
	    throw std::invalid_argument("getSubFrame(): pic == NULL!");
	}

	int frameWidth = pic->w/numX;
	int frameHeight = pic->h/numY;

	return getSubPicture(pic, frameWidth*i, frameHeight*j, frameWidth, frameHeight);
}

SDL_Surface* combinePictures(SDL_Surface* basePicture, SDL_Surface* topPicture, int x, int y, bool bFreeBasePicture, bool bFreeTopPicture) {

    if((basePicture == NULL) || (topPicture == NULL)) {
        if(bFreeBasePicture) SDL_FreeSurface(basePicture);
        if(bFreeTopPicture) SDL_FreeSurface(topPicture);
        return NULL;
    }

    SDL_Surface* dest = copySurface(basePicture);
    if(dest == NULL) {
        if(bFreeBasePicture) SDL_FreeSurface(basePicture);
        if(bFreeTopPicture) SDL_FreeSurface(topPicture);
        return NULL;
    }

    SDL_Rect destRect = calcDrawingRect(topPicture, x, y);
	SDL_BlitSurface(topPicture, NULL, dest, &destRect);

	if(bFreeBasePicture) SDL_FreeSurface(basePicture);
    if(bFreeTopPicture) SDL_FreeSurface(topPicture);

	return dest;
}


SDL_Surface* rotateSurfaceLeft(SDL_Surface* inputPic, bool bFreeInputPic) {
	if(inputPic == NULL) {
		throw std::invalid_argument("rotateSurfaceLeft(): inputPic == NULL!");
	}

	SDL_Surface *returnPic;

	// create new picture surface
	if((returnPic = SDL_CreateRGBSurface(0,inputPic->h,inputPic->w,8,0,0,0,0))== NULL) {
	    if(bFreeInputPic) SDL_FreeSurface(inputPic);
		throw std::runtime_error("rotateSurfaceLeft(): Cannot create new Picture!");
	}

	SDL_SetPaletteColors(returnPic->format->palette, inputPic->format->palette->colors, 0, inputPic->format->palette->ncolors);
	Uint32 ckey;
	bool has_ckey = !SDL_GetColorKey(inputPic, &ckey);
	if (has_ckey) {
	    SDL_SetColorKey(returnPic, SDL_TRUE, ckey);
    }
	if (inputPic->flags & SDL_RLEACCEL) {
            SDL_SetSurfaceRLE(returnPic, SDL_TRUE);
    }

	SDL_LockSurface(returnPic);
	SDL_LockSurface(inputPic);

	//Now we can copy pixel by pixel
	for(int y = 0; y < inputPic->h;y++) {
		for(int x = 0; x < inputPic->w; x++) {
			char val = *( ((char*) (inputPic->pixels)) + y*inputPic->pitch + x);
			*( ((char*) (returnPic->pixels)) + (returnPic->h - x - 1)*returnPic->pitch + y) = val;
		}
	}

	SDL_UnlockSurface(inputPic);
	SDL_UnlockSurface(returnPic);

    if(bFreeInputPic) SDL_FreeSurface(inputPic);

	return returnPic;
}


SDL_Surface* rotateSurfaceRight(SDL_Surface* inputPic, bool bFreeInputPic) {
	if(inputPic == NULL) {
		throw std::invalid_argument("rotateSurfaceRight(): inputPic == NULL!");
	}

	SDL_Surface *returnPic;

	// create new picture surface
	if((returnPic = SDL_CreateRGBSurface(0,inputPic->h,inputPic->w,8,0,0,0,0))== NULL) {
	    if(bFreeInputPic) SDL_FreeSurface(inputPic);
		throw std::runtime_error("rotateSurfaceRight(): Cannot create new Picture!");
	}

	SDL_SetPaletteColors(returnPic->format->palette, inputPic->format->palette->colors, 0, inputPic->format->palette->ncolors);
	Uint32 ckey;
	bool has_ckey = !SDL_GetColorKey(inputPic, &ckey);
	if (has_ckey) {
	    SDL_SetColorKey(returnPic, SDL_TRUE, ckey);
    }
	if (inputPic->flags & SDL_RLEACCEL) {
            SDL_SetSurfaceRLE(returnPic, SDL_TRUE);
    }

	SDL_LockSurface(returnPic);
	SDL_LockSurface(inputPic);

	//Now we can copy pixel by pixel
	for(int y = 0; y < inputPic->h;y++) {
		for(int x = 0; x < inputPic->w; x++) {
			char val = *( ((char*) (inputPic->pixels)) + y*inputPic->pitch + x);
			*( ((char*) (returnPic->pixels)) + x*returnPic->pitch + (returnPic->w - y - 1)) = val;
		}
	}

	SDL_UnlockSurface(inputPic);
	SDL_UnlockSurface(returnPic);

    if(bFreeInputPic) SDL_FreeSurface(inputPic);

	return returnPic;
}


SDL_Surface* flipHSurface(SDL_Surface* inputPic, bool bFreeInputPic) {
	if(inputPic == NULL) {
		throw std::invalid_argument("flipHSurface(): inputPic == NULL!");
	}

	SDL_Surface *returnPic;

    // create new picture surface
    if(inputPic->format->BitsPerPixel == 8) {
        if((returnPic = SDL_CreateRGBSurface(0, inputPic->w, inputPic->h, 8, 0, 0, 0, 0))== NULL) {
            throw std::runtime_error("flipHSurface(): Cannot create new Picture!");
        }
        SDL_SetPaletteColors(returnPic->format->palette, inputPic->format->palette->colors, 0, inputPic->format->palette->ncolors);
        Uint32 ckey;
        bool has_ckey = !SDL_GetColorKey(inputPic, &ckey);
        if (has_ckey) {
            SDL_SetColorKey(returnPic, SDL_TRUE, ckey);
        }
        if (inputPic->flags & SDL_RLEACCEL) {
            SDL_SetSurfaceRLE(returnPic, SDL_TRUE);
        }
	} else {
        if((returnPic = SDL_CreateRGBSurface(0, inputPic->w, inputPic->h, 32, RMASK, GMASK, BMASK, AMASK))== NULL) {
            throw std::runtime_error("flipHSurface(): Cannot create new Picture!");
        }
	}

	SDL_LockSurface(returnPic);
	SDL_LockSurface(inputPic);

	//Now we can copy pixel by pixel
	for(int y = 0; y < inputPic->h;y++) {
		for(int x = 0; x < inputPic->w; x++) {
            putPixel(returnPic, x, inputPic->h - y - 1, getPixel(inputPic, x, y));
		}
	}

	SDL_UnlockSurface(inputPic);
	SDL_UnlockSurface(returnPic);

    if(bFreeInputPic) SDL_FreeSurface(inputPic);

	return returnPic;
}


SDL_Surface* flipVSurface(SDL_Surface* inputPic, bool bFreeInputPic) {
	if(inputPic == NULL) {
		throw std::invalid_argument("flipVSurface(): inputPic == NULL!");
	}

	SDL_Surface *returnPic;

    // create new picture surface
    if(inputPic->format->BitsPerPixel == 8) {
        if((returnPic = SDL_CreateRGBSurface(0, inputPic->w, inputPic->h, 8, 0, 0, 0, 0))== NULL) {
            throw std::runtime_error("flipVSurface(): Cannot create new Picture!");
        }
        SDL_SetPaletteColors(returnPic->format->palette, inputPic->format->palette->colors, 0, inputPic->format->palette->ncolors);
        Uint32 ckey;
        bool has_ckey = !SDL_GetColorKey(inputPic, &ckey);
        if (has_ckey) {
            SDL_SetColorKey(returnPic, SDL_TRUE, ckey);
        }
        if (inputPic->flags & SDL_RLEACCEL) {
            SDL_SetSurfaceRLE(returnPic, SDL_TRUE);
        }
	} else {
        if((returnPic = SDL_CreateRGBSurface(0, inputPic->w, inputPic->h, 32, RMASK, GMASK, BMASK, AMASK))== NULL) {
            throw std::runtime_error("flipVSurface(): Cannot create new Picture!");
        }
	}

	SDL_LockSurface(returnPic);
	SDL_LockSurface(inputPic);

	//Now we can copy pixel by pixel
	for(int y = 0; y < inputPic->h;y++) {
		for(int x = 0; x < inputPic->w; x++) {
            putPixel(returnPic, inputPic->w - x - 1, y, getPixel(inputPic, x, y));
		}
	}

	SDL_UnlockSurface(inputPic);
	SDL_UnlockSurface(returnPic);

    if(bFreeInputPic) SDL_FreeSurface(inputPic);

	return returnPic;
}


SDL_Surface* createShadowSurface(SDL_Surface* source) {
    if(source == NULL) {
	    throw std::invalid_argument("createShadowSurface(): source == NULL!");
	}

	SDL_Surface *retPic;

	if((retPic = SDL_ConvertSurface(source,source->format,source->flags)) == NULL) {
	    throw std::runtime_error("createShadowSurface(): Cannot copy image!");
	}

	if(retPic->format->BytesPerPixel == 1) {
        SDL_SetSurfaceBlendMode(retPic, SDL_BLENDMODE_NONE);
    }

	if(SDL_LockSurface(retPic) != 0) {
	    SDL_FreeSurface(retPic);
	    throw std::runtime_error("createShadowSurface(): Cannot lock image!");
	}

	for(int i = 0; i < retPic->w; i++) {
		for(int j = 0; j < retPic->h; j++) {
			Uint8* pixel = &((Uint8*)retPic->pixels)[j * retPic->pitch + i];
			if(*pixel != PALCOLOR_TRANSPARENT) {
                *pixel = PALCOLOR_BLACK;
			}
		}
	}
	SDL_UnlockSurface(retPic);

	return retPic;
}


SDL_Surface* mapSurfaceColorRange(SDL_Surface* source, int srcColor, int destColor, bool bFreeSource) {
	SDL_Surface *retPic;

    if(bFreeSource) {
        retPic = source;
    } else {
        if((retPic = SDL_ConvertSurface(source,source->format,source->flags)) == NULL) {
            throw std::runtime_error("mapSurfaceColorRange(): Cannot copy image!");
        }
    }

    if (retPic->format->BytesPerPixel == 1) {
        SDL_SetSurfaceBlendMode(retPic, SDL_BLENDMODE_NONE);
    }

	if(SDL_LockSurface(retPic) != 0) {
	    SDL_FreeSurface(retPic);
	    throw std::runtime_error("mapSurfaceColorRange(): Cannot lock image!");
	}

    for(int y = 0; y < retPic->h; ++y) {
        Uint8* p = (Uint8*) retPic->pixels + y * retPic->pitch;
        for(int x = 0; x < retPic->w; ++x, ++p) {
			if ((*p >= srcColor) && (*p < srcColor + 7))
				*p = *p - srcColor + destColor;
		}
	}
	SDL_UnlockSurface(retPic);

	return retPic;
}
