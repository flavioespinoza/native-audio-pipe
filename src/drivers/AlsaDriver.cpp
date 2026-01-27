#include "drivers/AlsaDriver.h"

namespace nap {

class AlsaDriver::Impl {
public:
    DriverState state = DriverState::Uninitialized;
    std::string lastError;
    AudioStreamConfig config;
    AudioCallback callback;

    // ALSA-specific settings
    int periods = 2;
    int periodSize = 512;
};

AlsaDriver::AlsaDriver()
    : pImpl(std::make_unique<Impl>()) {}

AlsaDriver::~AlsaDriver() {
    shutdown();
}

AlsaDriver::AlsaDriver(AlsaDriver&&) noexcept = default;
AlsaDriver& AlsaDriver::operator=(AlsaDriver&&) noexcept = default;

bool AlsaDriver::initialize() {
#ifdef __linux__
    // TODO: Initialize ALSA
    // snd_pcm_open()
    pImpl->state = DriverState::Initialized;
    return true;
#else
    pImpl->lastError = "ALSA is only available on Linux";
    pImpl->state = DriverState::Error;
    return false;
#endif
}

void AlsaDriver::shutdown() {
    if (pImpl->state == DriverState::Running) {
        stop();
    }
    // TODO: Cleanup ALSA resources
    // snd_pcm_close()
    pImpl->state = DriverState::Uninitialized;
}

bool AlsaDriver::start() {
    if (pImpl->state != DriverState::Initialized) {
        pImpl->lastError = "Driver not initialized";
        return false;
    }

    // TODO: Start ALSA stream
    // snd_pcm_prepare()
    // snd_pcm_start()
    pImpl->state = DriverState::Running;
    return true;
}

void AlsaDriver::stop() {
    if (pImpl->state == DriverState::Running) {
        // TODO: Stop ALSA stream
        // snd_pcm_drop()
        pImpl->state = DriverState::Stopped;
    }
}

DriverState AlsaDriver::getState() const {
    return pImpl->state;
}

std::string AlsaDriver::getLastError() const {
    return pImpl->lastError;
}

std::vector<AudioDeviceInfo> AlsaDriver::getAvailableDevices() const {
    std::vector<AudioDeviceInfo> devices;

    // TODO: Enumerate ALSA devices
    // snd_device_name_hint()

    // Return stub device
    AudioDeviceInfo defaultDevice;
    defaultDevice.id = "hw:0,0";
    defaultDevice.name = "ALSA Default Device";
    defaultDevice.maxInputChannels = 2;
    defaultDevice.maxOutputChannels = 2;
    defaultDevice.supportedSampleRates = {44100.0, 48000.0, 96000.0, 192000.0};
    defaultDevice.supportedBufferSizes = {64, 128, 256, 512, 1024, 2048, 4096};
    defaultDevice.isDefault = true;
    devices.push_back(defaultDevice);

    return devices;
}

AudioDeviceInfo AlsaDriver::getDefaultDevice() const {
    auto devices = getAvailableDevices();
    for (const auto& device : devices) {
        if (device.isDefault) return device;
    }
    return AudioDeviceInfo{};
}

bool AlsaDriver::configure(const AudioStreamConfig& config) {
    if (pImpl->state == DriverState::Running) {
        pImpl->lastError = "Cannot configure while running";
        return false;
    }

    // TODO: Set ALSA hardware parameters
    // snd_pcm_hw_params_set_rate()
    // snd_pcm_hw_params_set_channels()
    // snd_pcm_hw_params_set_period_size()

    pImpl->config = config;
    return true;
}

AudioStreamConfig AlsaDriver::getCurrentConfig() const {
    return pImpl->config;
}

void AlsaDriver::setAudioCallback(AudioCallback callback) {
    pImpl->callback = std::move(callback);
}

std::string AlsaDriver::getDriverName() const {
    return "ALSA";
}

std::string AlsaDriver::getDriverVersion() const {
    return "1.0.0-stub";
}

bool AlsaDriver::isAvailable() const {
#ifdef __linux__
    // TODO: Check if ALSA is available
    return true;
#else
    return false;
#endif
}

void AlsaDriver::setHardwareParams(int periods, int periodSize) {
    pImpl->periods = periods;
    pImpl->periodSize = periodSize;
}

} // namespace nap
