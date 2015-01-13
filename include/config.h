
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

#ifndef VERSION
    #define VERSION "0.96.3"
#endif

#ifndef PACKAGE
	#define PACKAGE "dunelegacy"
#endif

#define VERSIONSTRING   PACKAGE VERSION

#ifndef DUNELEGACY_DATADIR
    #define DUNELEGACY_DATADIR "."
#endif

#ifndef CONFIGFILENAME
	#define CONFIGFILENAME "Dune Legacy.ini"
#endif

#ifndef LOGFILENAME
	#define LOGFILENAME "Dune Legacy.log"
#endif

// Used by misc/FileSystem's getResourcesBundlePath && TextManager ctor
// TODO: Find a better home for this (i.e.: platform.h)
#if defined(__APPLE__) || defined (MACOSX) || defined(macintosh) || defined(Macintosh)
    #define DUNELEGACY_PLATFORM_OSX
#endif
