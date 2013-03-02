
#include "StrictMathTestCaseBase.h"

#include <misc/strictmath.h>

#include <cppunit/extensions/HelperMacros.h>

#include <map>

class StrictMathTestCaseSin: public StrictMathTestCaseBase  {

	CPPUNIT_TEST_SUITE(StrictMathTestCaseSin);

	CPPUNIT_TEST(testSin);

	CPPUNIT_TEST_SUITE_END();

public:
	void setUp();
	void tearDown();

	void testSin();
};

