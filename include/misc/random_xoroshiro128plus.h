#ifndef RANDOM_XOROSHIRO128PLUS_H
#define RANDOM_XOROSHIRO128PLUS_H

#include <array>
#include <random>

namespace ExtraGenerators {
using namespace std;

class xoroshiro128plus {
    // From http://xorshift.di.unimi.it/
    // http://xorshift.di.unimi.it/xoroshiro128plus.c
    // "Written in 2016 by David Blackman and Sebastiano Vigna (vigna@acm.org)"
    uint64_t s0_;
    uint64_t s1_;

    static uint64_t rotl(const uint64_t x, int k) { return (x << k) | (x >> (64 - k)); }

public:
    typedef uint64_t result_type;

    explicit xoroshiro128plus(result_type x = 1) { seed(x); }

    template<class Seq>
    explicit xoroshiro128plus(Seq& seq) {
        seed(seq);
    }

    result_type operator()() {
        const result_type s0  = s0_;
        result_type s1        = s1_;
        const uint64_t result = s0 + s1;

        s1 ^= s0;
        s0_ = rotl(s0, 55) ^ s1 ^ (s1 << 14);
        s1_ = rotl(s1, 36);

        return result;
    }

    static constexpr result_type min() { return numeric_limits<result_type>::min(); }

    static constexpr result_type max() { return numeric_limits<result_type>::max(); }

    static constexpr size_t seed_words =
        (sizeof(xoroshiro128plus::s0_) + sizeof(xoroshiro128plus::s1_)) / sizeof(unsigned int);

    void seed(result_type s) {
        seed_seq seq{s};

        seed(seq);
    }

    template<class Seq>
    void seed(Seq& seq) {
        array<unsigned int, seed_words> buffer;

        seq.generate(begin(buffer), end(buffer));

        s0_ = buffer[0];
        s0_ <<= 32;
        s0_ |= buffer[1];

        s1_ = buffer[2];
        s1_ <<= 32;
        s1_ |= buffer[3];
    }

    void discard(unsigned long long count) { (*this)(); }
};
} // namespace ExtraGenerators

#endif // RANDOM_XOROSHIRO128PLUS_H
