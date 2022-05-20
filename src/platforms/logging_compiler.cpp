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

#include "logging.h"

#include "misc/SDL2pp.h"

namespace {

#if defined(__clang_version__)
void log_clang() {
    sdl2::log_info("Compiler: Clang " __clang_version__);
}
#elif defined(__GNUC_PATCHLEVEL__)
void log_gcc() {
#    ifdef __MINGW32__
    sdl2::log_info("Compiler: MinGW %d.%d.%d", __GNUC__, __GNUC_MINOR__, __GNUC_PATCHLEVEL__);
#    else
    sdl2::log_info("Compiler: GCC %d.%d.%d", __GNUC__, __GNUC_MINOR__, __GNUC_PATCHLEVEL__);
#    endif
}
#elif defined(_MSC_VER)
void log_msvc() {
#    if defined(_MSC_FULL_VER)
    sdl2::log_info("Compiler: MSVC %d.%d.%d.%d", _MSC_VER / 100, _MSC_VER % 100, _MSC_FULL_VER % 100000, _MSC_BUILD);

    sdl2::log_info("   MSVC runtime: "
#        if defined(_MT)
                   "MT "
#        endif
#        if defined(_DLL)
                   "DLL"
#        else
                   "Static"
#        endif
    );
#    endif // _MSC_FULL_VER

    sdl2::log_info("   Instruction set: "
#    if defined(_M_IX86)
                   "x86"
#    elif defined(_M_X64)
                   "x64"
#    elif defined(_M_ARM64)
                   "ARM64"
#    elif defined(_M_ARM)
                   "ARM"
#    else
                   "Unknown"
#    endif
                   "/"
#    if defined(__AVX512F__)
                   "AVX512"
#    elif defined(__AVX2__)
                   "AVX2"
#    elif defined(__AVX__)
                   "AVX"
#    elif defined(_M_IX86_FP)
#        if _M_IX86_FP == 0
                   "x87 FPU"
#        elif _M_IX86_FP == 1
                   "SSE"
#        elif _M_IX86_FP == 2
                   "SSE2"
#        else
                   "Unknown"
#        endif
#    else
                   "Default"
#    endif
    );

#    if defined(_CONTROL_FLOW_GUARD)
    sdl2::log_info("   Control flow guard");
#    endif
}
#endif

} // namespace

void dune::log_compiler_info() {
#if defined(__clang_version__)
    log_clang();
#elif defined(__GNUC_PATCHLEVEL__)
    log_gcc();
#elif defined(_MSC_VER)
    log_msvc();
#endif // _MSC_VER
}
