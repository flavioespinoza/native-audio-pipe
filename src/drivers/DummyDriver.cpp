#include "drivers/DummyDriver.h"
#include <vector>
#include <thread>
#include <chrono>

namespace nap {

class DummyDriver::Impl {
public:
    DriverState state = DriverState::Uninitialized;
    std::string lastError;
    AudioStreamConfig config;
    AudioCallback callback;

    bool simulateRealtime = false;
    uint64_t processedSamples = 0;
    uint64_t processedBlocks = 0;

    std::vector<float> inputBuffer;
    std::vector<float> outputBuffer;
};

DummyDriver::DummyDriver()
    : pImpl(std::make_unique<Impl>()) {}

DummyDriver::~DummyDriver() {
    shutdown();
}

DummyDriver::DummyDriver(DummyDriver&&) noexcept = default;
DummyDriver& DummyDriver::operator=(DummyDriver&&) noexcept = default;

bool DummyDriver::initialize() {
    pImpl->state = DriverState::Initialized;
    return true;
}

void DummyDriver::shutdown() {
    if (pImpl->state == DriverState::Running) {
        stop();
    }
    pImpl->state = DriverState::Uninitialized;
}

bool DummyDriver::start() {
    if (pImpl->state != DriverState::Initialized) {
        pImpl->lastError = "Driver not initialized";
        return false;
    }

    // Allocate buffers
    int totalInputSamples = pImpl->config.bufferSize * pImpl->config.inputChannels;
    int totalOutputSamples = pImpl->config.bufferSize * pImpl->config.outputChannels;

    pImpl->inputBuffer.resize(totalInputSamples, 0.0f);
    pImpl->outputBuffer.resize(totalOutputSamples, 0.0f);

    pImpl->state = DriverState::Running;
    return true;
}

void DummyDriver::stop() {
    if (pImpl->state == DriverState::Running) {
        pImpl->state = DriverState::Stopped;
    }
}

DriverState DummyDriver::getState() const {
    return pImpl->state;
}

std::string DummyDriver::getLastError() const {
    return pImpl->lastError;
}

std::vector<AudioDeviceInfo> DummyDriver::getAvailableDevices() const {
    std::vector<AudioDeviceInfo> devices;

    AudioDeviceInfo dummyDevice;
    dummyDevice.id = "dummy";
    dummyDevice.name = "Dummy Audio Device (Silent)";
    dummyDevice.maxInputChannels = 128;
    dummyDevice.maxOutputChannels = 128;
    dummyDevice.supportedSampleRates = {22050.0, 44100.0, 48000.0, 88200.0, 96000.0, 176400.0, 192000.0};
    dummyDevice.supportedBufferSizes = {16, 32, 64, 128, 256, 512, 1024, 2048, 4096, 8192};
    dummyDevice.isDefault = true;
    devices.push_back(dummyDevice);

    return devices;
}

AudioDeviceInfo DummyDriver::getDefaultDevice() const {
    return getAvailableDevices()[0];
}

bool DummyDriver::configure(const AudioStreamConfig& config) {
    if (pImpl->state == DriverState::Running) {
        pImpl->lastError = "Cannot configure while running";
        return false;
    }

    pImpl->config = config;
    return true;
}

AudioStreamConfig DummyDriver::getCurrentConfig() const {
    return pImpl->config;
}

void DummyDriver::setAudioCallback(AudioCallback callback) {
    pImpl->callback = std::move(callback);
}

std::string DummyDriver::getDriverName() const {
    return "Dummy";
}

std::string DummyDriver::getDriverVersion() const {
    return "1.0.0";
}

bool DummyDriver::isAvailable() const {
    return true;  // Always available
}

void DummyDriver::processBlock(int numFrames) {
    if (pImpl->state != DriverState::Running || !pImpl->callback) {
        return;
    }

    // Ensure buffers are sized correctly
    int inputSize = numFrames * pImpl->config.inputChannels;
    int outputSize = numFrames * pImpl->config.outputChannels;

    if (pImpl->inputBuffer.size() < static_cast<size_t>(inputSize)) {
        pImpl->inputBuffer.resize(inputSize, 0.0f);
    }
    if (pImpl->outputBuffer.size() < static_cast<size_t>(outputSize)) {
        pImpl->outputBuffer.resize(outputSize, 0.0f);
    }

    // Clear output buffer
    std::fill(pImpl->outputBuffer.begin(), pImpl->outputBuffer.begin() + outputSize, 0.0f);

    // Call the audio callback
    pImpl->callback(
        pImpl->inputBuffer.data(),
        pImpl->outputBuffer.data(),
        numFrames,
        pImpl->config.inputChannels,
        pImpl->config.outputChannels
    );

    // Update statistics
    pImpl->processedSamples += numFrames;
    pImpl->processedBlocks++;

    // Simulate real-time if enabled
    if (pImpl->simulateRealtime && pImpl->config.sampleRate > 0) {
        double blockDuration = static_cast<double>(numFrames) / pImpl->config.sampleRate;
        auto sleepDuration = std::chrono::duration<double>(blockDuration);
        std::this_thread::sleep_for(sleepDuration);
    }
}

void DummyDriver::processBlocks(int numBlocks) {
    for (int i = 0; i < numBlocks; ++i) {
        processBlock(pImpl->config.bufferSize);
    }
}

void DummyDriver::setSimulateRealtime(bool enable) {
    pImpl->simulateRealtime = enable;
}

bool DummyDriver::isSimulatingRealtime() const {
    return pImpl->simulateRealtime;
}

uint64_t DummyDriver::getProcessedSamples() const {
    return pImpl->processedSamples;
}

uint64_t DummyDriver::getProcessedBlocks() const {
    return pImpl->processedBlocks;
}

void DummyDriver::resetStatistics() {
    pImpl->processedSamples = 0;
    pImpl->processedBlocks = 0;
}

} // namespace nap
