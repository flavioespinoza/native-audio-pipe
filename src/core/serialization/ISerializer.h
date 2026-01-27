#ifndef NAP_ISERIALIZER_H
#define NAP_ISERIALIZER_H

#include <string>
#include <vector>
#include <cstdint>

namespace nap {

/**
 * @brief Interface for serialization/deserialization of audio graphs
 *
 * Defines the contract for saving and loading audio graph state
 * to various formats (JSON, binary, etc.).
 */
class ISerializer {
public:
    virtual ~ISerializer() = default;

    // Serialization
    virtual std::string serialize() const = 0;
    virtual std::vector<uint8_t> serializeBinary() const = 0;

    // Deserialization
    virtual bool deserialize(const std::string& data) = 0;
    virtual bool deserializeBinary(const std::vector<uint8_t>& data) = 0;

    // File operations
    virtual bool saveToFile(const std::string& filepath) const = 0;
    virtual bool loadFromFile(const std::string& filepath) = 0;

    // Validation
    virtual bool isValid() const = 0;
    virtual std::string getLastError() const = 0;

    // Format info
    virtual std::string getFormatName() const = 0;
    virtual std::string getFileExtension() const = 0;
};

} // namespace nap

#endif // NAP_ISERIALIZER_H
