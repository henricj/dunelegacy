#ifndef DUNE_WAIT_EVENT_H
#define DUNE_WAIT_EVENT_H

#include <SDL2/SDL.h>

namespace dune {
bool Dune_WaitEvent(SDL_Event* event, uint32_t timeout);
}

#endif // DUNE_WAIT_EVENT_H
