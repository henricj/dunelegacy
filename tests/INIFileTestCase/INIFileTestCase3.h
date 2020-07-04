
#include <FileClasses/INIFile.h>

#ifdef COLOR_DEFAULT
#    undef COLOR_DEFAULT
#endif

#ifdef COLOR_RED
#    undef COLOR_RED
#endif

#ifdef COLOR_GREEN
#    undef COLOR_GREEN
#endif

#ifdef COLOR_YELLOW
#    undef COLOR_YELLOW
#endif

#include "gtest/gtest.h"

class INIFileTestCase3: public testing::Test  {
protected:
	bool fileCompare(std::string filename1, std::string filename2);

	std::unique_ptr<INIFile> pINIFile;
};

