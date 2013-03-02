

#include <cppunit/extensions/HelperMacros.h>

class FileSystemTestCase: public CppUnit::TestFixture  {

	CPPUNIT_TEST_SUITE(FileSystemTestCase);

	CPPUNIT_TEST(testBasename);
	CPPUNIT_TEST(testBasenameExtension);
	CPPUNIT_TEST(testDirname);

	CPPUNIT_TEST_SUITE_END();

public:
	void setUp();
	void tearDown();

	void testBasename();
	void testBasenameExtension();
	void testDirname();

private:

};

