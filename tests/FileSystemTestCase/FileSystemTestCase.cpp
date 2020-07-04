#include "FileSystemTestCase.h"

#include <misc/FileSystem.h>

TEST_F(FileSystemTestCase, basename) {
    EXPECT_EQ(getBasename("test"), "test");
    EXPECT_EQ(getBasename("/test"), "test");
    //EXPECT_EQ(getBasename("/test/"), "test");
    EXPECT_EQ(getBasename("/test/"), "");
    EXPECT_EQ(getBasename("./test.txt"), "test.txt");
    EXPECT_EQ(getBasename("/bla/test.txt"), "test.txt");
    EXPECT_EQ(getBasename("/"), "/");
}

TEST_F(FileSystemTestCase, basenameExtension) {
    EXPECT_EQ(getBasename("test.txt", true), "test");
    EXPECT_EQ(getBasename("/test.txt", true), "test");
    //EXPECT_EQ(getBasename("/test.txt/", true), "test");
    EXPECT_EQ(getBasename("/test.txt/", true), "");
    EXPECT_EQ(getBasename("./test.txt.txt", true), "test.txt");
    EXPECT_EQ(getBasename("/bla/test.txt.txt", true), "test.txt");
}

TEST_F(FileSystemTestCase, dirname) {
    EXPECT_EQ(getDirname("/test/test"), "/test");
    //EXPECT_EQ(getDirname("/test/test/"), "/test");
    EXPECT_EQ(getDirname("/test/test/"), "/test/test");
    //EXPECT_EQ(getDirname("test"), ".");
    EXPECT_EQ(getDirname("test"), "");
    EXPECT_EQ(getDirname("/"), "/");
}
