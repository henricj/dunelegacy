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

#include <initializer_list>

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
    inline void setSeed(Uint32 newSeed) {
        seed = newSeed;
    }

    /**
        Returns the current seed value.
        \return the current seed value
    */
    inline Uint32 getSeed() const {
        return seed;
    }

    /**
        Returns the maximum integer returned by rand()
        \return The maximum integer
    */
    inline Uint32 getMaxRandom() const {
        return 2147483646;
    }

    /**
        Calculates a random number with the "Linear congruential generator" (see numerical recipes for more details)
        \return a random integer on interval [0; getMaxRandom()]
    */
    inline Uint32 rand() {
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
    inline Uint32 rand(Uint32 min, Uint32 max) {
        return (rand() % (max - min + 1))+min;
    }

    /**
        Calculates a random number with the "Linear congruential generator" (see numerical recipes for more details)
        Don't call this method if max < min.
        \param  min min is the smallest possible value that is returned
        \param  max max is the greatest possible value that is returned
        \return a random integer on interval [min; max]
    */
    inline Sint32 rand(Sint32 min, Sint32 max) {
        return (((Sint32) rand()) % (max - min + 1))+min;
    }

    /**
        Returns an float value on the interval [0;1]
        \return a random float on [0;1]
    */
    inline float randFloat() {
        return ((float) rand()) / ((float) getMaxRandom());
    }

    /**
        Returns an double value on the interval [0;1]
        \return a random double on [0;1]
    */
    inline double randDouble() {
        return ((double) rand()) / ((double) getMaxRandom());
    }

    /**
        Returns an FixPoint value on the interval [0;1]
        \return a random FixPoint on [0;1]
    */
    inline FixPoint randFixPoint() {
        return FixPoint(rand()) / getMaxRandom();
    }



    /**
        Returns an boolean value
        \return true or false
    */
    inline bool randBool() {
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

#endif // RANDOM_H
