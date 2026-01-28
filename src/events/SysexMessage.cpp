#include "SysexMessage.h"
#include <sstream>
#include <iomanip>
#include <algorithm>

namespace nap {
namespace events {

SysexMessage::SysexMessage() : timestamp_(0) {
    data_.push_back(0xF0);
}

SysexMessage::SysexMessage(const std::vector<uint8_t>& data)
    : data_(data), timestamp_(0) {
    ensureFraming();
}

SysexMessage::SysexMessage(const uint8_t* data, size_t size)
    : data_(data, data + size), timestamp_(0) {
    ensureFraming();
}

void SysexMessage::ensureFraming() {
    if (data_.empty() || data_.front() != 0xF0) {
        data_.insert(data_.begin(), 0xF0);
    }
}

SysexMessage SysexMessage::identityRequest(uint8_t deviceId) {
    return SysexMessage({
        0xF0, 0x7E, deviceId, 0x06, 0x01, 0xF7
    });
}

SysexMessage SysexMessage::identityReply(uint8_t deviceId,
                                          uint8_t manufacturerId1,
                                          uint8_t manufacturerId2,
                                          uint8_t manufacturerId3,
                                          uint8_t familyMsb, uint8_t familyLsb,
                                          uint8_t modelMsb, uint8_t modelLsb,
                                          uint8_t versionMsb, uint8_t versionLsb,
                                          uint8_t version2, uint8_t version3) {
    if (manufacturerId1 == 0x00) {
        // Extended manufacturer ID
        return SysexMessage({
            0xF0, 0x7E, deviceId, 0x06, 0x02,
            manufacturerId1, manufacturerId2, manufacturerId3,
            familyLsb, familyMsb, modelLsb, modelMsb,
            versionLsb, versionMsb, version2, version3, 0xF7
        });
    } else {
        // Standard manufacturer ID
        return SysexMessage({
            0xF0, 0x7E, deviceId, 0x06, 0x02,
            manufacturerId1,
            familyLsb, familyMsb, modelLsb, modelMsb,
            versionLsb, versionMsb, version2, version3, 0xF7
        });
    }
}

SysexMessage SysexMessage::masterVolume(uint8_t deviceId, uint16_t volume) {
    return SysexMessage({
        0xF0, 0x7F, deviceId, 0x04, 0x01,
        static_cast<uint8_t>(volume & 0x7F),
        static_cast<uint8_t>((volume >> 7) & 0x7F),
        0xF7
    });
}

SysexMessage SysexMessage::masterBalance(uint8_t deviceId, uint16_t balance) {
    return SysexMessage({
        0xF0, 0x7F, deviceId, 0x04, 0x02,
        static_cast<uint8_t>(balance & 0x7F),
        static_cast<uint8_t>((balance >> 7) & 0x7F),
        0xF7
    });
}

SysexMessage SysexMessage::masterFineTuning(uint8_t deviceId, uint16_t tuning) {
    return SysexMessage({
        0xF0, 0x7F, deviceId, 0x04, 0x03,
        static_cast<uint8_t>(tuning & 0x7F),
        static_cast<uint8_t>((tuning >> 7) & 0x7F),
        0xF7
    });
}

SysexMessage SysexMessage::masterCoarseTuning(uint8_t deviceId, uint16_t tuning) {
    return SysexMessage({
        0xF0, 0x7F, deviceId, 0x04, 0x04,
        static_cast<uint8_t>(tuning & 0x7F),
        static_cast<uint8_t>((tuning >> 7) & 0x7F),
        0xF7
    });
}

SysexMessage SysexMessage::globalParameterControl(uint8_t deviceId,
                                                    uint8_t slotPath1, uint8_t slotPath2,
                                                    uint8_t parameterMsb, uint8_t parameterLsb,
                                                    uint8_t valueMsb, uint8_t valueLsb) {
    return SysexMessage({
        0xF0, 0x7F, deviceId, 0x04, 0x05,
        slotPath1, slotPath2,
        parameterMsb, parameterLsb,
        valueMsb, valueLsb,
        0xF7
    });
}

SysexMessage SysexMessage::gmSystemOn(uint8_t deviceId) {
    return SysexMessage({
        0xF0, 0x7E, deviceId, 0x09, 0x01, 0xF7
    });
}

SysexMessage SysexMessage::gmSystemOff(uint8_t deviceId) {
    return SysexMessage({
        0xF0, 0x7E, deviceId, 0x09, 0x02, 0xF7
    });
}

SysexMessage SysexMessage::gm2SystemOn(uint8_t deviceId) {
    return SysexMessage({
        0xF0, 0x7E, deviceId, 0x09, 0x03, 0xF7
    });
}

SysexMessage SysexMessage::createManufacturerMessage(uint32_t manufacturerId,
                                                       const std::vector<uint8_t>& data) {
    std::vector<uint8_t> sysex;
    sysex.push_back(0xF0);

    if (manufacturerId <= 0x7F) {
        // Standard 1-byte manufacturer ID
        sysex.push_back(static_cast<uint8_t>(manufacturerId));
    } else {
        // Extended 3-byte manufacturer ID
        sysex.push_back(0x00);
        sysex.push_back(static_cast<uint8_t>((manufacturerId >> 8) & 0x7F));
        sysex.push_back(static_cast<uint8_t>(manufacturerId & 0x7F));
    }

    sysex.insert(sysex.end(), data.begin(), data.end());
    sysex.push_back(0xF7);

    return SysexMessage(sysex);
}

const std::vector<uint8_t>& SysexMessage::data() const {
    return data_;
}

size_t SysexMessage::size() const {
    return data_.size();
}

uint8_t SysexMessage::operator[](size_t index) const {
    return index < data_.size() ? data_[index] : 0;
}

std::vector<uint8_t> SysexMessage::getPayload() const {
    if (data_.size() < 2) return {};

    size_t start = 1; // Skip F0
    size_t end = data_.size();
    if (!data_.empty() && data_.back() == 0xF7) {
        end--; // Skip F7
    }

    if (start >= end) return {};
    return std::vector<uint8_t>(data_.begin() + start, data_.begin() + end);
}

bool SysexMessage::isValid() const {
    return !data_.empty() && data_.front() == 0xF0;
}

bool SysexMessage::isComplete() const {
    return isValid() && !data_.empty() && data_.back() == 0xF7;
}

bool SysexMessage::isUniversal() const {
    return isValid() && data_.size() > 1 &&
           (data_[1] == 0x7E || data_[1] == 0x7F);
}

bool SysexMessage::isNonRealTime() const {
    return isValid() && data_.size() > 1 && data_[1] == 0x7E;
}

bool SysexMessage::isRealTime() const {
    return isValid() && data_.size() > 1 && data_[1] == 0x7F;
}

uint32_t SysexMessage::getManufacturerId() const {
    if (!isValid() || data_.size() < 2) return 0;

    if (isUniversal()) {
        return data_[1];
    }

    if (data_[1] == 0x00 && data_.size() >= 4) {
        // Extended manufacturer ID
        return (static_cast<uint32_t>(data_[2]) << 8) | data_[3];
    }

    return data_[1];
}

bool SysexMessage::isExtendedManufacturerId() const {
    return isValid() && data_.size() >= 4 && data_[1] == 0x00;
}

std::string SysexMessage::getManufacturerName() const {
    uint32_t id = getManufacturerId();

    switch (id) {
        case 0x7E: return "Universal Non-Real-Time";
        case 0x7F: return "Universal Real-Time";
        case MANUFACTURER_SEQUENTIAL: return "Sequential";
        case MANUFACTURER_MOOG: return "Moog";
        case MANUFACTURER_OBERHEIM: return "Oberheim";
        case MANUFACTURER_KAWAI: return "Kawai";
        case MANUFACTURER_ROLAND: return "Roland";
        case MANUFACTURER_KORG: return "Korg";
        case MANUFACTURER_YAMAHA: return "Yamaha";
        case MANUFACTURER_AKAI: return "Akai";
        default: break;
    }

    // Check extended IDs
    if (isExtendedManufacturerId()) {
        uint32_t extId = (static_cast<uint32_t>(data_[2]) << 8) | data_[3];
        if (extId == 0x2109) return "Native Instruments";
    }

    return "Unknown";
}

std::optional<uint8_t> SysexMessage::getDeviceId() const {
    if (!isUniversal() || data_.size() < 3) return std::nullopt;
    return data_[2];
}

std::optional<uint8_t> SysexMessage::getSubId1() const {
    if (!isUniversal() || data_.size() < 4) return std::nullopt;
    return data_[3];
}

std::optional<uint8_t> SysexMessage::getSubId2() const {
    if (!isUniversal() || data_.size() < 5) return std::nullopt;
    return data_[4];
}

std::optional<SysexMessage::IdentityReply> SysexMessage::parseIdentityReply() const {
    if (!isNonRealTime() || data_.size() < 15) return std::nullopt;

    auto subId1 = getSubId1();
    auto subId2 = getSubId2();

    if (!subId1 || !subId2 || *subId1 != 0x06 || *subId2 != 0x02) {
        return std::nullopt;
    }

    IdentityReply reply;
    reply.deviceId = data_[2];

    size_t offset = 5;
    if (data_[offset] == 0x00) {
        // Extended manufacturer ID
        reply.manufacturerId = (static_cast<uint32_t>(data_[offset + 1]) << 8) |
                                data_[offset + 2];
        offset += 3;
    } else {
        reply.manufacturerId = data_[offset];
        offset += 1;
    }

    if (offset + 8 > data_.size()) return std::nullopt;

    reply.familyCode = (static_cast<uint16_t>(data_[offset + 1]) << 8) |
                        data_[offset];
    reply.modelNumber = (static_cast<uint16_t>(data_[offset + 3]) << 8) |
                         data_[offset + 2];
    reply.softwareRevision = (static_cast<uint32_t>(data_[offset + 7]) << 24) |
                              (static_cast<uint32_t>(data_[offset + 6]) << 16) |
                              (static_cast<uint32_t>(data_[offset + 5]) << 8) |
                              data_[offset + 4];

    return reply;
}

void SysexMessage::append(uint8_t byte) {
    // Remove trailing F7 if present
    if (!data_.empty() && data_.back() == 0xF7) {
        data_.pop_back();
    }
    data_.push_back(byte);
}

void SysexMessage::append(const uint8_t* data, size_t size) {
    // Remove trailing F7 if present
    if (!data_.empty() && data_.back() == 0xF7) {
        data_.pop_back();
    }
    data_.insert(data_.end(), data, data + size);
}

void SysexMessage::clear() {
    data_.clear();
    data_.push_back(0xF0);
}

uint8_t SysexMessage::calculateRolandChecksum() const {
    // Roland checksum is calculated on address and data bytes
    // Checksum = 128 - (sum of bytes mod 128)
    if (data_.size() < 8) return 0;

    uint32_t sum = 0;
    // Skip F0, manufacturer ID (0x41), device ID, model ID
    // Checksum is typically over address (3-4 bytes) and data
    for (size_t i = 5; i < data_.size() - 2; ++i) {
        sum += data_[i];
    }

    return static_cast<uint8_t>((128 - (sum % 128)) % 128);
}

bool SysexMessage::verifyRolandChecksum() const {
    if (data_.size() < 8) return false;

    uint8_t expected = calculateRolandChecksum();
    uint8_t actual = data_[data_.size() - 2]; // Checksum is second to last byte

    return expected == actual;
}

bool SysexMessage::operator==(const SysexMessage& other) const {
    return data_ == other.data_;
}

bool SysexMessage::operator!=(const SysexMessage& other) const {
    return !(*this == other);
}

std::string SysexMessage::toString() const {
    std::ostringstream oss;

    oss << "SysEx [" << data_.size() << " bytes]";

    if (isUniversal()) {
        if (isNonRealTime()) {
            oss << " Non-RT";
        } else {
            oss << " RT";
        }

        auto devId = getDeviceId();
        if (devId) {
            oss << " dev=" << (int)*devId;
        }

        auto sub1 = getSubId1();
        auto sub2 = getSubId2();
        if (sub1 && sub2) {
            oss << " sub=" << std::hex << (int)*sub1 << "/" << (int)*sub2;
        }
    } else {
        oss << " Manufacturer: " << getManufacturerName();
    }

    return oss.str();
}

std::string SysexMessage::toHexString() const {
    std::ostringstream oss;
    oss << std::hex << std::setfill('0');

    for (size_t i = 0; i < data_.size(); ++i) {
        oss << std::setw(2) << (int)data_[i];
        if (i < data_.size() - 1) oss << " ";
    }

    return oss.str();
}

void SysexMessage::setTimestamp(uint64_t timestamp) {
    timestamp_ = timestamp;
}

uint64_t SysexMessage::getTimestamp() const {
    return timestamp_;
}

} // namespace events
} // namespace nap
