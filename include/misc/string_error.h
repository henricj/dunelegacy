#ifndef STRING_ERROR_H
#define STRING_ERROR_H

#include <fmt/core.h>

#include <array>

namespace dune {

inline std::string string_error(int errnum) {
#if defined(HAVE_STRERROR_S) || defined(HAVE_STRERROR_R) || defined(HAVE_GNU_STRERROR_R)
    std::array<char, 256> buffer{};
#endif

#if HAVE_STRERROR_S
    if (const auto error = strerror_s(buffer.data(), buffer.size(), errnum); 0 == error)
        return buffer.data();
#elif HAVE_STRERROR_R
    if (const auto error = strerror_r(errnum, buffer.data(), buffer.size()); 0 == error)
        return buffer.data();
#elif HAVE_GNU_STRERROR_R
    if (const auto* error_string = strerror_r(errnum, buffer.data(), buffer.size()))
        return error_string;
#else
    if (const auto* error_string = strerror(errnum))
        return error_string;
#endif

    return fmt::format("error {}", errnum);
}

} // namespace dune

#endif // STRING_ERROR_H
