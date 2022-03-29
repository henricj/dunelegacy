#ifndef LEMIRE_UNIFORM_UINT32_DISTRIBUTION_H
#define LEMIRE_UNIFORM_UINT32_DISTRIBUTION_H
#pragma once

#include <cstdint>
#include <limits>

// Generate uniform random numbers in the closed range [0U, upper_bound]
// It requires a generator with uniform output range [0U, ~0U]
class lemire_uniform_uint32_distribution {
    const uint32_t upper_bound_;

public:
    explicit lemire_uniform_uint32_distribution(uint32_t inclusive_upper_bound)
        : upper_bound_(inclusive_upper_bound) { }

    template<typename Generator>
    static uint32_t generate(Generator& generator, uint32_t inclusive_upper_bound) {
        static_assert(Generator::min() == 0 && Generator::max() == std::numeric_limits<uint32_t>::max(),
                      "The generator must be full range");

        // From https://github.com/lemire/FastShuffleExperiments/blob/master/TOMACS_RCR/cpp/rangedrand.h
        uint64_t random32bit = generator();
        auto multi_result    = random32bit * inclusive_upper_bound;
        auto leftover        = static_cast<uint32_t>(multi_result);

        if (leftover < inclusive_upper_bound) {
            // Avoid unary minus on an unsigned integer (many tools complain about iT).
            const auto threshold = (0u - inclusive_upper_bound) % inclusive_upper_bound;

            while (leftover < threshold) {
                random32bit  = generator();
                multi_result = random32bit * inclusive_upper_bound;
                leftover     = static_cast<uint32_t>(multi_result);
            }
        }

        return multi_result >> 32; // [0, range)
    }

    template<typename Generator>
    uint32_t operator()(Generator& generator) {
        return generate(generator, upper_bound_);
    }

    template<typename Generator>
    uint32_t operator()(Generator& generator, uint32_t inclusive_upper_bound) {
        return generate(generator, inclusive_upper_bound);
    }
};

#endif // LEMIRE_UNIFORM_UINT32_DISTRIBUTION_H
