#include "ASIODriver.h"
#include "ASIOBufferSwitch.h"
#include <mutex>
#include <atomic>
#include <cstring>

namespace nap {
namespace drivers {

class ASIODriver::Impl {
public:
    std::atomic<bool> running{false};
    std::atomic<bool> driverLoaded{false};
    std::string currentDeviceId;
    std::string driverName;
    std::string driverVersion;

    double sampleRate = 44100.0;
    size_t bufferSize = 512;
    size_t inputChannels = 2;
    size_t outputChannels = 2;

    AudioCallback audioCallback;
    std::mutex callbackMutex;

    // Double-buffering for ASIO
    std::vector<std::vector<float>> inputBuffers[2];
    std::vector<std::vector<float>> outputBuffers[2];
    size_t currentBufferIndex = 0;

    std::vector<double> supportedSampleRates;
    std::vector<size_t> supportedBufferSizes;
    std::vector<std::string> clockSources;
    std::string currentClockSource;

    uint64_t samplePosition = 0;
    double inputLatency = 0.0;
    double outputLatency = 0.0;

    std::unique_ptr<ASIOBufferSwitch> bufferSwitch;

    void allocateBuffers() {
        for (int i = 0; i < 2; ++i) {
            inputBuffers[i].resize(inputChannels);
            outputBuffers[i].resize(outputChannels);
            for (size_t ch = 0; ch < inputChannels; ++ch) {
                inputBuffers[i][ch].resize(bufferSize, 0.0f);
            }
            for (size_t ch = 0; ch < outputChannels; ++ch) {
                outputBuffers[i][ch].resize(bufferSize, 0.0f);
            }
        }
    }

    void initializeSupportedFormats() {
        supportedSampleRates = {44100.0, 48000.0, 88200.0, 96000.0, 176400.0, 192000.0};
        supportedBufferSizes = {32, 64, 128, 256, 512, 1024, 2048, 4096};
        clockSources = {"Internal", "S/PDIF", "ADAT", "Word Clock"};
        currentClockSource = "Internal";
    }
};

ASIODriver::ASIODriver() : pImpl(std::make_unique<Impl>()) {
    pImpl->initializeSupportedFormats();
    pImpl->allocateBuffers();
    pImpl->bufferSwitch = std::make_unique<ASIOBufferSwitch>();
}

ASIODriver::~ASIODriver() {
    if (pImpl->running) {
        stop();
    }
    if (pImpl->driverLoaded) {
        unloadDriver();
    }
}

ASIODriver::ASIODriver(ASIODriver&&) noexcept = default;
ASIODriver& ASIODriver::operator=(ASIODriver&&) noexcept = default;

bool ASIODriver::initialize() {
    pImpl->initializeSupportedFormats();
    return true;
}

void ASIODriver::shutdown() {
    stop();
    unloadDriver();
}

bool ASIODriver::start() {
    if (!pImpl->driverLoaded) {
        return false;
    }
    if (pImpl->running) {
        return true;
    }

    pImpl->samplePosition = 0;
    pImpl->running = true;

    if (pImpl->bufferSwitch) {
        pImpl->bufferSwitch->start();
    }

    return true;
}

void ASIODriver::stop() {
    if (!pImpl->running) {
        return;
    }

    if (pImpl->bufferSwitch) {
        pImpl->bufferSwitch->stop();
    }

    pImpl->running = false;
}

bool ASIODriver::isRunning() const {
    return pImpl->running;
}

std::vector<DeviceInfo> ASIODriver::enumerateDevices() {
    std::vector<DeviceInfo> devices;

    // Simulated ASIO device enumeration
    // In real implementation, would query ASIO registry
    DeviceInfo device;
    device.id = "asio_default";
    device.name = "ASIO Driver";
    device.inputChannels = 2;
    device.outputChannels = 2;
    device.defaultSampleRate = 44100.0;
    device.isDefault = true;
    devices.push_back(device);

    return devices;
}

bool ASIODriver::selectDevice(const std::string& deviceId) {
    if (pImpl->running) {
        return false;
    }

    pImpl->currentDeviceId = deviceId;
    return loadDriver(deviceId);
}

std::string ASIODriver::getCurrentDeviceId() const {
    return pImpl->currentDeviceId;
}

bool ASIODriver::setSampleRate(double sampleRate) {
    auto& rates = pImpl->supportedSampleRates;
    if (std::find(rates.begin(), rates.end(), sampleRate) == rates.end()) {
        return false;
    }

    pImpl->sampleRate = sampleRate;
    return true;
}

double ASIODriver::getSampleRate() const {
    return pImpl->sampleRate;
}

std::vector<double> ASIODriver::getSupportedSampleRates() const {
    return pImpl->supportedSampleRates;
}

bool ASIODriver::setBufferSize(size_t bufferSize) {
    auto& sizes = pImpl->supportedBufferSizes;
    if (std::find(sizes.begin(), sizes.end(), bufferSize) == sizes.end()) {
        return false;
    }

    pImpl->bufferSize = bufferSize;
    pImpl->allocateBuffers();
    return true;
}

size_t ASIODriver::getBufferSize() const {
    return pImpl->bufferSize;
}

std::vector<size_t> ASIODriver::getSupportedBufferSizes() const {
    return pImpl->supportedBufferSizes;
}

size_t ASIODriver::getInputChannelCount() const {
    return pImpl->inputChannels;
}

size_t ASIODriver::getOutputChannelCount() const {
    return pImpl->outputChannels;
}

void ASIODriver::setAudioCallback(AudioCallback callback) {
    std::lock_guard<std::mutex> lock(pImpl->callbackMutex);
    pImpl->audioCallback = std::move(callback);

    if (pImpl->bufferSwitch) {
        pImpl->bufferSwitch->setProcessCallback([this](size_t bufferIndex) {
            std::lock_guard<std::mutex> lock(pImpl->callbackMutex);
            if (pImpl->audioCallback) {
                // Prepare pointers
                std::vector<const float*> inputs(pImpl->inputChannels);
                std::vector<float*> outputs(pImpl->outputChannels);

                for (size_t ch = 0; ch < pImpl->inputChannels; ++ch) {
                    inputs[ch] = pImpl->inputBuffers[bufferIndex][ch].data();
                }
                for (size_t ch = 0; ch < pImpl->outputChannels; ++ch) {
                    outputs[ch] = pImpl->outputBuffers[bufferIndex][ch].data();
                }

                pImpl->audioCallback(inputs.data(), outputs.data(),
                                     pImpl->bufferSize,
                                     pImpl->inputChannels,
                                     pImpl->outputChannels);

                pImpl->samplePosition += pImpl->bufferSize;
            }
        });
    }
}

double ASIODriver::getStreamLatency() const {
    return (pImpl->inputLatency + pImpl->outputLatency) / pImpl->sampleRate;
}

uint64_t ASIODriver::getStreamTime() const {
    return pImpl->samplePosition;
}

bool ASIODriver::loadDriver(const std::string& driverName) {
    if (pImpl->driverLoaded) {
        unloadDriver();
    }

    // Simulated driver loading
    pImpl->driverName = driverName;
    pImpl->driverVersion = "1.0.0";
    pImpl->driverLoaded = true;

    // Query driver capabilities
    pImpl->inputLatency = pImpl->bufferSize;
    pImpl->outputLatency = pImpl->bufferSize;

    return true;
}

void ASIODriver::unloadDriver() {
    if (!pImpl->driverLoaded) {
        return;
    }

    stop();
    pImpl->driverLoaded = false;
    pImpl->driverName.clear();
    pImpl->driverVersion.clear();
}

bool ASIODriver::isDriverLoaded() const {
    return pImpl->driverLoaded;
}

std::string ASIODriver::getDriverVersion() const {
    return pImpl->driverVersion;
}

std::string ASIODriver::getDriverName() const {
    return pImpl->driverName;
}

float* ASIODriver::getInputBuffer(size_t channel, size_t bufferIndex) {
    if (channel >= pImpl->inputChannels || bufferIndex >= 2) {
        return nullptr;
    }
    return pImpl->inputBuffers[bufferIndex][channel].data();
}

float* ASIODriver::getOutputBuffer(size_t channel, size_t bufferIndex) {
    if (channel >= pImpl->outputChannels || bufferIndex >= 2) {
        return nullptr;
    }
    return pImpl->outputBuffers[bufferIndex][channel].data();
}

bool ASIODriver::openControlPanel() {
    // In real implementation, would call ASIOControlPanel()
    return pImpl->driverLoaded;
}

std::vector<std::string> ASIODriver::getClockSources() const {
    return pImpl->clockSources;
}

bool ASIODriver::setClockSource(const std::string& source) {
    auto& sources = pImpl->clockSources;
    if (std::find(sources.begin(), sources.end(), source) == sources.end()) {
        return false;
    }
    pImpl->currentClockSource = source;
    return true;
}

std::string ASIODriver::getCurrentClockSource() const {
    return pImpl->currentClockSource;
}

} // namespace drivers
} // namespace nap
