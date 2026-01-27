#include <gtest/gtest.h>
#include "core/serialization/PresetManager.h"
#include "core/serialization/JsonSerializer.h"

namespace nap {
namespace test {

class PresetManagerTest : public ::testing::Test {
protected:
    void SetUp() override {
        manager = std::make_unique<PresetManager>();
    }

    std::unique_ptr<PresetManager> manager;
};

TEST_F(PresetManagerTest, Construction) {
    EXPECT_TRUE(manager->getPresetDirectory().empty());
    EXPECT_TRUE(manager->getCurrentPresetName().empty());
    EXPECT_FALSE(manager->hasUnsavedChanges());
}

TEST_F(PresetManagerTest, SetPresetDirectory) {
    manager->setPresetDirectory("/tmp/presets");
    EXPECT_EQ(manager->getPresetDirectory(), "/tmp/presets");
}

TEST_F(PresetManagerTest, SetSerializer) {
    auto serializer = std::make_shared<JsonSerializer>();
    manager->setSerializer(serializer);
    EXPECT_EQ(manager->getSerializer(), serializer);
}

TEST_F(PresetManagerTest, ModifiedFlag) {
    EXPECT_FALSE(manager->hasUnsavedChanges());

    manager->markAsModified();
    EXPECT_TRUE(manager->hasUnsavedChanges());

    manager->clearModifiedFlag();
    EXPECT_FALSE(manager->hasUnsavedChanges());
}

TEST_F(PresetManagerTest, SaveWithoutSerializer) {
    manager->setPresetDirectory("/tmp/presets");
    EXPECT_FALSE(manager->savePreset("test"));
    EXPECT_FALSE(manager->getLastError().empty());
}

TEST_F(PresetManagerTest, SaveWithoutDirectory) {
    auto serializer = std::make_shared<JsonSerializer>();
    manager->setSerializer(serializer);
    EXPECT_FALSE(manager->savePreset("test"));
}

TEST_F(PresetManagerTest, LoadNonexistent) {
    auto serializer = std::make_shared<JsonSerializer>();
    manager->setSerializer(serializer);
    EXPECT_FALSE(manager->loadPreset("nonexistent"));
}

TEST_F(PresetManagerTest, PresetExistsEmpty) {
    EXPECT_FALSE(manager->presetExists("any"));
}

TEST_F(PresetManagerTest, GetPresetNamesEmpty) {
    auto names = manager->getPresetNames();
    EXPECT_TRUE(names.empty());
}

TEST_F(PresetManagerTest, GetCategoriesEmpty) {
    auto categories = manager->getCategories();
    EXPECT_TRUE(categories.empty());
}

TEST_F(PresetManagerTest, Callbacks) {
    bool loadedCalled = false;
    bool savedCalled = false;

    manager->setPresetLoadedCallback([&](const std::string&) {
        loadedCalled = true;
    });

    manager->setPresetSavedCallback([&](const std::string&) {
        savedCalled = true;
    });

    // Callbacks won't be called without proper setup, but verify they can be set
    EXPECT_FALSE(loadedCalled);
    EXPECT_FALSE(savedCalled);
}

TEST_F(PresetManagerTest, GetPresetInfo) {
    auto info = manager->getPresetInfo("nonexistent");
    EXPECT_TRUE(info.name.empty());
}

} // namespace test
} // namespace nap
