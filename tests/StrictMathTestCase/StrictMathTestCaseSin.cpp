#include "StrictMathTestCaseSin.h"

#include <cppunit/extensions/HelperMacros.h>

CPPUNIT_TEST_SUITE_REGISTRATION(StrictMathTestCaseSin);


void StrictMathTestCaseSin::setUp() {
	loadReferenceData(TESTSRC "/StrictMathTestCase/sin.ref");
}

void StrictMathTestCaseSin::tearDown() {

}

void StrictMathTestCaseSin::testSin() {
	std::vector<std::pair<float,float> >::iterator iter;

	for(iter = referenceData.begin(); iter != referenceData.end(); ++iter) {
		float argument = iter->first;
		float referenceResult = iter->second;

		float result = strictmath::sin(argument);

		if(isnan(result) && isnan(referenceResult)) {
			continue;
		}

		CPPUNIT_ASSERT_EQUAL_MESSAGE("sin(" + float2String(argument) + "f)", referenceResult, result);
	}
}



