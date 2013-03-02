#include "StrictMathTestCaseFloor.h"

#include <cppunit/extensions/HelperMacros.h>

CPPUNIT_TEST_SUITE_REGISTRATION(StrictMathTestCaseFloor);


void StrictMathTestCaseFloor::setUp() {
	loadReferenceData(TESTSRC "/StrictMathTestCase/floor.ref");
}

void StrictMathTestCaseFloor::tearDown() {

}

void StrictMathTestCaseFloor::testFloor() {
	std::vector<std::pair<float,float> >::iterator iter;

	for(iter = referenceData.begin(); iter != referenceData.end(); ++iter) {
		float argument = iter->first;
		float referenceResult = iter->second;

		float result = strictmath::floor(argument);

		if(isnan(result) && isnan(referenceResult)) {
			continue;
		}

		CPPUNIT_ASSERT_EQUAL_MESSAGE("floor(" + float2String(argument) + "f)", referenceResult, result);
	}
}



