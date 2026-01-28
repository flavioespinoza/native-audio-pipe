#ifndef NAP_ASIO_DRIVER_H
#define NAP_ASIO_DRIVER_H

#include "../IAudioDriver.h"
#include <memory>
#include <string>
#include <vector>
#include <functional>

namespace nap {
namespace drivers {

/**
 * @brief ASIO driver implementation for Windows professional audio
 *
 * Provides low-latency audio I/O using the Steinberg ASIO protocol.
 * Supports buffer sizes down to 32 samples with compatible hardware.
 */
class ASIODriver : public IAudioDriver {
public:
    ASIODriver();
    ~ASIODriver() override;

    // Prevent copying
    ASIODriver(const ASIODriver&) = delete;
    ASIODriver& operator=(const ASIODriver&) = delete;

    // Move semantics
    ASIODriver(ASIODriver&&) noexcept;
    ASIODriver& operator=(ASIODriver&&) noexcept;

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

    // ASIO-specific methods
    bool loadDriver(const std::string& driverName);
    void unloadDriver();
    bool isDriverLoaded() const;

    std::string getDriverVersion() const;
    std::string getDriverName() const;

    // Direct buffer access for ASIO buffer switch
    float* getInputBuffer(size_t channel, size_t bufferIndex);
    float* getOutputBuffer(size_t channel, size_t bufferIndex);

    // Control panel
    bool openControlPanel();

    // Clock sources
    std::vector<std::string> getClockSources() const;
    bool setClockSource(const std::string& source);
    std::string getCurrentClockSource() const;

private:
    class Impl;
    std::unique_ptr<Impl> pImpl;
};

} // namespace drivers
} // namespace nap

#endif // NAP_ASIO_DRIVER_H
