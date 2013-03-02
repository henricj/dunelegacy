TESTS = runtests
check_PROGRAMS = $(TESTS)

runtests_SOURCES =  testmain.cpp\
					$(NULL)\
                    ../src/FileClasses/INIFile.cpp\
                    $(NULL)\
                    INIFileTestCase/INIFileTestCase1.cpp\
                    INIFileTestCase/INIFileTestCase2.cpp\
                    INIFileTestCase/INIFileTestCase3.cpp\
                    $(NULL)\
                    ../src/misc/strictmath.cpp\
                    $(NULL)\
                    StrictMathTestCase/StrictMathTestCaseAbs.cpp\
                    StrictMathTestCase/StrictMathTestCaseFloor.cpp\
                    StrictMathTestCase/StrictMathTestCaseCeil.cpp\
                    StrictMathTestCase/StrictMathTestCaseSin.cpp\
                    StrictMathTestCase/StrictMathTestCaseCos.cpp\
                    StrictMathTestCase/StrictMathTestCaseTan.cpp\
                    StrictMathTestCase/StrictMathTestCaseASin.cpp\
                    StrictMathTestCase/StrictMathTestCaseACos.cpp\
                    StrictMathTestCase/StrictMathTestCaseATan.cpp\
                    StrictMathTestCase/StrictMathTestCaseSqrt.cpp\
                    $(NULL)\
                    ../src/misc/FileSystem.cpp\
                    $(NULL)\
                    FileSystemTestCase/FileSystemTestCase.cpp\
                    $(NULL)

EXTRA_DIST = INIFileTestCase/INIFileTestCase1.h\
             INIFileTestCase/INIFileTestCase2.h\
             INIFileTestCase/INIFileTestCase3.h\
             INIFileTestCase/INIFileTestCase1.ini\
             INIFileTestCase/INIFileTestCase2.ini\
             INIFileTestCase/INIFileTestCase3.ini\
             INIFileTestCase/INIFileTestCase2.ini.ref1\
             INIFileTestCase/INIFileTestCase2.ini.ref2\
             INIFileTestCase/INIFileTestCase2.ini.ref3\
             INIFileTestCase/INIFileTestCase3.ini.ref1\
             INIFileTestCase/INIFileTestCase3.ini.ref2\
             INIFileTestCase/INIFileTestCase3.ini.ref3\
             INIFileTestCase/INIFileTestCase3.ini.ref4\
             StrictMathTestCase/StrictMathTestCaseBase.h\
             StrictMathTestCase/StrictMathTestCaseAbs.h\
             StrictMathTestCase/StrictMathTestCaseFloor.h\
             StrictMathTestCase/StrictMathTestCaseCeil.h\
             StrictMathTestCase/StrictMathTestCaseSin.h\
             StrictMathTestCase/StrictMathTestCaseCos.h\
             StrictMathTestCase/StrictMathTestCaseTan.h\
             StrictMathTestCase/StrictMathTestCaseASin.h\
             StrictMathTestCase/StrictMathTestCaseACos.h\
             StrictMathTestCase/StrictMathTestCaseATan.h\
             StrictMathTestCase/StrictMathTestCaseSqrt.h\
             StrictMathTestCase/abs.ref\
             StrictMathTestCase/floor.ref\
             StrictMathTestCase/ceil.ref\
             StrictMathTestCase/sin.ref\
             StrictMathTestCase/cos.ref\
             StrictMathTestCase/tan.ref\
             StrictMathTestCase/asin.ref\
             StrictMathTestCase/acos.ref\
             StrictMathTestCase/atan.ref\
             StrictMathTestCase/sqrt.ref\
             FileSystemTestCase/FileSystemTestCase.h\
             $(NULL)



runtests_CXXFLAGS = $(CPPUNIT_CFLAGS) -DTESTSRC=\"$(srcdir)\" -I$(top_srcdir)/include
runtests_LDADD = $(CPPUNIT_LIBS) -lcppunit
