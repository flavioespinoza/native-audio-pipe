#include <gtest/gtest.h>
#include "drivers/DummyDriver.h"

namespace nap {
namespace test {

class DummyDriverTest : public ::testing::Test {
protected:
    void SetUp() override {
        driver = std::make_unique<DummyDriver>();
    }

    void TearDown() override {
        driver->shutdown();
    }

    std::unique_ptr<DummyDriver> driver;
};

TEST_F(DummyDriverTest, Construction) {
    EXPECT_EQ(driver->getState(), DriverState::Uninitialized);
    EXPECT_EQ(driver->getDriverName(), "Dummy");
}

TEST_F(DummyDriverTest, AlwaysAvailable) {
    EXPECT_TRUE(driver->isAvailable());
}

TEST_F(DummyDriverTest, Initialize) {
    EXPECT_TRUE(driver->initialize());
    EXPECT_EQ(driver->getState(), DriverState::Initialized);
}

TEST_F(DummyDriverTest, StartStop) {
    driver->initialize();

    EXPECT_TRUE(driver->start());
    EXPECT_EQ(driver->getState(), DriverState::Running);

    driver->stop();
    EXPECT_EQ(driver->getState(), DriverState::Stopped);
}

TEST_F(DummyDriverTest, Configure) {
    AudioStreamConfig config;
    config.sampleRate = 48000.0;
    config.bufferSize = 256;
    config.inputChannels = 2;
    config.outputChannels = 2;

    EXPECT_TRUE(driver->configure(config));

    auto currentConfig = driver->getCurrentConfig();
    EXPECT_EQ(currentConfig.sampleRate, 48000.0);
    EXPECT_EQ(currentConfig.bufferSize, 256);
}

TEST_F(DummyDriverTest, ProcessBlock) {
    AudioStreamConfig config;
    config.sampleRate = 44100.0;
    config.bufferSize = 512;
    config.outputChannels = 2;
    driver->configure(config);

    driver->initialize();
    driver->start();

    int callbackCount = 0;
    driver->setAudioCallback([&](const float*, float* output, int numFrames, int, int numOutCh) {
        callbackCount++;
        // Fill with test signal
        for (int i = 0; i < numFrames * numOutCh; ++i) {
            output[i] = 0.5f;
        }
    });

    driver->processBlock(512);
    EXPECT_EQ(callbackCount, 1);
    EXPECT_EQ(driver->getProcessedSamples(), 512u);
    EXPECT_EQ(driver->getProcessedBlocks(), 1u);
}

TEST_F(DummyDriverTest, ProcessMultipleBlocks) {
    AudioStreamConfig config;
    config.bufferSize = 256;
    config.outputChannels = 2;
    driver->configure(config);

    driver->initialize();
    driver->start();

    int callbackCount = 0;
    driver->setAudioCallback([&](const float*, float*, int, int, int) {
        callbackCount++;
    });

    driver->processBlocks(10);
    EXPECT_EQ(callbackCount, 10);
    EXPECT_EQ(driver->getProcessedBlocks(), 10u);
}

TEST_F(DummyDriverTest, Statistics) {
    driver->initialize();
    driver->start();
    driver->setAudioCallback([](const float*, float*, int, int, int) {});

    AudioStreamConfig config;
    config.bufferSize = 256;
    driver->configure(config);

    // Reinitialize after config
    driver->shutdown();
    driver->initialize();
    driver->start();

    driver->processBlock(256);
    driver->processBlock(256);

    EXPECT_EQ(driver->getProcessedSamples(), 512u);
    EXPECT_EQ(driver->getProcessedBlocks(), 2u);

    driver->resetStatistics();
    EXPECT_EQ(driver->getProcessedSamples(), 0u);
    EXPECT_EQ(driver->getProcessedBlocks(), 0u);
}

TEST_F(DummyDriverTest, SimulateRealtime) {
    driver->setSimulateRealtime(true);
    EXPECT_TRUE(driver->isSimulatingRealtime());

    driver->setSimulateRealtime(false);
    EXPECT_FALSE(driver->isSimulatingRealtime());
}

TEST_F(DummyDriverTest, GetAvailableDevices) {
    auto devices = driver->getAvailableDevices();
    EXPECT_EQ(devices.size(), 1u);

    auto& device = devices[0];
    EXPECT_EQ(device.id, "dummy");
    EXPECT_GT(device.maxInputChannels, 0);
    EXPECT_GT(device.maxOutputChannels, 0);
}

TEST_F(DummyDriverTest, WideRangeSupport) {
    auto devices = driver->getAvailableDevices();
    auto& device = devices[0];

    // Dummy should support many sample rates
    EXPECT_GE(device.supportedSampleRates.size(), 5u);

    // And many buffer sizes
    EXPECT_GE(device.supportedBufferSizes.size(), 5u);
}

TEST_F(DummyDriverTest, ConfigureWhileRunningFails) {
    driver->initialize();
    driver->start();

    AudioStreamConfig config;
    EXPECT_FALSE(driver->configure(config));
}

TEST_F(DummyDriverTest, ProcessWithoutCallback) {
    driver->initialize();
    driver->start();

    // Should not crash without callback
    driver->processBlock(256);
    SUCCEED();
}

} // namespace test
} // namespace nap
