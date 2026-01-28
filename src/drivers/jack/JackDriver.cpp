#include "JackDriver.h"
#include "JackClient.h"
#include <mutex>
#include <atomic>

namespace nap {
namespace drivers {

class JackDriver::Impl {
public:
    std::atomic<bool> running{false};
    std::atomic<bool> connectedToServer{false};
    std::string clientName = "NAP";
    std::string serverName;

    double sampleRate = 48000.0;
    size_t bufferSize = 256;
    size_t inputChannels = 2;
    size_t outputChannels = 2;

    AudioCallback audioCallback;
    std::mutex callbackMutex;

    std::vector<std::vector<float>> inputBuffers;
    std::vector<std::vector<float>> outputBuffers;

    std::vector<std::string> inputPortNames;
    std::vector<std::string> outputPortNames;
    std::vector<std::string> availableInputPorts;
    std::vector<std::string> availableOutputPorts;

    uint64_t framePosition = 0;
    TransportState transportState = TransportState::Stopped;
    double transportBPM = 120.0;
    bool isTimebaseMasterFlag = false;
    bool freewheelMode = false;

    ShutdownCallback shutdownCallback;
    XrunCallback xrunCallback;
    PortConnectCallback portConnectCallback;

    std::unique_ptr<JackClient> jackClient;

    void allocateBuffers() {
        inputBuffers.resize(inputChannels);
        outputBuffers.resize(outputChannels);
        for (size_t ch = 0; ch < inputChannels; ++ch) {
            inputBuffers[ch].resize(bufferSize, 0.0f);
        }
        for (size_t ch = 0; ch < outputChannels; ++ch) {
            outputBuffers[ch].resize(bufferSize, 0.0f);
        }
    }
};

JackDriver::JackDriver() : pImpl(std::make_unique<Impl>()) {
    pImpl->allocateBuffers();
}

JackDriver::~JackDriver() {
    shutdown();
}

JackDriver::JackDriver(JackDriver&&) noexcept = default;
JackDriver& JackDriver::operator=(JackDriver&&) noexcept = default;

bool JackDriver::initialize() {
    pImpl->jackClient = std::make_unique<JackClient>();
    return pImpl->jackClient->initialize(pImpl->clientName);
}

void JackDriver::shutdown() {
    stop();
    disconnectFromServer();
    pImpl->jackClient.reset();
}

bool JackDriver::start() {
    if (!pImpl->connectedToServer) {
        return false;
    }
    if (pImpl->running) {
        return true;
    }

    pImpl->framePosition = 0;
    pImpl->running = true;

    if (pImpl->jackClient) {
        pImpl->jackClient->activate();
    }

    return true;
}

void JackDriver::stop() {
    if (!pImpl->running) {
        return;
    }

    if (pImpl->jackClient) {
        pImpl->jackClient->deactivate();
    }

    pImpl->running = false;
}

bool JackDriver::isRunning() const {
    return pImpl->running;
}

std::vector<DeviceInfo> JackDriver::enumerateDevices() {
    std::vector<DeviceInfo> devices;

    // JACK exposes a single "device" - the JACK server itself
    DeviceInfo jackServer;
    jackServer.id = "jack:" + pImpl->serverName;
    jackServer.name = "JACK Audio Server";
    jackServer.inputChannels = pImpl->inputChannels;
    jackServer.outputChannels = pImpl->outputChannels;
    jackServer.defaultSampleRate = pImpl->sampleRate;
    jackServer.isDefault = true;
    devices.push_back(jackServer);

    return devices;
}

bool JackDriver::selectDevice(const std::string& deviceId) {
    // For JACK, selecting a device means connecting to a server
    return connectToServer();
}

std::string JackDriver::getCurrentDeviceId() const {
    return "jack:" + pImpl->serverName;
}

bool JackDriver::setSampleRate(double sampleRate) {
    // JACK sample rate is determined by the server
    // This is a no-op; query the actual rate from JACK
    return true;
}

double JackDriver::getSampleRate() const {
    if (pImpl->jackClient) {
        return pImpl->jackClient->getSampleRate();
    }
    return pImpl->sampleRate;
}

std::vector<double> JackDriver::getSupportedSampleRates() const {
    // JACK only supports the server's sample rate
    return {pImpl->sampleRate};
}

bool JackDriver::setBufferSize(size_t bufferSize) {
    // JACK buffer size is determined by the server
    if (pImpl->jackClient) {
        return pImpl->jackClient->setBufferSize(bufferSize);
    }
    return false;
}

size_t JackDriver::getBufferSize() const {
    if (pImpl->jackClient) {
        return pImpl->jackClient->getBufferSize();
    }
    return pImpl->bufferSize;
}

std::vector<size_t> JackDriver::getSupportedBufferSizes() const {
    return {32, 64, 128, 256, 512, 1024, 2048};
}

size_t JackDriver::getInputChannelCount() const {
    return pImpl->inputChannels;
}

size_t JackDriver::getOutputChannelCount() const {
    return pImpl->outputChannels;
}

void JackDriver::setAudioCallback(AudioCallback callback) {
    std::lock_guard<std::mutex> lock(pImpl->callbackMutex);
    pImpl->audioCallback = std::move(callback);

    if (pImpl->jackClient) {
        pImpl->jackClient->setProcessCallback(
            [this](size_t nframes) {
                std::lock_guard<std::mutex> lock(pImpl->callbackMutex);
                if (pImpl->audioCallback) {
                    std::vector<const float*> inputs(pImpl->inputChannels);
                    std::vector<float*> outputs(pImpl->outputChannels);

                    for (size_t ch = 0; ch < pImpl->inputChannels; ++ch) {
                        inputs[ch] = pImpl->inputBuffers[ch].data();
                    }
                    for (size_t ch = 0; ch < pImpl->outputChannels; ++ch) {
                        outputs[ch] = pImpl->outputBuffers[ch].data();
                    }

                    pImpl->audioCallback(inputs.data(), outputs.data(),
                                         nframes, pImpl->inputChannels,
                                         pImpl->outputChannels);

                    pImpl->framePosition += nframes;
                }
            });
    }
}

double JackDriver::getStreamLatency() const {
    return static_cast<double>(pImpl->bufferSize) / pImpl->sampleRate;
}

uint64_t JackDriver::getStreamTime() const {
    return pImpl->framePosition;
}

bool JackDriver::connectToServer(const std::string& serverName) {
    if (pImpl->connectedToServer) {
        return true;
    }

    pImpl->serverName = serverName;

    if (pImpl->jackClient) {
        if (pImpl->jackClient->connect(serverName)) {
            pImpl->sampleRate = pImpl->jackClient->getSampleRate();
            pImpl->bufferSize = pImpl->jackClient->getBufferSize();
            pImpl->connectedToServer = true;
            pImpl->allocateBuffers();
            return true;
        }
    }

    return false;
}

void JackDriver::disconnectFromServer() {
    if (!pImpl->connectedToServer) {
        return;
    }

    if (pImpl->jackClient) {
        pImpl->jackClient->disconnect();
    }

    pImpl->connectedToServer = false;
}

bool JackDriver::isConnectedToServer() const {
    return pImpl->connectedToServer;
}

bool JackDriver::createInputPorts(size_t count, const std::string& baseName) {
    pImpl->inputPortNames.clear();
    pImpl->inputChannels = count;

    for (size_t i = 0; i < count; ++i) {
        std::string portName = baseName + "_" + std::to_string(i + 1);
        pImpl->inputPortNames.push_back(pImpl->clientName + ":" + portName);
    }

    pImpl->allocateBuffers();
    return true;
}

bool JackDriver::createOutputPorts(size_t count, const std::string& baseName) {
    pImpl->outputPortNames.clear();
    pImpl->outputChannels = count;

    for (size_t i = 0; i < count; ++i) {
        std::string portName = baseName + "_" + std::to_string(i + 1);
        pImpl->outputPortNames.push_back(pImpl->clientName + ":" + portName);
    }

    pImpl->allocateBuffers();
    return true;
}

std::vector<std::string> JackDriver::getInputPortNames() const {
    return pImpl->inputPortNames;
}

std::vector<std::string> JackDriver::getOutputPortNames() const {
    return pImpl->outputPortNames;
}

std::vector<std::string> JackDriver::getAvailablePorts(bool isInput) const {
    if (pImpl->jackClient) {
        return pImpl->jackClient->getAvailablePorts(isInput);
    }
    return isInput ? pImpl->availableInputPorts : pImpl->availableOutputPorts;
}

bool JackDriver::connectPorts(const std::string& srcPort, const std::string& dstPort) {
    if (pImpl->jackClient) {
        return pImpl->jackClient->connectPorts(srcPort, dstPort);
    }
    return false;
}

bool JackDriver::disconnectPorts(const std::string& srcPort, const std::string& dstPort) {
    if (pImpl->jackClient) {
        return pImpl->jackClient->disconnectPorts(srcPort, dstPort);
    }
    return false;
}

std::vector<std::string> JackDriver::getPortConnections(const std::string& portName) const {
    if (pImpl->jackClient) {
        return pImpl->jackClient->getPortConnections(portName);
    }
    return {};
}

JackDriver::TransportState JackDriver::getTransportState() const {
    return pImpl->transportState;
}

bool JackDriver::setTransportState(TransportState state) {
    pImpl->transportState = state;
    return true;
}

uint64_t JackDriver::getTransportFrame() const {
    return pImpl->framePosition;
}

bool JackDriver::setTransportFrame(uint64_t frame) {
    pImpl->framePosition = frame;
    return true;
}

double JackDriver::getTransportBPM() const {
    return pImpl->transportBPM;
}

bool JackDriver::becomeTimebaseMaster(bool conditional) {
    pImpl->isTimebaseMasterFlag = true;
    return true;
}

void JackDriver::releaseTimebase() {
    pImpl->isTimebaseMasterFlag = false;
}

bool JackDriver::isTimebaseMaster() const {
    return pImpl->isTimebaseMasterFlag;
}

bool JackDriver::setFreewheelMode(bool enabled) {
    pImpl->freewheelMode = enabled;
    return true;
}

bool JackDriver::isInFreewheelMode() const {
    return pImpl->freewheelMode;
}

std::string JackDriver::getClientName() const {
    return pImpl->clientName;
}

float JackDriver::getCPULoad() const {
    if (pImpl->jackClient) {
        return pImpl->jackClient->getCPULoad();
    }
    return 0.0f;
}

bool JackDriver::isRealtime() const {
    if (pImpl->jackClient) {
        return pImpl->jackClient->isRealtime();
    }
    return true;
}

void JackDriver::setShutdownCallback(ShutdownCallback callback) {
    pImpl->shutdownCallback = std::move(callback);
}

void JackDriver::setXrunCallback(XrunCallback callback) {
    pImpl->xrunCallback = std::move(callback);
}

void JackDriver::setPortConnectCallback(PortConnectCallback callback) {
    pImpl->portConnectCallback = std::move(callback);
}

} // namespace drivers
} // namespace nap
