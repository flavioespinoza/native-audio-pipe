#include <gtest/gtest.h>
#include "drivers/JackDriver.h"

namespace nap {
namespace test {

class JackDriverTest : public ::testing::Test {
protected:
    void SetUp() override {
        driver = std::make_unique<JackDriver>();
    }

    void TearDown() override {
        driver->shutdown();
    }

    std::unique_ptr<JackDriver> driver;
};

TEST_F(JackDriverTest, Construction) {
    EXPECT_EQ(driver->getState(), DriverState::Uninitialized);
    EXPECT_EQ(driver->getDriverName(), "JACK");
}

TEST_F(JackDriverTest, DefaultClientName) {
    EXPECT_EQ(driver->getClientName(), "nap_client");
}

TEST_F(JackDriverTest, SetClientName) {
    driver->setClientName("my_audio_app");
    EXPECT_EQ(driver->getClientName(), "my_audio_app");
}

TEST_F(JackDriverTest, GetAvailableDevices) {
    auto devices = driver->getAvailableDevices();
    EXPECT_EQ(devices.size(), 1u);  // JACK is a single "device"

    auto& device = devices[0];
    EXPECT_EQ(device.id, "jack_server");
    EXPECT_GT(device.maxInputChannels, 0);
    EXPECT_GT(device.maxOutputChannels, 0);
}

TEST_F(JackDriverTest, Initialize) {
    EXPECT_TRUE(driver->initialize());
    EXPECT_EQ(driver->getState(), DriverState::Initialized);
}

TEST_F(JackDriverTest, StartStop) {
    driver->initialize();

    EXPECT_TRUE(driver->start());
    EXPECT_EQ(driver->getState(), DriverState::Running);

    driver->stop();
    EXPECT_EQ(driver->getState(), DriverState::Stopped);
}

TEST_F(JackDriverTest, StartWithoutInitialize) {
    EXPECT_FALSE(driver->start());
    EXPECT_FALSE(driver->getLastError().empty());
}

TEST_F(JackDriverTest, Configure) {
    AudioStreamConfig config;
    config.sampleRate = 48000.0;
    config.bufferSize = 64;
    config.inputChannels = 2;
    config.outputChannels = 2;

    EXPECT_TRUE(driver->configure(config));
}

TEST_F(JackDriverTest, GetAvailablePorts) {
    auto inputPorts = driver->getAvailablePorts(true);
    auto outputPorts = driver->getAvailablePorts(false);

    EXPECT_FALSE(inputPorts.empty());
    EXPECT_FALSE(outputPorts.empty());
}

TEST_F(JackDriverTest, ConnectPorts) {
    EXPECT_TRUE(driver->connectToPort("system:capture_1", "nap_client:input_1"));
}

TEST_F(JackDriverTest, DisconnectPorts) {
    EXPECT_TRUE(driver->disconnectPort("system:capture_1", "nap_client:input_1"));
}

TEST_F(JackDriverTest, IsAvailable) {
    // JACK driver always reports available in stub mode
    EXPECT_TRUE(driver->isAvailable());
}

TEST_F(JackDriverTest, Shutdown) {
    driver->initialize();
    driver->start();
    driver->shutdown();

    EXPECT_EQ(driver->getState(), DriverState::Uninitialized);
}

} // namespace test
} // namespace nap
