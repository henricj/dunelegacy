#include "FileSystemTestCase.h"

#include <misc/FileSystem.h>

#include <cppunit/extensions/HelperMacros.h>

CPPUNIT_TEST_SUITE_REGISTRATION(FileSystemTestCase);


void FileSystemTestCase::setUp() {
}

void FileSystemTestCase::tearDown() {
}

void FileSystemTestCase::testBasename() {
	CPPUNIT_ASSERT(getBasename("test") == "test");
	CPPUNIT_ASSERT(getBasename("/test") == "test");
	CPPUNIT_ASSERT(getBasename("/test/") == "test");
	CPPUNIT_ASSERT(getBasename("./test.txt") == "test.txt");
	CPPUNIT_ASSERT(getBasename("/bla/test.txt") == "test.txt");
	CPPUNIT_ASSERT(getBasename("/") == "/");
}

void FileSystemTestCase::testBasenameExtension() {
	CPPUNIT_ASSERT(getBasename("test.txt", true) == "test");
	CPPUNIT_ASSERT(getBasename("/test.txt", true) == "test");
	CPPUNIT_ASSERT(getBasename("/test.txt/", true) == "test");
	CPPUNIT_ASSERT(getBasename("./test.txt.txt", true) == "test.txt");
	CPPUNIT_ASSERT(getBasename("/bla/test.txt.txt", true) == "test.txt");
}

void FileSystemTestCase::testDirname() {
	CPPUNIT_ASSERT(getDirname("/test/test") == "/test");
	CPPUNIT_ASSERT(getDirname("/test/test/") == "/test");
	CPPUNIT_ASSERT(getDirname("test") == ".");
	CPPUNIT_ASSERT(getDirname("/") == "/");

}


