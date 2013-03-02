#include "StrictMathTestCaseAbs.h"

#include <cppunit/extensions/HelperMacros.h>

CPPUNIT_TEST_SUITE_REGISTRATION(StrictMathTestCaseAbs);


void StrictMathTestCaseAbs::setUp() {
	loadReferenceData(TESTSRC "/StrictMathTestCase/abs.ref");
}

void StrictMathTestCaseAbs::tearDown() {

}

void StrictMathTestCaseAbs::testAbs() {
	std::vector<std::pair<float,float> >::iterator iter;

	for(iter = referenceData.begin(); iter != referenceData.end(); ++iter) {
		float argument = iter->first;
		float referenceResult = iter->second;

		float result = strictmath::abs(argument);

		if(isnan(result) && isnan(referenceResult)) {
			continue;
		}

		CPPUNIT_ASSERT_EQUAL_MESSAGE("abs(" + float2String(argument) + "f)", referenceResult, result);
	}
}



