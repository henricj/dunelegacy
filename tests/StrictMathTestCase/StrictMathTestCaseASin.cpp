#include "StrictMathTestCaseASin.h"

#include <cppunit/extensions/HelperMacros.h>

CPPUNIT_TEST_SUITE_REGISTRATION(StrictMathTestCaseASin);


void StrictMathTestCaseASin::setUp() {
	loadReferenceData(TESTSRC "/StrictMathTestCase/asin.ref");
}

void StrictMathTestCaseASin::tearDown() {

}

void StrictMathTestCaseASin::testASin() {
	std::vector<std::pair<float,float> >::iterator iter;

	for(iter = referenceData.begin(); iter != referenceData.end(); ++iter) {
		float argument = iter->first;
		float referenceResult = iter->second;

		float result = strictmath::asin(argument);

		if(isnan(result) && isnan(referenceResult)) {
			continue;
		}

		CPPUNIT_ASSERT_EQUAL_MESSAGE("asin(" + float2String(argument) + "f)", referenceResult, result);
	}
}



