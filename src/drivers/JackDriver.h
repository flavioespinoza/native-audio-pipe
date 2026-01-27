#ifndef NAP_JACK_DRIVER_H
#define NAP_JACK_DRIVER_H

#include "drivers/IAudioDriver.h"
#include <memory>

namespace nap {

/**
 * @brief JACK Audio Connection Kit driver
 *
 * Provides professional-grade audio I/O through JACK server.
 * Supports ultra-low latency and inter-application audio routing.
 * This is a stub implementation for cross-platform structure.
 */
class JackDriver : public IAudioDriver {
public:
    JackDriver();
    ~JackDriver() override;

    // Non-copyable, movable
    JackDriver(const JackDriver&) = delete;
    JackDriver& operator=(const JackDriver&) = delete;
    JackDriver(JackDriver&&) noexcept;
    JackDriver& operator=(JackDriver&&) noexcept;

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

    // JACK-specific
    void setClientName(const std::string& name);
    std::string getClientName() const;
    bool connectToPort(const std::string& sourcePort, const std::string& destPort);
    bool disconnectPort(const std::string& sourcePort, const std::string& destPort);
    std::vector<std::string> getAvailablePorts(bool isInput) const;

private:
    class Impl;
    std::unique_ptr<Impl> pImpl;
};

} // namespace nap

#endif // NAP_JACK_DRIVER_H
