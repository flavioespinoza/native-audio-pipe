#include "drivers/ASIODriver.h"

namespace nap {

class ASIODriver::Impl {
public:
    DriverState state = DriverState::Uninitialized;
    std::string lastError;
    AudioStreamConfig config;
    AudioCallback callback;
    int inputLatency = 0;
    int outputLatency = 0;
};

ASIODriver::ASIODriver()
    : pImpl(std::make_unique<Impl>()) {}

ASIODriver::~ASIODriver() {
    shutdown();
}

ASIODriver::ASIODriver(ASIODriver&&) noexcept = default;
ASIODriver& ASIODriver::operator=(ASIODriver&&) noexcept = default;

bool ASIODriver::initialize() {
#ifdef _WIN32
    // TODO: Initialize ASIO
    // ASIOInit()
    pImpl->state = DriverState::Initialized;
    return true;
#else
    pImpl->lastError = "ASIO is only available on Windows";
    pImpl->state = DriverState::Error;
    return false;
#endif
}

void ASIODriver::shutdown() {
    if (pImpl->state == DriverState::Running) {
        stop();
    }
    // TODO: Cleanup ASIO resources
    // ASIOExit()
    pImpl->state = DriverState::Uninitialized;
}

bool ASIODriver::start() {
    if (pImpl->state != DriverState::Initialized) {
        pImpl->lastError = "Driver not initialized";
        return false;
    }

    // TODO: Start ASIO
    // ASIOStart()
    pImpl->state = DriverState::Running;
    return true;
}

void ASIODriver::stop() {
    if (pImpl->state == DriverState::Running) {
        // TODO: Stop ASIO
        // ASIOStop()
        pImpl->state = DriverState::Stopped;
    }
}

DriverState ASIODriver::getState() const {
    return pImpl->state;
}

std::string ASIODriver::getLastError() const {
    return pImpl->lastError;
}

std::vector<AudioDeviceInfo> ASIODriver::getAvailableDevices() const {
    std::vector<AudioDeviceInfo> devices;

    // TODO: Enumerate ASIO drivers
    // CoCreateInstance for each registered ASIO driver

    // Return stub device
    AudioDeviceInfo defaultDevice;
    defaultDevice.id = "asio_default";
    defaultDevice.name = "ASIO Device";
    defaultDevice.maxInputChannels = 8;
    defaultDevice.maxOutputChannels = 8;
    defaultDevice.supportedSampleRates = {44100.0, 48000.0, 88200.0, 96000.0, 176400.0, 192000.0};
    defaultDevice.supportedBufferSizes = {32, 64, 128, 256, 512, 1024, 2048};
    defaultDevice.isDefault = true;
    devices.push_back(defaultDevice);

    return devices;
}

AudioDeviceInfo ASIODriver::getDefaultDevice() const {
    auto devices = getAvailableDevices();
    for (const auto& device : devices) {
        if (device.isDefault) return device;
    }
    return AudioDeviceInfo{};
}

bool ASIODriver::configure(const AudioStreamConfig& config) {
    if (pImpl->state == DriverState::Running) {
        pImpl->lastError = "Cannot configure while running";
        return false;
    }

    // TODO: Create ASIO buffers
    // ASIOCreateBuffers()

    pImpl->config = config;
    return true;
}

AudioStreamConfig ASIODriver::getCurrentConfig() const {
    return pImpl->config;
}

void ASIODriver::setAudioCallback(AudioCallback callback) {
    pImpl->callback = std::move(callback);
}

std::string ASIODriver::getDriverName() const {
    return "ASIO";
}

std::string ASIODriver::getDriverVersion() const {
    return "1.0.0-stub";
}

bool ASIODriver::isAvailable() const {
#ifdef _WIN32
    // TODO: Check if any ASIO drivers are installed
    return true;
#else
    return false;
#endif
}

bool ASIODriver::openControlPanel() {
    // TODO: Open ASIO control panel
    // ASIOControlPanel()
    return true;
}

int ASIODriver::getInputLatency() const {
    // TODO: Query ASIO latencies
    // ASIOGetLatencies()
    return pImpl->inputLatency;
}

int ASIODriver::getOutputLatency() const {
    return pImpl->outputLatency;
}

bool ASIODriver::canSampleRate(double sampleRate) const {
    // TODO: Query ASIO sample rate support
    // ASIOCanSampleRate()
    (void)sampleRate;
    return true;
}

} // namespace nap
