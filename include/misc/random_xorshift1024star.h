#ifndef RANDOM_XORSHIFT1024STAR_H
#define RANDOM_XORSHIFT1024STAR_H

#include <array>
#include <cstdint>
#include <random>

namespace ExtraGenerators {
using namespace std;

class xorshift1024star {
    // From http://xorshift.di.unimi.it/
    // http://xorshift.di.unimi.it/xorshift1024star.c
    // "Written in 2014-2015 by Sebastiano Vigna (vigna@acm.org)"
    int p_;
    uint64_t s_[16];

public:
    typedef uint64_t result_type;

    explicit xorshift1024star(result_type x = 1) {
        seed(x);
    }

    template<class Seq>
    explicit xorshift1024star(Seq& seq) {
        seed(seq);
    }

    result_type operator()() {
        const uint64_t s0 = s_[p_];
        p_                = (p_ + 1) & 15;
        uint64_t s1       = s_[p_];

        s1 ^= s1 << 31;

        const uint64_t sp = s1 ^ s0 ^ (s1 >> 11) ^ (s0 >> 30);
        s_[p_]            = sp;

        return sp * UINT64_C(1181783497276652981);
    }

    static constexpr result_type min() {
        return numeric_limits<result_type>::min();
    }

    static constexpr result_type max() {
        return numeric_limits<result_type>::max();
    }

    static constexpr size_t seed_words = sizeof(xorshift1024star::s_) / sizeof(unsigned int);

    void seed(result_type s) {
        std::seed_seq seq {s};

        seed(seq);
    }

    template<class Seq>
    void seed(Seq& seq) {
        array<unsigned int, seed_words> buffer;

        seq.generate(begin(buffer), end(buffer));

        auto pb = begin(buffer);
        for (auto s = begin(s_); s != end(s_); ++s) {
            uint64_t v = *pb++;

            v |= static_cast<uint64_t>(*pb++) << 32;

            *s = v;
        }

        p_ = 0;
    }

    void discard(unsigned long long count) {
        (*this)();
    }
};
} // namespace ExtraGenerators

#endif // RANDOM_XORSHIFT1024STAR_H
