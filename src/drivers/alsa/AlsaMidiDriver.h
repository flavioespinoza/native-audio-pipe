#ifndef NAP_ALSA_MIDI_DRIVER_H
#define NAP_ALSA_MIDI_DRIVER_H

#include <memory>
#include <string>
#include <vector>
#include <functional>
#include <cstdint>

namespace nap {
namespace drivers {

/**
 * @brief ALSA MIDI driver for Linux MIDI I/O
 *
 * Provides MIDI input and output using the ALSA sequencer API.
 * Supports virtual ports, hardware MIDI devices, and routing.
 */
class AlsaMidiDriver {
public:
    struct MidiPortInfo {
        std::string id;
        std::string name;
        std::string clientName;
        int clientId;
        int portId;
        bool isInput;
        bool isOutput;
        bool isHardware;
        bool isVirtual;
    };

    struct MidiEvent {
        enum class Type : uint8_t {
            NoteOff = 0x80,
            NoteOn = 0x90,
            PolyPressure = 0xA0,
            ControlChange = 0xB0,
            ProgramChange = 0xC0,
            ChannelPressure = 0xD0,
            PitchBend = 0xE0,
            SysEx = 0xF0,
            TimeCode = 0xF1,
            SongPosition = 0xF2,
            SongSelect = 0xF3,
            TuneRequest = 0xF6,
            Clock = 0xF8,
            Start = 0xFA,
            Continue = 0xFB,
            Stop = 0xFC,
            ActiveSensing = 0xFE,
            Reset = 0xFF
        };

        Type type;
        uint8_t channel;
        uint8_t data1;
        uint8_t data2;
        uint64_t timestamp;
        std::vector<uint8_t> sysexData;
    };

    using MidiCallback = std::function<void(const MidiEvent& event)>;

    AlsaMidiDriver();
    ~AlsaMidiDriver();

    // Prevent copying
    AlsaMidiDriver(const AlsaMidiDriver&) = delete;
    AlsaMidiDriver& operator=(const AlsaMidiDriver&) = delete;

    // Move semantics
    AlsaMidiDriver(AlsaMidiDriver&&) noexcept;
    AlsaMidiDriver& operator=(AlsaMidiDriver&&) noexcept;

    // Core operations
    bool initialize(const std::string& clientName = "NAP MIDI");
    void shutdown();
    bool isInitialized() const;

    // Port enumeration
    std::vector<MidiPortInfo> enumeratePorts() const;
    std::vector<MidiPortInfo> getInputPorts() const;
    std::vector<MidiPortInfo> getOutputPorts() const;

    // Port management
    bool createInputPort(const std::string& name);
    bool createOutputPort(const std::string& name);
    bool deletePort(const std::string& portName);

    // Connection
    bool connectInput(const std::string& sourceName);
    bool connectOutput(const std::string& destName);
    bool disconnectInput(const std::string& sourceName);
    bool disconnectOutput(const std::string& destName);
    std::vector<std::string> getConnectedInputs() const;
    std::vector<std::string> getConnectedOutputs() const;

    // Start/stop
    bool start();
    void stop();
    bool isRunning() const;

    // MIDI input
    void setMidiCallback(MidiCallback callback);

    // MIDI output
    bool sendEvent(const MidiEvent& event);
    bool sendNoteOn(uint8_t channel, uint8_t note, uint8_t velocity);
    bool sendNoteOff(uint8_t channel, uint8_t note, uint8_t velocity = 0);
    bool sendControlChange(uint8_t channel, uint8_t controller, uint8_t value);
    bool sendProgramChange(uint8_t channel, uint8_t program);
    bool sendPitchBend(uint8_t channel, int16_t value);
    bool sendSysEx(const std::vector<uint8_t>& data);

    // Timing
    uint64_t getCurrentTime() const;
    void setTimebase(double ppq);

    // Virtual port features
    bool createVirtualInputPort(const std::string& name);
    bool createVirtualOutputPort(const std::string& name);

private:
    class Impl;
    std::unique_ptr<Impl> pImpl;
};

} // namespace drivers
} // namespace nap

#endif // NAP_ALSA_MIDI_DRIVER_H
