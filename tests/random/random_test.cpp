#include "misc/Random.h"

#include <algorithm>
#include <array>
#include <string_view>

#include "gtest/gtest.h"

namespace {
RandomFactory create_random_factory(int n) {
    std::array<uint8_t, 64> seed;

    std::iota(seed.begin(), seed.end(), n);

    return {seed};
}
} // namespace

TEST(Random, create) {
    EXPECT_NO_THROW(auto factory = create_random_factory(1));
}

TEST(Random, distinct) {
    const auto factory = create_random_factory(2);

    auto rng1 = factory.create("One");
    auto rng2 = factory.create("Two");

    const auto v1 = rng1.rand();
    const auto v2 = rng2.rand();

    EXPECT_NE(v1, v2);
}

TEST(Random, repeatable) {
    const auto factory = create_random_factory(3);

    auto rng = factory.create("repeatable");

    const auto actual       = rng.rand();
    constexpr auto expected = 0xbd4694c9;

    EXPECT_EQ(actual, expected);
}

TEST(Random, state) {
    const auto factory = create_random_factory(4);

    auto rng = factory.create("state");

    const auto expected = rng.getState();

    rng.setState(expected);

    const auto actual = rng.getState();

    ASSERT_EQ(actual.size(), expected.size());
    EXPECT_TRUE(std::equal(actual.begin(), actual.end(), expected.begin()));
}

TEST(Random, state_change) {
    const auto factory = create_random_factory(4);

    auto rng = factory.create("state_change");

    const auto expected = rng.getState();

    (void)rng.rand();

    const auto actual = rng.getState();

    ASSERT_EQ(actual.size(), expected.size());
    EXPECT_FALSE(std::equal(actual.begin(), actual.end(), expected.begin()));
}

TEST(Random, state_repeatable) {
    const auto factory = create_random_factory(4);

    auto rng = factory.create("state_repeatable");

    const auto state = rng.getState();

    const auto expected = rng.rand();

    rng.setState(state);

    const auto actual = rng.rand();

    EXPECT_EQ(actual, expected);
}

TEST(Random, range) {
    constexpr auto low  = 15;
    constexpr auto high = low + 50;

    const auto factory = create_random_factory(5);

    auto rng = factory.create("range");

    std::vector<bool> flags(high - low + 1);

    for (auto i = 0; i < 10000; ++i) {
        const auto v = rng.rand(low, high);

        ASSERT_GE(v, low);
        ASSERT_LE(v, high);

        const auto offset = v - low;

        flags[offset] = true;
    }

    const auto all_true = std::all_of(std::begin(flags), std::end(flags), [](auto f) { return f; });

    EXPECT_TRUE(all_true);
}

TEST(Random, FixPoint) {
    const auto factory = create_random_factory(6);

    auto rng = factory.create("FixPoint");

    auto min = FixPt_MAX;
    auto max = FixPt_MIN;
    constexpr auto one = FixPoint{1};

    for (auto i = 0; i < 10000; ++i) {
        const auto v = rng.randFixPoint();

        if (v < min)
            min = v;
        if (v > max)
            max = v;
    }

    EXPECT_LT(min, max);
    EXPECT_LE(FixPt_ZERO, min);
    EXPECT_LT(max, one);
}

TEST(Random, boolean) {
    const auto factory = create_random_factory(7);

    auto rng = factory.create("boolean");

    std::array<int, 2> count{};

    for (auto i = 0; i < 1000; ++i) {
        const auto v = rng.randBool();

        ++count[v ? 1 : 0];
    }

    EXPECT_GT(count[0], 400);
    EXPECT_GT(count[1], 400);
}

TEST(Random, RandOf1) {
    const auto factory = create_random_factory(8);

    auto rng = factory.create("RandOf1");

    for (auto i = 0; i < 1000; ++i) {
        const auto v = rng.getRandOf(1);

        EXPECT_EQ(v, 1);
    }
}

TEST(Random, RandOf2) {
    const auto factory = create_random_factory(9);

    auto rng = factory.create("RandOf2");

    std::array<int, 2> count{};

    for (auto i = 0; i < 1000; ++i) {
        const auto v = rng.getRandOf(true, false);

        ++count[v ? 1 : 0];
    }

    EXPECT_GT(count[0], 400);
    EXPECT_GT(count[1], 400);
}

TEST(Random, RandOf3) {
    const auto factory = create_random_factory(10);

    auto rng = factory.create("RandOf3");

    std::array<int, 3> count{};

    for (auto i = 0; i < 1000; ++i) {
        const auto v = rng.getRandOf(0, 1, 2);

        ASSERT_GE(v, 0);
        ASSERT_LE(v, 2);

        ++count[v];
    }

    EXPECT_GT(count[0], 270);
    EXPECT_GT(count[1], 270);
    EXPECT_GT(count[2], 270);
}
