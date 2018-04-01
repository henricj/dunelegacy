AC_INIT(dunelegacy, 0.96.4, http://dunelegacy.sourceforge.net)

AC_CANONICAL_BUILD

AC_USE_SYSTEM_EXTENSIONS

AM_INIT_AUTOMAKE([subdir-objects])

AC_CONFIG_MACRO_DIR([m4])

AC_PROG_CXX

if test "$prefix" = "" ; then
	dunelegacydatadir='data'
else
	dunelegacydatadir='${pkgdatadir}'
fi

AC_SUBST(dunelegacydatadir)

dnl Some flags for gcc
CXXFLAGS="-std=c++14 -pthread -fPIC -O3 -pipe -pedantic -Wall -Wextra -Wno-unused-parameter -Wno-shift-negative-value -DNDEBUG"
CFLAGS="-std=c99 -pthread -fPIC -O3 -pipe -pedantic -Wall -Wextra -Wno-unused-parameter -Wno-shift-negative-value -DNDEBUG"
dnl Additional flags: -Wconversion -Wno-sign-conversion -Wlogical-op

dnl Some flags for clang
dnl CXXFLAGS="-std=c++14 -O3 -pipe -pedantic -Weverything -Wno-sign-conversion -Wno-conversion -Wno-shadow -Wno-missing-noreturn -Wno-padded -Wno-packed -Wno-weak-vtables -Wno-c++11-narrowing -Wno-cast-align -Wno-unreachable-code -Wno-missing-prototypes -Wno-float-equal -Wno-global-constructors -Wno-exit-time-destructors -Wno-format-nonliteral -Wno-unused-macros -Wreserved-id-macro -Wextra -Wno-unused-parameter -Wno-narrowing"


AC_ARG_ENABLE([debug],
            [AS_HELP_STRING([--enable-debug],
              [compile with debug information @<:@default=disabled@:>@])],
            [],
            [])

if test "$enable_debug" = "yes" ; then
	CXXFLAGS="$CXXFLAGS -g"
fi

dnl Check for SDL library
dnl Check for SDL_mixer library.
SDL_VERSION=2.0.0
AM_PATH_SDL2($SDL_VERSION, : , AC_MSG_ERROR([*** SDL2 version $SDL_VERSION not found!]))
dnl PKG_CHECK_MODULES(SDL, sdl2 SDL2_mixer)
CFLAGS="$CFLAGS $SDL_CFLAGS"
CXXFLAGS="$CXXFLAGS $SDL_CFLAGS"
LIBS="$LIBS $SDL_LIBS"

dnl Check for SDL_mixer library.
AC_CHECK_LIB(SDL2_mixer, 
       Mix_OpenAudio, 
       LIBS="$LIBS -lSDL2_mixer", 
       AC_MSG_ERROR([*** SDL2_mixer library not found!])
)

dnl enet defines
AC_CHECK_FUNC(gethostbyaddr_r, [AC_DEFINE(HAS_GETHOSTBYADDR_R)])
AC_CHECK_FUNC(gethostbyname_r, [AC_DEFINE(HAS_GETHOSTBYNAME_R)])
AC_CHECK_FUNC(poll, [AC_DEFINE(HAS_POLL)])
AC_CHECK_FUNC(fcntl, [AC_DEFINE(HAS_FCNTL)])
AC_CHECK_FUNC(inet_pton, [AC_DEFINE(HAS_INET_PTON)])
AC_CHECK_FUNC(inet_ntop, [AC_DEFINE(HAS_INET_NTOP)])

AC_CHECK_MEMBER(struct msghdr.msg_flags, [AC_DEFINE(HAS_MSGHDR_FLAGS)], , [#include <sys/socket.h>])

AC_CHECK_TYPE(socklen_t, [AC_DEFINE(HAS_SOCKLEN_T)], , 
              #include <sys/types.h>
              #include <sys/socket.h>
)

AC_EGREP_HEADER(MSG_MAXIOVLEN, /usr/include/sys/socket.h, AC_DEFINE(ENET_BUFFER_MAXIMUM, [MSG_MAXIOVLEN]))
AC_EGREP_HEADER(MSG_MAXIOVLEN, socket.h, AC_DEFINE(ENET_BUFFER_MAXIMUM, [MSG_MAXIOVLEN]))

if test "${host}" = "i686-w64-mingw32" ; then
        LIBS="$LIBS -lws2_32 -lwinmm -static-libgcc -static-libstdc++ -Wl,-Bstatic -lstdc++ -lpthread -Wl,-Bdynamic resource.o"
		dunelegacydatadir='.'
elif test "${host}" = "x86_64-w64-mingw32" ; then
        LIBS="$LIBS -lws2_32 -lwinmm -static-libgcc -static-libstdc++ -Wl,-Bstatic -lstdc++ -lpthread -Wl,-Bdynamic resource.o"
		dunelegacydatadir='.'
fi

AC_CONFIG_FILES(Makefile src/Makefile tests/Makefile)
AC_OUTPUT
