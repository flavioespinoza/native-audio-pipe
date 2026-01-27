#ifndef NAP_FLANGER_H
#define NAP_FLANGER_H

#include "../../api/IAudioNode.h"
#include <memory>

namespace nap {

/**
 * @brief Audio effect node that applies flanging effect.
 */
class Flanger : public IAudioNode {
public:
    Flanger();
    ~Flanger() override;

    Flanger(const Flanger&) = delete;
    Flanger& operator=(const Flanger&) = delete;
    Flanger(Flanger&&) noexcept;
    Flanger& operator=(Flanger&&) noexcept;

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

    void setRate(float rateHz);
    float getRate() const;
    void setDepth(float depth);
    float getDepth() const;
    void setFeedback(float feedback);
    float getFeedback() const;
    void setDelay(float delayMs);
    float getDelay() const;
    void setMix(float mix);
    float getMix() const;

private:
    class Impl;
    std::unique_ptr<Impl> m_impl;
};

} // namespace nap

#endif // NAP_FLANGER_H
