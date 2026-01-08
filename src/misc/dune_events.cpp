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

#include "misc/dune_events.h"

#include "globals.h"

#include "GUI/GUIStyle.h"

#include <misc/dune_sdl.h>

#if defined(_WIN32)
#    include <Windows.h>

#    include <WinUser.h>
#endif

namespace dune {

bool Dune_WaitEvent(SDL_Event* event, uint32_t timeout) {
    assert(event);

    if (timeout < 1)
        return SDL_PollEvent(event);

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

    return SDL_PollEvent(event);
#else
    return SDL_WaitEventTimeout(event, timeout);
#endif
}

#if defined(_WIN32)
namespace {
inline constexpr auto local_win32_WM_DPICHANGED = 0x02E0;

// SDL3: Event filter callback returns bool instead of int
extern "C" bool dune_watch_events([[maybe_unused]] void* userdata, SDL_Event* event) {
    switch (event->type) {
        case SDL_EVENT_WINDOW_DISPLAY_SCALE_CHANGED: {
            // SDL3 provides display scale change events directly
            auto* const window = dune::globals::window.get();
            if (window) {
                const auto scale = SDL_GetWindowDisplayScale(window);
                GUIStyle::getInstance().setDisplayDpi(scale);
            }
        } break;
    }

    return true; // SDL3: return true to allow event, false to drop it
}

} // namespace

DuneEventWatcher::DuneEventWatcher() {
    SDL_AddEventWatch(dune_watch_events, nullptr);
}

DuneEventWatcher::~DuneEventWatcher() {
    SDL_RemoveEventWatch(dune_watch_events, nullptr);
}

#else  // defined(_WIN32)
DuneEventWatcher::DuneEventWatcher()  = default;
DuneEventWatcher::~DuneEventWatcher() = default;
#endif // defined(_WIN32)

} // namespace dune
