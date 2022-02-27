#include <misc/Random.h>

#ifdef COLOR_DEFAULT
#    undef COLOR_DEFAULT
#endif

#ifdef COLOR_RED
#    undef COLOR_RED
#endif

#ifdef COLOR_GREEN
#    undef COLOR_GREEN
#endif

#ifdef COLOR_YELLOW
#    undef COLOR_YELLOW
#endif

#include "gtest/gtest.h"

namespace
{
Random create_random(int n)
{
    std::array<uint8_t, Random::state_bytes> state;

    std::iota(state.begin(), state.end(), n);

    return Random::create(state);
}
}

TEST(rng_serialization, one) {
    auto rng = create_random(1);

    EXPECT_EQ(0xbc52e8c6, rng.rand());
    EXPECT_EQ(0x61f88f25, rng.rand());

    for(auto i = 0; i < 1000; ++i)
        rng.rand();

    EXPECT_EQ(0x492339af, rng.rand());
    EXPECT_EQ(0xd8a622fb, rng.rand());
}

TEST(rng_serialization, two) {
    auto rng = create_random(2);

    EXPECT_EQ(0x52e97f61, rng.rand());
    EXPECT_EQ(0xf88f25bc, rng.rand());

    for(auto i = 0; i < 1000; ++i)
        rng.rand();

    EXPECT_EQ(0xcd3d3b46, rng.rand());
    EXPECT_EQ(0x3e348ece, rng.rand());
}

TEST(rng_serialization, copy) {
    auto rng      = create_random(3);
    Random rng_copy{rng};

    for(auto i = 0; i < 100; ++i)
        EXPECT_EQ(rng.rand(), rng_copy.rand());
}

TEST(rng_serialization, state_copy) {
    auto rng = create_random(4);

    const auto state = rng.getState();

    auto rng_copy = Random::create(state);

    for(auto i = 0; i < 100; ++i)
        EXPECT_EQ(rng.rand(), rng_copy.rand());
}
