
#include <misc/strictmath.h>

#include <cppunit/extensions/HelperMacros.h>

#include <string>
#include <map>
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <utility>
#include <limits>
#include <stdlib.h>

class StrictMathTestCaseBase: public CppUnit::TestFixture  {

protected:
	std::string float2String(float x) {
		std::ostringstream os;
    	if (!(os << x))
        	throw std::runtime_error("float failed!");
    	return os.str();
	}

	void loadReferenceData(std::string filename) {
		std::ifstream file(filename.c_str());
		std::string completeline;
		while(std::getline(file, completeline)) {
			if(completeline.empty()) {
				continue;
			}

			if(completeline.at(0) == '#') {
				// comment
				continue;
			}


			std::istringstream linestream(completeline);

			std::string strArgument;
			std::string strResult;

			linestream >> strArgument;
			linestream >> strResult;

			float argument;
			float result;

			if(strArgument == "nan") {
				argument = std::numeric_limits<float>::quiet_NaN();
			} else if(strArgument == "-nan") {
				argument = -std::numeric_limits<float>::quiet_NaN();
			} else {
				argument = atof(strArgument.c_str());
			}

			if(strResult == "nan") {
				result = std::numeric_limits<float>::quiet_NaN();
			} else if(strResult == "-nan") {
				result = -std::numeric_limits<float>::quiet_NaN();
			} else {
				result = atof(strResult.c_str());
			}

			referenceData.push_back(std::make_pair(argument,result));
    	}
	}

	std::vector<std::pair<float, float> > referenceData;
};

