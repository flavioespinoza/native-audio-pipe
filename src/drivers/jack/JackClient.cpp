#include "JackClient.h"
#include <mutex>
#include <atomic>
#include <map>

namespace nap {
namespace drivers {

class JackClient::Impl {
public:
    std::string clientName;
    std::string serverName;

    std::atomic<bool> connected{false};
    std::atomic<bool> active{false};

    double sampleRate = 48000.0;
    size_t bufferSize = 256;
    std::atomic<float> cpuLoad{0.0f};
    bool realtimeMode = true;

    std::map<std::string, std::vector<float>> audioBuffers;
    std::vector<std::string> ownPorts;
    std::map<std::string, std::vector<std::string>> portConnections;

    TransportState transportState = TransportState::Stopped;
    std::atomic<uint64_t> transportFrame{0};

    ProcessCallback processCallback;
    BufferSizeCallback bufferSizeCallback;
    SampleRateCallback sampleRateCallback;
    ShutdownCallback shutdownCallback;
    XrunCallback xrunCallback;
    PortRegistrationCallback portRegistrationCallback;
    PortConnectCallback portConnectCallback;

    std::mutex mutex;
};

JackClient::JackClient() : pImpl(std::make_unique<Impl>()) {}

JackClient::~JackClient() {
    disconnect();
}

JackClient::JackClient(JackClient&&) noexcept = default;
JackClient& JackClient::operator=(JackClient&&) noexcept = default;

bool JackClient::initialize(const std::string& clientName) {
    pImpl->clientName = clientName;
    return true;
}

bool JackClient::connect(const std::string& serverName) {
    if (pImpl->connected) {
        return true;
    }

    pImpl->serverName = serverName;
    pImpl->connected = true;

    // Simulated connection - would use jack_client_open in real implementation
    return true;
}

void JackClient::disconnect() {
    if (!pImpl->connected) {
        return;
    }

    deactivate();

    pImpl->ownPorts.clear();
    pImpl->portConnections.clear();
    pImpl->audioBuffers.clear();

    pImpl->connected = false;
}

bool JackClient::isConnected() const {
    return pImpl->connected;
}

bool JackClient::activate() {
    if (!pImpl->connected || pImpl->active) {
        return pImpl->active;
    }

    pImpl->active = true;
    return true;
}

void JackClient::deactivate() {
    pImpl->active = false;
}

bool JackClient::isActive() const {
    return pImpl->active;
}

double JackClient::getSampleRate() const {
    return pImpl->sampleRate;
}

size_t JackClient::getBufferSize() const {
    return pImpl->bufferSize;
}

bool JackClient::setBufferSize(size_t size) {
    if (pImpl->active) {
        return false;
    }

    pImpl->bufferSize = size;

    // Resize audio buffers
    for (auto& [portName, buffer] : pImpl->audioBuffers) {
        buffer.resize(size, 0.0f);
    }

    if (pImpl->bufferSizeCallback) {
        pImpl->bufferSizeCallback(size);
    }

    return true;
}

float JackClient::getCPULoad() const {
    return pImpl->cpuLoad;
}

bool JackClient::isRealtime() const {
    return pImpl->realtimeMode;
}

bool JackClient::createInputPort(const std::string& name) {
    std::lock_guard<std::mutex> lock(pImpl->mutex);

    std::string fullName = pImpl->clientName + ":" + name;
    pImpl->ownPorts.push_back(fullName);
    pImpl->audioBuffers[fullName].resize(pImpl->bufferSize, 0.0f);

    if (pImpl->portRegistrationCallback) {
        pImpl->portRegistrationCallback(fullName, true);
    }

    return true;
}

bool JackClient::createOutputPort(const std::string& name) {
    std::lock_guard<std::mutex> lock(pImpl->mutex);

    std::string fullName = pImpl->clientName + ":" + name;
    pImpl->ownPorts.push_back(fullName);
    pImpl->audioBuffers[fullName].resize(pImpl->bufferSize, 0.0f);

    if (pImpl->portRegistrationCallback) {
        pImpl->portRegistrationCallback(fullName, true);
    }

    return true;
}

bool JackClient::createMidiInputPort(const std::string& name) {
    return createInputPort(name + "_midi");
}

bool JackClient::createMidiOutputPort(const std::string& name) {
    return createOutputPort(name + "_midi");
}

bool JackClient::unregisterPort(const std::string& name) {
    std::lock_guard<std::mutex> lock(pImpl->mutex);

    auto it = std::find(pImpl->ownPorts.begin(), pImpl->ownPorts.end(), name);
    if (it == pImpl->ownPorts.end()) {
        return false;
    }

    pImpl->ownPorts.erase(it);
    pImpl->audioBuffers.erase(name);
    pImpl->portConnections.erase(name);

    if (pImpl->portRegistrationCallback) {
        pImpl->portRegistrationCallback(name, false);
    }

    return true;
}

std::vector<std::string> JackClient::getAvailablePorts(bool isInput) const {
    // Simulated available ports
    if (isInput) {
        return {"system:capture_1", "system:capture_2"};
    } else {
        return {"system:playback_1", "system:playback_2"};
    }
}

std::vector<std::string> JackClient::getOwnPorts() const {
    std::lock_guard<std::mutex> lock(pImpl->mutex);
    return pImpl->ownPorts;
}

std::vector<std::string> JackClient::getPortConnections(const std::string& portName) const {
    std::lock_guard<std::mutex> lock(pImpl->mutex);

    auto it = pImpl->portConnections.find(portName);
    if (it != pImpl->portConnections.end()) {
        return it->second;
    }
    return {};
}

bool JackClient::portExists(const std::string& portName) const {
    std::lock_guard<std::mutex> lock(pImpl->mutex);

    return std::find(pImpl->ownPorts.begin(), pImpl->ownPorts.end(), portName)
           != pImpl->ownPorts.end();
}

bool JackClient::connectPorts(const std::string& srcPort, const std::string& dstPort) {
    std::lock_guard<std::mutex> lock(pImpl->mutex);

    pImpl->portConnections[srcPort].push_back(dstPort);

    if (pImpl->portConnectCallback) {
        pImpl->portConnectCallback(srcPort, dstPort, true);
    }

    return true;
}

bool JackClient::disconnectPorts(const std::string& srcPort, const std::string& dstPort) {
    std::lock_guard<std::mutex> lock(pImpl->mutex);

    auto it = pImpl->portConnections.find(srcPort);
    if (it == pImpl->portConnections.end()) {
        return false;
    }

    auto& connections = it->second;
    auto connIt = std::find(connections.begin(), connections.end(), dstPort);
    if (connIt == connections.end()) {
        return false;
    }

    connections.erase(connIt);

    if (pImpl->portConnectCallback) {
        pImpl->portConnectCallback(srcPort, dstPort, false);
    }

    return true;
}

bool JackClient::disconnectAllPorts(const std::string& portName) {
    std::lock_guard<std::mutex> lock(pImpl->mutex);

    auto it = pImpl->portConnections.find(portName);
    if (it != pImpl->portConnections.end()) {
        for (const auto& dst : it->second) {
            if (pImpl->portConnectCallback) {
                pImpl->portConnectCallback(portName, dst, false);
            }
        }
        it->second.clear();
    }

    return true;
}

float* JackClient::getAudioBuffer(const std::string& portName, size_t nframes) {
    std::lock_guard<std::mutex> lock(pImpl->mutex);

    auto it = pImpl->audioBuffers.find(portName);
    if (it != pImpl->audioBuffers.end()) {
        return it->second.data();
    }
    return nullptr;
}

void* JackClient::getMidiBuffer(const std::string& portName, size_t nframes) {
    // Simulated MIDI buffer - would return jack_port_get_buffer in real implementation
    return nullptr;
}

size_t JackClient::getMidiEventCount(void* midiBuffer) {
    return 0;
}

bool JackClient::getMidiEvent(void* midiBuffer, size_t index,
                               uint8_t** data, size_t* size, uint32_t* time) {
    return false;
}

bool JackClient::writeMidiEvent(void* midiBuffer, uint32_t time,
                                 const uint8_t* data, size_t size) {
    return false;
}

void JackClient::clearMidiBuffer(void* midiBuffer) {
    // No-op in simulation
}

JackClient::TransportState JackClient::getTransportState() const {
    return pImpl->transportState;
}

void JackClient::transportStart() {
    pImpl->transportState = TransportState::Rolling;
}

void JackClient::transportStop() {
    pImpl->transportState = TransportState::Stopped;
}

void JackClient::transportLocate(uint64_t frame) {
    pImpl->transportFrame = frame;
}

uint64_t JackClient::getTransportFrame() const {
    return pImpl->transportFrame;
}

void JackClient::setProcessCallback(ProcessCallback callback) {
    pImpl->processCallback = std::move(callback);
}

void JackClient::setBufferSizeCallback(BufferSizeCallback callback) {
    pImpl->bufferSizeCallback = std::move(callback);
}

void JackClient::setSampleRateCallback(SampleRateCallback callback) {
    pImpl->sampleRateCallback = std::move(callback);
}

void JackClient::setShutdownCallback(ShutdownCallback callback) {
    pImpl->shutdownCallback = std::move(callback);
}

void JackClient::setXrunCallback(XrunCallback callback) {
    pImpl->xrunCallback = std::move(callback);
}

void JackClient::setPortRegistrationCallback(PortRegistrationCallback callback) {
    pImpl->portRegistrationCallback = std::move(callback);
}

void JackClient::setPortConnectCallback(PortConnectCallback callback) {
    pImpl->portConnectCallback = std::move(callback);
}

std::string JackClient::getClientName() const {
    return pImpl->clientName;
}

} // namespace drivers
} // namespace nap
