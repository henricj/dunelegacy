#ifndef RANDOM_XOSHIRO256STARSTAR_H
#define RANDOM_XOSHIRO256STARSTAR_H
#pragma once

#include <array>
#include <gsl/gsl>
#include <random>

namespace ExtraGenerators {
using namespace std;

class xoshiro256starstar {
    // From http://xorshift.di.unimi.it/
    // http://prng.di.unimi.it/xoshiro256starstar.c
    // "Written in 2018 by David Blackman and Sebastiano Vigna (vigna@acm.org)"
    uint64_t s0_;
    uint64_t s1_;
    uint64_t s2_;
    uint64_t s3_;

    static uint64_t rotl(const uint64_t x, int k) { return (x << k) | (x >> (64 - k)); }

public:
    typedef uint64_t result_type;

    explicit xoshiro256starstar(unsigned int x = 1) { seed(x); }

    template<class Seq>
    explicit xoshiro256starstar(Seq& seq) {
        seed(seq);
    }

    result_type operator()() {
        const uint64_t result = rotl(s1_ * 5, 7) * 9;
        const uint64_t t      = s1_ << 17;

        s2_ ^= s0_;
        s3_ ^= s1_;
        s1_ ^= s2_;
        s0_ ^= s3_;

        s2_ ^= t;

        s3_ = rotl(s3_, 45);

        return result;
    }

    void jump() {
        static constexpr uint64_t JUMP[] = {0x180ec6d33cfd0aba, 0xd5a61266f0c9392c, 0xa9582618e03fc9aa,
                                            0x39abdc4529b1661c};

        uint64_t s0 = 0;
        uint64_t s1 = 0;
        uint64_t s2 = 0;
        uint64_t s3 = 0;
        for (const auto i : JUMP)
            for (int b = 0; b < 64; b++) {
                if (i & UINT64_C(1) << b) {
                    s0 ^= s0_;
                    s1 ^= s1_;
                    s2 ^= s2_;
                    s3 ^= s3_;
                }
                operator()();
            }

        s0_ = s0;
        s1_ = s1;
        s2_ = s2;
        s3_ = s3;
    }

    static constexpr result_type min() { return numeric_limits<result_type>::min(); }

    static constexpr result_type max() { return numeric_limits<result_type>::max(); }

    static constexpr size_t state_words = 4;
    typedef uint64_t state_type;

    void set_state(gsl::span<const state_type, state_words> s) noexcept {
        s0_ = s[0];
        s1_ = s[1];
        s2_ = s[2];
        s3_ = s[3];
    }

    void get_state(gsl::span<state_type, state_words> s) const noexcept {
        s[0] = s0_;
        s[1] = s1_;
        s[2] = s2_;
        s[3] = s3_;
    }

    static constexpr size_t seed_words = 4 * sizeof(s0_) / sizeof(unsigned int);

    void seed(unsigned int s) {
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

        s2_ = buffer[4];
        s2_ <<= 32;
        s2_ |= buffer[5];

        s3_ = buffer[6];
        s3_ <<= 32;
        s3_ |= buffer[7];
    }

    void discard(unsigned long long count) {
        while (count-- > 0)
            (*this)();
    }
};

} // namespace ExtraGenerators

#endif // RANDOM_XOSHIRO256STARSTAR_H
