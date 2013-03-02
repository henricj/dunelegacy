
#include "StrictMathTestCaseBase.h"

#include <misc/strictmath.h>

#include <cppunit/extensions/HelperMacros.h>

#include <map>

class StrictMathTestCaseCos: public StrictMathTestCaseBase  {

	CPPUNIT_TEST_SUITE(StrictMathTestCaseCos);

	CPPUNIT_TEST(testCos);

	CPPUNIT_TEST_SUITE_END();

public:
	void setUp();
	void tearDown();

	void testCos();
};

