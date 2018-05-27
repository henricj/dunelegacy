#ifndef BLENDBLITTER_H
#define BLENDBLITTER_H

#include <misc/SDL2pp.h>
#include <misc/draw_util.h>
#include <misc/exceptions.h>
#include <mmath.h>

#include <stdio.h>
#include <stdlib.h>

// Step by step one more pixel of the source image is blitted to the destination image
class BlendBlitter {
public:
    BlendBlitter(sdl2::surface_ptr SrcPic, SDL_Surface* DestPic, SDL_Rect DestPicRect, int numSteps = 50) {
        src = std::move(SrcPic);
        dest = DestPic;
        destRect = DestPicRect;
        this->numSteps = numSteps;

        N = static_cast<Uint64>(src->w)*static_cast<Uint64>(src->h);
        // compute next greater 2^x value
        m = N;
        m |= (m >> 1);
        m |= (m >> 2);
        m |= (m >> 4);
        m |= (m >> 8);
        m |= (m >> 16);
        m |= (m >> 32);
        m++;

        c = getRandomInt(0, (int) ((m/2)-1)) * 2 + 1;  // c is any odd number from [0;m]
        a = getRandomInt(0, (int) ((m/4)-1)) * 4 + 1;  // (a-1) is divisible by all prime factors of log_2(m), and 4

        currentValue = getRandomInt(0, (int) (m-1));

        StepsLeft = numSteps;
    }

    ~BlendBlitter() = default;

    Uint64 getNextValue() {
        do {
            currentValue = (a*currentValue + c) % m;
        } while (currentValue >= N);

        return currentValue;
    }

    /**
        Blits the next pixel to the destination surface
        \return The number of steps to do
    */
    int nextStep() {
        if(StepsLeft <= 0) {
            return 0;
        }
        StepsLeft--;

        sdl2::surface_lock lock_dest{ dest };
        sdl2::surface_lock lock_src{ src.get() };

        Uint64 numPixelsPerStep = (N / numSteps) + 1;
        for(Uint64 i=0;i<numPixelsPerStep;i++) {
            Uint64 cur = getNextValue();

            int x = (int) (cur % src->w);
            int y = (int) (cur / src->w);

            Uint32 color = getPixel(src.get(), x, y);

            if(color != 0) {

                if( (destRect.x + x < dest->w) && (destRect.x + x >= 0) &&
                    (destRect.x + x <= destRect.x + destRect.w) &&
                    (destRect.y + y < dest->h) && (destRect.y + y >= 0) &&
                    (destRect.y + y <= destRect.y + destRect.h) ) {

                    // is inside destRect and the destination surface
                    putPixel(dest, destRect.x + x, destRect.y + y, color);
                }
            }
        }

        return StepsLeft;
    }


private:
    sdl2::surface_ptr src;
    SDL_Surface* dest;
    SDL_Rect    destRect;
    int numSteps;
    int StepsLeft;

    Uint64 N;
    Uint64 m;
    Uint64 a;
    Uint64 c;
    Uint64 currentValue;
};

#endif //BLENDBLITTER_H
