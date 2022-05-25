#include "INIFileTestCase3.h"

#include <fstream>

TEST_F(INIFileTestCase3, removeKey) {
    INIFile inifile("INIFileTestCase3.ini");

    EXPECT_GE(inifile.lines(), 5);

    inifile.removeKey("Section2", "Key1");
    inifile.removeKey("Section2", "Key3");
    inifile.removeKey("Section2", "Key5");
    inifile.removeKey("", "Key3");

    EXPECT_TRUE(inifile.saveChangesTo("INIFileTestCase3.ini.out1"));

    EXPECT_TRUE(fileCompare("INIFileTestCase3.ini.out1", "INIFileTestCase3.ini.ref1"));
}

TEST_F(INIFileTestCase3, clearSection) {
    INIFile inifile("INIFileTestCase3.ini");

    EXPECT_GE(inifile.lines(), 5);

    inifile.clearSection("Section3");
    inifile.clearSection("");
    inifile.clearSection("Section7");

    EXPECT_TRUE(inifile.saveChangesTo("INIFileTestCase3.ini.out2"));

    EXPECT_TRUE(fileCompare("INIFileTestCase3.ini.out2", "INIFileTestCase3.ini.ref2"));
}

TEST_F(INIFileTestCase3, removeSection) {
    INIFile inifile("INIFileTestCase3.ini");

    EXPECT_GE(inifile.lines(), 5);

    inifile.removeSection("Section3");
    inifile.removeSection("");
    inifile.removeSection("Section7");

    EXPECT_TRUE(inifile.saveChangesTo("INIFileTestCase3.ini.out3"));

    EXPECT_TRUE(fileCompare("INIFileTestCase3.ini.out3", "INIFileTestCase3.ini.ref3"));
}

TEST_F(INIFileTestCase3, clearSectionAndAddKeys) {
    INIFile inifile("INIFileTestCase3.ini");

    EXPECT_GE(inifile.lines(), 5);

    inifile.clearSection("Section3");
    inifile.setStringValue("Section3", "Key1", "a");
    inifile.setStringValue("Section3", "Key2", "b");
    inifile.setStringValue("Section3", "Key3", "c");
    inifile.setStringValue("Section3", "Key4", "d");
    inifile.setStringValue("Section3", "Key5", "e");

    EXPECT_TRUE(inifile.saveChangesTo("INIFileTestCase3.ini.out4"));

    EXPECT_TRUE(fileCompare("INIFileTestCase3.ini.out4", "INIFileTestCase3.ini.ref4"));
}

bool INIFileTestCase3::fileCompare(std::string filename1, std::string filename2) {

    std::ifstream fp1{filename1};
    std::vector<char> file1{std::istream_iterator<char>{fp1}, std::istream_iterator<char>{}};

    std::ifstream fp2{filename2};
    std::vector<char> file2{std::istream_iterator<char>{fp2}, std::istream_iterator<char>{}};

    return std::ranges::equal(file1, file2);
}
