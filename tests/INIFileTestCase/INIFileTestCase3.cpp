#include "INIFileTestCase3.h"

#include <stdio.h>


TEST_F(INIFileTestCase3, removeKey) {
    Dune::Engine::INIFile inifile("INIFileTestCase3.ini");

    EXPECT_NE(inifile.begin(), inifile.end());

	inifile.removeKey("Section2","Key1");
	inifile.removeKey("Section2","Key3");
	inifile.removeKey("Section2","Key5");
	inifile.removeKey("","Key3");

	EXPECT_TRUE(inifile.saveChangesTo("INIFileTestCase3.ini.out1"));

	EXPECT_TRUE(fileCompare("INIFileTestCase3.ini.out1", "INIFileTestCase3.ini.ref1"));
}

TEST_F(INIFileTestCase3, clearSection) {
    Dune::Engine::INIFile inifile("INIFileTestCase3.ini");

    EXPECT_NE(inifile.begin(), inifile.end());

    inifile.clearSection("Section3");
	inifile.clearSection("");
	inifile.clearSection("Section7");

	EXPECT_TRUE(inifile.saveChangesTo("INIFileTestCase3.ini.out2"));

	EXPECT_TRUE(fileCompare("INIFileTestCase3.ini.out2", "INIFileTestCase3.ini.ref2"));
}

TEST_F(INIFileTestCase3, removeSection) {
    Dune::Engine::INIFile inifile("INIFileTestCase3.ini");

    EXPECT_NE(inifile.begin(), inifile.end());

    inifile.removeSection("Section3");
	inifile.removeSection("");
	inifile.removeSection("Section7");

	EXPECT_TRUE(inifile.saveChangesTo("INIFileTestCase3.ini.out3"));

	EXPECT_TRUE(fileCompare("INIFileTestCase3.ini.out3", "INIFileTestCase3.ini.ref3"));
}

TEST_F(INIFileTestCase3, clearSectionAndAddKeys) {
    Dune::Engine::INIFile inifile("INIFileTestCase3.ini");

    EXPECT_NE(inifile.begin(), inifile.end());

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

	FILE* fp1 = fopen(filename1.c_str(), "r");
	if(fp1 == NULL) {
		perror("fileCompare");
		return false;	
	}

	FILE* fp2 = fopen(filename2.c_str(), "r");
	if(fp2 == NULL) {
		perror("fileCompare");
		fclose(fp1);
		return false;	
	}

	while(!feof(fp1) && !feof(fp2)) {
		if(fgetc(fp1) != fgetc(fp2)) {
			fclose(fp1);
			fclose(fp2);
			return false;
		}	
	}

	bool equal = feof(fp1) && feof(fp2);
		
	fclose(fp1);
	fclose(fp2);
	return equal;
}

