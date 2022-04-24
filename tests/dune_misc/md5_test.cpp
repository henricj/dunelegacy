#include "misc/md5.h"

#include <gtest/gtest.h>

namespace {
inline constexpr auto MD5_SIZE = 16;

auto compute_md5(std::span<const char> input) {
    md5_context ctx;

    md5_starts(&ctx);

    if (!input.empty())
        md5_update(&ctx, reinterpret_cast<const unsigned char*>(input.data()), input.size());

    std::array<unsigned char, MD5_SIZE> output{};

    md5_finish(&ctx, output.data());

    return output;
}

} // namespace

// Samples from https://en.wikipedia.org/wiki/MD5#MD5_hashes

TEST(md5, fox1) {
    static constexpr auto sample = std::to_array("The quick brown fox jumps over the lazy dog");
    static constexpr std::array<unsigned char, MD5_SIZE> expected{0x9e, 0x10, 0x7d, 0x9d, 0x37, 0x2b, 0xb6, 0x82,
                                                                  0x6b, 0xd8, 0x1d, 0x35, 0x42, 0xa4, 0x19, 0xd6};

    const auto actual = compute_md5({sample.data(), sample.size() - 1U});

    ASSERT_EQ(actual.size(), expected.size());
    EXPECT_TRUE(std::equal(actual.begin(), actual.end(), expected.begin()));
}

TEST(md5, fox2) {
    static constexpr auto sample = std::to_array("The quick brown fox jumps over the lazy dog.");
    static constexpr std::array<unsigned char, MD5_SIZE> expected{0xe4, 0xd9, 0x09, 0xc2, 0x90, 0xd0, 0xfb, 0x1c,
                                                                  0xa0, 0x68, 0xff, 0xad, 0xdf, 0x22, 0xcb, 0xd0};

    const auto actual = compute_md5({sample.data(), sample.size() - 1U});

    ASSERT_EQ(actual.size(), expected.size());
    EXPECT_TRUE(std::equal(actual.begin(), actual.end(), expected.begin()));
}

TEST(md5, null) {
    static constexpr std::array<unsigned char, MD5_SIZE> expected{0xd4, 0x1d, 0x8c, 0xd9, 0x8f, 0x00, 0xb2, 0x04,
                                                                  0xe9, 0x80, 0x09, 0x98, 0xec, 0xf8, 0x42, 0x7e};

    const auto actual = compute_md5({});

    ASSERT_EQ(actual.size(), expected.size());
    EXPECT_TRUE(std::equal(actual.begin(), actual.end(), expected.begin()));
}

// Samples from https://www.nist.gov/itl/ssd/software-quality-group/nsrl-test-data

TEST(md5, nist1) {
    static constexpr auto sample = std::to_array("abc");
    static constexpr std::array<unsigned char, MD5_SIZE> expected{0x90, 0x01, 0x50, 0x98, 0x3c, 0xd2, 0x4f, 0xb0,
                                                                  0xd6, 0x96, 0x3f, 0x7d, 0x28, 0xe1, 0x7f, 0x72};

    const auto actual = compute_md5({sample.data(), sample.size() - 1U});

    ASSERT_EQ(actual.size(), expected.size());
    EXPECT_TRUE(std::equal(actual.begin(), actual.end(), expected.begin()));
}

TEST(md5, nist2) {
    static constexpr auto sample = std::to_array("abcdbcdecdefdefgefghfghighijhijkijkljklmklmnlmnomnopnopq");
    static constexpr std::array<unsigned char, MD5_SIZE> expected{0x82, 0x15, 0xef, 0x07, 0x96, 0xa2, 0x0b, 0xca,
                                                                  0xaa, 0xe1, 0x16, 0xd3, 0x87, 0x6c, 0x66, 0x4a};

    const auto actual = compute_md5({sample.data(), sample.size() - 1U});

    ASSERT_EQ(actual.size(), expected.size());
    EXPECT_TRUE(std::equal(actual.begin(), actual.end(), expected.begin()));
}

TEST(md5, nist_long) {
    static constexpr auto repetitions = 1'000'000;
    static constexpr auto input       = std::to_array({'a'});
    static constexpr std::array<unsigned char, MD5_SIZE> expected{0x77, 0x07, 0xd6, 0xae, 0x4e, 0x02, 0x7c, 0x70,
                                                                  0xee, 0xa2, 0xa9, 0x35, 0xc2, 0x29, 0x6f, 0x21};

    md5_context ctx;

    md5_starts(&ctx);

    for (auto i = 0; i < repetitions; ++i)
        md5_update(&ctx, reinterpret_cast<const unsigned char*>(input.data()), input.size());

    std::array<unsigned char, MD5_SIZE> actual{};

    md5_finish(&ctx, actual.data());

    ASSERT_EQ(actual.size(), expected.size());
    EXPECT_TRUE(std::equal(actual.begin(), actual.end(), expected.begin()));
}
