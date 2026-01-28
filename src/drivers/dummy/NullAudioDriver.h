#ifndef NAP_NULL_AUDIO_DRIVER_H
#define NAP_NULL_AUDIO_DRIVER_H

#include "../IAudioDriver.h"
#include <memory>
#include <string>
#include <vector>

namespace nap {
namespace drivers {

/**
 * @brief Null/Dummy audio driver for testing and CI/CD
 *
 * Provides a fully functional audio driver interface without actual
 * hardware interaction. Useful for unit testing, benchmarking, and
 * continuous integration environments.
 */
class NullAudioDriver : public IAudioDriver {
public:
    NullAudioDriver();
    ~NullAudioDriver() override;

    // Prevent copying
    NullAudioDriver(const NullAudioDriver&) = delete;
    NullAudioDriver& operator=(const NullAudioDriver&) = delete;

    // Move semantics
    NullAudioDriver(NullAudioDriver&&) noexcept;
    NullAudioDriver& operator=(NullAudioDriver&&) noexcept;

    // IAudioDriver interface
    bool initialize() override;
    void shutdown() override;
    bool start() override;
    void stop() override;
    bool isRunning() const override;

    std::vector<DeviceInfo> enumerateDevices() override;
    bool selectDevice(const std::string& deviceId) override;
    std::string getCurrentDeviceId() const override;

    bool setSampleRate(double sampleRate) override;
    double getSampleRate() const override;
    std::vector<double> getSupportedSampleRates() const override;

    bool setBufferSize(size_t bufferSize) override;
    size_t getBufferSize() const override;
    std::vector<size_t> getSupportedBufferSizes() const override;

    size_t getInputChannelCount() const override;
    size_t getOutputChannelCount() const override;

    void setAudioCallback(AudioCallback callback) override;
    double getStreamLatency() const override;
    uint64_t getStreamTime() const override;

    // Test-specific methods

    // Configure virtual device
    void setVirtualDeviceChannels(size_t inputChannels, size_t outputChannels);

    // Processing mode
    enum class ProcessingMode {
        Realtime,       // Simulates realtime with timing
        AsFastAsPossible, // Process as fast as possible (for benchmarking)
        Manual          // Only process when explicitly triggered
    };

    void setProcessingMode(ProcessingMode mode);
    ProcessingMode getProcessingMode() const;

    // Manual processing trigger
    void processBlock();
    void processBlocks(size_t count);

    // Input simulation
    void setInputData(size_t channel, const float* data, size_t sampleCount);
    void setInputSilence();
    void setInputNoise(float amplitude = 0.5f);
    void setInputSineWave(float frequency, float amplitude = 0.5f);

    // Output capture
    const float* getOutputData(size_t channel) const;
    float getOutputPeak(size_t channel) const;
    float getOutputRMS(size_t channel) const;

    // Statistics
    uint64_t getBlocksProcessed() const;
    uint64_t getSamplesProcessed() const;
    double getAverageProcessingTimeNs() const;
    double getMaxProcessingTimeNs() const;
    void resetStatistics();

    // Simulate errors
    void simulateXrun();
    void simulateDeviceDisconnect();
    void simulateDeviceReconnect();

    // Callback counting (for testing)
    uint64_t getCallbackCount() const;

private:
    class Impl;
    std::unique_ptr<Impl> pImpl;
};

} // namespace drivers
} // namespace nap

#endif // NAP_NULL_AUDIO_DRIVER_H
