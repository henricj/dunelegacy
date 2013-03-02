
#include "StrictMathTestCaseBase.h"

#include <misc/strictmath.h>

#include <cppunit/extensions/HelperMacros.h>

#include <map>

class StrictMathTestCaseATan: public StrictMathTestCaseBase  {

	CPPUNIT_TEST_SUITE(StrictMathTestCaseATan);

	CPPUNIT_TEST(testATan);

	CPPUNIT_TEST_SUITE_END();

public:
	void setUp();
	void tearDown();

	void testATan();
};

