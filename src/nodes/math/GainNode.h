#ifndef NAP_GAINNODE_H
#define NAP_GAINNODE_H

#include "../../api/IAudioNode.h"
#include <memory>

namespace nap {

/**
 * @brief Audio node that applies gain (volume) adjustment to the signal.
 */
class GainNode : public IAudioNode {
public:
    GainNode();
    ~GainNode() override;

    GainNode(const GainNode&) = delete;
    GainNode& operator=(const GainNode&) = delete;
    GainNode(GainNode&&) noexcept;
    GainNode& operator=(GainNode&&) noexcept;

    // IAudioNode interface
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

    // GainNode specific
    void setGain(float gainLinear);
    float getGain() const;
    void setGainDb(float gainDb);
    float getGainDb() const;

private:
    class Impl;
    std::unique_ptr<Impl> m_impl;
};

} // namespace nap

#endif // NAP_GAINNODE_H
