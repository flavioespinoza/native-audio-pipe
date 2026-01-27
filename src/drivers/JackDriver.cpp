#include "drivers/JackDriver.h"

namespace nap {

class JackDriver::Impl {
public:
    DriverState state = DriverState::Uninitialized;
    std::string lastError;
    AudioStreamConfig config;
    AudioCallback callback;
    std::string clientName = "nap_client";
};

JackDriver::JackDriver()
    : pImpl(std::make_unique<Impl>()) {}

JackDriver::~JackDriver() {
    shutdown();
}

JackDriver::JackDriver(JackDriver&&) noexcept = default;
JackDriver& JackDriver::operator=(JackDriver&&) noexcept = default;

bool JackDriver::initialize() {
    // TODO: Connect to JACK server
    // jack_client_open()
    pImpl->state = DriverState::Initialized;
    return true;
}

void JackDriver::shutdown() {
    if (pImpl->state == DriverState::Running) {
        stop();
    }
    // TODO: Cleanup JACK resources
    // jack_client_close()
    pImpl->state = DriverState::Uninitialized;
}

bool JackDriver::start() {
    if (pImpl->state != DriverState::Initialized) {
        pImpl->lastError = "Driver not initialized";
        return false;
    }

    // TODO: Activate JACK client
    // jack_activate()
    pImpl->state = DriverState::Running;
    return true;
}

void JackDriver::stop() {
    if (pImpl->state == DriverState::Running) {
        // TODO: Deactivate JACK client
        // jack_deactivate()
        pImpl->state = DriverState::Stopped;
    }
}

DriverState JackDriver::getState() const {
    return pImpl->state;
}

std::string JackDriver::getLastError() const {
    return pImpl->lastError;
}

std::vector<AudioDeviceInfo> JackDriver::getAvailableDevices() const {
    std::vector<AudioDeviceInfo> devices;

    // JACK presents itself as a single "device"
    AudioDeviceInfo jackDevice;
    jackDevice.id = "jack_server";
    jackDevice.name = "JACK Audio Server";
    jackDevice.maxInputChannels = 64;  // JACK supports many channels
    jackDevice.maxOutputChannels = 64;
    // JACK uses server-defined sample rate
    jackDevice.supportedSampleRates = {44100.0, 48000.0, 96000.0};
    // JACK uses server-defined buffer size
    jackDevice.supportedBufferSizes = {32, 64, 128, 256, 512, 1024};
    jackDevice.isDefault = true;
    devices.push_back(jackDevice);

    return devices;
}

AudioDeviceInfo JackDriver::getDefaultDevice() const {
    auto devices = getAvailableDevices();
    if (!devices.empty()) return devices[0];
    return AudioDeviceInfo{};
}

bool JackDriver::configure(const AudioStreamConfig& config) {
    // JACK configuration is controlled by the JACK server
    // We can only request ports, not change sample rate or buffer size
    pImpl->config = config;
    return true;
}

AudioStreamConfig JackDriver::getCurrentConfig() const {
    // TODO: Query actual JACK server settings
    // jack_get_sample_rate()
    // jack_get_buffer_size()
    return pImpl->config;
}

void JackDriver::setAudioCallback(AudioCallback callback) {
    pImpl->callback = std::move(callback);
}

std::string JackDriver::getDriverName() const {
    return "JACK";
}

std::string JackDriver::getDriverVersion() const {
    return "1.0.0-stub";
}

bool JackDriver::isAvailable() const {
    // TODO: Check if JACK server is running
    // jack_client_open() with JackNoStartServer
    return true;
}

void JackDriver::setClientName(const std::string& name) {
    pImpl->clientName = name;
}

std::string JackDriver::getClientName() const {
    return pImpl->clientName;
}

bool JackDriver::connectToPort(const std::string& sourcePort, const std::string& destPort) {
    // TODO: Connect JACK ports
    // jack_connect()
    (void)sourcePort;
    (void)destPort;
    return true;
}

bool JackDriver::disconnectPort(const std::string& sourcePort, const std::string& destPort) {
    // TODO: Disconnect JACK ports
    // jack_disconnect()
    (void)sourcePort;
    (void)destPort;
    return true;
}

std::vector<std::string> JackDriver::getAvailablePorts(bool isInput) const {
    std::vector<std::string> ports;

    // TODO: Get available JACK ports
    // jack_get_ports()
    if (isInput) {
        ports.push_back("system:capture_1");
        ports.push_back("system:capture_2");
    } else {
        ports.push_back("system:playback_1");
        ports.push_back("system:playback_2");
    }

    return ports;
}

} // namespace nap
