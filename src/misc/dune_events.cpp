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

#include <SDL2/SDL.h>

#if defined(_WIN32)
#    include <Windows.h>

#    include <WinUser.h>

#    include <SDL2/SDL_syswm.h>
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

#if defined(_WIN32)
namespace {
inline constexpr auto local_win32_WM_DPICHANGED = 0x02E0;

extern "C" int dune_watch_events(void* userdata, SDL_Event* event) {
    switch (event->type) {
        case SDL_SYSWMEVENT: {
            assert(event && event->syswm.msg);

            const auto& win_msg = event->syswm.msg->msg.win;

            switch (win_msg.msg) {
                case local_win32_WM_DPICHANGED: {
                    const auto dpi   = HIWORD(event->syswm.msg->msg.win.wParam);
                    const auto ratio = static_cast<float>(dpi) / static_cast<float>(USER_DEFAULT_SCREEN_DPI);

                    GUIStyle::getInstance().setDisplayDpi(ratio);

                    const RECT* const prcNewWindow =
                        reinterpret_cast<RECT*>(event->syswm.msg->msg.win.lParam); // NOLINT(performance-no-int-to-ptr)
                    SDL_SetWindowPosition(window, prcNewWindow->left, prcNewWindow->top);
                    SDL_SetWindowSize(window, prcNewWindow->right - prcNewWindow->left,
                                      prcNewWindow->bottom - prcNewWindow->top);

                } break;
            }
        } break;
    }

    return 0;
}

} // namespace

DuneEventWatcher::DuneEventWatcher() {
    SDL_AddEventWatch(dune_watch_events, nullptr);
}

DuneEventWatcher::~DuneEventWatcher() {
    SDL_DelEventWatch(dune_watch_events, nullptr);
}

#else  // defined(_WIN32)
DuneEventWatcher::DuneEventWatcher() = default;
DuneEventWatcher::~DuneEventWatcher() = default;
#endif // defined(_WIN32)

} // namespace dune
