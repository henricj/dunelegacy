#include "INIFileTestCase3.h"

#include <cppunit/extensions/HelperMacros.h>

#include <stdio.h>

CPPUNIT_TEST_SUITE_REGISTRATION(INIFileTestCase3);


void INIFileTestCase3::setUp() {
}

void INIFileTestCase3::tearDown() {
}

void INIFileTestCase3::testRemoveKey() {
	INIFile inifile(TESTSRC "/INIFileTestCase/INIFileTestCase3.ini");

	inifile.removeKey("Section2","Key1");
	inifile.removeKey("Section2","Key3");
	inifile.removeKey("Section2","Key5");
	inifile.removeKey("","Key3");


	inifile.saveChangesTo("INIFileTestCase3.ini.out1");

	CPPUNIT_ASSERT(fileCompare("INIFileTestCase3.ini.out1", TESTSRC "/INIFileTestCase/INIFileTestCase3.ini.ref1"));
}

void INIFileTestCase3::testClearSection() {
	INIFile inifile(TESTSRC "/INIFileTestCase/INIFileTestCase3.ini");

	inifile.clearSection("Section3");
	inifile.clearSection("");
	inifile.clearSection("Section7");

	inifile.saveChangesTo("INIFileTestCase3.ini.out2");

	CPPUNIT_ASSERT(fileCompare("INIFileTestCase3.ini.out2", TESTSRC "/INIFileTestCase/INIFileTestCase3.ini.ref2"));
}

void INIFileTestCase3::testRemoveSection() {
	INIFile inifile(TESTSRC "/INIFileTestCase/INIFileTestCase3.ini");

	inifile.removeSection("Section3");
	inifile.removeSection("");
	inifile.removeSection("Section7");

	inifile.saveChangesTo("INIFileTestCase3.ini.out3");

	CPPUNIT_ASSERT(fileCompare("INIFileTestCase3.ini.out3", TESTSRC "/INIFileTestCase/INIFileTestCase3.ini.ref3"));
}

void INIFileTestCase3::testClearSectionAndAddKeys() {
	INIFile inifile(TESTSRC "/INIFileTestCase/INIFileTestCase3.ini");

	inifile.clearSection("Section3");
	inifile.setStringValue("Section3", "Key1", "a");
	inifile.setStringValue("Section3", "Key2", "b");
	inifile.setStringValue("Section3", "Key3", "c");
	inifile.setStringValue("Section3", "Key4", "d");
	inifile.setStringValue("Section3", "Key5", "e");

	inifile.saveChangesTo("INIFileTestCase3.ini.out4");

	CPPUNIT_ASSERT(fileCompare("INIFileTestCase3.ini.out4", TESTSRC "/INIFileTestCase/INIFileTestCase3.ini.ref4"));
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

