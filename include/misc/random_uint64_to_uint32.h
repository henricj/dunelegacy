#ifndef RANDOM_UINT64_TO_UINT32_H
#define RANDOM_UINT64_TO_UINT32_H

#include <cstdint>

#include <gsl/gsl>

namespace ExtraGenerators {
using namespace std;

template<typename Generator>
class uint64_to_uint32 {
    static_assert(Generator::min() == std::numeric_limits<uint64_t>::min()
                  && Generator::max() == std::numeric_limits<uint64_t>::max());

public:
    typedef uint32_t result_type;
    static inline constexpr size_t seed_words = Generator::seed_words;

private:
    static_assert(sizeof(result_type) == sizeof(typename Generator::state_type)
                  || 2 * sizeof(result_type) == sizeof(typename Generator::state_type));

    static inline constexpr size_t local_state_bytes = 1 + sizeof(result_type);
    static inline constexpr size_t local_state_words = 1 + local_state_bytes / sizeof(typename Generator::state_type);

public:
    static inline constexpr size_t state_words = Generator::state_words + local_state_words;
    using state_type                           = typename Generator::state_type;

    uint64_to_uint32() = default;
    explicit uint64_to_uint32(unsigned int seed) : generator_{seed} { }

    template<typename Seq>
    explicit uint64_to_uint32(Seq& seed_seq) : generator_{seed_seq} { }

    static constexpr result_type min() { return numeric_limits<result_type>::min(); }
    static constexpr result_type max() { return numeric_limits<result_type>::max(); }

    result_type operator()() {
        if (have_pending_) {
            have_pending_ = false;
            return pending_;
        }

        const auto result = generator_();

        pending_      = result >> 32;
        have_pending_ = true;

        return static_cast<result_type>(result);
    }

    void set_state(gsl::span<const state_type> s) {
        if (s.size() < Generator::state_words)
            throw std::invalid_argument{"Generator state is too short"};

        if (s.size() == Generator::state_words) {
            generator_.set_state(s.template subspan<0, Generator::state_words>());
            have_pending_ = false;
            pending_      = decltype(pending_){};
            return;
        }

        if (s.size() < state_words)
            throw std::invalid_argument{"Generator state is invalid short"};

        generator_.set_state(s.template subspan<local_state_words, Generator::state_words>());
        if constexpr (2 == local_state_words) {
            have_pending_ = 0 != s[0];
            pending_      = static_cast<result_type>(s[1]);
        } else if constexpr (1 == local_state_words) {
            const auto s0 = s[0];

            have_pending_ = 0 != s0 >> (8 * sizeof pending_);

            pending_ = static_cast<result_type>(s0 & ~result_type{});
        }
    }

    void get_state(gsl::span<state_type, state_words> s) const noexcept {
        generator_.get_state(s.template subspan<local_state_words, Generator::state_words>());

        if constexpr (2 == local_state_words) {
            s[0] = have_pending_ ? 0 : 1;
            s[1] = pending_;
        } else if constexpr (1 == local_state_words) {
            state_type s0 = pending_;
            if (have_pending_)
                s0 |= static_cast<state_type>(1) << (8 * sizeof pending_);
            s[0] = s0;
        }
    }

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
    Generator generator_;
    result_type pending_{};
    bool have_pending_{};
};

} // namespace ExtraGenerators

#endif // RANDOM_UINT64_TO_UINT32_H
