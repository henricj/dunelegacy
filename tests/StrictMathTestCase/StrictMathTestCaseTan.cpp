#include "StrictMathTestCaseTan.h"

#include <cppunit/extensions/HelperMacros.h>

CPPUNIT_TEST_SUITE_REGISTRATION(StrictMathTestCaseTan);


void StrictMathTestCaseTan::setUp() {
	loadReferenceData(TESTSRC "/StrictMathTestCase/tan.ref");
}

void StrictMathTestCaseTan::tearDown() {

}

void StrictMathTestCaseTan::testTan() {
	std::vector<std::pair<float,float> >::iterator iter;

	for(iter = referenceData.begin(); iter != referenceData.end(); ++iter) {
		float argument = iter->first;
		float referenceResult = iter->second;

		float result = strictmath::tan(argument);

		if(isnan(result) && isnan(referenceResult)) {
			continue;
		}

		CPPUNIT_ASSERT_EQUAL_MESSAGE("tan(" + float2String(argument) + "f)", referenceResult, result);
	}
}



