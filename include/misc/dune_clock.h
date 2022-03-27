#ifndef DUNE_CLOCK_H
#define DUNE_CLOCK_H

#include <chrono>

namespace dune {
using dune_clock = std::chrono::steady_clock;

template<typename TTick = uint32_t>
constexpr auto as_dune_clock_duration(TTick milliseconds) {
    return std::chrono::duration_cast<dune_clock::duration>(std::chrono::duration<TTick, std::milli>(milliseconds));
}

template<typename TTick = uint32_t>
constexpr auto as_milliseconds(dune_clock::duration duration) {
    return std::chrono::duration_cast<std::chrono::duration<TTick, std::milli>>(duration).count();
}
} // namespace dune

#endif // DUNE_CLOCK_H
