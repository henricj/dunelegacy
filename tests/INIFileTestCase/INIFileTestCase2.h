
#include <FileClasses/INIFile.h>

#include <cppunit/extensions/HelperMacros.h>

class INIFileTestCase2: public CppUnit::TestFixture  {

	CPPUNIT_TEST_SUITE(INIFileTestCase2);

	CPPUNIT_TEST(testModifyData);
	CPPUNIT_TEST(testAddKey);
	CPPUNIT_TEST(testAddSectionAndKeys);

	CPPUNIT_TEST_SUITE_END();

public:
	void setUp();
	void tearDown();

	void testModifyData();
	void testAddKey();
	void testAddSectionAndKeys();

private:
	bool fileCompare(std::string filename1, std::string filename2);

	INIFile* pINIFile;
};

