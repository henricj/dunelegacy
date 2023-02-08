/*
 *  This file is part of Dune Legacy.
 *
 *  Dune Legacy is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  Dune Legacy is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with Dune Legacy.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <FileClasses/INIFile.h>

#include <gtest/gtest.h>

TEST(INIFileSimple, initialize) {
    INIFile file(true, {});

    EXPECT_TRUE(true);
}

TEST(INIFileSimple, set_bool) {
    INIFile file(true, {});

    file.setBoolValue({}, "Key1", true);

    EXPECT_TRUE(true);
}

TEST(INIFileSimple, get_nonexistent_bool) {
    const INIFile file(true, {});

    const auto value = file.getBoolValue({}, "Key1");

    EXPECT_FALSE(value);
}

TEST(INIFileSimple, set_get_bool_true) {
    INIFile file(true, {});

    file.setBoolValue({}, "Key1", true);

    const auto value = file.getBoolValue({}, "Key1");

    EXPECT_TRUE(value);
}

TEST(INIFileSimple, set_get_bool_false) {
    INIFile file(true, {});

    file.setBoolValue({}, "Key1", false);

    const auto value = file.getBoolValue({}, "Key1");

    EXPECT_FALSE(value);
}

TEST(INIFileSimple, add_section) {
    INIFile file(true, {});

    file.setBoolValue("Simple", "Key1", false);

    const auto value = file.getBoolValue("Simple", "Key1");

    EXPECT_FALSE(value);
}

TEST(INIFileSimple, add_remove_section) {
    INIFile file(true, {});

    file.setBoolValue("Simple", "Key1", true);

    const auto value = file.getBoolValue("Simple", "Key1");

    EXPECT_TRUE(value);

    file.removeSection("Simple");

    const auto value2 = file.getBoolValue("Simple", "Key1");

    EXPECT_FALSE(value2);
}

TEST(INIFileSimple, add_remove_section2) {
    INIFile file(true, {});

    file.setIntValue("", "RootKey1", 123);
    file.setBoolValue("Simple", "Key1", true);

    const auto value = file.getBoolValue("Simple", "Key1");

    EXPECT_TRUE(value);

    file.removeSection("Simple");

    const auto valueR= file.getIntValue("", "RootKey1");

    EXPECT_EQ(valueR, 123);

    const auto value2 = file.getBoolValue("Simple", "Key1");

    EXPECT_FALSE(value2);
}

TEST(INIFileSimple, clear_root_section) {
    INIFile file(true, {});

    file.setBoolValue({}, "Key1", true);

    file.clearSection("");

    const auto value = file.getBoolValue({}, "Key1");

    EXPECT_FALSE(value);
}

TEST(INIFileSimple, clear_root_section2) {
    INIFile file(true, {});

    file.setBoolValue("Section1", "Key1", true);

    file.clearSection("");

    const auto value = file.getBoolValue("Section1", "Key1");

    EXPECT_TRUE(value);
}


TEST(INIFileSimple, clear_root_section3) {
    INIFile file(true, {});

    file.setIntValue("", "KeyR", 100);
    file.setIntValue("Section1", "Key1", 200);

    file.clearSection("");

    const auto valueR = file.getIntValue("", "Key1");
    EXPECT_EQ(valueR, 0);

    const auto value1 = file.getIntValue("Section1", "Key1");
    EXPECT_EQ(value1, 200);
}


TEST(INIFileSimple, remove_root_section) {
    INIFile file(true, {});
    INIFile file2(true, {});

    file.setBoolValue({}, "Key1", true);

    file.removeSection("");

    const auto value = file.getBoolValue({}, "Key1");

    EXPECT_FALSE(value);
}

TEST(INIFileSimple, remove_root_section2) {
    INIFile file(true, {});

    file.setBoolValue("Section1", "Key1", true);

    file.removeSection("");

    const auto value = file.getBoolValue("Section1", "Key1");

    EXPECT_TRUE(value);
}

TEST(INIFileSimple, remove_root_section3) {
    INIFile file(true, {});

    file.setIntValue("", "KeyR", 100);
    file.setIntValue("Section1", "Key1", 200);

    file.removeSection("");

    const auto valueR = file.getIntValue("", "Key1");
    EXPECT_EQ(valueR, 0);

    const auto value1 = file.getIntValue("Section1", "Key1");
    EXPECT_EQ(value1, 200);
}

