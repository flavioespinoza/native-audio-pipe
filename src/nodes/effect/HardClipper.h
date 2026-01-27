#ifndef NAP_HARDCLIPPER_H
#define NAP_HARDCLIPPER_H

#include "../../api/IAudioNode.h"
#include <memory>

namespace nap {

/**
 * @brief Audio effect node that applies hard clipping distortion.
 */
class HardClipper : public IAudioNode {
public:
    HardClipper();
    ~HardClipper() override;

    HardClipper(const HardClipper&) = delete;
    HardClipper& operator=(const HardClipper&) = delete;
    HardClipper(HardClipper&&) noexcept;
    HardClipper& operator=(HardClipper&&) noexcept;

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

    // HardClipper specific
    void setThreshold(float threshold);
    float getThreshold() const;
    void setInputGain(float gainDb);
    float getInputGain() const;
    void setOutputGain(float gainDb);
    float getOutputGain() const;

private:
    class Impl;
    std::unique_ptr<Impl> m_impl;
};

} // namespace nap

#endif // NAP_HARDCLIPPER_H
