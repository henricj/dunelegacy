#include "INIFileTestCase2.h"

#include <cppunit/extensions/HelperMacros.h>

#include <stdio.h>

CPPUNIT_TEST_SUITE_REGISTRATION(INIFileTestCase2);


void INIFileTestCase2::setUp() {
}

void INIFileTestCase2::tearDown() {
}

void INIFileTestCase2::testModifyData() {
	INIFile inifile(TESTSRC "/INIFileTestCase/INIFileTestCase2.ini");

	inifile.setStringValue("General", "StringKey", "Some New Value");
	inifile.setStringValue("", "anonymousKey", "SomeNewValue");
	inifile.setBoolValue("General","BoolKey", false);
	inifile.setIntValue("General","IntKey", 0);

	inifile.saveChangesTo("INIFileTestCase2.ini.out1");

	CPPUNIT_ASSERT(fileCompare("INIFileTestCase2.ini.out1", TESTSRC "/INIFileTestCase/INIFileTestCase2.ini.ref1"));
}

void INIFileTestCase2::testAddKey() {
	INIFile inifile(TESTSRC "/INIFileTestCase/INIFileTestCase2.ini");

	inifile.setStringValue("General", "NewStringKey", "NewValue");
	inifile.setBoolValue("General", "NewBoolKey", true);
	inifile.setStringValue("EmptySection", "NewKey", "Value with comment char # ");

	inifile.saveChangesTo("INIFileTestCase2.ini.out2");

	CPPUNIT_ASSERT(fileCompare("INIFileTestCase2.ini.out2", TESTSRC "/INIFileTestCase/INIFileTestCase2.ini.ref2"));
}

void INIFileTestCase2::testAddSectionAndKeys() {
	INIFile inifile(TESTSRC "/INIFileTestCase/INIFileTestCase2.ini");

	inifile.setStringValue("NewSection", "NewStringKey", "NewValue");

	inifile.saveChangesTo("INIFileTestCase2.ini.out3");

	CPPUNIT_ASSERT(fileCompare("INIFileTestCase2.ini.out3", TESTSRC "/INIFileTestCase/INIFileTestCase2.ini.ref3"));
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

