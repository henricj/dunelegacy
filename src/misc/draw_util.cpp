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
#include <misc/exceptions.h>

#include <globals.h>


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
        return COLOR_RGBA(r,g,b,a);

    default:
        THROW(std::runtime_error, "getPixel(): Invalid bpp value!");
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
    int min = x1;
    int max = x2;

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
    int min = y1;
    int max = y2;

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
        int min = x1;
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

sdl2::surface_ptr renderReadSurface(SDL_Renderer* renderer) {
    assert(renderer == ::renderer);
    const SDL_Rect rendererSize = getRendererSize();
    sdl2::surface_ptr pScreen{ SDL_CreateRGBSurface(0, rendererSize.w, rendererSize.h, SCREEN_BPP, RMASK, GMASK, BMASK, AMASK) };
    if((pScreen == nullptr) || (SDL_RenderReadPixels(renderer, nullptr, SCREEN_FORMAT, pScreen->pixels, pScreen->pitch) != 0)) {
        SDL_Log("Warning: renderReadSurface() failed: %s", SDL_GetError());
        return nullptr;
    }

    SDL_version version;
    SDL_GetVersion(&version);

    if((version.major <= 2) && (version.minor <= 0) && (version.patch <= 4)) {
        // Fix bug in SDL2 OpenGL Backend (SDL bug #2740 and #3350) in SDL version <= 2.0.4
        SDL_RendererInfo rendererInfo;
        SDL_GetRendererInfo(renderer, &rendererInfo);
        if(strcmp(rendererInfo.name, "opengl") == 0) {
            if(SDL_GetRenderTarget(renderer) != nullptr) {
                return flipHSurface(pScreen.get());
            }
        }
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


sdl2::surface_ptr copySurface(SDL_Surface* inSurface) {
    //return SDL_DisplayFormat(inSurface);
    sdl2::surface_ptr surface{ SDL_ConvertSurface(inSurface, inSurface->format, inSurface->flags) };
    if( surface == nullptr) {
        THROW(std::invalid_argument, std::string("copySurface(): SDL_ConvertSurface() failed: ") + std::string(SDL_GetError()));
    }

    SDL_BlendMode mode;
    SDL_GetSurfaceBlendMode(inSurface, &mode);
    SDL_SetSurfaceBlendMode(surface.get(), mode);

    return surface;
}


sdl2::surface_ptr convertSurfaceToDisplayFormat(SDL_Surface* inSurface) {

    sdl2::surface_ptr pSurface{ SDL_ConvertSurfaceFormat(inSurface, SCREEN_FORMAT, 0) };
    if(pSurface == nullptr) {
        THROW(std::invalid_argument, std::string("convertSurfaceToDisplayFormat(): SDL_ConvertSurfaceFormat() failed: ") + std::string(SDL_GetError()));
    }

    return pSurface;
}


sdl2::texture_ptr convertSurfaceToTexture(SDL_Surface* inSurface) {
    if(inSurface == nullptr) {
        return nullptr;
    }

    if(inSurface->w <= 0 || inSurface->h <= 0) {
        return nullptr;
    }

    if(inSurface->w > 2048 || inSurface->h > 2048) {
        SDL_Log("Warning: Size of texture created in convertSurfaceToTexture is %dx%d; may exceed hardware limits on older GPUs!", inSurface->w, inSurface->h);
    }

    sdl2::texture_ptr pTexture{ SDL_CreateTextureFromSurface(renderer, inSurface) };

    if( pTexture == nullptr) {
        THROW(std::invalid_argument, std::string("convertSurfaceToTexture(): SDL_CreateTextureFromSurface() failed: ") + std::string(SDL_GetError()));
    }

    return pTexture;
}


sdl2::surface_ptr scaleSurface(SDL_Surface *surf, double ratio) {

    sdl2::surface_ptr scaled{ SDL_CreateRGBSurface(0, static_cast<int>(surf->w * ratio),static_cast<int>(surf->h * ratio),8,0,0,0,0) };
    if(scaled == nullptr) {
        return nullptr;
    }
    SDL_SetPaletteColors(scaled->format->palette, surf->format->palette->colors, 0, surf->format->palette->ncolors);
    Uint32 ckey;
    const auto has_ckey = !SDL_GetColorKey(surf, &ckey);
    if (has_ckey) {
        SDL_SetColorKey(scaled.get(), SDL_TRUE, ckey);
    }
    if (surf->flags & SDL_RLEACCEL) {
        SDL_SetSurfaceRLE(scaled.get(), SDL_TRUE);
    }

    sdl2::surface_lock lock_scaled{ scaled.get() };
    sdl2::surface_lock lock_surf{ surf };

    int X2 = static_cast<int>(surf->w * ratio);
    int Y2 = static_cast<int>(surf->h * ratio);

    for(int x = 0;x < X2;++x)
        for(int y = 0;y < Y2;++y)
            putPixel(scaled.get(),x,y,getPixel(surf,static_cast<int>(x / ratio), static_cast<int>(y / ratio)));

    return scaled;
}


sdl2::surface_ptr getSubPicture(SDL_Surface* pic, int left, int top, int width, int height) {
    if(pic == nullptr) {
        THROW(std::invalid_argument, "getSubPicture(): pic == nullptr!");
    }

    sdl2::surface_ptr returnPic;

    // create new picture surface
    if(pic->format->BitsPerPixel == 8) {
        returnPic = sdl2::surface_ptr{ SDL_CreateRGBSurface(0, width, height, 8, 0, 0, 0, 0) };
        if(returnPic == nullptr) {
            THROW(std::runtime_error, "getSubPicture(): Cannot create new Picture!");
        }
        SDL_SetPaletteColors(returnPic->format->palette, pic->format->palette->colors, 0, pic->format->palette->ncolors);
        Uint32 ckey;
        const auto has_ckey = !SDL_GetColorKey(pic, &ckey);
        if (has_ckey) {
            SDL_SetColorKey(returnPic.get(), SDL_TRUE, ckey);
        }
        if (pic->flags & SDL_RLEACCEL) {
            SDL_SetSurfaceRLE(returnPic.get(), SDL_TRUE);
        }
    } else {
        returnPic = sdl2::surface_ptr{ SDL_CreateRGBSurface(0, width, height, 32, RMASK, GMASK, BMASK, AMASK) };
        if(returnPic == nullptr) {
            THROW(std::runtime_error, "getSubPicture(): Cannot create new Picture!");
        }
    }

    SDL_Rect srcRect = {left,top,width,height};
    SDL_BlitSurface(pic,&srcRect,returnPic.get(),nullptr);

    return returnPic;
}


sdl2::surface_ptr getSubFrame(SDL_Surface* pic, int i, int j, int numX, int numY) {
    if(pic == nullptr) {
        THROW(std::invalid_argument, "getSubFrame(): pic == nullptr!");
    }

    const auto frameWidth = pic->w/numX;
    const auto frameHeight = pic->h/numY;

    return getSubPicture(pic, frameWidth*i, frameHeight*j, frameWidth, frameHeight);
}


sdl2::surface_ptr combinePictures(SDL_Surface* basePicture, SDL_Surface* topPicture, int x, int y) {
    if((basePicture == nullptr) || (topPicture == nullptr)) {
        return nullptr;
    }

    sdl2::surface_ptr dest{ copySurface(basePicture) };
    if(dest == nullptr) {
        return nullptr;
    }

    SDL_Rect destRect = calcDrawingRect(topPicture, x, y);
    SDL_BlitSurface(topPicture, nullptr, dest.get(), &destRect);

    return dest;
}



sdl2::surface_ptr rotateSurfaceLeft(SDL_Surface* inputPic) {
    if (inputPic == nullptr) {
        THROW(std::invalid_argument, "rotateSurface(): inputPic == nullptr!");
    }

    // create new picture surface
    sdl2::surface_ptr returnPic{ SDL_CreateRGBSurface(0, inputPic->h, inputPic->w, 8, 0, 0, 0, 0) };
    if (returnPic == nullptr) {
        THROW(std::runtime_error, "rotateSurface(): Cannot create new Picture!");
    }

    SDL_SetPaletteColors(returnPic->format->palette, inputPic->format->palette->colors, 0, inputPic->format->palette->ncolors);
    Uint32 ckey;
    const auto has_ckey = !SDL_GetColorKey(inputPic, &ckey);
    if (has_ckey) {
        SDL_SetColorKey(returnPic.get(), SDL_TRUE, ckey);
    }
    if (inputPic->flags & SDL_RLEACCEL) {
        SDL_SetSurfaceRLE(returnPic.get(), SDL_TRUE);
    }

    sdl2::surface_lock lock_pic{ returnPic.get() };
    sdl2::surface_lock lock_input{ inputPic };

    //Now we can copy pixel by pixel
    for(int y = 0; y < inputPic->h;y++) {
        for(int x = 0; x < inputPic->w; x++) {
            char val = *( ((char*) (inputPic->pixels)) + y*inputPic->pitch + x);
            *( ((char*) (returnPic->pixels)) + (returnPic->h - x - 1)*returnPic->pitch + y) = val;
        }
    }

    return returnPic;
}


sdl2::surface_ptr rotateSurfaceRight(SDL_Surface* inputPic) {
    if (inputPic == nullptr) {
        THROW(std::invalid_argument, "rotateSurface(): inputPic == nullptr!");
    }

    // create new picture surface
    sdl2::surface_ptr returnPic{ SDL_CreateRGBSurface(0, inputPic->h, inputPic->w, 8, 0, 0, 0, 0) };
    if (returnPic == nullptr) {
        THROW(std::runtime_error, "rotateSurface(): Cannot create new Picture!");
    }

    SDL_SetPaletteColors(returnPic->format->palette, inputPic->format->palette->colors, 0, inputPic->format->palette->ncolors);
    Uint32 ckey;
    const auto has_ckey = !SDL_GetColorKey(inputPic, &ckey);
    if (has_ckey) {
        SDL_SetColorKey(returnPic.get(), SDL_TRUE, ckey);
    }
    if (inputPic->flags & SDL_RLEACCEL) {
        SDL_SetSurfaceRLE(returnPic.get(), SDL_TRUE);
    }

    sdl2::surface_lock lock_pic{ returnPic.get() };
    sdl2::surface_lock lock_input{ inputPic };

    //Now we can copy pixel by pixel
    for(int y = 0; y < inputPic->h;y++) {
        for(int x = 0; x < inputPic->w; x++) {
            char val = *( ((char*) (inputPic->pixels)) + y*inputPic->pitch + x);
            *( ((char*) (returnPic->pixels)) + x*returnPic->pitch + (returnPic->w - y - 1)) = val;
        }
    }

    return returnPic;
}


sdl2::surface_ptr flipHSurface(SDL_Surface* inputPic) {
    if (inputPic == nullptr) {
        THROW(std::invalid_argument, "flipHSurface(): inputPic == nullptr!");
    }

    sdl2::surface_ptr returnPic;

    // create new picture surface
    if (inputPic->format->BitsPerPixel == 8) {
        returnPic = sdl2::surface_ptr{ SDL_CreateRGBSurface(0, inputPic->w, inputPic->h, 8, 0, 0, 0, 0) };
        if (returnPic == nullptr) {
            THROW(std::runtime_error, "flipHSurface(): Cannot create new Picture!");
        }
        SDL_SetPaletteColors(returnPic->format->palette, inputPic->format->palette->colors, 0, inputPic->format->palette->ncolors);
        Uint32 ckey;
        const auto has_ckey = !SDL_GetColorKey(inputPic, &ckey);
        if (has_ckey) {
            SDL_SetColorKey(returnPic.get(), SDL_TRUE, ckey);
        }
        if (inputPic->flags & SDL_RLEACCEL) {
            SDL_SetSurfaceRLE(returnPic.get(), SDL_TRUE);
        }
    }
    else {
        returnPic = sdl2::surface_ptr{ SDL_CreateRGBSurface(0, inputPic->w, inputPic->h, 32, RMASK, GMASK, BMASK, AMASK) };
        if (returnPic == nullptr) {
            THROW(std::runtime_error, "flipHSurface(): Cannot create new Picture!");
        }
    }

    sdl2::surface_lock lock_pic{ returnPic.get() };
    sdl2::surface_lock lock_input{ inputPic };

    //Now we can copy pixel by pixel
    for(int y = 0; y < inputPic->h;y++) {
        for(int x = 0; x < inputPic->w; x++) {
            putPixel(returnPic.get(), x, inputPic->h - y - 1, getPixel(inputPic, x, y));
        }
    }

    return returnPic;
}


sdl2::surface_ptr flipVSurface(SDL_Surface* inputPic) {
    if (inputPic == nullptr) {
        THROW(std::invalid_argument, "flipHSurface(): inputPic == nullptr!");
    }

    sdl2::surface_ptr returnPic;

    // create new picture surface
    if (inputPic->format->BitsPerPixel == 8) {
        returnPic = sdl2::surface_ptr{ SDL_CreateRGBSurface(0, inputPic->w, inputPic->h, 8, 0, 0, 0, 0) };
        if (returnPic == nullptr) {
            THROW(std::runtime_error, "flipHSurface(): Cannot create new Picture!");
        }
        SDL_SetPaletteColors(returnPic->format->palette, inputPic->format->palette->colors, 0, inputPic->format->palette->ncolors);
        Uint32 ckey;
        const auto has_ckey = !SDL_GetColorKey(inputPic, &ckey);
        if (has_ckey) {
            SDL_SetColorKey(returnPic.get(), SDL_TRUE, ckey);
        }
        if (inputPic->flags & SDL_RLEACCEL) {
            SDL_SetSurfaceRLE(returnPic.get(), SDL_TRUE);
        }
    }
    else {
        returnPic = sdl2::surface_ptr{ SDL_CreateRGBSurface(0, inputPic->w, inputPic->h, 32, RMASK, GMASK, BMASK, AMASK) };
        if (returnPic == nullptr) {
            THROW(std::runtime_error, "flipHSurface(): Cannot create new Picture!");
        }
    }

    sdl2::surface_lock lock_pic{ returnPic.get() };
    sdl2::surface_lock lock_input{ inputPic };

    //Now we can copy pixel by pixel
    for(int y = 0; y < inputPic->h;y++) {
        for(int x = 0; x < inputPic->w; x++) {
            putPixel(returnPic.get(), inputPic->w - x - 1, y, getPixel(inputPic, x, y));
        }
    }

    return returnPic;
}


sdl2::surface_ptr createShadowSurface(SDL_Surface* source) {
    if(source == nullptr) {
        THROW(std::invalid_argument, "createShadowSurface(): source == nullptr!");
    }

    sdl2::surface_ptr retPic{ SDL_ConvertSurface(source, source->format, source->flags) };

    if(retPic == nullptr) {
        THROW(std::runtime_error, "createShadowSurface(): Cannot copy image!");
    }

    if(retPic->format->BytesPerPixel == 1) {
        SDL_SetSurfaceBlendMode(retPic.get(), SDL_BLENDMODE_NONE);
    }

    sdl2::surface_lock lock{ retPic.get() };

    for (auto j = 0; j < retPic->h; ++j) {
        Uint8 * const RESTRICT p = static_cast<Uint8*>(retPic->pixels) + j * retPic->pitch;
        for(auto i = 0; i < retPic->w; ++i) {
            if(p[i] != PALCOLOR_TRANSPARENT) {
                p[i] = PALCOLOR_BLACK;
            }
        }
    }

    SDL_Color transparent = { 0, 0, 0, 128 };
    SDL_SetPaletteColors(retPic->format->palette, &transparent, PALCOLOR_BLACK, 1);

    return retPic;
}


sdl2::surface_ptr mapSurfaceColorRange(SDL_Surface* source, int srcColor, int destColor) {
    if (!source)
        THROW(std::runtime_error, "mapSurfaceColorRange(): Null source!");

    sdl2::surface_ptr retPic{ SDL_ConvertSurface(source,source->format,source->flags) };

    if (!source)
        THROW(std::runtime_error, "mapSurfaceColorRange(): Cannot copy image!");

    if (retPic->format->BytesPerPixel == 1) {
        SDL_SetSurfaceBlendMode(retPic.get(), SDL_BLENDMODE_NONE);
    }

    sdl2::surface_lock lock{ retPic.get() };

    for(auto y = 0; y < retPic->h; ++y) {
        Uint8* RESTRICT p = static_cast<Uint8*>(retPic->pixels) + y * retPic->pitch;
        for(auto x = 0; x < retPic->w; ++x, ++p) {
            if ((*p >= srcColor) && (*p < srcColor + 7))
                *p = *p - srcColor + destColor;
        }
    }

    return retPic;
}

