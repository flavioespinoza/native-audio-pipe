#ifndef NAP_ASIO_DRIVER_H
#define NAP_ASIO_DRIVER_H

#include "drivers/IAudioDriver.h"
#include <memory>

namespace nap {

/**
 * @brief ASIO driver for Windows professional audio
 *
 * Provides low-latency audio I/O through Steinberg ASIO protocol.
 * This is a stub implementation for cross-platform structure.
 */
class ASIODriver : public IAudioDriver {
public:
    ASIODriver();
    ~ASIODriver() override;

    // Non-copyable, movable
    ASIODriver(const ASIODriver&) = delete;
    ASIODriver& operator=(const ASIODriver&) = delete;
    ASIODriver(ASIODriver&&) noexcept;
    ASIODriver& operator=(ASIODriver&&) noexcept;

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

    // ASIO-specific
    bool openControlPanel();
    int getInputLatency() const;
    int getOutputLatency() const;
    bool canSampleRate(double sampleRate) const;

private:
    class Impl;
    std::unique_ptr<Impl> pImpl;
};

} // namespace nap

#endif // NAP_ASIO_DRIVER_H
