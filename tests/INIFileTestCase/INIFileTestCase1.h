
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

class INIFileTestCase1 : public testing::Test {

public:
    ~INIFileTestCase1();

    void SetUp() override;
    void TearDown() override;

protected:
    std::unique_ptr<INIFile> pINIFile;
    //(TESTSRC "/INIFileTestCase/INIFileTestCase1.ini");
};
