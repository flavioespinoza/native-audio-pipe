#ifndef NAP_ALSA_DRIVER_H
#define NAP_ALSA_DRIVER_H

#include "drivers/IAudioDriver.h"
#include <memory>

namespace nap {

/**
 * @brief ALSA driver for Linux systems
 *
 * Provides low-level audio I/O through ALSA (Advanced Linux Sound Architecture).
 * This is a stub implementation for cross-platform structure.
 */
class AlsaDriver : public IAudioDriver {
public:
    AlsaDriver();
    ~AlsaDriver() override;

    // Non-copyable, movable
    AlsaDriver(const AlsaDriver&) = delete;
    AlsaDriver& operator=(const AlsaDriver&) = delete;
    AlsaDriver(AlsaDriver&&) noexcept;
    AlsaDriver& operator=(AlsaDriver&&) noexcept;

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

    // ALSA-specific
    void setHardwareParams(int periods, int periodSize);

private:
    class Impl;
    std::unique_ptr<Impl> pImpl;
};

} // namespace nap

#endif // NAP_ALSA_DRIVER_H
