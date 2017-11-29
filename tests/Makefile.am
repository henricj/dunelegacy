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
                    ../src/misc/FileSystem.cpp\
                    ../src/misc/format.cpp\
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
             FileSystemTestCase/FileSystemTestCase.h\
             $(NULL)



runtests_CXXFLAGS = $(CPPUNIT_CFLAGS) -DTESTSRC=\"$(srcdir)\" -I$(top_srcdir)/include
runtests_LDADD = $(CPPUNIT_LIBS) -lcppunit
