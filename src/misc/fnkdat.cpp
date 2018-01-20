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

#include <errno.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <misc/fnkdat.h>

/* version is automatically generated
   #define FNKDAT_VERSION "0.0.8"
 */

#include <stdio.h>

#include "config.h"

#ifndef PACKAGE
#   error PACKAGE is not defined
#endif

#ifndef FNKDAT_PKGDATADIR
#  define FNKDAT_PKGDATADIR "/usr/share/" PACKAGE
#endif

#ifndef FNKDAT_GAMESUBDIR
#  define FNKDAT_GAMESUBDIR "games/"
#endif

/*
 * BSD seems to put their game stuff in /var/games
 * as opposed to /var/lib/games
 */
#ifndef FNKDAT_PKGLIBDIR
#  ifdef __FreeBSD__
#     ifndef FNKDAT_GAMESUBDIR
#        define FNKDAT_GAMESUBDIR "lib/"
#     endif /* GAMESUBDIR */
#     define FNKDAT_PKGLIBDIR "/var/" FNKDAT_GAMESUBDIR PACKAGE
#  else
#     define FNKDAT_PKGLIBDIR "/var/lib/" FNKDAT_GAMESUBDIR PACKAGE
#  endif /* __FreeBSD__ */
#endif /* FNKDAT_PKGLIBDIR */

#ifndef FNKDAT_SYSCONFDIR
#  define FNKDAT_SYSCONFDIR "/etc"
#endif

/*
 * Used to make sure we don't append past the end of a buffer
 */
#define FNKDAT_S(op)                                \
   if ((len = total - (int) _tcslen(buffer)) <= 0) {    \
      errno = ENOMEM;                               \
      return -1;                                    \
   }                                                \
   op;


/*
 * do the mkdir(s), if asked to
 */
#define FNKDAT_MKDIRS




/************************
 * WIN32 IMPLEMENTATION *
 ************************/
#ifdef _WIN32

#define WIN32_LEAN_AND_MEAN
#ifndef STRICT
#define STRICT
#endif
#include <windows.h>
#include <direct.h>
#include <Shlobj.h>
#include <Shlwapi.h>
#include <cctype>

/*
 * Constants passed to the silly-ass MS function
 */
#define CSIDL_APPDATA         0x001a
#define CSIDL_FLAG_CREATE     0x8000
#define CSIDL_COMMON_APPDATA  0x0023
#define SHGFP_TYPE_CURRENT    0

/* these are used by the common functions defined below */
#define FNKDAT_FILE_SEPARATOR _T('\\')
#define stat _stat

#if defined(_UNICODE) || defined(UNICODE)
#  define FNKDAT_U   "W"
#else
#  define FNKDAT_U   "A"
#endif

/* defined below */
static int fnkdat_mkdirs(_TCHAR* buffer, int rlevel);

/* pointer to function types*/
typedef BOOL (WINAPI *LPGETUSERNAME) (LPTSTR,LPDWORD);
typedef HRESULT (WINAPI *LPSHGETFOLDERPATH) (HWND,int,HANDLE,DWORD,LPTSTR);
/*
 * Get the requested info
 */
int fnkdat(const _TCHAR* target, _TCHAR* buffer0, int len, int flags) {

   /* Initialize, if requested to.  Note that initialize must be called
    * from a single thread before anything else.  Other then that,
    * there are no concurrency issues (as far as I know).
    */
   if (flags == FNKDAT_INIT) {
      return 0;
   }

   /* Uninitialize, if requested to -- probably not necessary but what
    * the hell, why not?
    */
   if (flags == FNKDAT_UNINIT) {
      return 0;
   }

   if (!buffer0)
       return -1;

   /* if target is absolute then simply return it
    */
   if (target) {
      if ((target[0] == _T('\\'))
          || (target[0] == _T('/'))
          || (target[0] && target[1] && target[1] == _T(':'))) {

         _tcsncpy(buffer0, target, len);
         return 0;
      }
   }

   wchar_t szPath[MAX_PATH];
   wchar_t buffer[MAX_PATH];

   if (len)
      buffer0[0] = _T('\0');

   /* save room for the null term char
   */
   const int total = len - 1;

   HRESULT hresult = S_OK;
   DWORD dwFlags = 0;

   const int rawflags = flags & (0xFFFFFFFF ^ FNKDAT_CREAT);

   if (rawflags == FNKDAT_USER)
      dwFlags = CSIDL_APPDATA;
   else if (rawflags == (FNKDAT_VAR | FNKDAT_DATA))
      dwFlags = CSIDL_COMMON_APPDATA;


   /* Get the user conf directory using the silly-ass function if it
      is available.
    */
   if (dwFlags
       && SUCCEEDED(hresult = SHGetFolderPathW(
         NULL,
         dwFlags | ((flags & FNKDAT_CREAT) ? CSIDL_FLAG_CREATE : 0),
         NULL,
         SHGFP_TYPE_CURRENT,
         szPath))) {

      if (!PathAppendW(szPath, L"" PACKAGE)) {
         errno = ENOMEM;
         return -1;
      }

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
   }
   else if ((flags == FNKDAT_CONF)
      || (flags == FNKDAT_USER)
      || (flags == FNKDAT_DATA)
      || (flags == (FNKDAT_VAR | FNKDAT_DATA))) {
      const wchar_t * szCommandLine = GetCommandLineW();

      const wchar_t * command_end;

      /* argv[0] may be quoted -- if so, skip the quote
         and whack everything after the end quote
       */
      if (szCommandLine[0] == L'"') {
         ++szCommandLine;

         command_end = wcschr(szCommandLine, L'"');

         if (!command_end)
            return -1;

           //szCommandLine++;
           //_tcsncpy(buffer, szCommandLine, len);
           //szTmp = buffer;

           //while(szTmp[0] && szTmp[0] != _T('"'))
           //   szTmp++;

           //szTmp[0] = _T('\0');

        /* otherwise, whack everything after the first
           space character
         */
      } else {
          for (command_end = szCommandLine; *command_end && !iswspace(*command_end); ++command_end)
          { }

          //_tcsncpy(buffer, szCommandLine, len);
          //szTmp = buffer;

          //while(szTmp[0] && !_istspace(szTmp[0]))
          //   szTmp++;

          //szTmp[0] = _T('\0');
       }

      if (command_end == szCommandLine)
         wcscpy(szPath, L".\\");
      else {
         const auto command_length = command_end - szCommandLine;

         assert(command_length > 0);

         if (command_length >= MAX_PATH - 1)
            return -1;

         memcpy(szPath, szCommandLine, sizeof(wchar_t) * command_length);

         szPath[command_length] = L'\0';
      }

      /* this only happens when we don't have the silly-ass function */
      if (flags & FNKDAT_USER) {
          PathAppendW(szPath, L"users");

         DWORD dwSize = MAX_PATH;

         /* Grab what windows thinks is the current user name */
         if (GetUserNameW(buffer, &dwSize) == TRUE) {
            PathAppendW(szPath, buffer);

         /* if that fails, make something up */
         } else {
             PathAppendW(szPath, L"default");
         }
      }


   /* If we get here the user gave a bad flag
      or !SUCCEEDED(hresult)
    */
   } else {
      errno = EINVAL;
      return -1;
   }

   /* replace unix path characters w/ windows path chars
   so that the fnk_mkdirs funtion works
   */
   for (int i = 0; szPath[i]; i++) {
       if (szPath[i] == L'/')
           szPath[i] = L'\\';
   }

   /* do the mkdir(s), if asked to */
   if ((flags & FNKDAT_CREAT)) {
       const auto ret = SHCreateDirectoryExW(nullptr, szPath, nullptr);

       if (ret != ERROR_SUCCESS && ret != ERROR_ALREADY_EXISTS)
           return -1;
   }

   /* append any given filename */
   if (target) {
       if (!MultiByteToWideChar(CP_UTF8, 0, target, -1, buffer, MAX_PATH))
           return -1;

       PathAppendW(szPath, buffer);
   }

   if(WideCharToMultiByte(CP_UTF8, 0, szPath, -1, buffer0, len, nullptr, nullptr) == 0) {
       return -1;
   }

   return 0;
}


/************************
 * UNIX IMPLEMENTATION  *
 ************************/
#else

#include <pwd.h>
#include <unistd.h>
#include <cstdlib>

#ifdef __APPLE__
#include <MacFunctions.h>
#endif

#ifndef FNKDAT_DIRMODE
#   define FNKDAT_DIRMODE 0775
#endif


/* these are used by the common functions defined below */
#define _TCHAR char
#define _T(s)  s
#define FNKDAT_FILE_SEPARATOR '/'
#define _tmkdir(d) mkdir(d, (mode_t)FNKDAT_DIRMODE)
#define _tcsrchr strrchr
#define _tcslen strlen
#define _tstat stat

/* defined below */
static int fnkdat_mkdirs(_TCHAR* buffer, int rlevel);


int fnkdat(const char* target, char* buffer, int len, int flags) {
   int total, rawflags;

   /* nothing to do */
   if (flags == FNKDAT_INIT
       || flags == FNKDAT_UNINIT) {
      return 0;
   }

   if(!buffer || len <= 0) {
      return -1;
   }

   buffer[0] = '\0';

   /* save room for the null term char
    */
   total = len - 1;

   rawflags = flags & (0xFFFFFFFF ^ FNKDAT_CREAT);


   /* when we've got an absolute path we simply return it */
   if (target && target[0] == '/') {
      strncpy(buffer, target, len);
      return 0;
   }

   if (rawflags == FNKDAT_USER) {

#ifdef __APPLE__
      getMacApplicationSupportFolder(buffer, len);
      FNKDAT_S(strncat(buffer, "/Dune Legacy", len));
#else
      {
         char* xdg_config = getenv("XDG_CONFIG_HOME");

         if(xdg_config == NULL) {
            struct passwd* pwent = getpwuid(getuid());

            if (!pwent)
               return -1;

            strncpy(buffer, pwent->pw_dir, len);
            FNKDAT_S(strncat(buffer, "/.config/" PACKAGE, len));
         } else {
            FNKDAT_S(strncpy(buffer, xdg_config, len));
            FNKDAT_S(strncat(buffer, "/" PACKAGE, len));
         }
      }
#endif

   } else if (rawflags == FNKDAT_CONF) {
      strncpy(buffer, FNKDAT_SYSCONFDIR, len);
      FNKDAT_S(strncat(buffer, "/" PACKAGE, len));

   } else if (rawflags == (FNKDAT_VAR | FNKDAT_DATA)) {
      strncpy(buffer, FNKDAT_PKGLIBDIR, len);

   } else if (rawflags == FNKDAT_DATA) {
      strncpy(buffer, FNKDAT_PKGDATADIR, len);

   } else {
      errno = EINVAL;
      return -1;
   }

   FNKDAT_S(strncat(buffer, "/", len));

   if (target) {
      FNKDAT_S(strncat(buffer, target, len));
   }


   /* do the mkdir(s), if asked to */
   if ((flags & FNKDAT_CREAT)
       && fnkdat_mkdirs(buffer, -1) < 0) {

      return -1;
   }

   return 0;
}

#endif /* _WIN32 */


/********************
 * COMMON FUNCTIONS *
 ********************/

/*
 * This will make the requested directory, along with
 * any necessary parent directory.
 */
static int fnkdat_mkdirs(_TCHAR* buffer, int rlevel) {

   _TCHAR* pos;
   struct stat statbuf;

   rlevel++;

   /* if this is the first time that we call this function,
      we want to skip past any filename that happens to
      be sitting there, and start working on directories
    */
   if (rlevel == 0) {

      /* if target has a file on the end, we don't
         want to make a directory w/ its name.  So
         we skip everything after the last FNKDAT_FILE_SEPARATOR.
       */
      pos = _tcsrchr(buffer, FNKDAT_FILE_SEPARATOR);
      if (pos)
         pos[0] = _T('\0');

      /* make the necessary directories.  If this fails,
         errno will already be set, so we simply return
         with an error
      */
      if (fnkdat_mkdirs(buffer, rlevel) < 0)
         return -1;

      if (pos)
         pos[0] = FNKDAT_FILE_SEPARATOR;

      return 0;
   }


   /* if the directory exists, then we have nothing to do */
   if (_tstat(buffer, &statbuf) == 0)
      return 0;

   switch (errno) {

      case ENOENT:
         pos = _tcsrchr(buffer, FNKDAT_FILE_SEPARATOR);
         if (pos)
            pos[0] = _T('\0');

         if (fnkdat_mkdirs(buffer, rlevel) < 0)
            return -1;

         if (pos)
            pos[0] = FNKDAT_FILE_SEPARATOR;

         if (_tmkdir(buffer) < 0)
            return -1;

         break;

      default:
         return -1;
   }


   return 0;
}

/* vi: set sw=3 ts=3 tw=78 et sts: */

