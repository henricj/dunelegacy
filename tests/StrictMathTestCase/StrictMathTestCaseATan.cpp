#include "StrictMathTestCaseATan.h"

#include <cppunit/extensions/HelperMacros.h>

CPPUNIT_TEST_SUITE_REGISTRATION(StrictMathTestCaseATan);


void StrictMathTestCaseATan::setUp() {
	loadReferenceData(TESTSRC "/StrictMathTestCase/atan.ref");
}

void StrictMathTestCaseATan::tearDown() {

}

void StrictMathTestCaseATan::testATan() {
	std::vector<std::pair<float,float> >::iterator iter;

	for(iter = referenceData.begin(); iter != referenceData.end(); ++iter) {
		float argument = iter->first;
		float referenceResult = iter->second;

		float result = strictmath::atan(argument);

		if(isnan(result) && isnan(referenceResult)) {
			continue;
		}

		CPPUNIT_ASSERT_EQUAL_MESSAGE("atan(" + float2String(argument) + "f)", referenceResult, result);
	}
}



