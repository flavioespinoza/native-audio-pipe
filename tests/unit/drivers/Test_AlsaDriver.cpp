#include <gtest/gtest.h>
#include "drivers/AlsaDriver.h"

namespace nap {
namespace test {

class AlsaDriverTest : public ::testing::Test {
protected:
    void SetUp() override {
        driver = std::make_unique<AlsaDriver>();
    }

    void TearDown() override {
        driver->shutdown();
    }

    std::unique_ptr<AlsaDriver> driver;
};

TEST_F(AlsaDriverTest, Construction) {
    EXPECT_EQ(driver->getState(), DriverState::Uninitialized);
    EXPECT_EQ(driver->getDriverName(), "ALSA");
}

TEST_F(AlsaDriverTest, GetDriverVersion) {
    EXPECT_FALSE(driver->getDriverVersion().empty());
}

TEST_F(AlsaDriverTest, GetAvailableDevices) {
    auto devices = driver->getAvailableDevices();
    EXPECT_FALSE(devices.empty());

    // Check stub device properties
    auto& device = devices[0];
    EXPECT_FALSE(device.id.empty());
    EXPECT_FALSE(device.name.empty());
    EXPECT_GT(device.maxOutputChannels, 0);
}

TEST_F(AlsaDriverTest, GetDefaultDevice) {
    auto device = driver->getDefaultDevice();
    EXPECT_TRUE(device.isDefault);
}

TEST_F(AlsaDriverTest, Configure) {
    AudioStreamConfig config;
    config.sampleRate = 96000.0;
    config.bufferSize = 128;
    config.outputChannels = 8;

    EXPECT_TRUE(driver->configure(config));

    auto currentConfig = driver->getCurrentConfig();
    EXPECT_EQ(currentConfig.sampleRate, 96000.0);
    EXPECT_EQ(currentConfig.bufferSize, 128);
    EXPECT_EQ(currentConfig.outputChannels, 8);
}

TEST_F(AlsaDriverTest, SetHardwareParams) {
    driver->setHardwareParams(4, 256);
    // Just verify it doesn't crash
    SUCCEED();
}

#ifdef __linux__
TEST_F(AlsaDriverTest, InitializeOnLinux) {
    EXPECT_TRUE(driver->initialize());
    EXPECT_EQ(driver->getState(), DriverState::Initialized);
}

TEST_F(AlsaDriverTest, IsAvailableOnLinux) {
    EXPECT_TRUE(driver->isAvailable());
}

TEST_F(AlsaDriverTest, StartStopOnLinux) {
    driver->initialize();
    EXPECT_TRUE(driver->start());
    EXPECT_EQ(driver->getState(), DriverState::Running);

    driver->stop();
    EXPECT_EQ(driver->getState(), DriverState::Stopped);
}
#else
TEST_F(AlsaDriverTest, InitializeOnNonLinux) {
    EXPECT_FALSE(driver->initialize());
    EXPECT_EQ(driver->getState(), DriverState::Error);
}

TEST_F(AlsaDriverTest, IsAvailableOnNonLinux) {
    EXPECT_FALSE(driver->isAvailable());
}
#endif

TEST_F(AlsaDriverTest, ConfigureWhileRunningFails) {
#ifdef __linux__
    driver->initialize();
    driver->start();

    AudioStreamConfig config;
    EXPECT_FALSE(driver->configure(config));
    EXPECT_FALSE(driver->getLastError().empty());
#else
    GTEST_SKIP() << "ALSA not available on this platform";
#endif
}

} // namespace test
} // namespace nap
