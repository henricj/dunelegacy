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

#include "dune_version.h"

#include "misc/SDL2pp.h"
#include "misc/string_util.h"

#include <SDL2/SDL.h>

#include <fmt/core.h>

#include <fcntl.h>
#include <filesystem>
#include <optional>
#include <unordered_map>

#ifndef WIN32_LEAN_AND_MEAN
#    define WIN32_LEAN_AND_MEAN
#endif
#include <Psapi.h>
#include <Windows.h>
#include <winternl.h>
#include <wow64apiset.h>

#include <cstdio>
#include <io.h>

namespace {

constexpr auto scale_MiB = 1.0 / (1024 * 1024);
constexpr auto scale_GiB = 1.0 / (1024 * 1024 * 1024);

std::optional<HKEY> open_registry_key(HKEY root, const char* path) {
    HKEY result = nullptr;
    if (ERROR_SUCCESS != RegOpenKeyA(root, path, &result))
        return std::nullopt;

    return result;
}

std::string read_registry_string(HKEY hKey, const char* name) {

    std::string value;

    // Get the size...

    DWORD type = 0;
    DWORD size = 0;

    { // Scope
        const auto result = RegQueryValueExA(hKey, name, nullptr, &type, nullptr, &size);
        if (result != ERROR_SUCCESS || type != REG_SZ || size < 2)
            return {};
    }

    // Now read the actual string value.

    value.resize(size);
    size = value.size();

    { // Scope
        const auto result = RegQueryValueExA(hKey, name, nullptr, &type, reinterpret_cast<LPBYTE>(value.data()), &size);
        if (result != ERROR_SUCCESS || type != REG_SZ)
            return {};
    }

    // Drop the trailing NUL
    value.resize(value.size() - 1);

    return value;
}

std::string read_windows_product_name() {
    const auto key = open_registry_key(HKEY_LOCAL_MACHINE, R"(SOFTWARE\Microsoft\Windows NT\CurrentVersion)");

    if (key.has_value())
        return read_registry_string(key.value(), "ProductName");

    return "<unknown>";
}

const std::unordered_map<DWORD, std::string> processor_types = {{PROCESSOR_ARCHITECTURE_AMD64, "x64"},
                                                                {PROCESSOR_ARCHITECTURE_ARM, "arm"},
                                                                {PROCESSOR_ARCHITECTURE_ARM64, "arm64"},
                                                                {PROCESSOR_ARCHITECTURE_IA64, "Itanium"},
                                                                {PROCESSOR_ARCHITECTURE_INTEL, "x86"}};

std::string_view get_processor_type(DWORD processor_type) {
    if (const auto it = processor_types.find(processor_type); it != processor_types.end())
        return it->second;

    return "unknown";
}

// C++20 has std::chrono::file_time...
// A FILETIME tick is 100ns
using file_time_duration = std::chrono::duration<uint64_t, std::ratio<100 * std::nano::num, std::nano::den>>;

file_time_duration convert_to_file_time_duration(FILETIME& time) {
    auto v = static_cast<uint64_t>(time.dwHighDateTime) << 32u;
    v |= time.dwLowDateTime;

    return file_time_duration{v};
}

void log_windows_version() {
    const auto name = read_windows_product_name();

    { // Scope
        using RtlGetVersionType = NTSTATUS(WINAPI*)(LPOSVERSIONINFOEXW);

        if (const auto ntdll_module = GetModuleHandleA("ntdll")) {
            // We disable C4191 here since GetProcAddress does return FARPROC, but the actual function really isn't
            // FARPROC.
#pragma warning(push)
#pragma warning(disable : 4191)
            auto* const RtlGetVersion =
                reinterpret_cast<RtlGetVersionType>(GetProcAddress(ntdll_module, "RtlGetVersion"));
#pragma warning(pop)

            if (RtlGetVersion) {
                RTL_OSVERSIONINFOEXW os_version{sizeof(os_version)};
                if (NT_SUCCESS(RtlGetVersion(&os_version))) {
                    sdl2::log_info("System: %s (%d.%d.%d)", name, os_version.dwMajorVersion, os_version.dwMinorVersion,
                                   os_version.dwBuildNumber);
                    return;
                }
            }
        }
    }

#if 0
	{ // Scope
        OSVERSIONINFOEX os_version{sizeof(OSVERSIONINFOEX)};
        if (GetVersionExA(reinterpret_cast<OSVERSIONINFO*>(&os_version))) {
            sdl2::log_info("System %s (%d.%d.%d)", name, os_version.dwMajorVersion, os_version.dwMinorVersion,
                           os_version.dwBuildNumber);
            return;
        }
    }

#endif // 0

    sdl2::log_info("System %s", name);
}

} // namespace

namespace dune {

void log_computer_info() {

    log_windows_version();

    { // Scope
        SYSTEM_INFO system_info;
        GetNativeSystemInfo(&system_info);

        sdl2::log_info("       %d core %s", system_info.dwNumberOfProcessors,
                       get_processor_type(system_info.wProcessorArchitecture));
    }

    { // Scope
        MEMORYSTATUSEX memory_status{sizeof(MEMORYSTATUSEX)};
        GlobalMemoryStatusEx(&memory_status);

        const auto free_GIB  = static_cast<double>(memory_status.ullAvailPhys) * scale_GiB;
        const auto total_GiB = static_cast<double>(memory_status.ullTotalPhys) * scale_GiB;
        sdl2::log_info("       %.1f GiB free of %.1f GiB total memory", free_GIB, total_GiB);
    }

    { // Scope
        BOOL is_wow64{};
        if (IsWow64Process(GetCurrentProcess(), &is_wow64)) {
            if (is_wow64)
                sdl2::log_info("       WoW64 detected");
        }
    }
}

void log_process_info() {
    PROCESS_MEMORY_COUNTERS pmc;
    if (GetProcessMemoryInfo(GetCurrentProcess(), &pmc, sizeof pmc)) {
        const auto working_set_MiB     = pmc.WorkingSetSize * scale_MiB;
        const auto working_set_max_MiB = pmc.PeakWorkingSetSize * scale_MiB;

        sdl2::log_info("Process: working set %.f MiB of %.f MiB maximum", working_set_MiB, working_set_max_MiB);
    } else
        sdl2::log_info("Process: working set unknown");

    FILETIME create_time;
    FILETIME exit_time;
    FILETIME kernel_time;
    FILETIME user_time;
    if (GetProcessTimes(GetCurrentProcess(), &create_time, &exit_time, &kernel_time, &user_time)) {
        const auto user_duration   = convert_to_file_time_duration(user_time);
        const auto kernel_duration = convert_to_file_time_duration(kernel_time);

        sdl2::log_info("        cpu time %s user %s kernel", format_duration(user_duration),
                       format_duration(kernel_duration));
    }
}

void log_sdk_info() {
#ifdef DUNE_WINDOWS_SDK_VERSION
    sdl2::log_info("   Windows SDK " DUNE_WINDOWS_SDK_VERSION " (%d.%d.%d.%d)", OSVER(WDK_NTDDI_VERSION) >> 24,
                   0xff & OSVER(WDK_NTDDI_VERSION) >> 16, SPVER(WDK_NTDDI_VERSION), SUBVER(WDK_NTDDI_VERSION));
#else
    sdl2::log_info("   Windows SDK %d.%d.%d.%d", OSVER(WDK_NTDDI_VERSION) >> 24,
                   0xff & (OSVER(WDK_NTDDI_VERSION) >> 16), SPVER(WDK_NTDDI_VERSION), SUBVER(WDK_NTDDI_VERSION));
#endif
    sdl2::log_info("   Minimum Windows %d.%d.%d.%d", OSVER(NTDDI_VERSION) >> 24, 0xff & OSVER(NTDDI_VERSION) >> 16,
                   SPVER(NTDDI_VERSION), SUBVER(NTDDI_VERSION));
}

void log_capture_output(const std::filesystem::path logfilePath) {
    FILE* discard{};

    if (freopen_s(&discard, R"(\\.\NUL)", "r", stdin))
        THROW(io_error, "Initializing stdin failed!");
    if (freopen_s(&discard, R"(\\.\NUL)", "a", stdout))
        THROW(io_error, "Initializing stdout failed!");
    if (freopen_s(&discard, R"(\\.\NUL)", "a", stderr))
        THROW(io_error, "Initializing stderr failed!");

    const auto fn_out = _fileno(stdout);
    const auto fn_err = _fileno(stderr);

    const auto wLogFilePath = logfilePath.wstring();

    const auto log_handle = CreateFileW(wLogFilePath.c_str(), GENERIC_WRITE, FILE_SHARE_READ, nullptr, CREATE_ALWAYS,
                                        FILE_ATTRIBUTE_NORMAL | FILE_FLAG_SEQUENTIAL_SCAN, nullptr);

    if (log_handle == INVALID_HANDLE_VALUE) {
        // use stdout in this error case as stderr is not yet ready
        THROW(io_error, "Opening logfile '%s' as stdout failed!",
              reinterpret_cast<const char*>(logfilePath.u8string().c_str()));
    }

    if (!SetStdHandle(STD_OUTPUT_HANDLE, log_handle))
        THROW(io_error, "Initializing output handle failed!");
    if (!SetStdHandle(STD_ERROR_HANDLE, log_handle))
        THROW(io_error, "Initializing error handle failed!");

    const auto log_fd = _open_osfhandle(reinterpret_cast<intptr_t>(log_handle), _O_TEXT);

    if (_dup2(log_fd, fn_out))
        THROW(io_error, "Redirecting output failed!");
    if (_dup2(log_fd, fn_err))
        THROW(io_error, "Redirecting error failed!");

    // No buffering
    setvbuf(stdout, nullptr, _IONBF, 0);
    setvbuf(stderr, nullptr, _IONBF, 0);
}

} // namespace dune
