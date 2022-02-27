#ifndef DUNE_LOCALTIME_H
#define DUNE_LOCALTIME_H

#include <ctime>
#include <optional>

namespace dune {

inline std::optional<std::tm> dune_localtime() {
    time_t rawtime;
    std::time(&rawtime);

    std::tm timeinfo;

#if HAVE_MS_LOCALTIME_S
    if(const auto error = localtime_s(&timeinfo, &rawtime))
        return std::nullopt;
#elif HAVE_LOCALTIME_R
    if(const auto* result = localtime_r(&rawtime, &timeinfo); !result)
        return std::nullopt;
#else
    if(const auto* result = localtime(&rawtime))
        timeinfo = *result;
    else
        return std::nullopt;
#endif

    return timeinfo;
}

} // namespace dune

#endif // DUNE_LOCALTIME
