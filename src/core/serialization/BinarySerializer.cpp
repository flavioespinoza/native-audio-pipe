#include "core/serialization/BinarySerializer.h"
#include <fstream>
#include <cstring>

namespace nap {

class BinarySerializer::Impl {
public:
    AudioGraph* graph = nullptr;
    ParameterGroup* parameterGroup = nullptr;
    std::string lastError;
    bool compressionEnabled = false;
    bool valid = false;
    std::vector<uint8_t> cachedData;

    void clearError() { lastError.clear(); }
    void setError(const std::string& error) { lastError = error; }

    void writeHeader(std::vector<uint8_t>& buffer) const {
        // Magic number (4 bytes)
        buffer.push_back((MAGIC_NUMBER >> 0) & 0xFF);
        buffer.push_back((MAGIC_NUMBER >> 8) & 0xFF);
        buffer.push_back((MAGIC_NUMBER >> 16) & 0xFF);
        buffer.push_back((MAGIC_NUMBER >> 24) & 0xFF);

        // Version (2 bytes)
        buffer.push_back((FORMAT_VERSION >> 0) & 0xFF);
        buffer.push_back((FORMAT_VERSION >> 8) & 0xFF);

        // Flags (2 bytes)
        uint16_t flags = compressionEnabled ? 0x0001 : 0x0000;
        buffer.push_back((flags >> 0) & 0xFF);
        buffer.push_back((flags >> 8) & 0xFF);
    }

    bool readHeader(const std::vector<uint8_t>& buffer) {
        if (buffer.size() < 8) {
            setError("Buffer too small for header");
            return false;
        }

        uint32_t magic = buffer[0] | (buffer[1] << 8) | (buffer[2] << 16) | (buffer[3] << 24);
        if (magic != MAGIC_NUMBER) {
            setError("Invalid magic number");
            return false;
        }

        uint16_t version = buffer[4] | (buffer[5] << 8);
        if (version > FORMAT_VERSION) {
            setError("Unsupported format version");
            return false;
        }

        return true;
    }
};

BinarySerializer::BinarySerializer()
    : pImpl(std::make_unique<Impl>()) {}

BinarySerializer::~BinarySerializer() = default;

BinarySerializer::BinarySerializer(BinarySerializer&&) noexcept = default;
BinarySerializer& BinarySerializer::operator=(BinarySerializer&&) noexcept = default;

std::string BinarySerializer::serialize() const {
    auto binary = serializeBinary();
    return std::string(binary.begin(), binary.end());
}

std::vector<uint8_t> BinarySerializer::serializeBinary() const {
    std::vector<uint8_t> buffer;
    buffer.reserve(1024);

    pImpl->writeHeader(buffer);

    // Placeholder: node count (4 bytes)
    uint32_t nodeCount = 0;
    buffer.push_back((nodeCount >> 0) & 0xFF);
    buffer.push_back((nodeCount >> 8) & 0xFF);
    buffer.push_back((nodeCount >> 16) & 0xFF);
    buffer.push_back((nodeCount >> 24) & 0xFF);

    // Placeholder: connection count (4 bytes)
    uint32_t connectionCount = 0;
    buffer.push_back((connectionCount >> 0) & 0xFF);
    buffer.push_back((connectionCount >> 8) & 0xFF);
    buffer.push_back((connectionCount >> 16) & 0xFF);
    buffer.push_back((connectionCount >> 24) & 0xFF);

    return buffer;
}

bool BinarySerializer::deserialize(const std::string& data) {
    return deserializeBinary(std::vector<uint8_t>(data.begin(), data.end()));
}

bool BinarySerializer::deserializeBinary(const std::vector<uint8_t>& data) {
    pImpl->clearError();

    if (!pImpl->readHeader(data)) {
        return false;
    }

    pImpl->cachedData = data;
    pImpl->valid = true;
    return true;
}

bool BinarySerializer::saveToFile(const std::string& filepath) const {
    pImpl->clearError();

    std::ofstream file(filepath, std::ios::binary);
    if (!file.is_open()) {
        pImpl->setError("Failed to open file for writing: " + filepath);
        return false;
    }

    auto data = serializeBinary();
    file.write(reinterpret_cast<const char*>(data.data()), data.size());
    file.close();

    if (file.fail()) {
        pImpl->setError("Failed to write to file: " + filepath);
        return false;
    }

    return true;
}

bool BinarySerializer::loadFromFile(const std::string& filepath) {
    pImpl->clearError();

    std::ifstream file(filepath, std::ios::binary | std::ios::ate);
    if (!file.is_open()) {
        pImpl->setError("Failed to open file for reading: " + filepath);
        return false;
    }

    std::streamsize size = file.tellg();
    file.seekg(0, std::ios::beg);

    std::vector<uint8_t> buffer(size);
    if (!file.read(reinterpret_cast<char*>(buffer.data()), size)) {
        pImpl->setError("Failed to read file: " + filepath);
        return false;
    }

    return deserializeBinary(buffer);
}

bool BinarySerializer::isValid() const {
    return pImpl->valid;
}

std::string BinarySerializer::getLastError() const {
    return pImpl->lastError;
}

std::string BinarySerializer::getFormatName() const {
    return "Binary";
}

std::string BinarySerializer::getFileExtension() const {
    return ".napb";
}

void BinarySerializer::setCompression(bool enabled) {
    pImpl->compressionEnabled = enabled;
}

bool BinarySerializer::isCompressionEnabled() const {
    return pImpl->compressionEnabled;
}

void BinarySerializer::setGraph(AudioGraph* graph) {
    pImpl->graph = graph;
}

AudioGraph* BinarySerializer::getGraph() const {
    return pImpl->graph;
}

void BinarySerializer::setParameterGroup(ParameterGroup* params) {
    pImpl->parameterGroup = params;
}

ParameterGroup* BinarySerializer::getParameterGroup() const {
    return pImpl->parameterGroup;
}

} // namespace nap
