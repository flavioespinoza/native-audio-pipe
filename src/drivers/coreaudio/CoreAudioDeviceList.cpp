#include "CoreAudioDeviceList.h"
#include <mutex>
#include <algorithm>

namespace nap {
namespace drivers {

class CoreAudioDeviceList::Impl {
public:
    std::vector<DeviceInfo> devices;
    std::string defaultInputDeviceId;
    std::string defaultOutputDeviceId;
    std::string defaultSystemOutputDeviceId;

    DeviceChangeCallback deviceChangeCallback;
    DefaultDeviceChangeCallback defaultDeviceChangeCallback;

    std::mutex mutex;

    void populateDefaultDevices() {
        // Simulated device enumeration for macOS
        devices.clear();

        // Built-in output
        DeviceInfo builtInOutput;
        builtInOutput.id = "BuiltInSpeaker";
        builtInOutput.name = "Built-in Output";
        builtInOutput.inputChannels = 0;
        builtInOutput.outputChannels = 2;
        builtInOutput.defaultSampleRate = 48000.0;
        builtInOutput.isDefault = true;
        devices.push_back(builtInOutput);

        // Built-in input
        DeviceInfo builtInInput;
        builtInInput.id = "BuiltInMicrophone";
        builtInInput.name = "Built-in Microphone";
        builtInInput.inputChannels = 2;
        builtInInput.outputChannels = 0;
        builtInInput.defaultSampleRate = 48000.0;
        builtInInput.isDefault = true;
        devices.push_back(builtInInput);

        // External audio interface (simulated)
        DeviceInfo externalInterface;
        externalInterface.id = "ExternalAudioInterface";
        externalInterface.name = "USB Audio Interface";
        externalInterface.inputChannels = 8;
        externalInterface.outputChannels = 8;
        externalInterface.defaultSampleRate = 44100.0;
        externalInterface.isDefault = false;
        devices.push_back(externalInterface);

        defaultInputDeviceId = "BuiltInMicrophone";
        defaultOutputDeviceId = "BuiltInSpeaker";
        defaultSystemOutputDeviceId = "BuiltInSpeaker";
    }
};

CoreAudioDeviceList::CoreAudioDeviceList() : pImpl(std::make_unique<Impl>()) {
    pImpl->populateDefaultDevices();
}

CoreAudioDeviceList::~CoreAudioDeviceList() = default;

CoreAudioDeviceList::CoreAudioDeviceList(CoreAudioDeviceList&&) noexcept = default;
CoreAudioDeviceList& CoreAudioDeviceList::operator=(CoreAudioDeviceList&&) noexcept = default;

void CoreAudioDeviceList::refresh() {
    std::lock_guard<std::mutex> lock(pImpl->mutex);
    pImpl->populateDefaultDevices();
}

std::vector<DeviceInfo> CoreAudioDeviceList::getDevices() const {
    std::lock_guard<std::mutex> lock(pImpl->mutex);
    return pImpl->devices;
}

std::vector<DeviceInfo> CoreAudioDeviceList::getInputDevices() const {
    std::lock_guard<std::mutex> lock(pImpl->mutex);
    std::vector<DeviceInfo> result;
    for (const auto& device : pImpl->devices) {
        if (device.inputChannels > 0) {
            result.push_back(device);
        }
    }
    return result;
}

std::vector<DeviceInfo> CoreAudioDeviceList::getOutputDevices() const {
    std::lock_guard<std::mutex> lock(pImpl->mutex);
    std::vector<DeviceInfo> result;
    for (const auto& device : pImpl->devices) {
        if (device.outputChannels > 0) {
            result.push_back(device);
        }
    }
    return result;
}

const DeviceInfo* CoreAudioDeviceList::getDeviceById(const std::string& deviceId) const {
    std::lock_guard<std::mutex> lock(pImpl->mutex);
    for (const auto& device : pImpl->devices) {
        if (device.id == deviceId) {
            return &device;
        }
    }
    return nullptr;
}

const DeviceInfo* CoreAudioDeviceList::getDeviceByName(const std::string& name) const {
    std::lock_guard<std::mutex> lock(pImpl->mutex);
    for (const auto& device : pImpl->devices) {
        if (device.name == name) {
            return &device;
        }
    }
    return nullptr;
}

std::string CoreAudioDeviceList::getDefaultInputDeviceId() const {
    std::lock_guard<std::mutex> lock(pImpl->mutex);
    return pImpl->defaultInputDeviceId;
}

std::string CoreAudioDeviceList::getDefaultOutputDeviceId() const {
    std::lock_guard<std::mutex> lock(pImpl->mutex);
    return pImpl->defaultOutputDeviceId;
}

std::string CoreAudioDeviceList::getDefaultSystemOutputDeviceId() const {
    std::lock_guard<std::mutex> lock(pImpl->mutex);
    return pImpl->defaultSystemOutputDeviceId;
}

std::string CoreAudioDeviceList::getDeviceManufacturer(const std::string& deviceId) const {
    if (deviceId.find("BuiltIn") != std::string::npos) {
        return "Apple Inc.";
    }
    return "Unknown Manufacturer";
}

std::string CoreAudioDeviceList::getDeviceUID(const std::string& deviceId) const {
    return "UID_" + deviceId;
}

std::string CoreAudioDeviceList::getDeviceModelUID(const std::string& deviceId) const {
    return "ModelUID_" + deviceId;
}

bool CoreAudioDeviceList::deviceSupportsFormat(const std::string& deviceId,
                                                double sampleRate, size_t channels) const {
    auto rates = getDeviceSampleRates(deviceId);
    if (std::find(rates.begin(), rates.end(), sampleRate) == rates.end()) {
        return false;
    }

    const auto* device = getDeviceById(deviceId);
    if (!device) {
        return false;
    }

    return channels <= std::max(device->inputChannels, device->outputChannels);
}

std::vector<double> CoreAudioDeviceList::getDeviceSampleRates(const std::string& deviceId) const {
    return {44100.0, 48000.0, 88200.0, 96000.0, 176400.0, 192000.0};
}

std::vector<size_t> CoreAudioDeviceList::getDeviceBufferSizes(const std::string& deviceId) const {
    return {32, 64, 128, 256, 512, 1024, 2048};
}

CoreAudioDeviceList::TransportType
CoreAudioDeviceList::getDeviceTransportType(const std::string& deviceId) const {
    if (deviceId.find("BuiltIn") != std::string::npos) {
        return TransportType::BuiltIn;
    }
    if (deviceId.find("USB") != std::string::npos ||
        deviceId.find("External") != std::string::npos) {
        return TransportType::USB;
    }
    if (deviceId.find("Bluetooth") != std::string::npos) {
        return TransportType::Bluetooth;
    }
    if (deviceId.find("Aggregate") != std::string::npos) {
        return TransportType::Aggregate;
    }
    return TransportType::Unknown;
}

void CoreAudioDeviceList::setDeviceChangeCallback(DeviceChangeCallback callback) {
    pImpl->deviceChangeCallback = std::move(callback);
}

void CoreAudioDeviceList::setDefaultDeviceChangeCallback(DefaultDeviceChangeCallback callback) {
    pImpl->defaultDeviceChangeCallback = std::move(callback);
}

bool CoreAudioDeviceList::isAggregateDevice(const std::string& deviceId) const {
    return deviceId.find("aggregate") != std::string::npos ||
           deviceId.find("Aggregate") != std::string::npos;
}

std::vector<std::string>
CoreAudioDeviceList::getAggregateSubdevices(const std::string& deviceId) const {
    // Return empty vector for non-aggregate devices
    if (!isAggregateDevice(deviceId)) {
        return {};
    }
    return {"subdevice1", "subdevice2"};
}

} // namespace drivers
} // namespace nap
