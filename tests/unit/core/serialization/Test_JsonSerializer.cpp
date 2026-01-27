#include <gtest/gtest.h>
#include "core/serialization/JsonSerializer.h"

namespace nap {
namespace test {

class JsonSerializerTest : public ::testing::Test {
protected:
    void SetUp() override {
        serializer = std::make_unique<JsonSerializer>();
    }

    std::unique_ptr<JsonSerializer> serializer;
};

TEST_F(JsonSerializerTest, Construction) {
    EXPECT_EQ(serializer->getFormatName(), "JSON");
    EXPECT_EQ(serializer->getFileExtension(), ".json");
    EXPECT_TRUE(serializer->isPrettyPrint());
}

TEST_F(JsonSerializerTest, SetPrettyPrint) {
    serializer->setPrettyPrint(false);
    EXPECT_FALSE(serializer->isPrettyPrint());

    serializer->setPrettyPrint(true);
    EXPECT_TRUE(serializer->isPrettyPrint());
}

TEST_F(JsonSerializerTest, SetIndentSize) {
    serializer->setIndentSize(4);
    EXPECT_EQ(serializer->getIndentSize(), 4);
}

TEST_F(JsonSerializerTest, SerializeProducesJson) {
    std::string json = serializer->serialize();
    EXPECT_FALSE(json.empty());
    EXPECT_EQ(json.front(), '{');
    EXPECT_EQ(json.back(), '}');
}

TEST_F(JsonSerializerTest, DeserializeValid) {
    std::string json = R"({"format": "nap-audio-graph"})";
    EXPECT_TRUE(serializer->deserialize(json));
    EXPECT_TRUE(serializer->isValid());
}

TEST_F(JsonSerializerTest, DeserializeEmpty) {
    EXPECT_FALSE(serializer->deserialize(""));
    EXPECT_FALSE(serializer->isValid());
    EXPECT_FALSE(serializer->getLastError().empty());
}

TEST_F(JsonSerializerTest, DeserializeInvalid) {
    EXPECT_FALSE(serializer->deserialize("not json"));
    EXPECT_FALSE(serializer->isValid());
}

TEST_F(JsonSerializerTest, SerializeBinary) {
    auto binary = serializer->serializeBinary();
    EXPECT_FALSE(binary.empty());
}

TEST_F(JsonSerializerTest, DeserializeBinary) {
    std::string json = R"({"format": "test"})";
    std::vector<uint8_t> binary(json.begin(), json.end());
    EXPECT_TRUE(serializer->deserializeBinary(binary));
}

TEST_F(JsonSerializerTest, GraphBinding) {
    EXPECT_EQ(serializer->getGraph(), nullptr);
    // Can't fully test without AudioGraph, but verify binding works
}

TEST_F(JsonSerializerTest, ParameterGroupBinding) {
    EXPECT_EQ(serializer->getParameterGroup(), nullptr);
}

} // namespace test
} // namespace nap
