#include "core/serialization/JsonSerializer.h"
#include <fstream>
#include <sstream>

namespace nap {

class JsonSerializer::Impl {
public:
    AudioGraph* graph = nullptr;
    ParameterGroup* parameterGroup = nullptr;
    std::string lastError;
    bool prettyPrint = true;
    int indentSize = 2;
    bool valid = false;
    std::string cachedJson;

    void clearError() { lastError.clear(); }
    void setError(const std::string& error) { lastError = error; }
};

JsonSerializer::JsonSerializer()
    : pImpl(std::make_unique<Impl>()) {}

JsonSerializer::~JsonSerializer() = default;

JsonSerializer::JsonSerializer(JsonSerializer&&) noexcept = default;
JsonSerializer& JsonSerializer::operator=(JsonSerializer&&) noexcept = default;

std::string JsonSerializer::serialize() const {
    if (!pImpl->graph && !pImpl->parameterGroup) {
        pImpl->lastError = "No graph or parameter group bound";
        return "{}";
    }

    // Placeholder JSON structure
    std::ostringstream oss;
    std::string indent = pImpl->prettyPrint ? std::string(pImpl->indentSize, ' ') : "";
    std::string newline = pImpl->prettyPrint ? "\n" : "";

    oss << "{" << newline;
    oss << indent << "\"format\": \"nap-audio-graph\"," << newline;
    oss << indent << "\"version\": \"1.0.0\"," << newline;
    oss << indent << "\"nodes\": []," << newline;
    oss << indent << "\"connections\": []," << newline;
    oss << indent << "\"parameters\": {}" << newline;
    oss << "}";

    return oss.str();
}

std::vector<uint8_t> JsonSerializer::serializeBinary() const {
    std::string json = serialize();
    return std::vector<uint8_t>(json.begin(), json.end());
}

bool JsonSerializer::deserialize(const std::string& data) {
    pImpl->clearError();

    if (data.empty()) {
        pImpl->setError("Empty JSON data");
        return false;
    }

    // Basic validation - check for JSON structure
    if (data.front() != '{' || data.back() != '}') {
        pImpl->setError("Invalid JSON format");
        return false;
    }

    pImpl->cachedJson = data;
    pImpl->valid = true;
    return true;
}

bool JsonSerializer::deserializeBinary(const std::vector<uint8_t>& data) {
    std::string json(data.begin(), data.end());
    return deserialize(json);
}

bool JsonSerializer::saveToFile(const std::string& filepath) const {
    pImpl->clearError();

    std::ofstream file(filepath);
    if (!file.is_open()) {
        pImpl->setError("Failed to open file for writing: " + filepath);
        return false;
    }

    file << serialize();
    file.close();

    if (file.fail()) {
        pImpl->setError("Failed to write to file: " + filepath);
        return false;
    }

    return true;
}

bool JsonSerializer::loadFromFile(const std::string& filepath) {
    pImpl->clearError();

    std::ifstream file(filepath);
    if (!file.is_open()) {
        pImpl->setError("Failed to open file for reading: " + filepath);
        return false;
    }

    std::stringstream buffer;
    buffer << file.rdbuf();

    if (file.fail()) {
        pImpl->setError("Failed to read file: " + filepath);
        return false;
    }

    return deserialize(buffer.str());
}

bool JsonSerializer::isValid() const {
    return pImpl->valid;
}

std::string JsonSerializer::getLastError() const {
    return pImpl->lastError;
}

std::string JsonSerializer::getFormatName() const {
    return "JSON";
}

std::string JsonSerializer::getFileExtension() const {
    return ".json";
}

void JsonSerializer::setPrettyPrint(bool enabled) {
    pImpl->prettyPrint = enabled;
}

bool JsonSerializer::isPrettyPrint() const {
    return pImpl->prettyPrint;
}

void JsonSerializer::setIndentSize(int spaces) {
    pImpl->indentSize = spaces;
}

int JsonSerializer::getIndentSize() const {
    return pImpl->indentSize;
}

void JsonSerializer::setGraph(AudioGraph* graph) {
    pImpl->graph = graph;
}

AudioGraph* JsonSerializer::getGraph() const {
    return pImpl->graph;
}

void JsonSerializer::setParameterGroup(ParameterGroup* params) {
    pImpl->parameterGroup = params;
}

ParameterGroup* JsonSerializer::getParameterGroup() const {
    return pImpl->parameterGroup;
}

} // namespace nap
