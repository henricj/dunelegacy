/*
   fnkdat - an interface for determining common directory names
   Copyright (C) 2001, 2002  David MacCormack
   $Header$

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public License
   as published by the Free Software Foundation; either version 2
   of the License, or (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.

   As a special exception, David MacCormack gives permission
   for additional uses of the text contained in the files named
   ``fnkdat.h'' and ``fnkdat.c'', hereafter known as FNKDAT.

   The exception is that, if you link the FNKDAT with other files
   to produce an executable, this does not by itself cause the
   resulting executable to be covered by the GNU General Public License.
   Your use of that executable is in no way restricted on account of
   linking the FNKDAT code into it.

   This exception does not however invalidate any other reasons why
   the executable file might be covered by the GNU General Public License.

   This exception applies only to the code released by David MacCormack
   under the name FNKDAT.  If you copy code from other software into a
   copy of FNKDAT, as the General Public License permits, the exception does
   not apply to the code that you add in this way.  To avoid misleading
   anyone as to the status of such modified files, you must delete
   this exception notice from them.

   If you write modifications of your own for FNKDAT, it is your choice
   whether to permit this exception to apply to your modifications.
   If you do not wish that, delete this exception notice.

   David MacCormack (djm at maccormack dot net)

*/

#include <array>
#include <cassert>
#include <cerrno>
#include <cstring>
#include <filesystem>
#include <misc/fnkdat.h>
#include <string>
#include <sys/stat.h>
#include <sys/types.h>

/* version is automatically generated
   #define FNKDAT_VERSION "0.0.8"
 */

#include <cstdio>

#include "config.h"

#ifndef PACKAGE
#    error PACKAGE is not defined
#endif

#ifndef FNKDAT_PKGDATADIR
#    define FNKDAT_PKGDATADIR "/usr/share/" PACKAGE
#endif

#ifndef FNKDAT_GAMESUBDIR
#    define FNKDAT_GAMESUBDIR "games/"
#endif

/*
 * BSD seems to put their game stuff in /var/games
 * as opposed to /var/lib/games
 */
#ifndef FNKDAT_PKGLIBDIR
#    ifdef __FreeBSD__
#        ifndef FNKDAT_GAMESUBDIR
#            define FNKDAT_GAMESUBDIR "lib/"
#        endif /* GAMESUBDIR */
#        define FNKDAT_PKGLIBDIR "/var/" FNKDAT_GAMESUBDIR PACKAGE
#    else
#        define FNKDAT_PKGLIBDIR "/var/lib/" FNKDAT_GAMESUBDIR PACKAGE
#    endif /* __FreeBSD__ */
#endif     /* FNKDAT_PKGLIBDIR */

#ifndef FNKDAT_SYSCONFDIR
#    define FNKDAT_SYSCONFDIR "/etc"
#endif

/*
 * do the mkdir(s), if asked to
 */
#define FNKDAT_MKDIRS

std::tuple<bool, std::filesystem::path> fnkdat(int flags) {
    /* Initialize, if requested to.  Note that initialize must be called
     * from a single thread before anything else.  Other then that,
     * there are no concurrency issues (as far as I know).
     */
    if (flags == FNKDAT_INIT) {
        return {true, std::filesystem::path {}};
    }

    /* Uninitialize, if requested to -- probably not necessary but what
     * the hell, why not?
     */
    if (flags == FNKDAT_UNINIT) {
        return {true, std::filesystem::path {}};
    }

    return {false, std::filesystem::path {}};
}

/************************
 * WIN32 IMPLEMENTATION *
 ************************/
#ifdef _WIN32

#    ifndef WIN32_LEAN_AND_MEAN
#        define WIN32_LEAN_AND_MEAN
#    endif
#    ifndef STRICT
#        define STRICT
#    endif
#    include <Shlobj.h>
#    include <Shlwapi.h>
#    include <Windows.h>
#    include <cctype>
#    include <direct.h>

/*
 * Constants passed to the silly-ass MS function
 */
#    define CSIDL_APPDATA        0x001a
#    define CSIDL_FLAG_CREATE    0x8000
#    define CSIDL_COMMON_APPDATA 0x0023
#    define SHGFP_TYPE_CURRENT   0
#else // _WIN32
#    include <cstdlib>
#    include <pwd.h>
#    include <unistd.h>

#    ifdef __APPLE__
#        include <MacFunctions.h>
#    endif
#endif // _WIN32

/*
 * Get the requested info
 */

std::tuple<bool, std::filesystem::path> fnkdat(const std::filesystem::path& target, int flags) {

    /* Initialize, if requested to.  Note that initialize must be called
     * from a single thread before anything else.  Other then that,
     * there are no concurrency issues (as far as I know).
     */
    if (flags == FNKDAT_INIT) {
        return {true, std::filesystem::path {}};
    }

    /* Uninitialize, if requested to -- probably not necessary but what
     * the hell, why not?
     */
    if (flags == FNKDAT_UNINIT) {
        return {true, std::filesystem::path {}};
    }

    /* if target is absolute then simply return it
     */
    if (!target.empty()) {
        if (target.is_absolute())
            return {true, target.lexically_normal().make_preferred()};
    }

    const int rawflags = flags & (0xFFFFFFFF ^ FNKDAT_CREAT);

    std::filesystem::path output_path;

#ifdef _WIN32
    std::array<wchar_t, MAX_PATH> szPath;
    std::array<wchar_t, MAX_PATH> buffer;

    /* save room for the null term char
     */
    HRESULT hresult = S_OK;
    DWORD dwFlags   = 0;

    if (rawflags == FNKDAT_USER)
        dwFlags = CSIDL_APPDATA;
    else if (rawflags == (FNKDAT_VAR | FNKDAT_DATA))
        dwFlags = CSIDL_COMMON_APPDATA;

    /* Get the user conf directory using the silly-ass function if it
       is available.
     */
    if (dwFlags
        && SUCCEEDED(hresult = SHGetFolderPathW(NULL, dwFlags | ((flags & FNKDAT_CREAT) ? CSIDL_FLAG_CREATE : 0), NULL,
                                                SHGFP_TYPE_CURRENT, &szPath[0]))) {

        output_path = &szPath[0];
        output_path /= L"" PACKAGE;

        /* We always compute the system conf and data directories
           relative to argv[0]

           Why not use SHGetFolderPath(...)
           for system conf??  Here's why.  I'm using this as the win32
           supplement for /etc/. If I used CSIDL_COMMON_APPDATA then
           it would only be available when that function is available
           which means I'd have to fallback to something different when
           it's not.  This would make app installation a royal pain
           because this system conf directory would vary depending on whether
           or not SHFOLDER.DLL happens to be installed.  I intend for this path
           to contain read-only "system" configuration data that is
           installed when the software is.  So, I'm saying it's relative
           to the executable, as that's what most existing software seems
           to do.
         */
    } else if (flags == FNKDAT_CONF || flags == FNKDAT_USER || flags == FNKDAT_DATA
               || flags == (FNKDAT_VAR | FNKDAT_DATA)) {
        const wchar_t* szCommandLine = GetCommandLineW();

        const wchar_t* command_end;

        /* argv[0] may be quoted -- if so, skip the quote
           and whack everything after the end quote
         */
        if (szCommandLine[0] == L'"') {
            ++szCommandLine;

            command_end = wcschr(szCommandLine, L'"');

            if (!command_end)
                return {false, std::filesystem::path {}};

            /* otherwise, whack everything after the first
               space character
             */
        } else {
            for (command_end = szCommandLine; *command_end && !iswspace(*command_end); ++command_end) { }
        }

        if (command_end == szCommandLine)
            output_path = "./";
        else {
            const auto command_length = command_end - szCommandLine;

            assert(command_length > 0);

            if (command_length >= MAX_PATH - 1)
                return {false, std::filesystem::path {}};

            output_path = std::wstring(szCommandLine, command_length);
        }

        /* this only happens when we don't have the silly-ass function */
        if (flags & FNKDAT_USER) {
            output_path /= L"users";

            DWORD dwSize = buffer.size();

            /* Grab what windows thinks is the current user name */
            if (GetUserNameW(&buffer[0], &dwSize) == TRUE) {
                output_path /= &buffer[0];

                /* if that fails, make something up */
            } else {
                output_path /= L"default";
            }
        }

        /* If we get here the user gave a bad flag
           or !SUCCEEDED(hresult)
         */
    } else {
        errno = EINVAL;
        return {false, std::filesystem::path {}};
    }
#else // _WIN32
    /************************
     * UNIX IMPLEMENTATION  *
     ************************/

    if (rawflags == FNKDAT_USER) {

        std::array<char, 1536> buffer;

#    ifdef __APPLE__
        {
            getMacApplicationSupportFolder(&buffer[0], buffer.size());

            output_path = buffer;
            output_path /= "Dune Legacy";
#    else
        {
            const char* xdg_config = getenv("XDG_CONFIG_HOME");

            if (xdg_config == nullptr) {
                const struct passwd* pwent = getpwuid(getuid());

                if (!pwent)
                    return {false, std::filesystem::path {}};

                output_path = pwent->pw_dir;
                output_path /= ".config";
            } else {
                output_path = xdg_config;
            }
            output_path /= PACKAGE;
        }
#    endif
        }
        else if (rawflags == FNKDAT_CONF) {
            output_path = FNKDAT_SYSCONFDIR;
            output_path /= PACKAGE;
        }
        else if (rawflags == (FNKDAT_VAR | FNKDAT_DATA)) {
            output_path = FNKDAT_PKGLIBDIR;
        }
        else if (rawflags == FNKDAT_DATA) {
            output_path = FNKDAT_PKGDATADIR;
        }
        else {
            errno = EINVAL;
            return {false, std::filesystem::path {}};
        }
#endif // _WIN32

    /* append any given filename */
    if (!target.empty()) {
        output_path /= target;
    }

    output_path = output_path.lexically_normal().make_preferred();

    /* do the mkdir(s), if asked to */
    if (flags & FNKDAT_CREAT) {
        const auto parent = output_path.parent_path();

        if (!parent.empty()) {
            std::error_code ec;
            if (!std::filesystem::create_directories(parent, ec) && ec)
                return {false, std::filesystem::path {}};

            std::filesystem::permissions(parent,
                                         std::filesystem::perms::owner_all | std::filesystem::perms::group_all
                                             | std::filesystem::perms::others_exec
                                             | std::filesystem::perms::others_read,
                                         std::filesystem::perm_options::replace, ec);
            if (ec) {
                return {false, std::filesystem::path {}};
            }
        }
    }

    return {true, output_path};
}
