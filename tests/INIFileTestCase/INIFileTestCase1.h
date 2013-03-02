
#include <FileClasses/INIFile.h>

#include <cppunit/extensions/HelperMacros.h>

class INIFileTestCase1: public CppUnit::TestFixture  {

	CPPUNIT_TEST_SUITE(INIFileTestCase1);

	CPPUNIT_TEST(testAnonymousSection);
	CPPUNIT_TEST(testHasSectionAndhasKey);
	CPPUNIT_TEST(testReadString);
	CPPUNIT_TEST(testReadInt);
	CPPUNIT_TEST(testReadBool);
	CPPUNIT_TEST(testReadDouble);
	CPPUNIT_TEST(testMixedCase);
	CPPUNIT_TEST(testExtreme);
	CPPUNIT_TEST(testIterateSections);
	CPPUNIT_TEST(testIterateKeys);

	CPPUNIT_TEST_SUITE_END();

public:
	void setUp();
	void tearDown();

	void testAnonymousSection();
	void testHasSectionAndhasKey();
	void testReadString();
	void testReadInt();
	void testReadBool();
	void testReadDouble();
	void testMixedCase();
	void testExtreme();
	void testIterateSections();
	void testIterateKeys();

private:
	INIFile* pINIFile;
};

