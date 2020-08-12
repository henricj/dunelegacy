
#ifndef PCH_H
#define PCH_H

#ifdef _WIN32
#    ifndef WINDDI_VERSION
#        define WINDDI_VERSION WINDDI_VERSION_VISTASP2
#    endif
#    ifndef _WIN32_WINNT
#        define _WIN32_WINNT _WIN32_WINNT_VISTA
#    endif
#    ifndef WIN32_LEAN_AND_MEAN
#        define WIN32_LEAN_AND_MEAN
#    endif
#    ifndef NOMINMAX
#        define NOMINMAX
#    endif
#endif // _WIN32

#ifdef __cplusplus

#    include <fmt/format.h>
#    include <fmt/printf.h>
#include <gsl/gsl>

#    include <misc/Random.h>
#    include <misc/exceptions.h>

#    include "engine_mmath.h"

#    include <algorithm>
#    include <array>
#    include <bitset>
#    include <cassert>
#    include <chrono>
#    include <cinttypes>
#    include <cmath>
#    include <cstdarg>
#    include <cstdio>
#    include <cstdlib>
#    include <cstring>
#    include <ctime>
#    include <deque>
#    include <exception>
#    include <filesystem>
#    include <functional>
#    include <future>
#    include <iostream>
#    include <limits>
#    include <list>
#    include <map>
#    include <memory>
#    include <mutex>
#    include <numeric>
#    include <queue>
#    include <random>
#    include <set>
#    include <string>
#    include <typeinfo>
#    include <unordered_map>
#    include <unordered_set>
#    include <utility>
#    include <vector>

#    include <cinttypes>
#    include <cstdio>
#    include <cstdlib>
#    include <cstring>

#else // __cplusplus
#    include <inttypes.h>
#    include <stdio.h>
#    include <stdlib.h>
#    include <string.h>
#endif // __cplusplus

#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>

#ifdef _WIN32
#    include <io.h>
#    include <windows.h>
#    include <winsock2.h>
#endif // _WIN32

#endif // PCH_H
