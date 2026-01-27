#include <gtest/gtest.h>
#include "drivers/PulseAudioDriver.h"

namespace nap {
namespace test {

class PulseAudioDriverTest : public ::testing::Test {
protected:
    void SetUp() override {
        driver = std::make_unique<PulseAudioDriver>();
    }

    void TearDown() override {
        driver->shutdown();
    }

    std::unique_ptr<PulseAudioDriver> driver;
};

TEST_F(PulseAudioDriverTest, Construction) {
    EXPECT_EQ(driver->getState(), DriverState::Uninitialized);
    EXPECT_EQ(driver->getDriverName(), "PulseAudio");
}

TEST_F(PulseAudioDriverTest, GetDriverVersion) {
    EXPECT_FALSE(driver->getDriverVersion().empty());
}

TEST_F(PulseAudioDriverTest, GetAvailableDevices) {
    auto devices = driver->getAvailableDevices();
    // Should return at least a stub device
    EXPECT_FALSE(devices.empty());
}

TEST_F(PulseAudioDriverTest, GetDefaultDevice) {
    auto device = driver->getDefaultDevice();
    // Stub should have a default device
    EXPECT_FALSE(device.name.empty());
}

TEST_F(PulseAudioDriverTest, Configure) {
    AudioStreamConfig config;
    config.sampleRate = 48000.0;
    config.bufferSize = 256;
    config.outputChannels = 2;

    EXPECT_TRUE(driver->configure(config));

    auto currentConfig = driver->getCurrentConfig();
    EXPECT_EQ(currentConfig.sampleRate, 48000.0);
    EXPECT_EQ(currentConfig.bufferSize, 256);
}

TEST_F(PulseAudioDriverTest, SetAudioCallback) {
    bool callbackSet = false;
    driver->setAudioCallback([&](const float*, float*, int, int, int) {
        callbackSet = true;
    });
    // Just verify it doesn't crash
    SUCCEED();
}

#ifdef __linux__
TEST_F(PulseAudioDriverTest, InitializeOnLinux) {
    EXPECT_TRUE(driver->initialize());
    EXPECT_EQ(driver->getState(), DriverState::Initialized);
}

TEST_F(PulseAudioDriverTest, IsAvailableOnLinux) {
    EXPECT_TRUE(driver->isAvailable());
}
#else
TEST_F(PulseAudioDriverTest, InitializeOnNonLinux) {
    EXPECT_FALSE(driver->initialize());
    EXPECT_EQ(driver->getState(), DriverState::Error);
}

TEST_F(PulseAudioDriverTest, IsAvailableOnNonLinux) {
    EXPECT_FALSE(driver->isAvailable());
}
#endif

TEST_F(PulseAudioDriverTest, StartWithoutInitialize) {
    EXPECT_FALSE(driver->start());
}

TEST_F(PulseAudioDriverTest, Shutdown) {
    driver->shutdown();
    EXPECT_EQ(driver->getState(), DriverState::Uninitialized);
}

} // namespace test
} // namespace nap
