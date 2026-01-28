#include "CoreAudioDriver.h"
#include "CoreAudioDeviceList.h"
#include <mutex>
#include <atomic>
#include <thread>

namespace nap {
namespace drivers {

class CoreAudioDriver::Impl {
public:
    std::atomic<bool> running{false};
    std::atomic<bool> initialized{false};
    std::string currentDeviceId;

    double sampleRate = 44100.0;
    size_t bufferSize = 512;
    size_t inputChannels = 2;
    size_t outputChannels = 2;

    AudioCallback audioCallback;
    std::mutex callbackMutex;

    std::vector<std::vector<float>> inputBuffers;
    std::vector<std::vector<float>> outputBuffers;

    std::vector<double> supportedSampleRates;
    std::vector<size_t> supportedBufferSizes;

    uint64_t samplePosition = 0;
    double inputLatency = 0.0;
    double outputLatency = 0.0;

    std::unique_ptr<CoreAudioDeviceList> deviceList;
    std::function<void()> deviceChangeCallback;

    bool hogModeEnabled = false;
    bool isMutedState = false;
    float masterVolume = 1.0f;
    uint32_t safetyOffset = 0;

    bool hasAggregate = false;
    std::string aggregateDeviceId;

    std::thread audioThread;
    std::atomic<bool> threadRunning{false};

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

    void initializeSupportedFormats() {
        supportedSampleRates = {44100.0, 48000.0, 88200.0, 96000.0, 176400.0, 192000.0};
        supportedBufferSizes = {32, 64, 128, 256, 512, 1024, 2048};
    }

    void audioThreadFunction() {
        while (threadRunning) {
            // Simulated audio processing
            std::lock_guard<std::mutex> lock(callbackMutex);
            if (audioCallback && running) {
                std::vector<const float*> inputs(inputChannels);
                std::vector<float*> outputs(outputChannels);

                for (size_t ch = 0; ch < inputChannels; ++ch) {
                    inputs[ch] = inputBuffers[ch].data();
                }
                for (size_t ch = 0; ch < outputChannels; ++ch) {
                    outputs[ch] = outputBuffers[ch].data();
                }

                audioCallback(inputs.data(), outputs.data(),
                             bufferSize, inputChannels, outputChannels);

                samplePosition += bufferSize;
            }

            // Sleep for buffer duration
            double bufferMs = (bufferSize / sampleRate) * 1000.0;
            std::this_thread::sleep_for(std::chrono::microseconds(
                static_cast<int64_t>(bufferMs * 1000)));
        }
    }
};

CoreAudioDriver::CoreAudioDriver() : pImpl(std::make_unique<Impl>()) {
    pImpl->deviceList = std::make_unique<CoreAudioDeviceList>();
}

CoreAudioDriver::~CoreAudioDriver() {
    shutdown();
}

CoreAudioDriver::CoreAudioDriver(CoreAudioDriver&&) noexcept = default;
CoreAudioDriver& CoreAudioDriver::operator=(CoreAudioDriver&&) noexcept = default;

bool CoreAudioDriver::initialize() {
    if (pImpl->initialized) {
        return true;
    }

    pImpl->initializeSupportedFormats();
    pImpl->allocateBuffers();
    pImpl->deviceList->refresh();
    pImpl->initialized = true;

    return true;
}

void CoreAudioDriver::shutdown() {
    stop();

    if (pImpl->hasAggregate) {
        destroyAggregateDevice();
    }

    pImpl->initialized = false;
}

bool CoreAudioDriver::start() {
    if (!pImpl->initialized) {
        return false;
    }
    if (pImpl->running) {
        return true;
    }

    pImpl->samplePosition = 0;
    pImpl->threadRunning = true;
    pImpl->running = true;

    pImpl->audioThread = std::thread(&Impl::audioThreadFunction, pImpl.get());

    return true;
}

void CoreAudioDriver::stop() {
    if (!pImpl->running) {
        return;
    }

    pImpl->running = false;
    pImpl->threadRunning = false;

    if (pImpl->audioThread.joinable()) {
        pImpl->audioThread.join();
    }
}

bool CoreAudioDriver::isRunning() const {
    return pImpl->running;
}

std::vector<DeviceInfo> CoreAudioDriver::enumerateDevices() {
    if (pImpl->deviceList) {
        pImpl->deviceList->refresh();
        return pImpl->deviceList->getDevices();
    }
    return {};
}

bool CoreAudioDriver::selectDevice(const std::string& deviceId) {
    if (pImpl->running) {
        return false;
    }

    if (pImpl->deviceList) {
        auto* device = pImpl->deviceList->getDeviceById(deviceId);
        if (device) {
            pImpl->currentDeviceId = deviceId;
            pImpl->inputChannels = device->inputChannels;
            pImpl->outputChannels = device->outputChannels;
            pImpl->allocateBuffers();
            return true;
        }
    }

    return false;
}

std::string CoreAudioDriver::getCurrentDeviceId() const {
    return pImpl->currentDeviceId;
}

bool CoreAudioDriver::setSampleRate(double sampleRate) {
    auto& rates = pImpl->supportedSampleRates;
    if (std::find(rates.begin(), rates.end(), sampleRate) == rates.end()) {
        return false;
    }

    pImpl->sampleRate = sampleRate;
    return true;
}

double CoreAudioDriver::getSampleRate() const {
    return pImpl->sampleRate;
}

std::vector<double> CoreAudioDriver::getSupportedSampleRates() const {
    return pImpl->supportedSampleRates;
}

bool CoreAudioDriver::setBufferSize(size_t bufferSize) {
    auto& sizes = pImpl->supportedBufferSizes;
    if (std::find(sizes.begin(), sizes.end(), bufferSize) == sizes.end()) {
        return false;
    }

    pImpl->bufferSize = bufferSize;
    pImpl->allocateBuffers();
    return true;
}

size_t CoreAudioDriver::getBufferSize() const {
    return pImpl->bufferSize;
}

std::vector<size_t> CoreAudioDriver::getSupportedBufferSizes() const {
    return pImpl->supportedBufferSizes;
}

size_t CoreAudioDriver::getInputChannelCount() const {
    return pImpl->inputChannels;
}

size_t CoreAudioDriver::getOutputChannelCount() const {
    return pImpl->outputChannels;
}

void CoreAudioDriver::setAudioCallback(AudioCallback callback) {
    std::lock_guard<std::mutex> lock(pImpl->callbackMutex);
    pImpl->audioCallback = std::move(callback);
}

double CoreAudioDriver::getStreamLatency() const {
    return (pImpl->inputLatency + pImpl->outputLatency + pImpl->safetyOffset) /
           pImpl->sampleRate;
}

uint64_t CoreAudioDriver::getStreamTime() const {
    return pImpl->samplePosition;
}

bool CoreAudioDriver::createAggregateDevice(const std::vector<std::string>& deviceIds) {
    if (pImpl->hasAggregate) {
        destroyAggregateDevice();
    }

    // Simulated aggregate device creation
    pImpl->aggregateDeviceId = "aggregate_" + std::to_string(deviceIds.size());
    pImpl->hasAggregate = true;

    return true;
}

bool CoreAudioDriver::destroyAggregateDevice() {
    if (!pImpl->hasAggregate) {
        return false;
    }

    pImpl->aggregateDeviceId.clear();
    pImpl->hasAggregate = false;

    return true;
}

bool CoreAudioDriver::hasAggregateDevice() const {
    return pImpl->hasAggregate;
}

void CoreAudioDriver::setDeviceChangeCallback(std::function<void()> callback) {
    pImpl->deviceChangeCallback = std::move(callback);
}

bool CoreAudioDriver::enableHogMode() {
    pImpl->hogModeEnabled = true;
    return true;
}

void CoreAudioDriver::disableHogMode() {
    pImpl->hogModeEnabled = false;
}

bool CoreAudioDriver::isHogModeEnabled() const {
    return pImpl->hogModeEnabled;
}

float CoreAudioDriver::getMasterVolume() const {
    return pImpl->masterVolume;
}

bool CoreAudioDriver::setMasterVolume(float volume) {
    if (volume < 0.0f || volume > 1.0f) {
        return false;
    }
    pImpl->masterVolume = volume;
    return true;
}

bool CoreAudioDriver::isMuted() const {
    return pImpl->isMutedState;
}

bool CoreAudioDriver::setMuted(bool muted) {
    pImpl->isMutedState = muted;
    return true;
}

uint32_t CoreAudioDriver::getSafetyOffset() const {
    return pImpl->safetyOffset;
}

bool CoreAudioDriver::setSafetyOffset(uint32_t samples) {
    pImpl->safetyOffset = samples;
    return true;
}

} // namespace drivers
} // namespace nap
