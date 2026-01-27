#include <gtest/gtest.h>
#include "../../../../src/nodes/source/FileStreamReader.h"

namespace nap { namespace test {

TEST(FileStreamReaderTest, InitiallyNoFileLoaded) {
    FileStreamReader reader;
    EXPECT_FALSE(reader.isFileLoaded());
}

TEST(FileStreamReaderTest, CanUnloadFile) {
    FileStreamReader reader;
    reader.unloadFile();
    EXPECT_FALSE(reader.isFileLoaded());
}

TEST(FileStreamReaderTest, DefaultLoopingIsFalse) {
    FileStreamReader reader;
    EXPECT_FALSE(reader.isLooping());
}

TEST(FileStreamReaderTest, CanEnableLooping) {
    FileStreamReader reader;
    reader.setLooping(true);
    EXPECT_TRUE(reader.isLooping());
}

TEST(FileStreamReaderTest, HasCorrectTypeName) {
    FileStreamReader reader;
    EXPECT_EQ(reader.getTypeName(), "FileStreamReader");
}

TEST(FileStreamReaderTest, IsSeekableWhenLoaded) {
    FileStreamReader reader;
    reader.loadFile("/fake/path.wav");
    EXPECT_TRUE(reader.isSeekable());
}

}} // namespace nap::test
