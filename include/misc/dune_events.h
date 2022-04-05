#ifndef DUNE_EVENTS_H
#define DUNE_EVENTS_H

#include <SDL2/SDL.h>

namespace dune {
bool Dune_WaitEvent(SDL_Event* event, uint32_t timeout);

class DuneEventWatcher final {
public:
    DuneEventWatcher();
    ~DuneEventWatcher();
};
} // namespace dune

#endif // DUNE_EVENTS_H
