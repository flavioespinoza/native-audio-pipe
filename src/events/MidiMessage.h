#ifndef NAP_MIDI_MESSAGE_H
#define NAP_MIDI_MESSAGE_H

#include <cstdint>
#include <vector>
#include <string>
#include <array>

namespace nap {
namespace events {

/**
 * @brief MIDI message representation
 *
 * Encapsulates MIDI channel messages, system messages, and provides
 * convenient factory methods and accessors for common MIDI operations.
 */
class MidiMessage {
public:
    // Message status bytes
    enum class Status : uint8_t {
        // Channel Voice Messages (0x8n - 0xEn, where n = channel)
        NoteOff = 0x80,
        NoteOn = 0x90,
        PolyPressure = 0xA0,
        ControlChange = 0xB0,
        ProgramChange = 0xC0,
        ChannelPressure = 0xD0,
        PitchBend = 0xE0,

        // System Common Messages
        SysEx = 0xF0,
        TimeCode = 0xF1,
        SongPosition = 0xF2,
        SongSelect = 0xF3,
        TuneRequest = 0xF6,
        EndOfSysEx = 0xF7,

        // System Real-Time Messages
        TimingClock = 0xF8,
        Start = 0xFA,
        Continue = 0xFB,
        Stop = 0xFC,
        ActiveSensing = 0xFE,
        SystemReset = 0xFF,

        // Invalid/Unknown
        Invalid = 0x00
    };

    // Common Control Change numbers
    enum class Controller : uint8_t {
        BankSelectMSB = 0,
        ModulationWheel = 1,
        BreathController = 2,
        FootController = 4,
        PortamentoTime = 5,
        DataEntryMSB = 6,
        Volume = 7,
        Balance = 8,
        Pan = 10,
        Expression = 11,
        EffectControl1 = 12,
        EffectControl2 = 13,
        GeneralPurpose1 = 16,
        GeneralPurpose2 = 17,
        GeneralPurpose3 = 18,
        GeneralPurpose4 = 19,
        BankSelectLSB = 32,
        DataEntryLSB = 38,
        Sustain = 64,
        Portamento = 65,
        Sostenuto = 66,
        SoftPedal = 67,
        LegatoFootswitch = 68,
        Hold2 = 69,
        SoundController1 = 70,
        SoundController2 = 71,
        SoundController3 = 72,
        SoundController4 = 73,
        SoundController5 = 74,
        PortamentoControl = 84,
        ReverbDepth = 91,
        TremoloDepth = 92,
        ChorusDepth = 93,
        DetuneDepth = 94,
        PhaserDepth = 95,
        DataIncrement = 96,
        DataDecrement = 97,
        NRPN_LSB = 98,
        NRPN_MSB = 99,
        RPN_LSB = 100,
        RPN_MSB = 101,
        AllSoundOff = 120,
        ResetAllControllers = 121,
        LocalControl = 122,
        AllNotesOff = 123,
        OmniModeOff = 124,
        OmniModeOn = 125,
        MonoModeOn = 126,
        PolyModeOn = 127
    };

    // Constructors
    MidiMessage();
    MidiMessage(const uint8_t* data, size_t size);
    MidiMessage(std::initializer_list<uint8_t> data);
    explicit MidiMessage(const std::vector<uint8_t>& data);

    // Factory methods for channel messages
    static MidiMessage noteOn(uint8_t channel, uint8_t note, uint8_t velocity);
    static MidiMessage noteOff(uint8_t channel, uint8_t note, uint8_t velocity = 0);
    static MidiMessage polyPressure(uint8_t channel, uint8_t note, uint8_t pressure);
    static MidiMessage controlChange(uint8_t channel, uint8_t controller, uint8_t value);
    static MidiMessage controlChange(uint8_t channel, Controller controller, uint8_t value);
    static MidiMessage programChange(uint8_t channel, uint8_t program);
    static MidiMessage channelPressure(uint8_t channel, uint8_t pressure);
    static MidiMessage pitchBend(uint8_t channel, int16_t value);
    static MidiMessage pitchBend(uint8_t channel, uint8_t lsb, uint8_t msb);

    // Factory methods for system messages
    static MidiMessage timingClock();
    static MidiMessage start();
    static MidiMessage continue_();
    static MidiMessage stop();
    static MidiMessage activeSensing();
    static MidiMessage systemReset();
    static MidiMessage tuneRequest();
    static MidiMessage songPosition(uint16_t beats);
    static MidiMessage songSelect(uint8_t song);
    static MidiMessage timeCode(uint8_t data);

    // Factory for RPN/NRPN
    static std::vector<MidiMessage> rpn(uint8_t channel, uint16_t parameter, uint16_t value);
    static std::vector<MidiMessage> nrpn(uint8_t channel, uint16_t parameter, uint16_t value);

    // Raw data access
    const uint8_t* data() const;
    size_t size() const;
    uint8_t operator[](size_t index) const;

    // Message type queries
    bool isValid() const;
    bool isChannelMessage() const;
    bool isSystemMessage() const;
    bool isSystemCommon() const;
    bool isSystemRealTime() const;

    bool isNoteOn() const;
    bool isNoteOff() const;
    bool isNoteOnOrOff() const;
    bool isPolyPressure() const;
    bool isControlChange() const;
    bool isProgramChange() const;
    bool isChannelPressure() const;
    bool isPitchBend() const;
    bool isSysEx() const;

    // Accessors for channel messages
    Status getStatus() const;
    uint8_t getStatusByte() const;
    uint8_t getChannel() const;  // 0-15
    uint8_t getNoteNumber() const;
    uint8_t getVelocity() const;
    uint8_t getPressure() const;
    uint8_t getControllerNumber() const;
    uint8_t getControllerValue() const;
    uint8_t getProgramNumber() const;
    int16_t getPitchBendValue() const;  // -8192 to 8191
    float getPitchBendNormalized() const;  // -1.0 to 1.0

    // Note helpers
    std::string getNoteName() const;
    int getOctave() const;
    float getNoteFrequency() const;

    // Comparison
    bool operator==(const MidiMessage& other) const;
    bool operator!=(const MidiMessage& other) const;

    // Utility
    std::string toString() const;
    static size_t getExpectedLength(uint8_t statusByte);

    // Timestamp (set by receiver)
    void setTimestamp(uint64_t timestamp);
    uint64_t getTimestamp() const;

private:
    std::array<uint8_t, 3> data_;
    size_t size_;
    uint64_t timestamp_;
};

} // namespace events
} // namespace nap

#endif // NAP_MIDI_MESSAGE_H
