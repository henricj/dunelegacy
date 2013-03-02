
#include <FileClasses/INIFile.h>

#include <cppunit/extensions/HelperMacros.h>

class INIFileTestCase3: public CppUnit::TestFixture  {

	CPPUNIT_TEST_SUITE(INIFileTestCase3);

	CPPUNIT_TEST(testRemoveKey);
	CPPUNIT_TEST(testClearSection);
	CPPUNIT_TEST(testRemoveSection);
	CPPUNIT_TEST(testClearSectionAndAddKeys);

	CPPUNIT_TEST_SUITE_END();

public:
	void setUp();
	void tearDown();

	void testRemoveKey();
	void testClearSection();
	void testRemoveSection();
	void testClearSectionAndAddKeys();

private:
	bool fileCompare(std::string filename1, std::string filename2);

	INIFile* pINIFile;
};

