#include "INIFileTestCase1.h"

INIFileTestCase1::~INIFileTestCase1() = default;

void INIFileTestCase1::SetUp() {
    std::filesystem::path path{"INIFileTestCase1.ini"};

    std::error_code ec;
    path = std::filesystem::absolute(path, ec);
    ASSERT_FALSE(ec);

    path = std::filesystem::canonical(path, ec);
    ASSERT_FALSE(ec);

    pINIFile = std::make_unique<INIFile>(path.u8string().c_str());

    // Make sure we loaded soemthing...
    ASSERT_NE(pINIFile->begin(), pINIFile->end());
}

void INIFileTestCase1::TearDown() { pINIFile.reset(); }

TEST_F(INIFileTestCase1, anonymousSection) {
    EXPECT_EQ(pINIFile->getStringValue("", "anonymous_section_key"), "exists");
    EXPECT_EQ(pINIFile->getStringValue("", "NonExistent", "SomeDefaultValue"), "SomeDefaultValue");
}

TEST_F(INIFileTestCase1, hasSectionAndhasKey) {
    EXPECT_EQ(pINIFile->hasSection(""), true);
    EXPECT_EQ(pINIFile->hasSection("Section6 extreme"), true);
    EXPECT_EQ(pINIFile->hasSection("NonExistent"), false);

    EXPECT_EQ(pINIFile->hasKey("", "anonymous_section_key"), true);
    EXPECT_EQ(pINIFile->hasKey("", "NonExistent"), false);
    EXPECT_EQ(pINIFile->hasKey("Section6 extreme", "a b c"), true);
    EXPECT_EQ(pINIFile->hasKey("Section6 extreme", "NonExistent"), false);
    EXPECT_EQ(pINIFile->hasKey("NonExistent", "NonExistent"), false);
}

TEST_F(INIFileTestCase1, readString) {
    EXPECT_EQ(pINIFile->getStringValue("Section1_StringKeys", "StringKey1"), "StringValue1");
    EXPECT_EQ(pINIFile->getStringValue("Section1_StringKeys", "StringKey2"), "StringValue2");
    EXPECT_EQ(pINIFile->getStringValue("Section1_StringKeys", "StringKey3"), "StringValue3");
    EXPECT_EQ(pINIFile->getStringValue("Section1_StringKeys", "StringKey4"), "Some text with multiple words ");
    EXPECT_EQ(pINIFile->getStringValue("Section1_StringKeys", "NonExistent", "SomeDefaultValue"), "SomeDefaultValue");
    EXPECT_EQ(pINIFile->getStringValue("NonExistent", "NonExistent", "SomeDefaultValue"), "SomeDefaultValue");
}

TEST_F(INIFileTestCase1, readInt) {
    EXPECT_EQ(pINIFile->getIntValue("Section2_IntKeys", "IntKey1"), 0);
    EXPECT_EQ(pINIFile->getIntValue("Section2_IntKeys", "IntKey2"), 0);
    EXPECT_EQ(pINIFile->getIntValue("Section2_IntKeys", "IntKey3"), 123);
    EXPECT_EQ(pINIFile->getIntValue("Section2_IntKeys", "IntKey4"), -5);
    EXPECT_EQ(pINIFile->getIntValue("Section2_IntKeys", "IntKey5"), 123);
    EXPECT_EQ(pINIFile->getIntValue("Section2_IntKeys", "NonExistent", 42), 42);
}

TEST_F(INIFileTestCase1, readBool) {
    EXPECT_EQ(pINIFile->getBoolValue("Section3_BoolKeys", "BoolKey1"), true);
    EXPECT_EQ(pINIFile->getBoolValue("Section3_BoolKeys", "BoolKey2"), false);
    EXPECT_EQ(pINIFile->getBoolValue("Section3_BoolKeys", "BoolKey3"), true);
    EXPECT_EQ(pINIFile->getBoolValue("Section3_BoolKeys", "BoolKey4"), false);
    EXPECT_EQ(pINIFile->getBoolValue("Section3_BoolKeys", "BoolKey5"), true);
    EXPECT_EQ(pINIFile->getBoolValue("Section3_BoolKeys", "BoolKey6"), false);
    EXPECT_EQ(pINIFile->getBoolValue("Section3_BoolKeys", "BoolKey7"), true);
    EXPECT_EQ(pINIFile->getBoolValue("Section3_BoolKeys", "BoolKey8"), false);
    EXPECT_EQ(pINIFile->getBoolValue("Section3_BoolKeys", "NonExistent", true), true);
    EXPECT_EQ(pINIFile->getBoolValue("Section3_BoolKeys", "NonExistent", false), false);
}

TEST_F(INIFileTestCase1, readDouble) {
    EXPECT_EQ(pINIFile->getDoubleValue("Section4_DoubleKeys", "DoubleKey1"), 0.0);
    EXPECT_EQ(pINIFile->getDoubleValue("Section4_DoubleKeys", "DoubleKey2"), 1.0);
    EXPECT_EQ(pINIFile->getDoubleValue("Section4_DoubleKeys", "DoubleKey3"), 0.5);
    EXPECT_EQ(pINIFile->getDoubleValue("Section4_DoubleKeys", "DoubleKey4"), -3.0);
    EXPECT_EQ(pINIFile->getDoubleValue("Section4_DoubleKeys", "NonExistent", 42.0), 42.0);
}

TEST_F(INIFileTestCase1, mixedCase) {
    EXPECT_EQ(pINIFile->getStringValue("Section5_MiXeDcAsE", "MiXeDcAsEkEy"), "mixedCaseString");
    EXPECT_EQ(pINIFile->getBoolValue("Section5_MiXeDcAsE", "BoolKey1"), true);
    EXPECT_EQ(pINIFile->getBoolValue("Section5_MiXeDcAsE", "BoolKey2"), false);
    EXPECT_EQ(pINIFile->getBoolValue("Section5_MiXeDcAsE", "BoolKey3"), true);
    EXPECT_EQ(pINIFile->getBoolValue("Section5_MiXeDcAsE", "BoolKey4"), false);
    EXPECT_EQ(pINIFile->getBoolValue("Section5_MiXeDcAsE", "BoolKey5"), true);
    EXPECT_EQ(pINIFile->getBoolValue("Section5_MiXeDcAsE", "BoolKey6"), false);
}

TEST_F(INIFileTestCase1, extreme) {
    EXPECT_EQ(pINIFile->getStringValue("Section6 extreme", "comment", "SomeDefaultValue"), "SomeDefaultValue");
    EXPECT_EQ(pINIFile->getStringValue("Section6 extreme", "#comment", "SomeDefaultValue"), "SomeDefaultValue");
    EXPECT_EQ(pINIFile->getStringValue("Section6 extreme", "a b c"), "xyz abc");
    EXPECT_EQ(pINIFile->getStringValue("Section6 extreme", "tricky string"), "comment #fake comment");
}

TEST_F(INIFileTestCase1, iterateSections) {
    INIFile::SectionIterator iter = pINIFile->begin();

    EXPECT_NE(iter, pINIFile->end());
    EXPECT_EQ(iter->getSectionName(), "");
    ++iter;
    EXPECT_NE(iter, pINIFile->end());
    EXPECT_EQ(iter->getSectionName(), "Section1_StringKeys");
    ++iter;
    EXPECT_NE(iter, pINIFile->end());
    EXPECT_EQ(iter->getSectionName(), "Section2_IntKeys");
    ++iter;
    EXPECT_NE(iter, pINIFile->end());
    EXPECT_EQ(iter->getSectionName(), "Section3_BoolKeys");
    ++iter;
    EXPECT_NE(iter, pINIFile->end());
    EXPECT_EQ(iter->getSectionName(), "Section4_DoubleKeys");
    ++iter;
    EXPECT_NE(iter, pINIFile->end());
    EXPECT_EQ(iter->getSectionName(), "Section5_mixedCase");
    ++iter;
    EXPECT_NE(iter, pINIFile->end());
    EXPECT_EQ(iter->getSectionName(), "Section6 extreme");
    ++iter;
    EXPECT_EQ(iter, pINIFile->end());
}

TEST_F(INIFileTestCase1, iterateKeys) {
    INIFile::KeyIterator iter = pINIFile->begin("Section6 extreme");

    EXPECT_NE(iter, pINIFile->end("Section6 extreme"));
    EXPECT_EQ(iter->getKeyName(), "a b c");
    EXPECT_EQ(iter->getStringValue(), "xyz abc");
    ++iter;
    EXPECT_NE(iter, pINIFile->end("Section6 extreme"));
    EXPECT_EQ(iter->getKeyName(), "tricky string");
    EXPECT_EQ(iter->getStringValue(), "comment #fake comment");
    ++iter;
    EXPECT_EQ(iter, pINIFile->end("Section6 extreme"));
}
