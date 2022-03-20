#include "INIFileTestCase2.h"

#include <cstdio>


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

