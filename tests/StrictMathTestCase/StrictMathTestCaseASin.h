
#include "StrictMathTestCaseBase.h"

#include <misc/strictmath.h>

#include <cppunit/extensions/HelperMacros.h>

#include <map>

class StrictMathTestCaseASin: public StrictMathTestCaseBase  {

	CPPUNIT_TEST_SUITE(StrictMathTestCaseASin);

	CPPUNIT_TEST(testASin);

	CPPUNIT_TEST_SUITE_END();

public:
	void setUp();
	void tearDown();

	void testASin();
};

