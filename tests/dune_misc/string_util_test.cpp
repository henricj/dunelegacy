#include "misc/string_util.h"

#include <gtest/gtest.h>

TEST(string_util, parse_one) {
    constexpr auto expected = 1;

    int actual{};
    ASSERT_TRUE(parseString("1", actual));

    EXPECT_EQ(expected, actual);
}

TEST(string_util, parse_negative_one) {
    constexpr auto expected = -1;

    int actual{};
    ASSERT_TRUE(parseString("-1", actual));

    EXPECT_EQ(expected, actual);
}

TEST(string_util, trailing_space) {
    constexpr static auto expected = 1;

    int actual{};
    ASSERT_TRUE(parseString("1 ", actual));

    EXPECT_EQ(expected, actual);
}

TEST(string_util, trailing_x) {
    int value{};
    EXPECT_FALSE(parseString("1x", value));
}

TEST(string_util, leading_space) {
    constexpr static auto expected = 1;

    int actual{};
    ASSERT_TRUE(parseString(" 1", actual));

    EXPECT_EQ(expected, actual);
}

TEST(string_util, leading_trailing_space) {
    constexpr static auto expected = 1;

    int actual{};
    ASSERT_TRUE(parseString(" 1 ", actual));

    EXPECT_EQ(expected, actual);
}

TEST(string_util, many_spaces_and_tabs) {
    constexpr static auto expected = 1;

    int actual{};
    ASSERT_TRUE(parseString("        \t1     \t   \t      ", actual));

    EXPECT_EQ(expected, actual);
}

TEST(string_util, trailing_space_and_x) {
    int value{};
    EXPECT_FALSE(parseString("1    x   ", value));
}

TEST(string_util, empty) {
    int value{};
    EXPECT_FALSE(parseString("", value));
}

TEST(string_util, only_space) {
    int value{};
    EXPECT_FALSE(parseString("        \t     \t   \t      ", value));
}

TEST(string_util, unsigned_value) {
    constexpr static auto expected = 1U;

    unsigned int actual{};
    ASSERT_TRUE(parseString("        \t1     \t   \t      ", actual));

    EXPECT_EQ(expected, actual);
}

TEST(string_util, unsigned_negative) {
    unsigned int value{};
    EXPECT_FALSE(parseString("-1", value));
}
