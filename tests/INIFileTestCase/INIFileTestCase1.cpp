#include "INIFileTestCase1.h"

#include <cppunit/extensions/HelperMacros.h>

CPPUNIT_TEST_SUITE_REGISTRATION(INIFileTestCase1);


void INIFileTestCase1::setUp() {
	pINIFile = new INIFile(TESTSRC "/INIFileTestCase/INIFileTestCase1.ini");
}

void INIFileTestCase1::tearDown() {
	delete pINIFile;
}

void INIFileTestCase1::testAnonymousSection() {
	CPPUNIT_ASSERT(pINIFile->getStringValue("", "anonymous_section_key") == "exists");
	CPPUNIT_ASSERT(pINIFile->getStringValue("", "NonExistent", "SomeDefaultValue") == "SomeDefaultValue");
}

void INIFileTestCase1::testHasSectionAndhasKey() {
	CPPUNIT_ASSERT(pINIFile->hasSection("") == true);
	CPPUNIT_ASSERT(pINIFile->hasSection("Section6 extreme") == true);
	CPPUNIT_ASSERT(pINIFile->hasSection("NonExistent") == false);

	CPPUNIT_ASSERT(pINIFile->hasKey("", "anonymous_section_key") == true);
	CPPUNIT_ASSERT(pINIFile->hasKey("", "NonExistent") == false);
	CPPUNIT_ASSERT(pINIFile->hasKey("Section6 extreme","a b c") == true);
	CPPUNIT_ASSERT(pINIFile->hasKey("Section6 extreme", "NonExistent") == false);
	CPPUNIT_ASSERT(pINIFile->hasKey("NonExistent", "NonExistent") == false);
}

void INIFileTestCase1::testReadString() {
	CPPUNIT_ASSERT(pINIFile->getStringValue("Section1_StringKeys", "StringKey1") == "StringValue1");
	CPPUNIT_ASSERT(pINIFile->getStringValue("Section1_StringKeys", "StringKey2") == "StringValue2");
	CPPUNIT_ASSERT(pINIFile->getStringValue("Section1_StringKeys", "StringKey3") == "StringValue3");
	CPPUNIT_ASSERT(pINIFile->getStringValue("Section1_StringKeys", "StringKey4") == "Some text with multiple words ");
	CPPUNIT_ASSERT(pINIFile->getStringValue("Section1_StringKeys", "NonExistent", "SomeDefaultValue") == "SomeDefaultValue");
	CPPUNIT_ASSERT(pINIFile->getStringValue("NonExistent", "NonExistent", "SomeDefaultValue") == "SomeDefaultValue");
}

void INIFileTestCase1::testReadInt() {
	CPPUNIT_ASSERT(pINIFile->getIntValue("Section2_IntKeys", "IntKey1") == 0);
	CPPUNIT_ASSERT(pINIFile->getIntValue("Section2_IntKeys", "IntKey2") == 0);
	CPPUNIT_ASSERT(pINIFile->getIntValue("Section2_IntKeys", "IntKey3") == 123);
	CPPUNIT_ASSERT(pINIFile->getIntValue("Section2_IntKeys", "IntKey4") == -5);
	CPPUNIT_ASSERT(pINIFile->getIntValue("Section2_IntKeys", "IntKey5") == 123);
	CPPUNIT_ASSERT(pINIFile->getIntValue("Section2_IntKeys", "NonExistent", 42) == 42);
}

void INIFileTestCase1::testReadBool() {
	CPPUNIT_ASSERT(pINIFile->getBoolValue("Section3_BoolKeys", "BoolKey1") == true);
	CPPUNIT_ASSERT(pINIFile->getBoolValue("Section3_BoolKeys", "BoolKey2") == false);
	CPPUNIT_ASSERT(pINIFile->getBoolValue("Section3_BoolKeys", "BoolKey3") == true);
	CPPUNIT_ASSERT(pINIFile->getBoolValue("Section3_BoolKeys", "BoolKey4") == false);
	CPPUNIT_ASSERT(pINIFile->getBoolValue("Section3_BoolKeys", "BoolKey5") == true);
	CPPUNIT_ASSERT(pINIFile->getBoolValue("Section3_BoolKeys", "BoolKey6") == false);
	CPPUNIT_ASSERT(pINIFile->getBoolValue("Section3_BoolKeys", "BoolKey7") == true);
	CPPUNIT_ASSERT(pINIFile->getBoolValue("Section3_BoolKeys", "BoolKey8") == false);
	CPPUNIT_ASSERT(pINIFile->getBoolValue("Section3_BoolKeys", "NonExistent", true) == true);
	CPPUNIT_ASSERT(pINIFile->getBoolValue("Section3_BoolKeys", "NonExistent", false) == false);
}

void INIFileTestCase1::testReadDouble() {
	CPPUNIT_ASSERT(pINIFile->getDoubleValue("Section4_DoubleKeys", "DoubleKey1") == 0.0);
	CPPUNIT_ASSERT(pINIFile->getDoubleValue("Section4_DoubleKeys", "DoubleKey2") == 1.0);
	CPPUNIT_ASSERT(pINIFile->getDoubleValue("Section4_DoubleKeys", "DoubleKey3") == 0.5);
	CPPUNIT_ASSERT(pINIFile->getDoubleValue("Section4_DoubleKeys", "DoubleKey4") == -3.0);
	CPPUNIT_ASSERT(pINIFile->getDoubleValue("Section4_DoubleKeys", "NonExistent", 42.0) == 42.0);
}

void INIFileTestCase1::testMixedCase() {
	CPPUNIT_ASSERT(pINIFile->getStringValue("Section5_MiXeDcAsE", "MiXeDcAsEkEy") == "mixedCaseString");
	CPPUNIT_ASSERT(pINIFile->getBoolValue("Section5_MiXeDcAsE", "BoolKey1") == true);
	CPPUNIT_ASSERT(pINIFile->getBoolValue("Section5_MiXeDcAsE", "BoolKey2") == false);
	CPPUNIT_ASSERT(pINIFile->getBoolValue("Section5_MiXeDcAsE", "BoolKey3") == true);
	CPPUNIT_ASSERT(pINIFile->getBoolValue("Section5_MiXeDcAsE", "BoolKey4") == false);
	CPPUNIT_ASSERT(pINIFile->getBoolValue("Section5_MiXeDcAsE", "BoolKey5") == true);
	CPPUNIT_ASSERT(pINIFile->getBoolValue("Section5_MiXeDcAsE", "BoolKey6") == false);
}

void INIFileTestCase1::testExtreme() {
	CPPUNIT_ASSERT(pINIFile->getStringValue("Section6 extreme", "comment", "SomeDefaultValue") == "SomeDefaultValue");
	CPPUNIT_ASSERT(pINIFile->getStringValue("Section6 extreme", "#comment", "SomeDefaultValue") == "SomeDefaultValue");
	CPPUNIT_ASSERT(pINIFile->getStringValue("Section6 extreme", "a b c") == "xyz abc");
	CPPUNIT_ASSERT(pINIFile->getStringValue("Section6 extreme", "tricky string") == "comment #fake comment");
}

void INIFileTestCase1::testIterateSections() {
	INIFile::SectionIterator iter = pINIFile->begin();

	CPPUNIT_ASSERT(iter != pINIFile->end());
	CPPUNIT_ASSERT(iter->getSectionName() == "");
	++iter;
	CPPUNIT_ASSERT(iter->getSectionName() == "Section1_StringKeys");
	++iter;
	CPPUNIT_ASSERT(iter->getSectionName() == "Section2_IntKeys");
	++iter;
	CPPUNIT_ASSERT(iter->getSectionName() == "Section3_BoolKeys");
	++iter;
	CPPUNIT_ASSERT(iter->getSectionName() == "Section4_DoubleKeys");
	++iter;
	CPPUNIT_ASSERT(iter->getSectionName() == "Section5_mixedCase");
	++iter;
	CPPUNIT_ASSERT(iter->getSectionName() == "Section6 extreme");
	++iter;
	CPPUNIT_ASSERT(iter == pINIFile->end());
}

void INIFileTestCase1::testIterateKeys() {
	INIFile::KeyIterator iter = pINIFile->begin("Section6 extreme");

	CPPUNIT_ASSERT(iter != pINIFile->end("Section6 extreme"));
	CPPUNIT_ASSERT(iter->getKeyName() == "a b c");
	CPPUNIT_ASSERT(iter->getStringValue() == "xyz abc");
	++iter;
	CPPUNIT_ASSERT(iter->getKeyName() == "tricky string");
	CPPUNIT_ASSERT(iter->getStringValue() == "comment #fake comment");
	++iter;
	CPPUNIT_ASSERT(iter == pINIFile->end("Section6 extreme"));
}


