
#include "StrictMathTestCaseBase.h"

#include <misc/strictmath.h>

#include <cppunit/extensions/HelperMacros.h>

#include <map>

class StrictMathTestCaseSqrt: public StrictMathTestCaseBase  {

	CPPUNIT_TEST_SUITE(StrictMathTestCaseSqrt);

	CPPUNIT_TEST(testSqrt);

	CPPUNIT_TEST_SUITE_END();

public:
	void setUp();
	void tearDown();

	void testSqrt();
};

