#include "core/serialization/PresetManager.h"
#include "core/serialization/ISerializer.h"
#include <unordered_map>
#include <algorithm>
#include <filesystem>
#include <chrono>

namespace nap {

class PresetManager::Impl {
public:
    std::string presetDirectory;
    std::unordered_map<std::string, PresetInfo> presets;
    std::string currentPresetName;
    bool modified = false;
    std::string lastError;

    std::shared_ptr<ISerializer> serializer;
    ParameterGroup* parameterGroup = nullptr;

    PresetLoadedCallback loadedCallback;
    PresetSavedCallback savedCallback;

    void clearError() { lastError.clear(); }
    void setError(const std::string& error) { lastError = error; }

    std::string buildPresetPath(const std::string& name) const {
        std::string ext = serializer ? serializer->getFileExtension() : ".preset";
        return presetDirectory + "/" + name + ext;
    }

    uint64_t currentTimestamp() const {
        return std::chrono::duration_cast<std::chrono::seconds>(
            std::chrono::system_clock::now().time_since_epoch()).count();
    }
};

PresetManager::PresetManager()
    : pImpl(std::make_unique<Impl>()) {}

PresetManager::~PresetManager() = default;

PresetManager::PresetManager(PresetManager&&) noexcept = default;
PresetManager& PresetManager::operator=(PresetManager&&) noexcept = default;

void PresetManager::setPresetDirectory(const std::string& path) {
    pImpl->presetDirectory = path;
}

std::string PresetManager::getPresetDirectory() const {
    return pImpl->presetDirectory;
}

void PresetManager::scanPresets() {
    pImpl->presets.clear();

    if (pImpl->presetDirectory.empty()) return;

    try {
        std::filesystem::path dirPath(pImpl->presetDirectory);
        if (!std::filesystem::exists(dirPath)) return;

        for (const auto& entry : std::filesystem::directory_iterator(dirPath)) {
            if (entry.is_regular_file()) {
                PresetInfo info;
                info.filepath = entry.path().string();
                info.name = entry.path().stem().string();
                info.createdTime = 0;
                info.modifiedTime = 0;
                pImpl->presets[info.name] = info;
            }
        }
    } catch (...) {
        pImpl->setError("Failed to scan preset directory");
    }
}

bool PresetManager::savePreset(const std::string& name, const std::string& category) {
    pImpl->clearError();

    if (!pImpl->serializer) {
        pImpl->setError("No serializer bound");
        return false;
    }

    if (pImpl->presetDirectory.empty()) {
        pImpl->setError("Preset directory not set");
        return false;
    }

    try {
        std::filesystem::create_directories(pImpl->presetDirectory);
    } catch (...) {
        pImpl->setError("Failed to create preset directory");
        return false;
    }

    std::string filepath = pImpl->buildPresetPath(name);
    if (!pImpl->serializer->saveToFile(filepath)) {
        pImpl->setError(pImpl->serializer->getLastError());
        return false;
    }

    PresetInfo info;
    info.name = name;
    info.filepath = filepath;
    info.category = category;
    info.createdTime = pImpl->currentTimestamp();
    info.modifiedTime = info.createdTime;
    pImpl->presets[name] = info;

    pImpl->currentPresetName = name;
    pImpl->modified = false;

    if (pImpl->savedCallback) {
        pImpl->savedCallback(name);
    }

    return true;
}

bool PresetManager::loadPreset(const std::string& name) {
    pImpl->clearError();

    if (!pImpl->serializer) {
        pImpl->setError("No serializer bound");
        return false;
    }

    auto it = pImpl->presets.find(name);
    if (it == pImpl->presets.end()) {
        pImpl->setError("Preset not found: " + name);
        return false;
    }

    if (!pImpl->serializer->loadFromFile(it->second.filepath)) {
        pImpl->setError(pImpl->serializer->getLastError());
        return false;
    }

    pImpl->currentPresetName = name;
    pImpl->modified = false;

    if (pImpl->loadedCallback) {
        pImpl->loadedCallback(name);
    }

    return true;
}

bool PresetManager::deletePreset(const std::string& name) {
    pImpl->clearError();

    auto it = pImpl->presets.find(name);
    if (it == pImpl->presets.end()) {
        pImpl->setError("Preset not found: " + name);
        return false;
    }

    try {
        std::filesystem::remove(it->second.filepath);
    } catch (...) {
        pImpl->setError("Failed to delete preset file");
        return false;
    }

    pImpl->presets.erase(it);

    if (pImpl->currentPresetName == name) {
        pImpl->currentPresetName.clear();
    }

    return true;
}

bool PresetManager::renamePreset(const std::string& oldName, const std::string& newName) {
    pImpl->clearError();

    auto it = pImpl->presets.find(oldName);
    if (it == pImpl->presets.end()) {
        pImpl->setError("Preset not found: " + oldName);
        return false;
    }

    if (pImpl->presets.find(newName) != pImpl->presets.end()) {
        pImpl->setError("Preset already exists: " + newName);
        return false;
    }

    std::string newPath = pImpl->buildPresetPath(newName);
    try {
        std::filesystem::rename(it->second.filepath, newPath);
    } catch (...) {
        pImpl->setError("Failed to rename preset file");
        return false;
    }

    PresetInfo info = it->second;
    info.name = newName;
    info.filepath = newPath;
    info.modifiedTime = pImpl->currentTimestamp();

    pImpl->presets.erase(it);
    pImpl->presets[newName] = info;

    if (pImpl->currentPresetName == oldName) {
        pImpl->currentPresetName = newName;
    }

    return true;
}

bool PresetManager::duplicatePreset(const std::string& name, const std::string& newName) {
    pImpl->clearError();

    auto it = pImpl->presets.find(name);
    if (it == pImpl->presets.end()) {
        pImpl->setError("Preset not found: " + name);
        return false;
    }

    if (pImpl->presets.find(newName) != pImpl->presets.end()) {
        pImpl->setError("Preset already exists: " + newName);
        return false;
    }

    std::string newPath = pImpl->buildPresetPath(newName);
    try {
        std::filesystem::copy_file(it->second.filepath, newPath);
    } catch (...) {
        pImpl->setError("Failed to duplicate preset file");
        return false;
    }

    PresetInfo info = it->second;
    info.name = newName;
    info.filepath = newPath;
    info.createdTime = pImpl->currentTimestamp();
    info.modifiedTime = info.createdTime;
    pImpl->presets[newName] = info;

    return true;
}

std::vector<std::string> PresetManager::getPresetNames() const {
    std::vector<std::string> names;
    names.reserve(pImpl->presets.size());
    for (const auto& pair : pImpl->presets) {
        names.push_back(pair.first);
    }
    std::sort(names.begin(), names.end());
    return names;
}

std::vector<std::string> PresetManager::getCategories() const {
    std::vector<std::string> categories;
    for (const auto& pair : pImpl->presets) {
        if (!pair.second.category.empty()) {
            if (std::find(categories.begin(), categories.end(), pair.second.category) == categories.end()) {
                categories.push_back(pair.second.category);
            }
        }
    }
    std::sort(categories.begin(), categories.end());
    return categories;
}

std::vector<std::string> PresetManager::getPresetsInCategory(const std::string& category) const {
    std::vector<std::string> names;
    for (const auto& pair : pImpl->presets) {
        if (pair.second.category == category) {
            names.push_back(pair.first);
        }
    }
    std::sort(names.begin(), names.end());
    return names;
}

bool PresetManager::presetExists(const std::string& name) const {
    return pImpl->presets.find(name) != pImpl->presets.end();
}

PresetInfo PresetManager::getPresetInfo(const std::string& name) const {
    auto it = pImpl->presets.find(name);
    if (it != pImpl->presets.end()) {
        return it->second;
    }
    return PresetInfo{};
}

std::string PresetManager::getCurrentPresetName() const {
    return pImpl->currentPresetName;
}

bool PresetManager::hasUnsavedChanges() const {
    return pImpl->modified;
}

void PresetManager::markAsModified() {
    pImpl->modified = true;
}

void PresetManager::clearModifiedFlag() {
    pImpl->modified = false;
}

void PresetManager::setSerializer(std::shared_ptr<ISerializer> serializer) {
    pImpl->serializer = std::move(serializer);
}

std::shared_ptr<ISerializer> PresetManager::getSerializer() const {
    return pImpl->serializer;
}

void PresetManager::setParameterGroup(ParameterGroup* params) {
    pImpl->parameterGroup = params;
}

ParameterGroup* PresetManager::getParameterGroup() const {
    return pImpl->parameterGroup;
}

void PresetManager::setPresetLoadedCallback(PresetLoadedCallback callback) {
    pImpl->loadedCallback = std::move(callback);
}

void PresetManager::setPresetSavedCallback(PresetSavedCallback callback) {
    pImpl->savedCallback = std::move(callback);
}

std::string PresetManager::getLastError() const {
    return pImpl->lastError;
}

} // namespace nap
