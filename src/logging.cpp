#include "logging.h"

#include "enet/enet.h"
#include "fmt/core.h"
#include "lodepng.h"

#include "dune_version.h"

#include "misc/SDL2pp.h"

#include <SDL2/SDL.h>

#include <fmt/core.h>

#include <filesystem>

#ifdef _WIN32
#    ifndef WIN32_LEAN_AND_MEAN
#        define WIN32_LEAN_AND_MEAN
#    endif
#    include <Psapi.h>
#    include <Windows.h>
#    include <wow64apiset.h>

#    include <cstdio>
#    include <io.h>
#    ifndef STDOUT_FILENO
#        define STDOUT_FILENO 1
#    endif
#    ifndef STDERR_FILENO
#        define STDERR_FILENO 2
#    endif

#else
#    include <pwd.h>
#    include <sys/types.h>
#    include <unistd.h>
#endif

namespace {

constexpr auto scale_MiB = 1.0 / (1024 * 1024);
constexpr auto scale_GiB = 1.0 / (1024 * 1024 * 1024);

#if defined(__clang_version__)
void log_clang() {
    sdl2::log_info("   Compiler: clang " __clang_version__);
}
#elif defined(__GNUC_PATCHLEVEL__)
void log_gcc() {
#    ifdef __MINGW32__
    sdl2::log_info("   Compiler: MinGW %d.%d.%d", __GNUC__, __GNUC_MINOR__, __GNUC_PATCHLEVEL__);
#    else
    sdl2::log_info("   Compiler: GCC %d.%d.%d", __GNUC__, __GNUC_MINOR__, __GNUC_PATCHLEVEL__);
#    endif
}
#elif defined(_MSC_VER)
void log_msvc() {
#    if defined(_MSC_FULL_VER)
    sdl2::log_info("   Compiler: MSVC %d.%d.%d.%d", _MSC_VER / 100, _MSC_VER % 100, _MSC_FULL_VER % 100000, _MSC_BUILD);

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

#if defined(NTDDI_VERSION)
void log_windows_sdk() {
#    ifdef DUNE_WINDOWS_SDK_VERSION
    sdl2::log_info("   Windows SDK " DUNE_WINDOWS_SDK_VERSION " (%d.%d.%d.%d)", OSVER(WDK_NTDDI_VERSION) >> 24,
                   0xff & OSVER(WDK_NTDDI_VERSION) >> 16, SPVER(WDK_NTDDI_VERSION), SUBVER(WDK_NTDDI_VERSION));
#    else
    sdl2::log_info("   Windows SDK %d.%d.%d.%d", OSVER(WDK_NTDDI_VERSION) >> 24,
                   0xff & (OSVER(WDK_NTDDI_VERSION) >> 16), SPVER(WDK_NTDDI_VERSION), SUBVER(WDK_NTDDI_VERSION));
#    endif
    sdl2::log_info("   Minimum Windows %d.%d.%d.%d", OSVER(NTDDI_VERSION) >> 24, 0xff & OSVER(NTDDI_VERSION) >> 16,
                   SPVER(NTDDI_VERSION), SUBVER(NTDDI_VERSION));
}
#endif // defined(NTDDI_VERSION)

void log_build_info() {
    sdl2::log_info("   %d bit build, C++ standard %d", 8 * sizeof(void*), __cplusplus);

#if defined(DEBUG)
    sdl2::log_info("   *** DEBUG build " __DATE__ " " __TIME__);
#endif

#if defined(DUNE_GIT_DESCRIBE)
#    if defined(DUNE_GIT_REPO_BRANCH)
    sdl2::log_info("   git " DUNE_GIT_REPO_BRANCH "/" DUNE_GIT_DESCRIBE " " DUNE_GIT_TIME);
#    else
    sdl2::log_info("   git " DUNE_GIT_DESCRIBE " " DUNE_GIT_TIME);
#    endif
    sdl2::log_info("   git " DUNE_GIT_REPO_URL);
#endif

#if defined(__SANITIZE_ADDRESS__)
    sdl2::log_info("   *** Address Sanitizer enabled");
#endif

#if defined(__clang_version__)
    log_clang();
#elif defined(__GNUC_PATCHLEVEL__)
    log_gcc();
#elif defined(_MSC_VER)
    log_msvc();
#endif // _MSC_VER

#if defined(NTDDI_VERSION)
    log_windows_sdk();
#endif

#if defined(FMT_VERSION)
    sdl2::log_info("fmt %d.%d.%d", FMT_VERSION / 10000, FMT_VERSION / 100 % 100, FMT_VERSION % 100);
#endif

    sdl2::log_info("lodepng %s", LODEPNG_VERSION_STRING);

#if defined(ENET_VERSION_MAJOR) && defined(ENET_VERSION_MINOR) && defined(ENET_VERSION_PATCH)
    sdl2::log_info("enet %d.%d.%d", ENET_VERSION_MAJOR, ENET_VERSION_MINOR, ENET_VERSION_PATCH);
#endif
}

#if defined(_WIN32)
namespace {

std::optional<HKEY> open_registry_key(HKEY root, const char* path) {
    HKEY result;
    if (ERROR_SUCCESS != RegOpenKeyA(root, path, &result))
        return std::nullopt;

    return result;
}

std::string read_registry_string(HKEY hKey, const char* name) {

    std::string value;

    // Get the size...

    DWORD type;
    DWORD size;

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

} // namespace
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
    if (auto it = processor_types.find(processor_type); it != processor_types.end())
        return it->second;

    return "unknown";
}

void log_computer_info_win32() {

    const auto name = read_windows_product_name();

    OSVERSIONINFOEX os_version{sizeof(OSVERSIONINFOEX)};
    if (GetVersionExA(reinterpret_cast<OSVERSIONINFO*>(&os_version))) {
        sdl2::log_info("System %s (%d.%d.%d)", name, os_version.dwMajorVersion, os_version.dwMinorVersion,
                       os_version.dwBuildNumber);
    } else
        sdl2::log_info("System %s", name);

    { // Scope
        SYSTEM_INFO system_info;
        GetNativeSystemInfo(&system_info);

        sdl2::log_info("       %d core %s", system_info.dwNumberOfProcessors,
                       get_processor_type(system_info.wProcessorArchitecture));
    }

    { // Scope
        MEMORYSTATUSEX memory_status{sizeof(MEMORYSTATUSEX)};
        GlobalMemoryStatusEx(&memory_status);

        const auto free_GIB  = memory_status.ullAvailPhys * scale_GiB;
        const auto total_GiB = memory_status.ullTotalPhys * scale_GiB;
        sdl2::log_info("       %.1f GiB free of %.1f GiB total memory", free_GIB, total_GiB);
    }

    { // Scope
        BOOL is_wow64;
        if (IsWow64Process(GetCurrentProcess(), &is_wow64)) {
            if (is_wow64)
                sdl2::log_info("       WoW64 detected");
        }
    }
}

namespace {

// C++20 has std::chrono::file_time...
// A FILETIME tick is 100ns
using file_time_duration = std::chrono::duration<uint64_t, std::ratio<100 * std::nano::num, std::nano::den>>;

file_time_duration convert_to_file_time_duration(FILETIME& time) {
    auto v = static_cast<uint64_t>(time.dwHighDateTime) << 32u;
    v |= time.dwLowDateTime;

    return file_time_duration{v};
}

template<typename TDuration>
std::string format_duration(TDuration d) {

    const auto hours = std::chrono::duration_cast<std::chrono::hours>(d);
    d -= hours;

    const auto minutes = std::chrono::duration_cast<std::chrono::minutes>(d);
    d -= minutes;

    const auto seconds = std::chrono::duration_cast<std::chrono::seconds>(d);
    d -= seconds;

    auto milliseconds = std::chrono::duration_cast<std::chrono::milliseconds>(d);

    return fmt::format("{}:{:02}:{:02}.{:03}", hours.count(), minutes.count(), seconds.count(), milliseconds.count());
}

} // namespace

void log_process_info_win32() {
    PROCESS_MEMORY_COUNTERS pmc;
    if (GetProcessMemoryInfo(GetCurrentProcess(), &pmc, sizeof pmc)) {
        const auto working_set_MiB     = pmc.WorkingSetSize * scale_MiB;
        const auto working_set_max_MiB = pmc.PeakWorkingSetSize * scale_MiB;

        sdl2::log_info("Process working set %.f MiB of %.f MiB maximum", working_set_MiB, working_set_max_MiB);
    } else
        sdl2::log_info("Process working set unknown");

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

#endif // defined(_WIN32)

void log_computer_info() {
#if defined(_WIN32)
    log_computer_info_win32();
#endif // defined(_WIN32)
}

void log_process_info() {
#if defined(_WIN32)
    log_process_info_win32();
#endif
}

std::filesystem::path getLogFilepath() {
    // determine path to config file
    auto [ok, tmp] = fnkdat(LOGFILENAME, FNKDAT_USER | FNKDAT_CREAT);

    if (!ok) {
        THROW(std::runtime_error, "fnkdat() failed!");
    }

    return tmp;
}

void logOutputFunction(void* userdata, int category, SDL_LogPriority priority, const char* message) {

    static constexpr std::string_view priorityStrings[] = {"<UNK> ",   "VERBOSE ", "DEBUG   ", "INFO    ",
                                                           "WARN    ", "ERROR   ", "CRITICAL"};

    static constexpr auto priorityStringsSize = static_cast<int>(std::size(priorityStrings));

    const auto n = static_cast<int>(priority);

    const auto priority_name = n >= 0 && n < priorityStringsSize ? priorityStrings[n] : priorityStrings[0];

    const auto output = fmt::format("{}:   {}\n", priority_name, message);
#ifdef _WIN32
    OutputDebugStringA(output.c_str());
#endif
    fmt::fprintf(stderr, output);
    fflush(stderr);
}

} // namespace

namespace dune {

void logging_initialize() {
    SDL_LogSetOutputFunction(logOutputFunction, nullptr);
    SDL_LogSetAllPriority(SDL_LOG_PRIORITY_WARN);
    SDL_LogSetPriority(SDL_LOG_CATEGORY_APPLICATION, SDL_LOG_PRIORITY_VERBOSE);
}

void logging_configure(bool capture_output) {
    if (capture_output) {
        // get utf8-encoded log file path
        const auto logfilePath = getLogFilepath();

#ifdef _WIN32

        FILE* discard = nullptr;
        if (freopen_s(&discard, R"(\\.\NUL)", "r", stdin))
            THROW(io_error, "Initializing stdin failed!");
        if (freopen_s(&discard, R"(\\.\NUL)", "a", stdout))
            THROW(io_error, "Initializing stdout failed!");
        if (freopen_s(&discard, R"(\\.\NUL)", "a", stderr))
            THROW(io_error, "Initializing stderr failed!");

        const auto fn_out = _fileno(stdout);
        const auto fn_err = _fileno(stderr);

        const auto wLogFilePath = logfilePath.wstring();

        const auto log_handle = CreateFileW(wLogFilePath.c_str(), GENERIC_WRITE, FILE_SHARE_READ, nullptr,
                                            CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL | FILE_FLAG_SEQUENTIAL_SCAN, nullptr);

        if (log_handle == INVALID_HANDLE_VALUE) {
            // use stdout in this error case as stderr is not yet ready
            THROW(io_error, "Opening logfile '%s' as stdout failed!", logfilePath.string().c_str());
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

#else

        char* pLogfilePath = (char*)logfilePath.c_str();

        int d = open(pLogfilePath, O_WRONLY | O_CREAT | O_TRUNC, 0644);
        if (d < 0) {
            THROW(io_error, "Opening logfile '%s' failed!", pLogfilePath);
        }
        // Hint: fileno(stdout) != STDOUT_FILENO on Win32
        if (dup2(d, fileno(stdout)) < 0) {
            THROW(io_error, "Redirecting stdout failed!");
        }

        // Hint: fileno(stderr) != STDERR_FILENO on Win32
        if (dup2(d, fileno(stderr)) < 0) {
            THROW(io_error, "Redirecting stderr failed!");
        }

#endif
    }

    sdl2::log_info("Starting Dune Legacy %s on %s", VERSION, SDL_GetPlatform());

    log_computer_info();

    log_process_info();

    log_build_info();
}

void logging_complete() {

    log_process_info();

    std::fflush(stderr);
    std::fflush(stdout);
}

} // namespace dune
