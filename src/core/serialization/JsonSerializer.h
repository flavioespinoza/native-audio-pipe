#ifndef NAP_JSON_SERIALIZER_H
#define NAP_JSON_SERIALIZER_H

#include "core/serialization/ISerializer.h"
#include <memory>

namespace nap {

// Forward declarations
class AudioGraph;
class ParameterGroup;

/**
 * @brief JSON-based serializer for audio graphs
 *
 * Serializes audio graph state to human-readable JSON format.
 * Supports pretty printing and compact output modes.
 */
class JsonSerializer : public ISerializer {
public:
    JsonSerializer();
    ~JsonSerializer() override;

    // Non-copyable, movable
    JsonSerializer(const JsonSerializer&) = delete;
    JsonSerializer& operator=(const JsonSerializer&) = delete;
    JsonSerializer(JsonSerializer&&) noexcept;
    JsonSerializer& operator=(JsonSerializer&&) noexcept;

    // ISerializer interface
    std::string serialize() const override;
    std::vector<uint8_t> serializeBinary() const override;
    bool deserialize(const std::string& data) override;
    bool deserializeBinary(const std::vector<uint8_t>& data) override;
    bool saveToFile(const std::string& filepath) const override;
    bool loadFromFile(const std::string& filepath) override;
    bool isValid() const override;
    std::string getLastError() const override;
    std::string getFormatName() const override;
    std::string getFileExtension() const override;

    // JSON-specific options
    void setPrettyPrint(bool enabled);
    bool isPrettyPrint() const;
    void setIndentSize(int spaces);
    int getIndentSize() const;

    // Graph binding
    void setGraph(AudioGraph* graph);
    AudioGraph* getGraph() const;

    // Parameter serialization
    void setParameterGroup(ParameterGroup* params);
    ParameterGroup* getParameterGroup() const;

private:
    class Impl;
    std::unique_ptr<Impl> pImpl;
};

} // namespace nap

#endif // NAP_JSON_SERIALIZER_H
