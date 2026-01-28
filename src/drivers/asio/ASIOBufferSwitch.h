#ifndef NAP_ASIO_BUFFER_SWITCH_H
#define NAP_ASIO_BUFFER_SWITCH_H

#include <memory>
#include <functional>
#include <cstddef>

namespace nap {
namespace drivers {

/**
 * @brief ASIO buffer switch callback handler
 *
 * Manages the ASIO double-buffering mechanism and coordinates
 * buffer switches between the driver and the audio processing callback.
 */
class ASIOBufferSwitch {
public:
    using ProcessCallback = std::function<void(size_t bufferIndex)>;
    using SampleRateChangedCallback = std::function<void(double newRate)>;
    using ResetRequestCallback = std::function<void()>;
    using LatencyChangedCallback = std::function<void()>;

    ASIOBufferSwitch();
    ~ASIOBufferSwitch();

    // Prevent copying
    ASIOBufferSwitch(const ASIOBufferSwitch&) = delete;
    ASIOBufferSwitch& operator=(const ASIOBufferSwitch&) = delete;

    // Move semantics
    ASIOBufferSwitch(ASIOBufferSwitch&&) noexcept;
    ASIOBufferSwitch& operator=(ASIOBufferSwitch&&) noexcept;

    // Core operations
    void start();
    void stop();
    bool isRunning() const;

    // Callback registration
    void setProcessCallback(ProcessCallback callback);
    void setSampleRateChangedCallback(SampleRateChangedCallback callback);
    void setResetRequestCallback(ResetRequestCallback callback);
    void setLatencyChangedCallback(LatencyChangedCallback callback);

    // Buffer switch handling (called by ASIO driver)
    void bufferSwitch(long bufferIndex, bool directProcess);
    void bufferSwitchTimeInfo(long bufferIndex, bool directProcess,
                               double samplePosition, double systemTime);

    // ASIO message handling
    long asioMessage(long selector, long value, void* message, double* opt);

    // Timing information
    double getSamplePosition() const;
    double getSystemTime() const;
    size_t getCurrentBufferIndex() const;

    // Statistics
    uint64_t getBufferSwitchCount() const;
    uint64_t getXrunCount() const;
    void resetStatistics();

private:
    class Impl;
    std::unique_ptr<Impl> pImpl;
};

} // namespace drivers
} // namespace nap

#endif // NAP_ASIO_BUFFER_SWITCH_H
