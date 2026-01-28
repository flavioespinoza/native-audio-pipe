#include "AlsaDriver.h"
#include <mutex>
#include <atomic>
#include <thread>
#include <cstring>

namespace nap {
namespace drivers {

class AlsaDriver::Impl {
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

    AlsaDriver::AccessMode accessMode = AlsaDriver::AccessMode::RWInterleaved;
    AlsaDriver::SampleFormat sampleFormat = AlsaDriver::SampleFormat::S32_LE;

    size_t periodSize = 256;
    size_t numPeriods = 2;

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
        supportedSampleRates = {44100.0, 48000.0, 88200.0, 96000.0, 192000.0};
        supportedBufferSizes = {64, 128, 256, 512, 1024, 2048, 4096};
    }

    void audioThreadFunction() {
        while (threadRunning) {
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

            double bufferMs = (bufferSize / sampleRate) * 1000.0;
            std::this_thread::sleep_for(std::chrono::microseconds(
                static_cast<int64_t>(bufferMs * 1000)));
        }
    }
};

AlsaDriver::AlsaDriver() : pImpl(std::make_unique<Impl>()) {}

AlsaDriver::~AlsaDriver() {
    shutdown();
}

AlsaDriver::AlsaDriver(AlsaDriver&&) noexcept = default;
AlsaDriver& AlsaDriver::operator=(AlsaDriver&&) noexcept = default;

bool AlsaDriver::initialize() {
    if (pImpl->initialized) {
        return true;
    }

    pImpl->initializeSupportedFormats();
    pImpl->allocateBuffers();
    pImpl->initialized = true;

    return true;
}

void AlsaDriver::shutdown() {
    stop();
    closeDevice();
    pImpl->initialized = false;
}

bool AlsaDriver::start() {
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

void AlsaDriver::stop() {
    if (!pImpl->running) {
        return;
    }

    pImpl->running = false;
    pImpl->threadRunning = false;

    if (pImpl->audioThread.joinable()) {
        pImpl->audioThread.join();
    }
}

bool AlsaDriver::isRunning() const {
    return pImpl->running;
}

std::vector<DeviceInfo> AlsaDriver::enumerateDevices() {
    std::vector<DeviceInfo> devices;

    // Simulated ALSA device enumeration
    DeviceInfo defaultDevice;
    defaultDevice.id = "default";
    defaultDevice.name = "Default ALSA Device";
    defaultDevice.inputChannels = 2;
    defaultDevice.outputChannels = 2;
    defaultDevice.defaultSampleRate = 48000.0;
    defaultDevice.isDefault = true;
    devices.push_back(defaultDevice);

    DeviceInfo hw0;
    hw0.id = "hw:0,0";
    hw0.name = "HDA Intel PCH";
    hw0.inputChannels = 2;
    hw0.outputChannels = 2;
    hw0.defaultSampleRate = 48000.0;
    hw0.isDefault = false;
    devices.push_back(hw0);

    DeviceInfo plughw0;
    plughw0.id = "plughw:0,0";
    plughw0.name = "HDA Intel PCH (plug)";
    plughw0.inputChannels = 2;
    plughw0.outputChannels = 2;
    plughw0.defaultSampleRate = 48000.0;
    plughw0.isDefault = false;
    devices.push_back(plughw0);

    return devices;
}

bool AlsaDriver::selectDevice(const std::string& deviceId) {
    if (pImpl->running) {
        return false;
    }

    pImpl->currentDeviceId = deviceId;
    return openDevice(deviceId, true, true);
}

std::string AlsaDriver::getCurrentDeviceId() const {
    return pImpl->currentDeviceId;
}

bool AlsaDriver::setSampleRate(double sampleRate) {
    auto& rates = pImpl->supportedSampleRates;
    if (std::find(rates.begin(), rates.end(), sampleRate) == rates.end()) {
        return false;
    }

    pImpl->sampleRate = sampleRate;
    return true;
}

double AlsaDriver::getSampleRate() const {
    return pImpl->sampleRate;
}

std::vector<double> AlsaDriver::getSupportedSampleRates() const {
    return pImpl->supportedSampleRates;
}

bool AlsaDriver::setBufferSize(size_t bufferSize) {
    auto& sizes = pImpl->supportedBufferSizes;
    if (std::find(sizes.begin(), sizes.end(), bufferSize) == sizes.end()) {
        return false;
    }

    pImpl->bufferSize = bufferSize;
    pImpl->allocateBuffers();
    return true;
}

size_t AlsaDriver::getBufferSize() const {
    return pImpl->bufferSize;
}

std::vector<size_t> AlsaDriver::getSupportedBufferSizes() const {
    return pImpl->supportedBufferSizes;
}

size_t AlsaDriver::getInputChannelCount() const {
    return pImpl->inputChannels;
}

size_t AlsaDriver::getOutputChannelCount() const {
    return pImpl->outputChannels;
}

void AlsaDriver::setAudioCallback(AudioCallback callback) {
    std::lock_guard<std::mutex> lock(pImpl->callbackMutex);
    pImpl->audioCallback = std::move(callback);
}

double AlsaDriver::getStreamLatency() const {
    double periodLatency = static_cast<double>(pImpl->periodSize * pImpl->numPeriods) /
                           pImpl->sampleRate;
    return periodLatency;
}

uint64_t AlsaDriver::getStreamTime() const {
    return pImpl->samplePosition;
}

bool AlsaDriver::setAccessMode(AccessMode mode) {
    if (pImpl->running) {
        return false;
    }
    pImpl->accessMode = mode;
    return true;
}

AlsaDriver::AccessMode AlsaDriver::getAccessMode() const {
    return pImpl->accessMode;
}

bool AlsaDriver::setPeriodSize(size_t frames) {
    if (pImpl->running) {
        return false;
    }
    pImpl->periodSize = frames;
    return true;
}

size_t AlsaDriver::getPeriodSize() const {
    return pImpl->periodSize;
}

bool AlsaDriver::setNumPeriods(size_t periods) {
    if (pImpl->running || periods < 2) {
        return false;
    }
    pImpl->numPeriods = periods;
    return true;
}

size_t AlsaDriver::getNumPeriods() const {
    return pImpl->numPeriods;
}

bool AlsaDriver::setSampleFormat(SampleFormat format) {
    if (pImpl->running) {
        return false;
    }
    pImpl->sampleFormat = format;
    return true;
}

AlsaDriver::SampleFormat AlsaDriver::getSampleFormat() const {
    return pImpl->sampleFormat;
}

bool AlsaDriver::openDevice(const std::string& deviceName, bool capture, bool playback) {
    // Simulated device opening
    pImpl->currentDeviceId = deviceName;
    return true;
}

void AlsaDriver::closeDevice() {
    pImpl->currentDeviceId.clear();
}

float AlsaDriver::getMixerVolume(const std::string& element) const {
    return 1.0f;
}

bool AlsaDriver::setMixerVolume(const std::string& element, float volume) {
    return volume >= 0.0f && volume <= 1.0f;
}

bool AlsaDriver::getMixerMute(const std::string& element) const {
    return false;
}

bool AlsaDriver::setMixerMute(const std::string& element, bool muted) {
    return true;
}

bool AlsaDriver::recover(int error) {
    // Simulated ALSA error recovery
    return true;
}

bool AlsaDriver::drain() {
    return true;
}

bool AlsaDriver::drop() {
    return true;
}

} // namespace drivers
} // namespace nap
