#include <gtest/gtest.h>
#include "core/serialization/BinarySerializer.h"

namespace nap {
namespace test {

class BinarySerializerTest : public ::testing::Test {
protected:
    void SetUp() override {
        serializer = std::make_unique<BinarySerializer>();
    }

    std::unique_ptr<BinarySerializer> serializer;
};

TEST_F(BinarySerializerTest, Construction) {
    EXPECT_EQ(serializer->getFormatName(), "Binary");
    EXPECT_EQ(serializer->getFileExtension(), ".napb");
    EXPECT_FALSE(serializer->isCompressionEnabled());
}

TEST_F(BinarySerializerTest, SetCompression) {
    serializer->setCompression(true);
    EXPECT_TRUE(serializer->isCompressionEnabled());

    serializer->setCompression(false);
    EXPECT_FALSE(serializer->isCompressionEnabled());
}

TEST_F(BinarySerializerTest, SerializeProducesBinary) {
    auto binary = serializer->serializeBinary();
    EXPECT_FALSE(binary.empty());

    // Check magic number (NAPB = 0x4E415042)
    ASSERT_GE(binary.size(), 4u);
    uint32_t magic = binary[0] | (binary[1] << 8) | (binary[2] << 16) | (binary[3] << 24);
    EXPECT_EQ(magic, BinarySerializer::MAGIC_NUMBER);
}

TEST_F(BinarySerializerTest, SerializeContainsVersion) {
    auto binary = serializer->serializeBinary();
    ASSERT_GE(binary.size(), 6u);

    uint16_t version = binary[4] | (binary[5] << 8);
    EXPECT_EQ(version, BinarySerializer::FORMAT_VERSION);
}

TEST_F(BinarySerializerTest, DeserializeValid) {
    auto binary = serializer->serializeBinary();
    EXPECT_TRUE(serializer->deserializeBinary(binary));
    EXPECT_TRUE(serializer->isValid());
}

TEST_F(BinarySerializerTest, DeserializeEmpty) {
    std::vector<uint8_t> empty;
    EXPECT_FALSE(serializer->deserializeBinary(empty));
    EXPECT_FALSE(serializer->getLastError().empty());
}

TEST_F(BinarySerializerTest, DeserializeInvalidMagic) {
    std::vector<uint8_t> invalid = {0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00};
    EXPECT_FALSE(serializer->deserializeBinary(invalid));
}

TEST_F(BinarySerializerTest, DeserializeTooSmall) {
    std::vector<uint8_t> tooSmall = {0x42, 0x50, 0x41, 0x4E};  // Only magic, no version
    EXPECT_FALSE(serializer->deserializeBinary(tooSmall));
}

TEST_F(BinarySerializerTest, SerializeString) {
    std::string str = serializer->serialize();
    EXPECT_FALSE(str.empty());
}

TEST_F(BinarySerializerTest, RoundTrip) {
    auto binary1 = serializer->serializeBinary();
    EXPECT_TRUE(serializer->deserializeBinary(binary1));
    EXPECT_TRUE(serializer->isValid());
}

} // namespace test
} // namespace nap
