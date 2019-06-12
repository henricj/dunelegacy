#ifndef RANDOM_XOSHIRO256STARSTAR_H
#define RANDOM_XOSHIRO256STARSTAR_H
#pragma once

#include <array>
#include <random>

namespace ExtraGenerators
{
using namespace std;

class xoshiro256starstar
{
    // From http://xorshift.di.unimi.it/
    // http://xoshiro.di.unimi.it/xoshiro256starstar.c
    // "Written in 2018 by David Blackman and Sebastiano Vigna (vigna@acm.org)"
    uint64_t s0_;
    uint64_t s1_;
    uint64_t s2_;
    uint64_t s3_;

    static uint64_t rotl(const uint64_t x, int k)
    {
        return (x << k) | (x >> (64 - k));
    }

public:
    typedef uint64_t result_type;

    explicit xoshiro256starstar(result_type x = 1)
    {
        seed(x);
    }

    template <class Seq>
    explicit xoshiro256starstar(Seq& seq)
    {
        seed(seq);
    }

    result_type operator()()
    {
        const uint64_t result_starstar = rotl(s1_ * 5, 7) * 9;

        const uint64_t t = s1_ << 17;

        s2_ ^= s0_;
        s3_ ^= s1_;
        s1_ ^= s2_;
        s0_ ^= s3_;

        s2_ ^= t;

        s3_ = rotl(s3_, 45);

        return result_starstar; 
    }

    static constexpr result_type min()
    {
        return numeric_limits<result_type>::min();
    }

    static constexpr result_type max()
    {
        return numeric_limits<result_type>::max();
    }

    static constexpr size_t seed_words = (4 * sizeof(xoshiro256starstar::s0_)) / sizeof(unsigned int);

    void seed(result_type s)
    {
        seed_seq seq{ s };

        seed(seq);
    }

    template <class Seq>
    void seed(Seq& seq)
    {
        array<unsigned int, seed_words> buffer;

        seq.generate(begin(buffer), end(buffer));

        s0_ = buffer[0];
        s0_ <<= 32;
        s0_ |= buffer[1];

        s1_ = buffer[2];
        s1_ <<= 32;
        s1_ |= buffer[3];

        s2_ = buffer[4];
        s2_ <<= 32;
        s2_ |= buffer[5];

        s3_ = buffer[6];
        s3_ <<= 32;
        s3_ |= buffer[7];
    }

    void discard(unsigned long long count)
    {
        (*this)();
    }
};
}

#endif // RANDOM_XOSHIRO256STARSTAR_H
