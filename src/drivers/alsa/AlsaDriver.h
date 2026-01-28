#ifndef NAP_ALSA_DRIVER_H
#define NAP_ALSA_DRIVER_H

#include "../IAudioDriver.h"
#include <memory>
#include <string>
#include <vector>

namespace nap {
namespace drivers {

/**
 * @brief ALSA driver implementation for Linux audio
 *
 * Provides audio I/O using the Advanced Linux Sound Architecture.
 * Supports both direct hardware access and software mixing through PulseAudio/PipeWire.
 */
class AlsaDriver : public IAudioDriver {
public:
    AlsaDriver();
    ~AlsaDriver() override;

    // Prevent copying
    AlsaDriver(const AlsaDriver&) = delete;
    AlsaDriver& operator=(const AlsaDriver&) = delete;

    // Move semantics
    AlsaDriver(AlsaDriver&&) noexcept;
    AlsaDriver& operator=(AlsaDriver&&) noexcept;

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

    // ALSA-specific methods
    enum class AccessMode {
        RWInterleaved,
        RWNonInterleaved,
        MMapInterleaved,
        MMapNonInterleaved
    };

    bool setAccessMode(AccessMode mode);
    AccessMode getAccessMode() const;

    // Period configuration
    bool setPeriodSize(size_t frames);
    size_t getPeriodSize() const;
    bool setNumPeriods(size_t periods);
    size_t getNumPeriods() const;

    // Hardware parameters
    enum class SampleFormat {
        S16_LE,
        S24_LE,
        S24_3LE,
        S32_LE,
        Float32_LE
    };

    bool setSampleFormat(SampleFormat format);
    SampleFormat getSampleFormat() const;

    // Device control
    bool openDevice(const std::string& deviceName, bool capture, bool playback);
    void closeDevice();

    // ALSA mixer
    float getMixerVolume(const std::string& element = "Master") const;
    bool setMixerVolume(const std::string& element, float volume);
    bool getMixerMute(const std::string& element = "Master") const;
    bool setMixerMute(const std::string& element, bool muted);

    // Recovery
    bool recover(int error);
    bool drain();
    bool drop();

private:
    class Impl;
    std::unique_ptr<Impl> pImpl;
};

} // namespace drivers
} // namespace nap

#endif // NAP_ALSA_DRIVER_H
