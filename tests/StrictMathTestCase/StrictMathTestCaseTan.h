
#include "StrictMathTestCaseBase.h"

#include <misc/strictmath.h>

#include <cppunit/extensions/HelperMacros.h>

#include <map>

class StrictMathTestCaseTan: public StrictMathTestCaseBase  {

	CPPUNIT_TEST_SUITE(StrictMathTestCaseTan);

	CPPUNIT_TEST(testTan);

	CPPUNIT_TEST_SUITE_END();

public:
	void setUp();
	void tearDown();

	void testTan();
};

