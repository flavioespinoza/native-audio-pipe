#ifndef NAP_COREAUDIO_DRIVER_H
#define NAP_COREAUDIO_DRIVER_H

#include "../IAudioDriver.h"
#include <memory>
#include <string>
#include <vector>

namespace nap {
namespace drivers {

/**
 * @brief Core Audio driver implementation for macOS
 *
 * Provides native audio I/O using Apple's Core Audio framework.
 * Supports Audio Units, device aggregation, and low-latency operation.
 */
class CoreAudioDriver : public IAudioDriver {
public:
    CoreAudioDriver();
    ~CoreAudioDriver() override;

    // Prevent copying
    CoreAudioDriver(const CoreAudioDriver&) = delete;
    CoreAudioDriver& operator=(const CoreAudioDriver&) = delete;

    // Move semantics
    CoreAudioDriver(CoreAudioDriver&&) noexcept;
    CoreAudioDriver& operator=(CoreAudioDriver&&) noexcept;

    // IAudioDriver interface
    bool initialize() override;
    void shutdown() override;
    bool start() override;
    void stop() override;
    bool isRunning() const override;

    std::vector<DeviceInfo> enumerateDevices() override;
    bool selectDevice(const std::string& deviceId) override;
    std::string getCurrentDeviceId() const override;

    bool setSampleRate(double sampleRate) override;
    double getSampleRate() const override;
    std::vector<double> getSupportedSampleRates() const override;

    bool setBufferSize(size_t bufferSize) override;
    size_t getBufferSize() const override;
    std::vector<size_t> getSupportedBufferSizes() const override;

    size_t getInputChannelCount() const override;
    size_t getOutputChannelCount() const override;

    void setAudioCallback(AudioCallback callback) override;
    double getStreamLatency() const override;
    uint64_t getStreamTime() const override;

    // Core Audio specific methods
    bool createAggregateDevice(const std::vector<std::string>& deviceIds);
    bool destroyAggregateDevice();
    bool hasAggregateDevice() const;

    // Device monitoring
    void setDeviceChangeCallback(std::function<void()> callback);

    // Hog mode (exclusive access)
    bool enableHogMode();
    void disableHogMode();
    bool isHogModeEnabled() const;

    // Volume control
    float getMasterVolume() const;
    bool setMasterVolume(float volume);
    bool isMuted() const;
    bool setMuted(bool muted);

    // Safety offset
    uint32_t getSafetyOffset() const;
    bool setSafetyOffset(uint32_t samples);

private:
    class Impl;
    std::unique_ptr<Impl> pImpl;
};

} // namespace drivers
} // namespace nap

#endif // NAP_COREAUDIO_DRIVER_H
