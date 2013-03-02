
#include "StrictMathTestCaseBase.h"

#include <misc/strictmath.h>

#include <cppunit/extensions/HelperMacros.h>

#include <map>

class StrictMathTestCaseFloor: public StrictMathTestCaseBase  {

	CPPUNIT_TEST_SUITE(StrictMathTestCaseFloor);

	CPPUNIT_TEST(testFloor);

	CPPUNIT_TEST_SUITE_END();

public:
	void setUp();
	void tearDown();

	void testFloor();
};

