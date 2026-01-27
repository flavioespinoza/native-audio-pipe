#ifndef NAP_PRESET_MANAGER_H
#define NAP_PRESET_MANAGER_H

#include <string>
#include <vector>
#include <memory>
#include <functional>

namespace nap {

// Forward declarations
class ISerializer;
class ParameterGroup;

/**
 * @brief Preset information structure
 */
struct PresetInfo {
    std::string name;
    std::string filepath;
    std::string category;
    std::string author;
    uint64_t createdTime;
    uint64_t modifiedTime;
};

/**
 * @brief Manager for storing and recalling named parameter states
 *
 * Handles preset organization, saving, loading, and categorization.
 */
class PresetManager {
public:
    using PresetLoadedCallback = std::function<void(const std::string& presetName)>;
    using PresetSavedCallback = std::function<void(const std::string& presetName)>;

    PresetManager();
    ~PresetManager();

    // Non-copyable, movable
    PresetManager(const PresetManager&) = delete;
    PresetManager& operator=(const PresetManager&) = delete;
    PresetManager(PresetManager&&) noexcept;
    PresetManager& operator=(PresetManager&&) noexcept;

    // Preset directory management
    void setPresetDirectory(const std::string& path);
    std::string getPresetDirectory() const;
    void scanPresets();

    // Preset operations
    bool savePreset(const std::string& name, const std::string& category = "");
    bool loadPreset(const std::string& name);
    bool deletePreset(const std::string& name);
    bool renamePreset(const std::string& oldName, const std::string& newName);
    bool duplicatePreset(const std::string& name, const std::string& newName);

    // Preset queries
    std::vector<std::string> getPresetNames() const;
    std::vector<std::string> getCategories() const;
    std::vector<std::string> getPresetsInCategory(const std::string& category) const;
    bool presetExists(const std::string& name) const;
    PresetInfo getPresetInfo(const std::string& name) const;

    // Current preset
    std::string getCurrentPresetName() const;
    bool hasUnsavedChanges() const;
    void markAsModified();
    void clearModifiedFlag();

    // Serializer binding
    void setSerializer(std::shared_ptr<ISerializer> serializer);
    std::shared_ptr<ISerializer> getSerializer() const;

    // Parameter binding
    void setParameterGroup(ParameterGroup* params);
    ParameterGroup* getParameterGroup() const;

    // Callbacks
    void setPresetLoadedCallback(PresetLoadedCallback callback);
    void setPresetSavedCallback(PresetSavedCallback callback);

    // Error handling
    std::string getLastError() const;

private:
    class Impl;
    std::unique_ptr<Impl> pImpl;
};

} // namespace nap

#endif // NAP_PRESET_MANAGER_H
