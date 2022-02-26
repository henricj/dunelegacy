#ifndef STRING_ERROR_H
#define STRING_ERROR_H

#include <fmt/core.h>

#include <array>
#include <cstring>

inline std::string string_error(int errnum) {
    std::array<char, 256> buffer;

    const auto error = strerror_s(buffer.data(), buffer.size(), errnum);

    if(0 == error)
        return buffer.data();

    return fmt::format(buffer.data(), buffer.size(), "error {}", errnum);
}

#endif // STRING_ERROR_H
