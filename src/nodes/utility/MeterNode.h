#ifndef NAP_METERNODE_H
#define NAP_METERNODE_H

#include "../../api/IAudioNode.h"
#include <memory>

namespace nap {

/**
 * @brief Audio utility node for measuring signal levels.
 */
class MeterNode : public IAudioNode {
public:
    MeterNode();
    ~MeterNode() override;

    MeterNode(const MeterNode&) = delete;
    MeterNode& operator=(const MeterNode&) = delete;
    MeterNode(MeterNode&&) noexcept;
    MeterNode& operator=(MeterNode&&) noexcept;

    void process(const float* inputBuffer, float* outputBuffer,
                 std::uint32_t numFrames, std::uint32_t numChannels) override;
    void prepare(double sampleRate, std::uint32_t blockSize) override;
    void reset() override;
    std::string getNodeId() const override;
    std::string getTypeName() const override;
    std::uint32_t getNumInputChannels() const override;
    std::uint32_t getNumOutputChannels() const override;
    bool isBypassed() const override;
    void setBypassed(bool bypassed) override;

    float getPeakLevel(std::uint32_t channel) const;
    float getRmsLevel(std::uint32_t channel) const;
    float getPeakLevelDb(std::uint32_t channel) const;
    float getRmsLevelDb(std::uint32_t channel) const;
    void resetPeaks();
    void setDecayRate(float decayPerSecond);
    float getDecayRate() const;

private:
    class Impl;
    std::unique_ptr<Impl> m_impl;
};

} // namespace nap

#endif // NAP_METERNODE_H
