#include "MidiMessage.h"
#include <cmath>
#include <sstream>
#include <iomanip>

namespace nap {
namespace events {

MidiMessage::MidiMessage() : data_{0, 0, 0}, size_(0), timestamp_(0) {}

MidiMessage::MidiMessage(const uint8_t* data, size_t size)
    : data_{0, 0, 0}, size_(std::min(size, size_t(3))), timestamp_(0) {
    for (size_t i = 0; i < size_; ++i) {
        data_[i] = data[i];
    }
}

MidiMessage::MidiMessage(std::initializer_list<uint8_t> data)
    : data_{0, 0, 0}, size_(std::min(data.size(), size_t(3))), timestamp_(0) {
    size_t i = 0;
    for (uint8_t byte : data) {
        if (i >= 3) break;
        data_[i++] = byte;
    }
}

MidiMessage::MidiMessage(const std::vector<uint8_t>& data)
    : data_{0, 0, 0}, size_(std::min(data.size(), size_t(3))), timestamp_(0) {
    for (size_t i = 0; i < size_; ++i) {
        data_[i] = data[i];
    }
}

MidiMessage MidiMessage::noteOn(uint8_t channel, uint8_t note, uint8_t velocity) {
    return MidiMessage({
        static_cast<uint8_t>(0x90 | (channel & 0x0F)),
        static_cast<uint8_t>(note & 0x7F),
        static_cast<uint8_t>(velocity & 0x7F)
    });
}

MidiMessage MidiMessage::noteOff(uint8_t channel, uint8_t note, uint8_t velocity) {
    return MidiMessage({
        static_cast<uint8_t>(0x80 | (channel & 0x0F)),
        static_cast<uint8_t>(note & 0x7F),
        static_cast<uint8_t>(velocity & 0x7F)
    });
}

MidiMessage MidiMessage::polyPressure(uint8_t channel, uint8_t note, uint8_t pressure) {
    return MidiMessage({
        static_cast<uint8_t>(0xA0 | (channel & 0x0F)),
        static_cast<uint8_t>(note & 0x7F),
        static_cast<uint8_t>(pressure & 0x7F)
    });
}

MidiMessage MidiMessage::controlChange(uint8_t channel, uint8_t controller, uint8_t value) {
    return MidiMessage({
        static_cast<uint8_t>(0xB0 | (channel & 0x0F)),
        static_cast<uint8_t>(controller & 0x7F),
        static_cast<uint8_t>(value & 0x7F)
    });
}

MidiMessage MidiMessage::controlChange(uint8_t channel, Controller controller, uint8_t value) {
    return controlChange(channel, static_cast<uint8_t>(controller), value);
}

MidiMessage MidiMessage::programChange(uint8_t channel, uint8_t program) {
    return MidiMessage({
        static_cast<uint8_t>(0xC0 | (channel & 0x0F)),
        static_cast<uint8_t>(program & 0x7F)
    });
}

MidiMessage MidiMessage::channelPressure(uint8_t channel, uint8_t pressure) {
    return MidiMessage({
        static_cast<uint8_t>(0xD0 | (channel & 0x0F)),
        static_cast<uint8_t>(pressure & 0x7F)
    });
}

MidiMessage MidiMessage::pitchBend(uint8_t channel, int16_t value) {
    // Convert from -8192..8191 to 0..16383
    uint16_t bendValue = static_cast<uint16_t>(value + 8192);
    return MidiMessage({
        static_cast<uint8_t>(0xE0 | (channel & 0x0F)),
        static_cast<uint8_t>(bendValue & 0x7F),
        static_cast<uint8_t>((bendValue >> 7) & 0x7F)
    });
}

MidiMessage MidiMessage::pitchBend(uint8_t channel, uint8_t lsb, uint8_t msb) {
    return MidiMessage({
        static_cast<uint8_t>(0xE0 | (channel & 0x0F)),
        static_cast<uint8_t>(lsb & 0x7F),
        static_cast<uint8_t>(msb & 0x7F)
    });
}

MidiMessage MidiMessage::timingClock() {
    return MidiMessage({0xF8});
}

MidiMessage MidiMessage::start() {
    return MidiMessage({0xFA});
}

MidiMessage MidiMessage::continue_() {
    return MidiMessage({0xFB});
}

MidiMessage MidiMessage::stop() {
    return MidiMessage({0xFC});
}

MidiMessage MidiMessage::activeSensing() {
    return MidiMessage({0xFE});
}

MidiMessage MidiMessage::systemReset() {
    return MidiMessage({0xFF});
}

MidiMessage MidiMessage::tuneRequest() {
    return MidiMessage({0xF6});
}

MidiMessage MidiMessage::songPosition(uint16_t beats) {
    return MidiMessage({
        0xF2,
        static_cast<uint8_t>(beats & 0x7F),
        static_cast<uint8_t>((beats >> 7) & 0x7F)
    });
}

MidiMessage MidiMessage::songSelect(uint8_t song) {
    return MidiMessage({0xF3, static_cast<uint8_t>(song & 0x7F)});
}

MidiMessage MidiMessage::timeCode(uint8_t data) {
    return MidiMessage({0xF1, static_cast<uint8_t>(data & 0x7F)});
}

std::vector<MidiMessage> MidiMessage::rpn(uint8_t channel, uint16_t parameter, uint16_t value) {
    return {
        controlChange(channel, Controller::RPN_MSB, (parameter >> 7) & 0x7F),
        controlChange(channel, Controller::RPN_LSB, parameter & 0x7F),
        controlChange(channel, Controller::DataEntryMSB, (value >> 7) & 0x7F),
        controlChange(channel, Controller::DataEntryLSB, value & 0x7F)
    };
}

std::vector<MidiMessage> MidiMessage::nrpn(uint8_t channel, uint16_t parameter, uint16_t value) {
    return {
        controlChange(channel, Controller::NRPN_MSB, (parameter >> 7) & 0x7F),
        controlChange(channel, Controller::NRPN_LSB, parameter & 0x7F),
        controlChange(channel, Controller::DataEntryMSB, (value >> 7) & 0x7F),
        controlChange(channel, Controller::DataEntryLSB, value & 0x7F)
    };
}

const uint8_t* MidiMessage::data() const {
    return data_.data();
}

size_t MidiMessage::size() const {
    return size_;
}

uint8_t MidiMessage::operator[](size_t index) const {
    return index < size_ ? data_[index] : 0;
}

bool MidiMessage::isValid() const {
    return size_ > 0 && (data_[0] & 0x80);
}

bool MidiMessage::isChannelMessage() const {
    return isValid() && (data_[0] & 0xF0) < 0xF0;
}

bool MidiMessage::isSystemMessage() const {
    return isValid() && (data_[0] & 0xF0) == 0xF0;
}

bool MidiMessage::isSystemCommon() const {
    return isSystemMessage() && data_[0] < 0xF8;
}

bool MidiMessage::isSystemRealTime() const {
    return isSystemMessage() && data_[0] >= 0xF8;
}

bool MidiMessage::isNoteOn() const {
    return (data_[0] & 0xF0) == 0x90 && data_[2] > 0;
}

bool MidiMessage::isNoteOff() const {
    return (data_[0] & 0xF0) == 0x80 ||
           ((data_[0] & 0xF0) == 0x90 && data_[2] == 0);
}

bool MidiMessage::isNoteOnOrOff() const {
    uint8_t status = data_[0] & 0xF0;
    return status == 0x80 || status == 0x90;
}

bool MidiMessage::isPolyPressure() const {
    return (data_[0] & 0xF0) == 0xA0;
}

bool MidiMessage::isControlChange() const {
    return (data_[0] & 0xF0) == 0xB0;
}

bool MidiMessage::isProgramChange() const {
    return (data_[0] & 0xF0) == 0xC0;
}

bool MidiMessage::isChannelPressure() const {
    return (data_[0] & 0xF0) == 0xD0;
}

bool MidiMessage::isPitchBend() const {
    return (data_[0] & 0xF0) == 0xE0;
}

bool MidiMessage::isSysEx() const {
    return data_[0] == 0xF0;
}

MidiMessage::Status MidiMessage::getStatus() const {
    if (!isValid()) return Status::Invalid;

    if (isChannelMessage()) {
        return static_cast<Status>(data_[0] & 0xF0);
    }
    return static_cast<Status>(data_[0]);
}

uint8_t MidiMessage::getStatusByte() const {
    return data_[0];
}

uint8_t MidiMessage::getChannel() const {
    return isChannelMessage() ? (data_[0] & 0x0F) : 0;
}

uint8_t MidiMessage::getNoteNumber() const {
    return data_[1] & 0x7F;
}

uint8_t MidiMessage::getVelocity() const {
    return data_[2] & 0x7F;
}

uint8_t MidiMessage::getPressure() const {
    if (isChannelPressure()) {
        return data_[1] & 0x7F;
    }
    return data_[2] & 0x7F;
}

uint8_t MidiMessage::getControllerNumber() const {
    return data_[1] & 0x7F;
}

uint8_t MidiMessage::getControllerValue() const {
    return data_[2] & 0x7F;
}

uint8_t MidiMessage::getProgramNumber() const {
    return data_[1] & 0x7F;
}

int16_t MidiMessage::getPitchBendValue() const {
    uint16_t value = (static_cast<uint16_t>(data_[2] & 0x7F) << 7) |
                     (data_[1] & 0x7F);
    return static_cast<int16_t>(value) - 8192;
}

float MidiMessage::getPitchBendNormalized() const {
    return static_cast<float>(getPitchBendValue()) / 8192.0f;
}

std::string MidiMessage::getNoteName() const {
    static const char* noteNames[] = {"C", "C#", "D", "D#", "E", "F",
                                       "F#", "G", "G#", "A", "A#", "B"};
    uint8_t note = getNoteNumber();
    int octave = (note / 12) - 1;
    int noteIndex = note % 12;

    std::ostringstream oss;
    oss << noteNames[noteIndex] << octave;
    return oss.str();
}

int MidiMessage::getOctave() const {
    return (getNoteNumber() / 12) - 1;
}

float MidiMessage::getNoteFrequency() const {
    // A4 = 440Hz, MIDI note 69
    return 440.0f * std::pow(2.0f, (getNoteNumber() - 69) / 12.0f);
}

bool MidiMessage::operator==(const MidiMessage& other) const {
    if (size_ != other.size_) return false;
    for (size_t i = 0; i < size_; ++i) {
        if (data_[i] != other.data_[i]) return false;
    }
    return true;
}

bool MidiMessage::operator!=(const MidiMessage& other) const {
    return !(*this == other);
}

std::string MidiMessage::toString() const {
    std::ostringstream oss;

    if (!isValid()) {
        oss << "Invalid";
        return oss.str();
    }

    if (isNoteOn()) {
        oss << "NoteOn ch=" << (int)getChannel()
            << " note=" << getNoteName()
            << " vel=" << (int)getVelocity();
    } else if (isNoteOff()) {
        oss << "NoteOff ch=" << (int)getChannel()
            << " note=" << getNoteName();
    } else if (isControlChange()) {
        oss << "CC ch=" << (int)getChannel()
            << " ctrl=" << (int)getControllerNumber()
            << " val=" << (int)getControllerValue();
    } else if (isProgramChange()) {
        oss << "PC ch=" << (int)getChannel()
            << " prog=" << (int)getProgramNumber();
    } else if (isPitchBend()) {
        oss << "PitchBend ch=" << (int)getChannel()
            << " val=" << getPitchBendValue();
    } else if (isChannelPressure()) {
        oss << "ChanPressure ch=" << (int)getChannel()
            << " pressure=" << (int)getPressure();
    } else if (isPolyPressure()) {
        oss << "PolyPressure ch=" << (int)getChannel()
            << " note=" << getNoteName()
            << " pressure=" << (int)getPressure();
    } else {
        oss << "0x" << std::hex << std::setfill('0');
        for (size_t i = 0; i < size_; ++i) {
            oss << std::setw(2) << (int)data_[i];
            if (i < size_ - 1) oss << " ";
        }
    }

    return oss.str();
}

size_t MidiMessage::getExpectedLength(uint8_t statusByte) {
    if (statusByte < 0x80) return 0;

    switch (statusByte & 0xF0) {
        case 0x80: return 3; // Note Off
        case 0x90: return 3; // Note On
        case 0xA0: return 3; // Poly Pressure
        case 0xB0: return 3; // Control Change
        case 0xC0: return 2; // Program Change
        case 0xD0: return 2; // Channel Pressure
        case 0xE0: return 3; // Pitch Bend
    }

    switch (statusByte) {
        case 0xF0: return 0; // SysEx (variable)
        case 0xF1: return 2; // Time Code
        case 0xF2: return 3; // Song Position
        case 0xF3: return 2; // Song Select
        case 0xF6: return 1; // Tune Request
        case 0xF7: return 1; // End of SysEx
        case 0xF8: return 1; // Timing Clock
        case 0xFA: return 1; // Start
        case 0xFB: return 1; // Continue
        case 0xFC: return 1; // Stop
        case 0xFE: return 1; // Active Sensing
        case 0xFF: return 1; // System Reset
    }

    return 0;
}

void MidiMessage::setTimestamp(uint64_t timestamp) {
    timestamp_ = timestamp;
}

uint64_t MidiMessage::getTimestamp() const {
    return timestamp_;
}

} // namespace events
} // namespace nap
