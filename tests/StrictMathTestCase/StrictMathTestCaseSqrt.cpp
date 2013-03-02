#include "StrictMathTestCaseSqrt.h"

#include <cppunit/extensions/HelperMacros.h>

CPPUNIT_TEST_SUITE_REGISTRATION(StrictMathTestCaseSqrt);


void StrictMathTestCaseSqrt::setUp() {
	loadReferenceData(TESTSRC "/StrictMathTestCase/sqrt.ref");
}

void StrictMathTestCaseSqrt::tearDown() {

}

void StrictMathTestCaseSqrt::testSqrt() {
	std::vector<std::pair<float,float> >::iterator iter;

	for(iter = referenceData.begin(); iter != referenceData.end(); ++iter) {
		float argument = iter->first;
		float referenceResult = iter->second;

		float result = strictmath::sqrt(argument);

		if(isnan(result) && isnan(referenceResult)) {
			continue;
		}

		CPPUNIT_ASSERT_EQUAL_MESSAGE("sqrt(" + float2String(argument) + "f)", referenceResult, result);
	}
}



