#include <misc/BlendBlitter.h>

#include "FileClasses/GFXManager.h"

#include "globals.h"

BlendBlitter::BlendBlitter(sdl2::surface_ptr SrcPic, SDL_Surface* DestPic, SDL_Rect DestPicRect, int numSteps)
    : src{std::move(SrcPic)}, dest{DestPic}, destRect{DestPicRect}, numSteps{numSteps}, StepsLeft(numSteps),
      N(static_cast<uint64_t>(src->w) * static_cast<uint64_t>(src->h)), m(N) {

    // compute next greater 2^x value

    m |= m >> 1;
    m |= m >> 2;
    m |= m >> 4;
    m |= m >> 8;
    m |= m >> 16;
    m |= m >> 32;
    m++;

    auto& random = dune::globals::pGFXManager->random();

    c = random.rand(0, static_cast<int>(m / 2 - 1)) * 2 + 1; // c is any odd number from [0;m]
    a = random.rand(0, static_cast<int>(m / 4 - 1)) * 4 + 1;
    // (a-1) is divisible by all prime factors of log_2(m), and 4

    currentValue = random.rand(0, static_cast<int>(m - 1));
}

int BlendBlitter::nextStep() {
    if (StepsLeft <= 0) {
        return 0;
    }

    StepsLeft--;

    sdl2::surface_lock lock_dest{dest};
    sdl2::surface_lock lock_src{src.get()};

    const uint64_t numPixelsPerStep = N / numSteps + 1;
    for (uint64_t i = 0; i < numPixelsPerStep; i++) {
        const auto cur = getNextValue();

        const auto x = static_cast<int>(cur % src->w);
        const auto y = static_cast<int>(cur / src->w);

        const uint32_t color = getPixel(src.get(), x, y);

        if (color != 0) {
            if (destRect.x + x < dest->w && destRect.x + x >= 0 && destRect.x + x <= destRect.x + destRect.w
                && destRect.y + y < dest->h && destRect.y + y >= 0 && destRect.y + y <= destRect.y + destRect.h) {
                // is inside destRect and the destination surface
                putPixel(dest, destRect.x + x, destRect.y + y, color);
            }
        }
    }

    return StepsLeft;
}
