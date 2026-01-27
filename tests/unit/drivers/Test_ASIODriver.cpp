#include <gtest/gtest.h>
#include "drivers/ASIODriver.h"

namespace nap {
namespace test {

class ASIODriverTest : public ::testing::Test {
protected:
    void SetUp() override {
        driver = std::make_unique<ASIODriver>();
    }

    void TearDown() override {
        driver->shutdown();
    }

    std::unique_ptr<ASIODriver> driver;
};

TEST_F(ASIODriverTest, Construction) {
    EXPECT_EQ(driver->getState(), DriverState::Uninitialized);
    EXPECT_EQ(driver->getDriverName(), "ASIO");
}

TEST_F(ASIODriverTest, GetDriverVersion) {
    EXPECT_FALSE(driver->getDriverVersion().empty());
}

TEST_F(ASIODriverTest, GetAvailableDevices) {
    auto devices = driver->getAvailableDevices();
    EXPECT_FALSE(devices.empty());

    auto& device = devices[0];
    EXPECT_FALSE(device.supportedSampleRates.empty());
    EXPECT_FALSE(device.supportedBufferSizes.empty());
}

TEST_F(ASIODriverTest, GetDefaultDevice) {
    auto device = driver->getDefaultDevice();
    EXPECT_TRUE(device.isDefault);
}

TEST_F(ASIODriverTest, Configure) {
    AudioStreamConfig config;
    config.sampleRate = 96000.0;
    config.bufferSize = 64;
    config.inputChannels = 8;
    config.outputChannels = 8;

    EXPECT_TRUE(driver->configure(config));

    auto currentConfig = driver->getCurrentConfig();
    EXPECT_EQ(currentConfig.sampleRate, 96000.0);
}

TEST_F(ASIODriverTest, CanSampleRate) {
    EXPECT_TRUE(driver->canSampleRate(44100.0));
    EXPECT_TRUE(driver->canSampleRate(48000.0));
    EXPECT_TRUE(driver->canSampleRate(96000.0));
}

TEST_F(ASIODriverTest, OpenControlPanel) {
    EXPECT_TRUE(driver->openControlPanel());
}

TEST_F(ASIODriverTest, GetLatencies) {
    EXPECT_GE(driver->getInputLatency(), 0);
    EXPECT_GE(driver->getOutputLatency(), 0);
}

#ifdef _WIN32
TEST_F(ASIODriverTest, InitializeOnWindows) {
    EXPECT_TRUE(driver->initialize());
    EXPECT_EQ(driver->getState(), DriverState::Initialized);
}

TEST_F(ASIODriverTest, IsAvailableOnWindows) {
    EXPECT_TRUE(driver->isAvailable());
}

TEST_F(ASIODriverTest, StartStopOnWindows) {
    driver->initialize();
    EXPECT_TRUE(driver->start());
    EXPECT_EQ(driver->getState(), DriverState::Running);

    driver->stop();
    EXPECT_EQ(driver->getState(), DriverState::Stopped);
}
#else
TEST_F(ASIODriverTest, InitializeOnNonWindows) {
    EXPECT_FALSE(driver->initialize());
    EXPECT_EQ(driver->getState(), DriverState::Error);
}

TEST_F(ASIODriverTest, IsAvailableOnNonWindows) {
    EXPECT_FALSE(driver->isAvailable());
}
#endif

TEST_F(ASIODriverTest, StartWithoutInitialize) {
    EXPECT_FALSE(driver->start());
}

TEST_F(ASIODriverTest, Shutdown) {
    driver->shutdown();
    EXPECT_EQ(driver->getState(), DriverState::Uninitialized);
}

} // namespace test
} // namespace nap
