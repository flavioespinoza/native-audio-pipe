#ifndef NAP_JACK_CLIENT_H
#define NAP_JACK_CLIENT_H

#include <memory>
#include <string>
#include <vector>
#include <functional>
#include <cstddef>

namespace nap {
namespace drivers {

/**
 * @brief Low-level JACK client wrapper
 *
 * Encapsulates JACK client operations and provides a clean C++ interface
 * for connecting to and interacting with the JACK audio server.
 */
class JackClient {
public:
    using ProcessCallback = std::function<void(size_t nframes)>;
    using BufferSizeCallback = std::function<void(size_t newSize)>;
    using SampleRateCallback = std::function<void(double newRate)>;
    using ShutdownCallback = std::function<void()>;
    using XrunCallback = std::function<int()>;
    using PortRegistrationCallback = std::function<void(const std::string& portName, bool registered)>;
    using PortConnectCallback = std::function<void(const std::string& portA, const std::string& portB, bool connected)>;

    JackClient();
    ~JackClient();

    // Prevent copying
    JackClient(const JackClient&) = delete;
    JackClient& operator=(const JackClient&) = delete;

    // Move semantics
    JackClient(JackClient&&) noexcept;
    JackClient& operator=(JackClient&&) noexcept;

    // Initialization
    bool initialize(const std::string& clientName);
    bool connect(const std::string& serverName = "default");
    void disconnect();
    bool isConnected() const;

    // Activation
    bool activate();
    void deactivate();
    bool isActive() const;

    // Server properties
    double getSampleRate() const;
    size_t getBufferSize() const;
    bool setBufferSize(size_t size);
    float getCPULoad() const;
    bool isRealtime() const;

    // Port creation
    bool createInputPort(const std::string& name);
    bool createOutputPort(const std::string& name);
    bool createMidiInputPort(const std::string& name);
    bool createMidiOutputPort(const std::string& name);
    bool unregisterPort(const std::string& name);

    // Port querying
    std::vector<std::string> getAvailablePorts(bool isInput) const;
    std::vector<std::string> getOwnPorts() const;
    std::vector<std::string> getPortConnections(const std::string& portName) const;
    bool portExists(const std::string& portName) const;

    // Port connection
    bool connectPorts(const std::string& srcPort, const std::string& dstPort);
    bool disconnectPorts(const std::string& srcPort, const std::string& dstPort);
    bool disconnectAllPorts(const std::string& portName);

    // Buffer access (in process callback)
    float* getAudioBuffer(const std::string& portName, size_t nframes);
    void* getMidiBuffer(const std::string& portName, size_t nframes);

    // MIDI helpers
    size_t getMidiEventCount(void* midiBuffer);
    bool getMidiEvent(void* midiBuffer, size_t index,
                      uint8_t** data, size_t* size, uint32_t* time);
    bool writeMidiEvent(void* midiBuffer, uint32_t time,
                        const uint8_t* data, size_t size);
    void clearMidiBuffer(void* midiBuffer);

    // Transport
    enum class TransportState {
        Stopped,
        Rolling,
        Starting,
        Looping
    };

    TransportState getTransportState() const;
    void transportStart();
    void transportStop();
    void transportLocate(uint64_t frame);
    uint64_t getTransportFrame() const;

    // Callbacks
    void setProcessCallback(ProcessCallback callback);
    void setBufferSizeCallback(BufferSizeCallback callback);
    void setSampleRateCallback(SampleRateCallback callback);
    void setShutdownCallback(ShutdownCallback callback);
    void setXrunCallback(XrunCallback callback);
    void setPortRegistrationCallback(PortRegistrationCallback callback);
    void setPortConnectCallback(PortConnectCallback callback);

    // Client name
    std::string getClientName() const;

private:
    class Impl;
    std::unique_ptr<Impl> pImpl;
};

} // namespace drivers
} // namespace nap

#endif // NAP_JACK_CLIENT_H
