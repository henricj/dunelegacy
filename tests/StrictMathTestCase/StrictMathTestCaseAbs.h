
#include "StrictMathTestCaseBase.h"

#include <misc/strictmath.h>

#include <cppunit/extensions/HelperMacros.h>

#include <map>

class StrictMathTestCaseAbs: public StrictMathTestCaseBase  {

	CPPUNIT_TEST_SUITE(StrictMathTestCaseAbs);

	CPPUNIT_TEST(testAbs);

	CPPUNIT_TEST_SUITE_END();

public:
	void setUp();
	void tearDown();

	void testAbs();
};

