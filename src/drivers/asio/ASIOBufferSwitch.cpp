#include "ASIOBufferSwitch.h"
#include <atomic>
#include <mutex>

namespace nap {
namespace drivers {

class ASIOBufferSwitch::Impl {
public:
    std::atomic<bool> running{false};
    std::atomic<size_t> currentBufferIndex{0};
    std::atomic<double> samplePosition{0.0};
    std::atomic<double> systemTime{0.0};

    std::atomic<uint64_t> bufferSwitchCount{0};
    std::atomic<uint64_t> xrunCount{0};

    ProcessCallback processCallback;
    SampleRateChangedCallback sampleRateChangedCallback;
    ResetRequestCallback resetRequestCallback;
    LatencyChangedCallback latencyChangedCallback;

    std::mutex callbackMutex;
};

ASIOBufferSwitch::ASIOBufferSwitch() : pImpl(std::make_unique<Impl>()) {}

ASIOBufferSwitch::~ASIOBufferSwitch() {
    stop();
}

ASIOBufferSwitch::ASIOBufferSwitch(ASIOBufferSwitch&&) noexcept = default;
ASIOBufferSwitch& ASIOBufferSwitch::operator=(ASIOBufferSwitch&&) noexcept = default;

void ASIOBufferSwitch::start() {
    pImpl->running = true;
    pImpl->bufferSwitchCount = 0;
    pImpl->xrunCount = 0;
}

void ASIOBufferSwitch::stop() {
    pImpl->running = false;
}

bool ASIOBufferSwitch::isRunning() const {
    return pImpl->running;
}

void ASIOBufferSwitch::setProcessCallback(ProcessCallback callback) {
    std::lock_guard<std::mutex> lock(pImpl->callbackMutex);
    pImpl->processCallback = std::move(callback);
}

void ASIOBufferSwitch::setSampleRateChangedCallback(SampleRateChangedCallback callback) {
    std::lock_guard<std::mutex> lock(pImpl->callbackMutex);
    pImpl->sampleRateChangedCallback = std::move(callback);
}

void ASIOBufferSwitch::setResetRequestCallback(ResetRequestCallback callback) {
    std::lock_guard<std::mutex> lock(pImpl->callbackMutex);
    pImpl->resetRequestCallback = std::move(callback);
}

void ASIOBufferSwitch::setLatencyChangedCallback(LatencyChangedCallback callback) {
    std::lock_guard<std::mutex> lock(pImpl->callbackMutex);
    pImpl->latencyChangedCallback = std::move(callback);
}

void ASIOBufferSwitch::bufferSwitch(long bufferIndex, bool directProcess) {
    if (!pImpl->running) {
        return;
    }

    pImpl->currentBufferIndex = static_cast<size_t>(bufferIndex);
    pImpl->bufferSwitchCount++;

    std::lock_guard<std::mutex> lock(pImpl->callbackMutex);
    if (pImpl->processCallback) {
        pImpl->processCallback(static_cast<size_t>(bufferIndex));
    }
}

void ASIOBufferSwitch::bufferSwitchTimeInfo(long bufferIndex, bool directProcess,
                                             double samplePosition, double systemTime) {
    pImpl->samplePosition = samplePosition;
    pImpl->systemTime = systemTime;

    bufferSwitch(bufferIndex, directProcess);
}

long ASIOBufferSwitch::asioMessage(long selector, long value, void* message, double* opt) {
    // ASIO message selectors
    enum {
        kAsioSelectorSupported = 1,
        kAsioEngineVersion,
        kAsioResetRequest,
        kAsioBufferSizeChange,
        kAsioResyncRequest,
        kAsioLatenciesChanged,
        kAsioSupportsTimeInfo,
        kAsioSupportsTimeCode,
        kAsioMMCCommand,
        kAsioSupportsInputMonitor,
        kAsioSupportsInputGain,
        kAsioSupportsInputMeter,
        kAsioSupportsOutputGain,
        kAsioSupportsOutputMeter,
        kAsioOverload
    };

    switch (selector) {
        case kAsioSelectorSupported:
            switch (value) {
                case kAsioResetRequest:
                case kAsioEngineVersion:
                case kAsioResyncRequest:
                case kAsioLatenciesChanged:
                case kAsioSupportsTimeInfo:
                case kAsioOverload:
                    return 1;
                default:
                    return 0;
            }

        case kAsioEngineVersion:
            return 2; // ASIO 2.0

        case kAsioResetRequest:
            {
                std::lock_guard<std::mutex> lock(pImpl->callbackMutex);
                if (pImpl->resetRequestCallback) {
                    pImpl->resetRequestCallback();
                }
            }
            return 1;

        case kAsioResyncRequest:
            return 1;

        case kAsioLatenciesChanged:
            {
                std::lock_guard<std::mutex> lock(pImpl->callbackMutex);
                if (pImpl->latencyChangedCallback) {
                    pImpl->latencyChangedCallback();
                }
            }
            return 1;

        case kAsioSupportsTimeInfo:
            return 1;

        case kAsioOverload:
            pImpl->xrunCount++;
            return 1;

        default:
            return 0;
    }
}

double ASIOBufferSwitch::getSamplePosition() const {
    return pImpl->samplePosition;
}

double ASIOBufferSwitch::getSystemTime() const {
    return pImpl->systemTime;
}

size_t ASIOBufferSwitch::getCurrentBufferIndex() const {
    return pImpl->currentBufferIndex;
}

uint64_t ASIOBufferSwitch::getBufferSwitchCount() const {
    return pImpl->bufferSwitchCount;
}

uint64_t ASIOBufferSwitch::getXrunCount() const {
    return pImpl->xrunCount;
}

void ASIOBufferSwitch::resetStatistics() {
    pImpl->bufferSwitchCount = 0;
    pImpl->xrunCount = 0;
}

} // namespace drivers
} // namespace nap
