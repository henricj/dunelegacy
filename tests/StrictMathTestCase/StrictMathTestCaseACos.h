
#include "StrictMathTestCaseBase.h"

#include <misc/strictmath.h>

#include <cppunit/extensions/HelperMacros.h>

#include <map>

class StrictMathTestCaseACos: public StrictMathTestCaseBase  {

	CPPUNIT_TEST_SUITE(StrictMathTestCaseACos);

	CPPUNIT_TEST(testACos);

	CPPUNIT_TEST_SUITE_END();

public:
	void setUp();
	void tearDown();

	void testACos();
};

