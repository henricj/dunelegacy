#include "StrictMathTestCaseACos.h"

#include <cppunit/extensions/HelperMacros.h>

CPPUNIT_TEST_SUITE_REGISTRATION(StrictMathTestCaseACos);


void StrictMathTestCaseACos::setUp() {
	loadReferenceData(TESTSRC "/StrictMathTestCase/acos.ref");
}

void StrictMathTestCaseACos::tearDown() {

}

void StrictMathTestCaseACos::testACos() {
	std::vector<std::pair<float,float> >::iterator iter;

	for(iter = referenceData.begin(); iter != referenceData.end(); ++iter) {
		float argument = iter->first;
		float referenceResult = iter->second;

		float result = strictmath::acos(argument);

		if(isnan(result) && isnan(referenceResult)) {
			continue;
		}

		CPPUNIT_ASSERT_EQUAL_MESSAGE("acos(" + float2String(argument) + "f)", referenceResult, result);
	}
}



