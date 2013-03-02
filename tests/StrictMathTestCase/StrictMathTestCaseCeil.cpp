#include "StrictMathTestCaseCeil.h"

#include <cppunit/extensions/HelperMacros.h>

CPPUNIT_TEST_SUITE_REGISTRATION(StrictMathTestCaseCeil);


void StrictMathTestCaseCeil::setUp() {
	loadReferenceData(TESTSRC "/StrictMathTestCase/ceil.ref");
}

void StrictMathTestCaseCeil::tearDown() {

}

void StrictMathTestCaseCeil::testCeil() {
	std::vector<std::pair<float,float> >::iterator iter;

	for(iter = referenceData.begin(); iter != referenceData.end(); ++iter) {
		float argument = iter->first;
		float referenceResult = iter->second;

		float result = strictmath::ceil(argument);

		if(isnan(result) && isnan(referenceResult)) {
			continue;
		}

		CPPUNIT_ASSERT_EQUAL_MESSAGE("ceil(" + float2String(argument) + "f)", referenceResult, result);
	}
}



