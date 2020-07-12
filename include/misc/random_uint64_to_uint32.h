#ifndef RANDOM_UINT64_TO_UINT32_H
#define RANDOM_UINT64_TO_UINT32_H

#include <cstdint>

#include <gsl/gsl>

namespace ExtraGenerators {
using namespace std;

template<typename Generator>
class uint64_to_uint32 {
    static_assert(Generator::min() == std::numeric_limits<uint64_t>::min() &&
                  Generator::max() == std::numeric_limits<uint64_t>::max());

public:
    typedef uint32_t result_type;
    static constexpr size_t seed_words = Generator::seed_words;

    static constexpr size_t state_words = Generator::state_words;
    typedef typename Generator::state_type state_type;

    uint64_to_uint32() = default;
    explicit uint64_to_uint32(unsigned int seed) : generator_{seed} { }

    template<typename Seq>
    explicit uint64_to_uint32(Seq& seed_seq) : generator_{seed_seq} { }

    static constexpr result_type min() { return numeric_limits<result_type>::min(); }
    static constexpr result_type max() { return numeric_limits<result_type>::max(); }

    result_type operator()() {
        if(have_pending_) {
            have_pending_ = false;
            return pending_;
        }

        const auto result = generator_();

        pending_      = result >> 32;
        have_pending_ = true;

        return static_cast<result_type>(result);
    }

    void set_state(gsl::span<const state_type, state_words> s) { generator_.set_state(s); }
    void get_state(gsl::span<state_type, state_words> s) const noexcept { generator_.get_state(s); }


    void seed(unsigned int seed) {
        generator_.seed(seed);
        have_pending_ = false;
    }

    template<class Seq>
    void seed(Seq& seq) {
        generator_.seed(seq);
        have_pending_ = false;
    }

private:
    Generator   generator_;
    result_type pending_;
    bool        have_pending_{};
};

} // namespace ExtraGenerators

#endif // RANDOM_UINT64_TO_UINT32_H
