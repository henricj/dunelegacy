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

#ifndef RANDOM_H
#define RANDOM_H

#include <fixmath/FixPoint.h>

#include "misc/lemire_uniform_uint32_distribution.h"
#include "misc/random_uint64_to_uint32.h"
#include "misc/random_xoshiro256starstar.h"

#include <array>
#include <span>
#include <vector>

class RandomFactory;

class Random final {
public:
    using generator_type = ExtraGenerators::uint64_to_uint32<ExtraGenerators::xoshiro256starstar>;

    static constexpr size_t state_bytes = generator_type::state_words * sizeof(generator_type::state_type);

protected:
    explicit Random(const generator_type& generator) : generator_{generator} { }

public:
    Random() = default;

    explicit Random(const Random& random)   = default;
    Random& operator=(const Random& random) = default;

    /// Destructor
    ~Random() = default;

    static Random create(std::span<const uint8_t> state) {
        generator_type generator;

        set_generator_state(generator, state);

        return Random(generator);
    }

    /**
        Sets the generator state to state
        \param state  the new state value
    */
    void setState(std::span<const uint8_t> state);

    /**
        Returns the current generator state.
        \return the current state
    */
    [[nodiscard]] std::array<uint8_t, state_bytes> getState() const;

    /**
        Returns the maximum integer returned by rand()
        \return The maximum integer
    */
    constexpr uint32_t getMaxRandom() { return decltype(generator_)::max(); }

    /**
        Calculates a random number with the "Linear congruential generator" (see numerical recipes for more details)
        \return a random integer on interval [0; getMaxRandom()]
    */
    uint32_t rand() { return generator_(); }

    /**
        Calculates a random number
        Don't call this method if max < min.
        \param  min min is the smallest possible value that is returned
        \param  max max is the greatest possible value that is returned
        \return a random integer on interval [min; max]
    */
    uint32_t rand(uint32_t min, uint32_t max) {
        return lemire_uniform_uint32_distribution{max - min + 1}(generator_) + min;
    }

    /**
        Calculates a random number with the "Linear congruential generator" (see numerical recipes for more details)
        Don't call this method if max < min.
        \param  min min is the smallest possible value that is returned
        \param  max max is the greatest possible value that is returned
        \return a random integer on interval [min; max]
    */
    int32_t rand(int32_t min, int32_t max) {
        const auto umax = static_cast<uint32_t>(max - min);

        return static_cast<int32_t>(lemire_uniform_uint32_distribution{umax + 1}(generator_)) + min;
    }

    /**
        Returns an FixPoint value on the interval [0;1]
        \return a random FixPoint on [0;1]
    */
    FixPoint randFixPoint() { return FixPoint::FromRawValue(rand()); }

    /**
        Returns an boolean value
        \return true or false
    */
    bool randBool() { return (rand() & (1 << 31)) == 0; }

    /**
        This method returns randomly one of the given parameters.
        \return one of the parameters, e.g. getRandOf({13,17,19}) returns 13, 17 or 19
    */
    template<typename T, typename... Args>
    T getRandOf(const T& first, const Args&... args) {
        std::array<T, sizeof...(Args) + 1> a{first, args...};

        return a[rand(0u, a.size() - 1u)];
    }

private:
    static void set_generator_state(generator_type& generator, std::span<const uint8_t> state);
    static void get_generator_state(const generator_type& generator, std::span<uint8_t, state_bytes> state);

    generator_type generator_;

    friend class RandomFactory;
};

class RandomFactory final {
public:
    static constexpr int seed_size = 64;

    RandomFactory() {
        const auto seed = createRandomSeed("Default");

        setSeed(seed);
    }

    RandomFactory(std::span<uint8_t> seed) { setSeed(seed); }

    void setSeed(std::span<const uint8_t> seed);
    [[nodiscard]] std::vector<uint8_t> getSeed() const;

    [[nodiscard]] Random create(std::string_view name) const;

    static std::vector<uint8_t> createRandomSeed(std::string_view name);

private:
    std::vector<uint8_t> seed_;
    std::array<unsigned char, seed_size> key_;

    bool initialized_{};
};

#endif // RANDOM_H
