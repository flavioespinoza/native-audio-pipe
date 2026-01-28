#ifndef NAP_SYSEX_MESSAGE_H
#define NAP_SYSEX_MESSAGE_H

#include <cstdint>
#include <vector>
#include <string>
#include <memory>
#include <optional>

namespace nap {
namespace events {

/**
 * @brief System Exclusive (SysEx) MIDI message
 *
 * Handles variable-length SysEx messages with support for manufacturer IDs,
 * Universal SysEx, and common device control messages.
 */
class SysexMessage {
public:
    // Universal SysEx sub-IDs
    enum class UniversalType : uint8_t {
        NonRealTime = 0x7E,
        RealTime = 0x7F
    };

    // Non-Real-Time Universal SysEx categories
    enum class NonRealTimeCategory : uint8_t {
        SampleDumpHeader = 0x01,
        SampleDataPacket = 0x02,
        SampleDumpRequest = 0x03,
        MIDITimeCode = 0x04,
        SampleDumpExtensions = 0x05,
        GeneralInformation = 0x06,
        FileDump = 0x07,
        MIDITuning = 0x08,
        GeneralMIDI = 0x09,
        DownloadableSounds = 0x0A,
        FileReferenceMessage = 0x0B,
        MIDIVisualControl = 0x0C,
        EndOfFile = 0x7B,
        Wait = 0x7C,
        Cancel = 0x7D,
        NAK = 0x7E,
        ACK = 0x7F
    };

    // Real-Time Universal SysEx categories
    enum class RealTimeCategory : uint8_t {
        MIDITimeCode = 0x01,
        MIDIShowControl = 0x02,
        NotationInformation = 0x03,
        DeviceControl = 0x04,
        RealTimeMTCCueing = 0x05,
        MIDIMachineControl = 0x06,
        MIDIMachineControlResponse = 0x07,
        MIDITuning = 0x08,
        ControllerDestination = 0x09,
        KeyBasedInstrumentControl = 0x0A,
        ScalablePolyphonyMIDI = 0x0B,
        MobilePhoneControl = 0x0C
    };

    // Constructors
    SysexMessage();
    explicit SysexMessage(const std::vector<uint8_t>& data);
    SysexMessage(const uint8_t* data, size_t size);

    // Factory methods for Universal SysEx
    static SysexMessage identityRequest(uint8_t deviceId = 0x7F);
    static SysexMessage identityReply(uint8_t deviceId,
                                       uint8_t manufacturerId1,
                                       uint8_t manufacturerId2,
                                       uint8_t manufacturerId3,
                                       uint8_t familyMsb, uint8_t familyLsb,
                                       uint8_t modelMsb, uint8_t modelLsb,
                                       uint8_t versionMsb, uint8_t versionLsb,
                                       uint8_t version2, uint8_t version3);

    // Device Control messages
    static SysexMessage masterVolume(uint8_t deviceId, uint16_t volume);
    static SysexMessage masterBalance(uint8_t deviceId, uint16_t balance);
    static SysexMessage masterFineTuning(uint8_t deviceId, uint16_t tuning);
    static SysexMessage masterCoarseTuning(uint8_t deviceId, uint16_t tuning);
    static SysexMessage globalParameterControl(uint8_t deviceId,
                                                uint8_t slotPath1, uint8_t slotPath2,
                                                uint8_t parameterMsb, uint8_t parameterLsb,
                                                uint8_t valueMsb, uint8_t valueLsb);

    // General MIDI messages
    static SysexMessage gmSystemOn(uint8_t deviceId = 0x7F);
    static SysexMessage gmSystemOff(uint8_t deviceId = 0x7F);
    static SysexMessage gm2SystemOn(uint8_t deviceId = 0x7F);

    // Custom manufacturer SysEx
    static SysexMessage createManufacturerMessage(uint32_t manufacturerId,
                                                    const std::vector<uint8_t>& data);

    // Data access
    const std::vector<uint8_t>& data() const;
    size_t size() const;
    uint8_t operator[](size_t index) const;

    // Raw data without F0/F7 framing
    std::vector<uint8_t> getPayload() const;

    // Validation
    bool isValid() const;
    bool isComplete() const;
    bool isUniversal() const;
    bool isNonRealTime() const;
    bool isRealTime() const;

    // Manufacturer ID handling
    uint32_t getManufacturerId() const;
    std::string getManufacturerName() const;
    bool isExtendedManufacturerId() const;

    // Universal SysEx accessors
    std::optional<uint8_t> getDeviceId() const;
    std::optional<uint8_t> getSubId1() const;
    std::optional<uint8_t> getSubId2() const;

    // Identity Reply parsing
    struct IdentityReply {
        uint8_t deviceId;
        uint32_t manufacturerId;
        uint16_t familyCode;
        uint16_t modelNumber;
        uint32_t softwareRevision;
    };
    std::optional<IdentityReply> parseIdentityReply() const;

    // Modification
    void append(uint8_t byte);
    void append(const uint8_t* data, size_t size);
    void clear();

    // Checksum support (Roland-style)
    uint8_t calculateRolandChecksum() const;
    bool verifyRolandChecksum() const;

    // Comparison
    bool operator==(const SysexMessage& other) const;
    bool operator!=(const SysexMessage& other) const;

    // Utility
    std::string toString() const;
    std::string toHexString() const;

    // Timestamp
    void setTimestamp(uint64_t timestamp);
    uint64_t getTimestamp() const;

    // Common manufacturer IDs
    static constexpr uint32_t MANUFACTURER_ROLAND = 0x41;
    static constexpr uint32_t MANUFACTURER_YAMAHA = 0x43;
    static constexpr uint32_t MANUFACTURER_KORG = 0x42;
    static constexpr uint32_t MANUFACTURER_KAWAI = 0x40;
    static constexpr uint32_t MANUFACTURER_MOOG = 0x04;
    static constexpr uint32_t MANUFACTURER_SEQUENTIAL = 0x01;
    static constexpr uint32_t MANUFACTURER_OBERHEIM = 0x10;
    static constexpr uint32_t MANUFACTURER_AKAI = 0x47;
    static constexpr uint32_t MANUFACTURER_NATIVE_INSTRUMENTS = 0x002109;

private:
    std::vector<uint8_t> data_;
    uint64_t timestamp_;

    void ensureFraming();
};

} // namespace events
} // namespace nap

#endif // NAP_SYSEX_MESSAGE_H
