#ifndef BLENDBLITTER_H
#define BLENDBLITTER_H

#include <misc/SDL2pp.h>

// Step by step one more pixel of the source image is blitted to the destination image
class BlendBlitter {
public:
    BlendBlitter(sdl2::surface_ptr SrcPic, SDL_Surface* DestPic, SDL_Rect DestPicRect, int numSteps = 50);

    ~BlendBlitter() = default;

    uint64_t getNextValue() {
        do {
            currentValue = (a * currentValue + c) % m;
        } while (currentValue >= N);

        return currentValue;
    }

    /**
        Blits the next pixel to the destination surface
        \return The number of steps to do
    */
    int nextStep();

private:
    sdl2::surface_ptr src;
    SDL_Surface* dest;
    SDL_Rect destRect{};
    int numSteps;
    int StepsLeft;

    uint64_t N;
    uint64_t m;
    uint64_t a;
    uint64_t c;
    uint64_t currentValue;
};

#endif // BLENDBLITTER_H
