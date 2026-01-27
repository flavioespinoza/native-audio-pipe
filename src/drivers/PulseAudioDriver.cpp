#include "drivers/PulseAudioDriver.h"

namespace nap {

class PulseAudioDriver::Impl {
public:
    DriverState state = DriverState::Uninitialized;
    std::string lastError;
    AudioStreamConfig config;
    AudioCallback callback;
};

PulseAudioDriver::PulseAudioDriver()
    : pImpl(std::make_unique<Impl>()) {}

PulseAudioDriver::~PulseAudioDriver() {
    shutdown();
}

PulseAudioDriver::PulseAudioDriver(PulseAudioDriver&&) noexcept = default;
PulseAudioDriver& PulseAudioDriver::operator=(PulseAudioDriver&&) noexcept = default;

bool PulseAudioDriver::initialize() {
#ifdef __linux__
    // TODO: Initialize PulseAudio connection
    // pa_threaded_mainloop_new()
    // pa_context_new()
    pImpl->state = DriverState::Initialized;
    return true;
#else
    pImpl->lastError = "PulseAudio is only available on Linux";
    pImpl->state = DriverState::Error;
    return false;
#endif
}

void PulseAudioDriver::shutdown() {
    if (pImpl->state == DriverState::Running) {
        stop();
    }
    // TODO: Cleanup PulseAudio resources
    pImpl->state = DriverState::Uninitialized;
}

bool PulseAudioDriver::start() {
    if (pImpl->state != DriverState::Initialized) {
        pImpl->lastError = "Driver not initialized";
        return false;
    }

    // TODO: Start PulseAudio stream
    // pa_stream_new()
    // pa_stream_connect_playback()
    pImpl->state = DriverState::Running;
    return true;
}

void PulseAudioDriver::stop() {
    if (pImpl->state == DriverState::Running) {
        // TODO: Stop PulseAudio stream
        // pa_stream_disconnect()
        pImpl->state = DriverState::Stopped;
    }
}

DriverState PulseAudioDriver::getState() const {
    return pImpl->state;
}

std::string PulseAudioDriver::getLastError() const {
    return pImpl->lastError;
}

std::vector<AudioDeviceInfo> PulseAudioDriver::getAvailableDevices() const {
    std::vector<AudioDeviceInfo> devices;

    // TODO: Enumerate PulseAudio sinks and sources
    // pa_context_get_sink_info_list()
    // pa_context_get_source_info_list()

    // Return stub device
    AudioDeviceInfo defaultDevice;
    defaultDevice.id = "pulse_default";
    defaultDevice.name = "PulseAudio Default Device";
    defaultDevice.maxInputChannels = 2;
    defaultDevice.maxOutputChannels = 2;
    defaultDevice.supportedSampleRates = {44100.0, 48000.0, 96000.0};
    defaultDevice.supportedBufferSizes = {128, 256, 512, 1024, 2048};
    defaultDevice.isDefault = true;
    devices.push_back(defaultDevice);

    return devices;
}

AudioDeviceInfo PulseAudioDriver::getDefaultDevice() const {
    auto devices = getAvailableDevices();
    for (const auto& device : devices) {
        if (device.isDefault) return device;
    }
    return AudioDeviceInfo{};
}

bool PulseAudioDriver::configure(const AudioStreamConfig& config) {
    if (pImpl->state == DriverState::Running) {
        pImpl->lastError = "Cannot configure while running";
        return false;
    }

    pImpl->config = config;
    return true;
}

AudioStreamConfig PulseAudioDriver::getCurrentConfig() const {
    return pImpl->config;
}

void PulseAudioDriver::setAudioCallback(AudioCallback callback) {
    pImpl->callback = std::move(callback);
}

std::string PulseAudioDriver::getDriverName() const {
    return "PulseAudio";
}

std::string PulseAudioDriver::getDriverVersion() const {
    return "1.0.0-stub";
}

bool PulseAudioDriver::isAvailable() const {
#ifdef __linux__
    // TODO: Check if PulseAudio daemon is running
    return true;
#else
    return false;
#endif
}

} // namespace nap
