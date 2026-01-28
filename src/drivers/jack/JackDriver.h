#ifndef NAP_JACK_DRIVER_H
#define NAP_JACK_DRIVER_H

#include "../IAudioDriver.h"
#include <memory>
#include <string>
#include <vector>

namespace nap {
namespace drivers {

/**
 * @brief JACK Audio Connection Kit driver for professional Linux audio
 *
 * Provides low-latency audio I/O using the JACK audio server.
 * Supports inter-application routing, MIDI, and transport synchronization.
 */
class JackDriver : public IAudioDriver {
public:
    JackDriver();
    ~JackDriver() override;

    // Prevent copying
    JackDriver(const JackDriver&) = delete;
    JackDriver& operator=(const JackDriver&) = delete;

    // Move semantics
    JackDriver(JackDriver&&) noexcept;
    JackDriver& operator=(JackDriver&&) noexcept;

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

    // JACK-specific methods
    bool connectToServer(const std::string& serverName = "default");
    void disconnectFromServer();
    bool isConnectedToServer() const;

    // Port management
    bool createInputPorts(size_t count, const std::string& baseName = "input");
    bool createOutputPorts(size_t count, const std::string& baseName = "output");
    std::vector<std::string> getInputPortNames() const;
    std::vector<std::string> getOutputPortNames() const;

    // Port connection (to other JACK clients)
    std::vector<std::string> getAvailablePorts(bool isInput) const;
    bool connectPorts(const std::string& srcPort, const std::string& dstPort);
    bool disconnectPorts(const std::string& srcPort, const std::string& dstPort);
    std::vector<std::string> getPortConnections(const std::string& portName) const;

    // Transport control
    enum class TransportState {
        Stopped,
        Rolling,
        Starting,
        Looping
    };

    TransportState getTransportState() const;
    bool setTransportState(TransportState state);
    uint64_t getTransportFrame() const;
    bool setTransportFrame(uint64_t frame);
    double getTransportBPM() const;

    // Timebase master
    bool becomeTimebaseMaster(bool conditional = false);
    void releaseTimebase();
    bool isTimebaseMaster() const;

    // Freewheel mode (faster than realtime processing)
    bool setFreewheelMode(bool enabled);
    bool isInFreewheelMode() const;

    // Client name
    std::string getClientName() const;

    // Server info
    float getCPULoad() const;
    bool isRealtime() const;

    // Callbacks
    using ShutdownCallback = std::function<void()>;
    using XrunCallback = std::function<void()>;
    using PortConnectCallback = std::function<void(const std::string&, const std::string&, bool connected)>;

    void setShutdownCallback(ShutdownCallback callback);
    void setXrunCallback(XrunCallback callback);
    void setPortConnectCallback(PortConnectCallback callback);

private:
    class Impl;
    std::unique_ptr<Impl> pImpl;
};

} // namespace drivers
} // namespace nap

#endif // NAP_JACK_DRIVER_H
