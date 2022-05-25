#include "INIFileTestCase2.h"

#include <fstream>
#include <ranges>

TEST_F(INIFileTestCase2, modifyData) {
	INIFile inifile("INIFileTestCase2.ini");

	inifile.setStringValue("General", "StringKey", "Some New Value");
	inifile.setStringValue("", "anonymousKey", "SomeNewValue");
	inifile.setBoolValue("General","BoolKey", false);
	inifile.setIntValue("General","IntKey", 0);

	EXPECT_TRUE(inifile.saveChangesTo("INIFileTestCase2.ini.out1"));

	EXPECT_TRUE(fileCompare("INIFileTestCase2.ini.out1", "INIFileTestCase2.ini.ref1"));
}

TEST_F(INIFileTestCase2, addKey) {
	INIFile inifile("INIFileTestCase2.ini");

	inifile.setStringValue("General", "NewStringKey", "NewValue");
	inifile.setBoolValue("General", "NewBoolKey", true);
	inifile.setStringValue("EmptySection", "NewKey", "Value with comment char # ");

	EXPECT_TRUE(inifile.saveChangesTo("INIFileTestCase2.ini.out2"));

	EXPECT_TRUE(fileCompare("INIFileTestCase2.ini.out2", "INIFileTestCase2.ini.ref2"));
}

TEST_F(INIFileTestCase2, addSectionAndKeys) {
	INIFile inifile("INIFileTestCase2.ini");

	inifile.setStringValue("NewSection", "NewStringKey", "NewValue");

	EXPECT_TRUE(inifile.saveChangesTo("INIFileTestCase2.ini.out3"));

	EXPECT_TRUE(fileCompare("INIFileTestCase2.ini.out3", "INIFileTestCase2.ini.ref3"));
}

bool INIFileTestCase2::fileCompare(std::string filename1, std::string filename2) {

    std::ifstream fp1{filename1};
    std::vector<char> file1{std::istream_iterator<char>{fp1}, std::istream_iterator<char>{}};

    std::ifstream fp2{filename2};
    std::vector<char> file2{std::istream_iterator<char>{fp2}, std::istream_iterator<char>{}};

    return std::ranges::equal(file1, file2);
}

