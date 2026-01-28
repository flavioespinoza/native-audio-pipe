#ifndef NAP_COREAUDIO_DEVICE_LIST_H
#define NAP_COREAUDIO_DEVICE_LIST_H

#include "../IAudioDriver.h"
#include <memory>
#include <string>
#include <vector>
#include <functional>

namespace nap {
namespace drivers {

/**
 * @brief Core Audio device enumeration and monitoring
 *
 * Manages the list of available Core Audio devices and provides
 * notifications when devices are added, removed, or changed.
 */
class CoreAudioDeviceList {
public:
    using DeviceChangeCallback = std::function<void(const std::string& deviceId, bool added)>;
    using DefaultDeviceChangeCallback = std::function<void(bool isInput, const std::string& newDeviceId)>;

    CoreAudioDeviceList();
    ~CoreAudioDeviceList();

    // Prevent copying
    CoreAudioDeviceList(const CoreAudioDeviceList&) = delete;
    CoreAudioDeviceList& operator=(const CoreAudioDeviceList&) = delete;

    // Move semantics
    CoreAudioDeviceList(CoreAudioDeviceList&&) noexcept;
    CoreAudioDeviceList& operator=(CoreAudioDeviceList&&) noexcept;

    // Device enumeration
    void refresh();
    std::vector<DeviceInfo> getDevices() const;
    std::vector<DeviceInfo> getInputDevices() const;
    std::vector<DeviceInfo> getOutputDevices() const;

    // Device lookup
    const DeviceInfo* getDeviceById(const std::string& deviceId) const;
    const DeviceInfo* getDeviceByName(const std::string& name) const;

    // Default devices
    std::string getDefaultInputDeviceId() const;
    std::string getDefaultOutputDeviceId() const;
    std::string getDefaultSystemOutputDeviceId() const;

    // Device properties
    std::string getDeviceManufacturer(const std::string& deviceId) const;
    std::string getDeviceUID(const std::string& deviceId) const;
    std::string getDeviceModelUID(const std::string& deviceId) const;

    // Device capabilities
    bool deviceSupportsFormat(const std::string& deviceId,
                              double sampleRate, size_t channels) const;
    std::vector<double> getDeviceSampleRates(const std::string& deviceId) const;
    std::vector<size_t> getDeviceBufferSizes(const std::string& deviceId) const;

    // Transport type
    enum class TransportType {
        Unknown,
        BuiltIn,
        PCI,
        USB,
        FireWire,
        Bluetooth,
        HDMI,
        DisplayPort,
        AirPlay,
        Thunderbolt,
        Virtual,
        Aggregate
    };
    TransportType getDeviceTransportType(const std::string& deviceId) const;

    // Device monitoring
    void setDeviceChangeCallback(DeviceChangeCallback callback);
    void setDefaultDeviceChangeCallback(DefaultDeviceChangeCallback callback);

    // Aggregate device management
    bool isAggregateDevice(const std::string& deviceId) const;
    std::vector<std::string> getAggregateSubdevices(const std::string& deviceId) const;

private:
    class Impl;
    std::unique_ptr<Impl> pImpl;
};

} // namespace drivers
} // namespace nap

#endif // NAP_COREAUDIO_DEVICE_LIST_H
