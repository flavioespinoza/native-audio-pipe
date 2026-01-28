#include "NullAudioDriver.h"
#include <mutex>
#include <atomic>
#include <thread>
#include <chrono>
#include <cmath>
#include <random>
#include <numeric>

namespace nap {
namespace drivers {

class NullAudioDriver::Impl {
public:
    std::atomic<bool> running{false};
    std::atomic<bool> initialized{false};
    std::string currentDeviceId = "null:default";

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

    std::atomic<uint64_t> samplePosition{0};
    std::atomic<uint64_t> blocksProcessed{0};
    std::atomic<uint64_t> callbackCount{0};

    ProcessingMode processingMode = ProcessingMode::Realtime;

    std::thread processingThread;
    std::atomic<bool> threadRunning{false};

    // Statistics
    std::vector<double> processingTimes;
    std::mutex statsMutex;
    double maxProcessingTime = 0.0;

    // Input simulation
    enum class InputMode {
        Silence,
        Noise,
        SineWave,
        Custom
    };
    InputMode inputMode = InputMode::Silence;
    float noiseAmplitude = 0.5f;
    float sineFrequency = 440.0f;
    float sineAmplitude = 0.5f;
    double sinePhase = 0.0;

    std::mt19937 rng{std::random_device{}()};
    std::uniform_real_distribution<float> noiseDist{-1.0f, 1.0f};

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
        supportedSampleRates = {8000.0, 11025.0, 16000.0, 22050.0, 32000.0,
                                44100.0, 48000.0, 88200.0, 96000.0,
                                176400.0, 192000.0};
        supportedBufferSizes = {16, 32, 64, 128, 256, 512, 1024, 2048, 4096, 8192};
    }

    void generateInput() {
        switch (inputMode) {
            case InputMode::Silence:
                for (auto& buffer : inputBuffers) {
                    std::fill(buffer.begin(), buffer.end(), 0.0f);
                }
                break;

            case InputMode::Noise:
                for (auto& buffer : inputBuffers) {
                    for (float& sample : buffer) {
                        sample = noiseDist(rng) * noiseAmplitude;
                    }
                }
                break;

            case InputMode::SineWave:
                {
                    double phaseIncrement = 2.0 * M_PI * sineFrequency / sampleRate;
                    for (size_t i = 0; i < bufferSize; ++i) {
                        float sample = sineAmplitude * static_cast<float>(std::sin(sinePhase));
                        sinePhase += phaseIncrement;
                        if (sinePhase >= 2.0 * M_PI) {
                            sinePhase -= 2.0 * M_PI;
                        }
                        for (auto& buffer : inputBuffers) {
                            buffer[i] = sample;
                        }
                    }
                }
                break;

            case InputMode::Custom:
                // Custom data already set
                break;
        }
    }

    void processOneBlock() {
        auto start = std::chrono::high_resolution_clock::now();

        generateInput();

        {
            std::lock_guard<std::mutex> lock(callbackMutex);
            if (audioCallback) {
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

                callbackCount++;
            }
        }

        samplePosition += bufferSize;
        blocksProcessed++;

        auto end = std::chrono::high_resolution_clock::now();
        double timeNs = std::chrono::duration<double, std::nano>(end - start).count();

        {
            std::lock_guard<std::mutex> lock(statsMutex);
            processingTimes.push_back(timeNs);
            if (timeNs > maxProcessingTime) {
                maxProcessingTime = timeNs;
            }
        }
    }

    void realtimeThreadFunction() {
        double bufferDurationMs = (bufferSize / sampleRate) * 1000.0;

        while (threadRunning) {
            auto nextTime = std::chrono::steady_clock::now() +
                           std::chrono::microseconds(static_cast<int64_t>(bufferDurationMs * 1000));

            if (running) {
                processOneBlock();
            }

            std::this_thread::sleep_until(nextTime);
        }
    }

    void asFastAsPossibleThreadFunction() {
        while (threadRunning && running) {
            processOneBlock();
        }
    }
};

NullAudioDriver::NullAudioDriver() : pImpl(std::make_unique<Impl>()) {
    pImpl->initializeSupportedFormats();
    pImpl->allocateBuffers();
}

NullAudioDriver::~NullAudioDriver() {
    shutdown();
}

NullAudioDriver::NullAudioDriver(NullAudioDriver&&) noexcept = default;
NullAudioDriver& NullAudioDriver::operator=(NullAudioDriver&&) noexcept = default;

bool NullAudioDriver::initialize() {
    if (pImpl->initialized) {
        return true;
    }

    pImpl->initializeSupportedFormats();
    pImpl->allocateBuffers();
    pImpl->initialized = true;

    return true;
}

void NullAudioDriver::shutdown() {
    stop();
    pImpl->initialized = false;
}

bool NullAudioDriver::start() {
    if (!pImpl->initialized) {
        return false;
    }
    if (pImpl->running) {
        return true;
    }

    pImpl->samplePosition = 0;
    pImpl->blocksProcessed = 0;
    pImpl->callbackCount = 0;
    resetStatistics();

    pImpl->running = true;

    if (pImpl->processingMode != ProcessingMode::Manual) {
        pImpl->threadRunning = true;

        if (pImpl->processingMode == ProcessingMode::Realtime) {
            pImpl->processingThread = std::thread(
                &Impl::realtimeThreadFunction, pImpl.get());
        } else {
            pImpl->processingThread = std::thread(
                &Impl::asFastAsPossibleThreadFunction, pImpl.get());
        }
    }

    return true;
}

void NullAudioDriver::stop() {
    if (!pImpl->running) {
        return;
    }

    pImpl->running = false;
    pImpl->threadRunning = false;

    if (pImpl->processingThread.joinable()) {
        pImpl->processingThread.join();
    }
}

bool NullAudioDriver::isRunning() const {
    return pImpl->running;
}

std::vector<DeviceInfo> NullAudioDriver::enumerateDevices() {
    std::vector<DeviceInfo> devices;

    DeviceInfo nullDevice;
    nullDevice.id = "null:default";
    nullDevice.name = "Null Audio Device";
    nullDevice.inputChannels = pImpl->inputChannels;
    nullDevice.outputChannels = pImpl->outputChannels;
    nullDevice.defaultSampleRate = 44100.0;
    nullDevice.isDefault = true;
    devices.push_back(nullDevice);

    return devices;
}

bool NullAudioDriver::selectDevice(const std::string& deviceId) {
    pImpl->currentDeviceId = deviceId;
    return true;
}

std::string NullAudioDriver::getCurrentDeviceId() const {
    return pImpl->currentDeviceId;
}

bool NullAudioDriver::setSampleRate(double sampleRate) {
    pImpl->sampleRate = sampleRate;
    return true;
}

double NullAudioDriver::getSampleRate() const {
    return pImpl->sampleRate;
}

std::vector<double> NullAudioDriver::getSupportedSampleRates() const {
    return pImpl->supportedSampleRates;
}

bool NullAudioDriver::setBufferSize(size_t bufferSize) {
    pImpl->bufferSize = bufferSize;
    pImpl->allocateBuffers();
    return true;
}

size_t NullAudioDriver::getBufferSize() const {
    return pImpl->bufferSize;
}

std::vector<size_t> NullAudioDriver::getSupportedBufferSizes() const {
    return pImpl->supportedBufferSizes;
}

size_t NullAudioDriver::getInputChannelCount() const {
    return pImpl->inputChannels;
}

size_t NullAudioDriver::getOutputChannelCount() const {
    return pImpl->outputChannels;
}

void NullAudioDriver::setAudioCallback(AudioCallback callback) {
    std::lock_guard<std::mutex> lock(pImpl->callbackMutex);
    pImpl->audioCallback = std::move(callback);
}

double NullAudioDriver::getStreamLatency() const {
    return static_cast<double>(pImpl->bufferSize) / pImpl->sampleRate;
}

uint64_t NullAudioDriver::getStreamTime() const {
    return pImpl->samplePosition;
}

void NullAudioDriver::setVirtualDeviceChannels(size_t inputChannels, size_t outputChannels) {
    pImpl->inputChannels = inputChannels;
    pImpl->outputChannels = outputChannels;
    pImpl->allocateBuffers();
}

void NullAudioDriver::setProcessingMode(ProcessingMode mode) {
    if (pImpl->running) {
        return; // Cannot change mode while running
    }
    pImpl->processingMode = mode;
}

NullAudioDriver::ProcessingMode NullAudioDriver::getProcessingMode() const {
    return pImpl->processingMode;
}

void NullAudioDriver::processBlock() {
    if (pImpl->processingMode != ProcessingMode::Manual) {
        return;
    }
    pImpl->processOneBlock();
}

void NullAudioDriver::processBlocks(size_t count) {
    if (pImpl->processingMode != ProcessingMode::Manual) {
        return;
    }
    for (size_t i = 0; i < count; ++i) {
        pImpl->processOneBlock();
    }
}

void NullAudioDriver::setInputData(size_t channel, const float* data, size_t sampleCount) {
    if (channel >= pImpl->inputChannels) {
        return;
    }

    pImpl->inputMode = Impl::InputMode::Custom;
    size_t copyCount = std::min(sampleCount, pImpl->bufferSize);
    std::copy(data, data + copyCount, pImpl->inputBuffers[channel].begin());
}

void NullAudioDriver::setInputSilence() {
    pImpl->inputMode = Impl::InputMode::Silence;
}

void NullAudioDriver::setInputNoise(float amplitude) {
    pImpl->inputMode = Impl::InputMode::Noise;
    pImpl->noiseAmplitude = amplitude;
}

void NullAudioDriver::setInputSineWave(float frequency, float amplitude) {
    pImpl->inputMode = Impl::InputMode::SineWave;
    pImpl->sineFrequency = frequency;
    pImpl->sineAmplitude = amplitude;
}

const float* NullAudioDriver::getOutputData(size_t channel) const {
    if (channel >= pImpl->outputChannels) {
        return nullptr;
    }
    return pImpl->outputBuffers[channel].data();
}

float NullAudioDriver::getOutputPeak(size_t channel) const {
    if (channel >= pImpl->outputChannels) {
        return 0.0f;
    }

    float peak = 0.0f;
    for (float sample : pImpl->outputBuffers[channel]) {
        float absSample = std::abs(sample);
        if (absSample > peak) {
            peak = absSample;
        }
    }
    return peak;
}

float NullAudioDriver::getOutputRMS(size_t channel) const {
    if (channel >= pImpl->outputChannels) {
        return 0.0f;
    }

    double sumSquares = 0.0;
    for (float sample : pImpl->outputBuffers[channel]) {
        sumSquares += sample * sample;
    }
    return static_cast<float>(std::sqrt(sumSquares / pImpl->bufferSize));
}

uint64_t NullAudioDriver::getBlocksProcessed() const {
    return pImpl->blocksProcessed;
}

uint64_t NullAudioDriver::getSamplesProcessed() const {
    return pImpl->samplePosition;
}

double NullAudioDriver::getAverageProcessingTimeNs() const {
    std::lock_guard<std::mutex> lock(pImpl->statsMutex);
    if (pImpl->processingTimes.empty()) {
        return 0.0;
    }
    double sum = std::accumulate(pImpl->processingTimes.begin(),
                                  pImpl->processingTimes.end(), 0.0);
    return sum / pImpl->processingTimes.size();
}

double NullAudioDriver::getMaxProcessingTimeNs() const {
    std::lock_guard<std::mutex> lock(pImpl->statsMutex);
    return pImpl->maxProcessingTime;
}

void NullAudioDriver::resetStatistics() {
    std::lock_guard<std::mutex> lock(pImpl->statsMutex);
    pImpl->processingTimes.clear();
    pImpl->maxProcessingTime = 0.0;
}

void NullAudioDriver::simulateXrun() {
    // Simulated xrun - just pause processing briefly
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
}

void NullAudioDriver::simulateDeviceDisconnect() {
    stop();
}

void NullAudioDriver::simulateDeviceReconnect() {
    start();
}

uint64_t NullAudioDriver::getCallbackCount() const {
    return pImpl->callbackCount;
}

} // namespace drivers
} // namespace nap
