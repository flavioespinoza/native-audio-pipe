#ifndef NAP_IAUDIO_DRIVER_H
#define NAP_IAUDIO_DRIVER_H

#include <string>
#include <vector>
#include <functional>
#include <cstdint>

namespace nap {

/**
 * @brief Audio device information structure
 */
struct AudioDeviceInfo {
    std::string id;
    std::string name;
    int maxInputChannels;
    int maxOutputChannels;
    std::vector<double> supportedSampleRates;
    std::vector<int> supportedBufferSizes;
    bool isDefault;
};

/**
 * @brief Audio stream configuration
 */
struct AudioStreamConfig {
    std::string deviceId;
    double sampleRate = 44100.0;
    int bufferSize = 512;
    int inputChannels = 0;
    int outputChannels = 2;
};

/**
 * @brief Driver state enumeration
 */
enum class DriverState {
    Uninitialized,
    Initialized,
    Running,
    Stopped,
    Error
};

/**
 * @brief Interface for platform-specific audio drivers
 *
 * Base interface for all audio hardware abstraction drivers.
 */
class IAudioDriver {
public:
    using AudioCallback = std::function<void(const float* input, float* output,
                                             int numFrames, int numInputChannels,
                                             int numOutputChannels)>;

    virtual ~IAudioDriver() = default;

    // Lifecycle
    virtual bool initialize() = 0;
    virtual void shutdown() = 0;
    virtual bool start() = 0;
    virtual void stop() = 0;

    // State
    virtual DriverState getState() const = 0;
    virtual std::string getLastError() const = 0;

    // Device enumeration
    virtual std::vector<AudioDeviceInfo> getAvailableDevices() const = 0;
    virtual AudioDeviceInfo getDefaultDevice() const = 0;

    // Configuration
    virtual bool configure(const AudioStreamConfig& config) = 0;
    virtual AudioStreamConfig getCurrentConfig() const = 0;

    // Callback
    virtual void setAudioCallback(AudioCallback callback) = 0;

    // Driver info
    virtual std::string getDriverName() const = 0;
    virtual std::string getDriverVersion() const = 0;
    virtual bool isAvailable() const = 0;
};

} // namespace nap

#endif // NAP_IAUDIO_DRIVER_H
