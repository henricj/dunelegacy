#include "misc/lemire_uniform_uint32_distribution.h"
#include "misc/random_xoshiro256starstar.h"
#include "misc/random_uint64_to_uint32.h"
#include "gtest/gtest.h"

class generatorN {
public:
    generatorN(uint32_t value) : value_{value} { }
    uint32_t                  operator()() { return value_; }
    static constexpr uint32_t min() noexcept { return std::numeric_limits<uint32_t>::min(); }
    static constexpr uint32_t max() noexcept { return std::numeric_limits<uint32_t>::max(); }

private:
    const uint32_t value_;
};

TEST(Lemire_uint32, one) {
    lemire_uniform_uint32_distribution d{1};
    generatorN                         g0{0};
    generatorN                         g1{1u};
    generatorN                         g2_31{1u << 31};

    EXPECT_EQ(0, d(g0));
    EXPECT_EQ(0, d(g1));
    EXPECT_EQ(0, d(g2_31));
}

TEST(Lemire_uint32, two) {
    lemire_uniform_uint32_distribution d{2};
    generatorN                         g0{0};
    generatorN                         g2_31_minus_1{(1u << 31) - 1};
    generatorN                         g2_31{1u << 31};
    generatorN                         g_neg0{~0U};

    EXPECT_EQ(0, d(g0));
    EXPECT_EQ(0, d(g2_31_minus_1));
    EXPECT_EQ(1, d(g2_31));
    EXPECT_EQ(1, d(g_neg0));
}



class LemireRandomTests : public testing::TestWithParam<std::tuple<uint32_t, uint32_t, size_t>> { };

TEST_P(LemireRandomTests, twoRng) {
    const auto [seed, size, tries] = GetParam();

    lemire_uniform_uint32_distribution                                     d{size};
    ExtraGenerators::uint64_to_uint32<ExtraGenerators::xoshiro256starstar> g{seed};

    std::vector<int> histogram;
    histogram.resize(size);

    for (auto i = 0; i < tries; ++i) {
        const auto n = d(g);

        EXPECT_LE(0, n);
        EXPECT_LT(n, histogram.size());

        ++histogram.at(n);
    }

    for (auto n : histogram) {
        EXPECT_GT(n, 0);
    }
}

//INSTANTIATE_TEST_SUITE_P(Lemire_uint32, LemireRandomTests,
//                         testing::Values(std::make_tuple<uint32_t, uint32_t, size_t>(1, 2, 1000)));
INSTANTIATE_TEST_SUITE_P(Lemire_uint32, LemireRandomTests,
                         testing::Values(std::make_tuple(1, 2, 1000), std::make_tuple(2, 2, 1000),
                                         std::make_tuple(3, 3, 1000), std::make_tuple(3, 4, 1000),
                                         std::make_tuple(4, 5, 1000), std::make_tuple(5, 6, 1000),
                                         std::make_tuple(4, 7, 1000), std::make_tuple(5, 10, 1000),
                                         std::make_tuple(6, 11, 1000), std::make_tuple(7, 100, 10000),
                                         std::make_tuple(7, 1000, 100000)));

