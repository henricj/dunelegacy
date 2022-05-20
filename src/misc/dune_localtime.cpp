#include "misc/dune_localtime.h"

#include <ctime>
#include <optional>
#include <string>

std::optional<std::tm> dune::dune_localtime() {
    time_t raw_time;
    std::time(&raw_time);

    std::tm time_info{};

#if HAVE_MS_LOCALTIME_S
    if (const auto error = localtime_s(&time_info, &raw_time))
        return std::nullopt;
#elif HAVE_LOCALTIME_R
    if (const auto* result = localtime_r(&raw_time, &time_info); !result)
        return std::nullopt;
#else
    if (const auto* result = localtime(&raw_time))
        time_info = *result;
    else
        return std::nullopt;
#endif

    return time_info;
}

std::string dune::dune_localtime_string() {
    const auto time_info = dune::dune_localtime();

    if (!time_info.has_value())
        return "<unknown>";

    char buffer[128];

    const auto size = strftime(buffer, std::size(buffer), "%F %T %Z (%z)", &time_info.value());

    if (size > 0 && size < sizeof buffer)
        return buffer;

    return "<invalid>";
}
