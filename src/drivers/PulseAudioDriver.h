#ifndef NAP_PULSE_AUDIO_DRIVER_H
#define NAP_PULSE_AUDIO_DRIVER_H

#include "drivers/IAudioDriver.h"
#include <memory>

namespace nap {

/**
 * @brief PulseAudio driver for Linux systems
 *
 * Provides audio I/O through PulseAudio sound server.
 * This is a stub implementation for cross-platform structure.
 */
class PulseAudioDriver : public IAudioDriver {
public:
    PulseAudioDriver();
    ~PulseAudioDriver() override;

    // Non-copyable, movable
    PulseAudioDriver(const PulseAudioDriver&) = delete;
    PulseAudioDriver& operator=(const PulseAudioDriver&) = delete;
    PulseAudioDriver(PulseAudioDriver&&) noexcept;
    PulseAudioDriver& operator=(PulseAudioDriver&&) noexcept;

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

private:
    class Impl;
    std::unique_ptr<Impl> pImpl;
};

} // namespace nap

#endif // NAP_PULSE_AUDIO_DRIVER_H
