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

#include <misc/SDL2pp.h>

#include "misc/RngSupport.h"
#include "misc/lemire_uniform_uint32_distribution.h"
#include "misc/random_uint64_to_uint32.h"
#include "misc/random_xoshiro256starstar.h"

#include <gsl/gsl>

class RandomFactory;

class Random final {
public:
    using generator_type = ExtraGenerators::uint64_to_uint32<ExtraGenerators::xoshiro256starstar>;
    static constexpr size_t seed_words =
        generator_type::state_words * sizeof(generator_type::state_type) / sizeof(Uint32);

protected:
    explicit Random(const generator_type& generator) : generator_{generator} { }
    explicit Random(generator_type&& generator) : generator_{std::move(generator)} { }

public:
    explicit Random(const Random& random) = default;
    Random& operator=(const Random& random) = default;

    /**
        Constructor which inits the seed value to seed
        \param  seed    the initial seed value
    */
    // explicit Random(Uint32 seed) : generator_{seed} { }

    /// Destructor
    ~Random();

    /**
        Sets the seed value to newSeed
        \param newSeed  the new seed value
    */
    void setSeed(gsl::span<const Uint32, seed_words> newSeed);
    void getSeed(gsl::span<Uint32, seed_words> seed) const;

    /**
        Returns the current seed value.
        \return the current seed value
    */
    [[nodiscard]] std::vector<Uint32> getSeed() const;

    /**
        Returns the maximum integer returned by rand()
        \return The maximum integer
    */
    constexpr Uint32 getMaxRandom() { return decltype(generator_)::max(); }

    /**
        Calculates a random number with the "Linear congruential generator" (see numerical recipes for more details)
        \return a random integer on interval [0; getMaxRandom()]
    */
    Uint32 rand() { return generator_(); }

    /**
        Calculates a random number
        Don't call this method if max < min.
        \param  min min is the smallest possible value that is returned
        \param  max max is the greatest possible value that is returned
        \return a random integer on interval [min; max]
    */
    Uint32 rand(Uint32 min, Uint32 max) { return lemire_uniform_uint32_distribution{max - min + 1}(generator_) + min; }

    /**
        Calculates a random number with the "Linear congruential generator" (see numerical recipes for more details)
        Don't call this method if max < min.
        \param  min min is the smallest possible value that is returned
        \param  max max is the greatest possible value that is returned
        \return a random integer on interval [min; max]
    */
    Sint32 rand(Sint32 min, Sint32 max) {
        const auto umax = static_cast<Uint32>(max - min);

        return static_cast<Sint32>(lemire_uniform_uint32_distribution{umax + 1}(generator_)) + min;
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
    generator_type generator_;
    static const FixPoint rand_scale_;

    friend class RandomFactory;
};

class RandomFactory final {
public:
    static constexpr int seed_size = 64;

    RandomFactory() {
        const auto seed = createRandomSeed("Default");

        setSeed(seed);
    }

    RandomFactory(gsl::span<Uint8> seed) { setSeed(seed); }

    void               setSeed(gsl::span<const Uint8> seed);
    [[nodiscard]] std::vector<Uint8> getSeed() const;

    [[nodiscard]] Random create(const std::string_view& name) const;

    static std::vector<Uint8> createRandomSeed(const std::string_view& name);

private:
    std::vector<Uint8> seed_;
    std::array<unsigned char, seed_size> key_;

    bool initialized_{};
};

#if 0
/// A class for generating random numbers (there are better algorithms but this one is quite fast)
class Random final {
public:
    /// Default constructor.
    Random() {
        seed = 0;
    };

    /**
        Constructor which inits the seed value to seed
        \param  seed    the initial seed value
    */
    explicit Random(Uint32 seed) {
        setSeed(seed);
    }

    /// Destructor
    ~Random();

    /**
        Sets the seed value to newSeed
        \param newSeed  the new seed value
    */
    void setSeed(Uint32 newSeed) {
        seed = newSeed;
    }

    /**
        Returns the current seed value.
        \return the current seed value
    */
    Uint32 getSeed() const {
        return seed;
    }

    /**
        Returns the maximum integer returned by rand()
        \return The maximum integer
    */
    Uint32 getMaxRandom() const {
        return 2147483646;
    }

    /**
        Calculates a random number with the "Linear congruential generator" (see numerical recipes for more details)
        \return a random integer on interval [0; getMaxRandom()]
    */
    Uint32 rand() {
        const Sint32 IA = 16807;
        const Sint32 IM = 2147483647;
        const Sint32 IQ = 127773;
        const Sint32 IR = 2836;
        const Uint32 MASK = 123459876;

        Uint32 k;

        Sint32 idum = seed ^ MASK;
        k = idum / IQ;
        idum = IA*(idum - k*IQ) - IR*k;
        if(idum < 0) {
            idum += IM;
        }

        seed = idum ^ MASK;
        return idum;
    }

    /**
        Calculates a random number with the "Linear congruential generator" (see numerical recipes for more details)
        Don't call this method if max < min.
        \param  min min is the smallest possible value that is returned
        \param  max max is the greatest possible value that is returned
        \return a random integer on interval [min; max]
    */
    Uint32 rand(Uint32 min, Uint32 max) {
        return (rand() % (max - min + 1))+min;
    }

    /**
        Calculates a random number with the "Linear congruential generator" (see numerical recipes for more details)
        Don't call this method if max < min.
        \param  min min is the smallest possible value that is returned
        \param  max max is the greatest possible value that is returned
        \return a random integer on interval [min; max]
    */
    Sint32 rand(Sint32 min, Sint32 max) {
        return (((Sint32) rand()) % (max - min + 1))+min;
    }

    /**
        Returns an float value on the interval [0;1]
        \return a random float on [0;1]
    */
    float randFloat() {
        return ((float) rand()) / ((float) getMaxRandom());
    }

    /**
        Returns an double value on the interval [0;1]
        \return a random double on [0;1]
    */
    double randDouble() {
        return ((double) rand()) / ((double) getMaxRandom());
    }

    /**
        Returns an FixPoint value on the interval [0;1]
        \return a random FixPoint on [0;1]
    */
    FixPoint randFixPoint() {
        return FixPoint(rand()) / getMaxRandom();
    }



    /**
        Returns an boolean value
        \return true or false
    */
    bool randBool() {
        return (rand() % 2 == 0);
    }

    /**
        This method returns randomly one of the given parameters.
        \return one of the parameters, e.g. getRandOf({13,17,19}) returns 13, 17 or 19
    */
    template<typename T>
    T getRandOf(std::initializer_list<T> initlist) {
        return initlist.begin()[rand(0, static_cast<Sint32>(initlist.size()-1))];
    }

private:
    Uint32 seed;
};
#endif // 0

#endif // RANDOM_H
