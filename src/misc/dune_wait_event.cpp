#include "misc/dune_wait_event.h"

#include <SDL2/SDL.h>

#if defined(_WIN32)
#    include <Windows.h>
#endif

namespace dune {
bool Dune_WaitEvent(SDL_Event* event, uint32_t timeout) {
    assert(event);

    if (timeout < 1)
        return false;

#if defined(_WIN32)
    // The API that SDL_WaitEventTimeout() uses has a minimum wait time of 10ms.
    // Asking for anything less will still result in a 10ms delay.
    // See win32's "SetTimer()" and note the USER_TIMER_MINIMUM in
    // the uElapsed section.
    // https://docs.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-settimer
    // see https://devblogs.microsoft.com/oldnewthing/20060126-00/
    const auto wStatus = MsgWaitForMultipleObjectsEx(0, nullptr, timeout, QS_ALLINPUT, MWMO_INPUTAVAILABLE);

    if (wStatus != WAIT_OBJECT_0)
        return false;

    return 0 != SDL_PollEvent(event);
#else
    return SDL_WaitEventTimeout(event, timeout);
#endif
}

} // namespace dune
