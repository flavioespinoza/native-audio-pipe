#ifndef NAP_BINARY_SERIALIZER_H
#define NAP_BINARY_SERIALIZER_H

#include "core/serialization/ISerializer.h"
#include <memory>

namespace nap {

// Forward declarations
class AudioGraph;
class ParameterGroup;

/**
 * @brief Binary serializer for audio graphs
 *
 * Serializes audio graph state to compact binary format.
 * Optimized for fast loading and small file sizes.
 */
class BinarySerializer : public ISerializer {
public:
    // Magic number for file format identification
    static constexpr uint32_t MAGIC_NUMBER = 0x4E415042; // "NAPB"
    static constexpr uint16_t FORMAT_VERSION = 1;

    BinarySerializer();
    ~BinarySerializer() override;

    // Non-copyable, movable
    BinarySerializer(const BinarySerializer&) = delete;
    BinarySerializer& operator=(const BinarySerializer&) = delete;
    BinarySerializer(BinarySerializer&&) noexcept;
    BinarySerializer& operator=(BinarySerializer&&) noexcept;

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

    // Binary-specific options
    void setCompression(bool enabled);
    bool isCompressionEnabled() const;

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

#endif // NAP_BINARY_SERIALIZER_H
