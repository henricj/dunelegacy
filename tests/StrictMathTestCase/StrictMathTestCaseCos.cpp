#include "StrictMathTestCaseCos.h"

#include <cppunit/extensions/HelperMacros.h>

CPPUNIT_TEST_SUITE_REGISTRATION(StrictMathTestCaseCos);


void StrictMathTestCaseCos::setUp() {
	loadReferenceData(TESTSRC "/StrictMathTestCase/cos.ref");
}

void StrictMathTestCaseCos::tearDown() {

}

void StrictMathTestCaseCos::testCos() {
	std::vector<std::pair<float,float> >::iterator iter;

	for(iter = referenceData.begin(); iter != referenceData.end(); ++iter) {
		float argument = iter->first;
		float referenceResult = iter->second;

		float result = strictmath::cos(argument);

		if(isnan(result) && isnan(referenceResult)) {
			continue;
		}

		CPPUNIT_ASSERT_EQUAL_MESSAGE("cos(" + float2String(argument) + "f)", referenceResult, result);
	}
}



