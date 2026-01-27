#ifndef NAP_BITCRUSHER_H
#define NAP_BITCRUSHER_H

#include "../../api/IAudioNode.h"
#include <memory>

namespace nap {

/**
 * @brief Audio effect node that reduces bit depth and sample rate.
 */
class BitCrusher : public IAudioNode {
public:
    BitCrusher();
    ~BitCrusher() override;

    BitCrusher(const BitCrusher&) = delete;
    BitCrusher& operator=(const BitCrusher&) = delete;
    BitCrusher(BitCrusher&&) noexcept;
    BitCrusher& operator=(BitCrusher&&) noexcept;

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

    void setBitDepth(std::uint32_t bits);
    std::uint32_t getBitDepth() const;
    void setDownsampleFactor(std::uint32_t factor);
    std::uint32_t getDownsampleFactor() const;
    void setMix(float mix);
    float getMix() const;

private:
    class Impl;
    std::unique_ptr<Impl> m_impl;
};

} // namespace nap

#endif // NAP_BITCRUSHER_H
