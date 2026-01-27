#ifndef NAP_DUMMY_DRIVER_H
#define NAP_DUMMY_DRIVER_H

#include "drivers/IAudioDriver.h"
#include <memory>

namespace nap {

/**
 * @brief Silent dummy driver for CI/CD testing
 *
 * Provides a no-op audio driver that simulates audio I/O
 * without requiring actual hardware. Useful for testing,
 * offline rendering, and headless operation.
 */
class DummyDriver : public IAudioDriver {
public:
    DummyDriver();
    ~DummyDriver() override;

    // Non-copyable, movable
    DummyDriver(const DummyDriver&) = delete;
    DummyDriver& operator=(const DummyDriver&) = delete;
    DummyDriver(DummyDriver&&) noexcept;
    DummyDriver& operator=(DummyDriver&&) noexcept;

    // IAudioDriver interface
    bool initialize() override;
    void shutdown() override;
    bool start() override;
    void stop() override;

    DriverState getState() const override;
    std::string getLastError() const override;

    std::vector<AudioDeviceInfo> getAvailableDevices() const override;
    AudioDeviceInfo getDefaultDevice() const override;

    bool configure(const AudioStreamConfig& config) override;
    AudioStreamConfig getCurrentConfig() const override;

    void setAudioCallback(AudioCallback callback) override;

    std::string getDriverName() const override;
    std::string getDriverVersion() const override;
    bool isAvailable() const override;

    // Dummy-specific: manual processing control
    void processBlock(int numFrames);
    void processBlocks(int numBlocks);

    // Timing simulation
    void setSimulateRealtime(bool enable);
    bool isSimulatingRealtime() const;

    // Statistics
    uint64_t getProcessedSamples() const;
    uint64_t getProcessedBlocks() const;
    void resetStatistics();

private:
    class Impl;
    std::unique_ptr<Impl> pImpl;
};

} // namespace nap

#endif // NAP_DUMMY_DRIVER_H
